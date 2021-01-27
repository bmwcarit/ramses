//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayDispatcher.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/RendererCommandUtils.h"
#include "Ramsh/Ramsh.h"
#include "Platform_Base/Platform_Base.h"

namespace ramses_internal
{
    DisplayDispatcher::DisplayDispatcher(
        const RendererConfig& config,
        RendererCommandBuffer& commandBuffer,
        IRendererSceneEventSender& rendererSceneSender)
        : m_rendererConfig{ config }
        , m_pendingCommandsToDispatch{ commandBuffer }
        , m_rendererSceneSender{ rendererSceneSender }
    {
    }

    void DisplayDispatcher::doOneLoop(ELoopMode loopMode, std::chrono::microseconds sleepTime)
    {
        m_tmpCommands.clear();
        m_pendingCommandsToDispatch.swapCommands(m_tmpCommands);
        // log only if there are commands other than scene update
        const bool logCommands = std::any_of(m_tmpCommands.cbegin(), m_tmpCommands.cend(), [&](const auto& c) {
            return !absl::holds_alternative<RendererCommand::UpdateScene>(c);
        });
        if (logCommands)
            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: dispatching {} commands (only other than scene update commands will be logged)", m_tmpCommands.size());

        for (auto&& cmd : m_tmpCommands)
        {
            if (logCommands && !absl::holds_alternative<RendererCommand::UpdateScene>(cmd))
                LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: dispatching command [{}]", RendererCommandUtils::ToString(cmd));

            preprocessCommand(cmd);
            dispatchCommand(std::move(cmd));
        }

        for (auto& display : m_displays)
        {
            auto& displayBundle = *display.second.m_displayBundle;
            auto& pendingCmds = display.second.m_pendingCommands;
            if (!pendingCmds.empty())
                displayBundle.pushAndConsumeCommands(pendingCmds);
            displayBundle.doOneLoop(loopMode, sleepTime);
        }
    }

    void DisplayDispatcher::preprocessCommand(const RendererCommand::Variant& cmd)
    {
        if (absl::holds_alternative<RendererCommand::CreateDisplay>(cmd))
        {
            // create display thread
            const auto& cmdData = absl::get<RendererCommand::CreateDisplay>(cmd);
            const auto displayHandle = cmdData.display;
            assert(m_displays.count(displayHandle) == 0u);
            auto bundle = createDisplayBundle();
            {
                std::lock_guard<std::mutex> l{ m_displayCreationLock };
                m_displays.insert(std::make_pair(displayHandle, std::move(bundle)));
            }

            // copy and push stashed broadcast commands to new display so that display receives all relevant commands
            // received until now (e.g. un/publish, limits, SC, etc.)
            RendererCommands stashedCommands{ m_stashedBroadcastCommandsForNewDisplays.size() };
            std::transform(m_stashedBroadcastCommandsForNewDisplays.cbegin(), m_stashedBroadcastCommandsForNewDisplays.cend(), stashedCommands.begin(), [](const auto& c)
            {
                return RendererCommandUtils::Copy(c);
            });
            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: pushing {} stashed broadcast commands to newly created display", stashedCommands.size());
            m_displays[displayHandle].m_displayBundle->pushAndConsumeCommands(stashedCommands);

            // push commands stashed for this specific display (e.g. set scene state)
            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: pushing {} stashed commands to newly created display", m_stashedCommandsForNewDisplays[displayHandle].size());
            m_displays[displayHandle].m_displayBundle->pushAndConsumeCommands(m_stashedCommandsForNewDisplays[displayHandle]);
            m_stashedCommandsForNewDisplays.erase(displayHandle);
        }
        else if (absl::holds_alternative<RendererCommand::SetSceneMapping>(cmd))
        {
            // set scene ownership so that future commands are dispatched to its display
            const auto& cmdData = absl::get<RendererCommand::SetSceneMapping>(cmd);
            m_sceneDisplayTracker.setSceneOwnership(cmdData.scene, cmdData.display);
        }
        else if (absl::holds_alternative<RendererCommand::ReceiveScene>(cmd))
        {
            // Special handling of referenced scenes, refScenes are fully handled by internal logic within DisplayBundle/SceneRefLogic,
            // therefore their ownership is not known at dispatcher level. When a subscription of referenced scene arrives its master
            // is queried from a thread-safe shared ownership registry.
            if (!m_sceneDisplayTracker.determineDisplayFromRendererCommand(cmd)->isValid())
            {
                const auto refScene = absl::get<RendererCommand::ReceiveScene>(cmd).info.sceneID;
                LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: missing scene {} display ownership when processing {}, assuming a referenced scene.", refScene, RendererCommandUtils::ToString(cmd));
                for (const auto& display : m_displays)
                {
                    const auto& displayBundle = *display.second.m_displayBundle;
                    const auto masterScene = displayBundle.findMasterSceneForReferencedScene(refScene);
                    if (masterScene.isValid())
                    {
                        const auto masterDisplay = m_sceneDisplayTracker.getSceneOwnership(masterScene);
                        LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: found master scene {} for referenced scene {} when processing {}, setting display ownership to display {}",
                            masterScene, refScene, RendererCommandUtils::ToString(cmd), masterDisplay);
                        m_sceneDisplayTracker.setSceneOwnership(refScene, masterDisplay);
                    }
                }
                if (!m_sceneDisplayTracker.getSceneOwnership(refScene).isValid())
                {
                    LOG_ERROR_P(CONTEXT_RENDERER, "DisplayDispatcher: could not find master scene for referenced scene {} when processing {}", refScene, RendererCommandUtils::ToString(cmd));
                }
            }
        }
    }

    DisplayDispatcher::Display DisplayDispatcher::createDisplayBundle()
    {
        std::unique_ptr<IPlatform> platform{ Platform_Base::CreatePlatform(m_rendererConfig) };
        auto displayBundle = std::make_unique<DisplayBundle>(m_rendererSceneSender, *platform, m_rendererConfig.getKPIFileName());
        return { std::move(platform), std::move(displayBundle), {} };
    }

    void DisplayDispatcher::dispatchCommand(RendererCommand::Variant&& cmd)
    {
        const auto cmdDisplay = m_sceneDisplayTracker.determineDisplayFromRendererCommand(cmd);
        if (cmdDisplay)
        {
            if (m_displays.count(*cmdDisplay) == 0)
            {
                if (absl::holds_alternative<RendererCommand::SetSceneMapping>(cmd) ||
                    absl::holds_alternative<RendererCommand::SetSceneState>(cmd))
                {
                    // Special case for commands that are to be dispatched only after their corresponding display is created, therefore cannot fail.
                    // This makes it possible that scene mapping/state can be set before display is even created.
                    LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher cannot dispatch command yet, display does not exist, will dispatch when display created. Command=[{}]", RendererCommandUtils::ToString(cmd));
                    m_stashedCommandsForNewDisplays[*cmdDisplay].push_back(std::move(cmd));
                }
                else
                {
                    // cannot dispatch, generate fail event
                    LOG_ERROR_P(CONTEXT_RENDERER, "DisplayDispatcher cannot dispatch command, display unknown. Command=[{}]", RendererCommandUtils::ToString(cmd));
                    auto failEvent = RendererCommandUtils::GenerateFailEventForCommand(cmd);
                    if (failEvent.eventType != ERendererEventType::Invalid)
                    {
                        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
                        m_injectedRendererEvents.push_back(std::move(failEvent));
                    }
                }
            }
            else
            {
                // dispatch command to display
                assert(m_displays.count(*cmdDisplay));
                m_displays[*cmdDisplay].m_pendingCommands.push_back(std::move(cmd));
            }
        }
        else
        {
            // command is to be broadcast, dispatch a copy to each display
            for (auto& display : m_displays)
                display.second.m_pendingCommands.push_back(RendererCommandUtils::Copy(cmd));

            m_stashedBroadcastCommandsForNewDisplays.push_back(std::move(cmd));
        }
    }

    void DisplayDispatcher::dispatchRendererEvents(RendererEventVector& events)
    {
        std::vector<DisplayHandle> destroyedDisplays;
        for (auto& display : m_displays)
        {
            m_tmpEvents.clear();
            {
                std::lock_guard<std::mutex> l{ m_displayCreationLock };
                display.second.m_displayBundle->dispatchRendererEvents(m_tmpEvents);
            }

            const bool isFirstDisplay = (display.first == m_displays.cbegin()->first);
            for (auto&& evt : m_tmpEvents)
            {
                if (evt.eventType == ERendererEventType::DisplayDestroyed)
                {
                    assert(evt.displayHandle == display.first);
                    destroyedDisplays.push_back(evt.displayHandle);
                }
                if (isFirstDisplay || !SceneDisplayTracker::IsEventResultOfBroadcastCommand(evt.eventType))
                    events.push_back(std::move(evt));
            }
        }

        for (const auto& display : destroyedDisplays)
        {
            std::lock_guard<std::mutex> l{ m_displayCreationLock };
            m_displays.erase(display);
        }

        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        events.insert(events.end(), std::make_move_iterator(m_injectedRendererEvents.begin()), std::make_move_iterator(m_injectedRendererEvents.end()));
        m_injectedRendererEvents.clear();
    }

    void DisplayDispatcher::dispatchSceneControlEvents(RendererEventVector& events)
    {
        for (auto& display : m_displays)
        {
            m_tmpEvents.clear();
            display.second.m_displayBundle->dispatchSceneControlEvents(m_tmpEvents);

            const bool isFirstDisplay = (display.first == m_displays.cbegin()->first);
            for (auto&& evt : m_tmpEvents)
            {
                if (isFirstDisplay || !SceneDisplayTracker::IsEventResultOfBroadcastCommand(evt.eventType))
                    events.push_back(std::move(evt));
            }
        }

        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        events.insert(events.end(), std::make_move_iterator(m_injectedSceneControlEvents.begin()), std::make_move_iterator(m_injectedSceneControlEvents.end()));
        m_injectedSceneControlEvents.clear();
    }

    void DisplayDispatcher::injectRendererEvent(RendererEvent&& event)
    {
        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        m_injectedRendererEvents.push_back(std::move(event));
    }

    void DisplayDispatcher::injectSceneControlEvent(RendererEvent&& event)
    {
        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        m_injectedSceneControlEvents.push_back(std::move(event));
    }

    IEmbeddedCompositingManager& DisplayDispatcher::getECManager(DisplayHandle display)
    {
        assert(m_displays.count(display) != 0);
        return m_displays[display].m_displayBundle->getECManager(display);
    }

    IEmbeddedCompositor& DisplayDispatcher::getEC(DisplayHandle display)
    {
        assert(m_displays.count(display) != 0);
        return m_displays[display].m_displayBundle->getEC(display);
    }

    void DisplayDispatcher::registerRamshCommands(Ramsh& ramsh)
    {
        ramsh.add(m_cmdPrintStatistics);
        ramsh.add(m_cmdTriggerPickEvent);
        ramsh.add(m_cmdSetClearColor);
        ramsh.add(m_cmdSkippingOfUnmodifiedBuffers);
        ramsh.add(m_cmdScreenshot);
        ramsh.add(m_cmdLogRendererInfo);
        ramsh.add(m_cmdShowFrameProfiler);
        ramsh.add(m_cmdLinkSceneData);
        ramsh.add(m_cmdUnlinkSceneData);
        ramsh.add(m_cmdSystemCompositorControllerListIviSurfaces);
        ramsh.add(m_cmdLinkSceneData);
        ramsh.add(m_cmdUnlinkSceneData);
        ramsh.add(m_cmdSystemCompositorControllerSetLayerVisibility);
        ramsh.add(m_cmdSystemCompositorControllerSetSurfaceVisibility);
        ramsh.add(m_cmdSystemCompositorControllerSetSurfaceOpacity);
        ramsh.add(m_cmdSystemCompositorControllerSetSurfaceDestRectangle);
        ramsh.add(m_cmdSystemCompositorControllerScreenshot);
        ramsh.add(m_cmdSystemCompositorControllerAddSurfaceToLayer);
        ramsh.add(m_cmdSystemCompositorControllerRemoveSurfaceFromLayer);
        ramsh.add(m_cmdSystemCompositorControllerDestroySurface);
        ramsh.add(m_cmdSetFrametimerValues);
    }
}
