//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYDISPATCHER_H
#define RAMSES_DISPLAYDISPATCHER_H

#include "RendererLib/DisplayBundle.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/SceneDisplayTracker.h"
#include "RendererLib/DisplayThread.h"
#include "RendererAPI/ELoopMode.h"
#include "RendererAPI/IPlatform.h"
#include "RendererCommands/Screenshot.h"
#include "RendererCommands/LogRendererInfo.h"
#include "RendererCommands/PrintStatistics.h"
#include "RendererCommands/TriggerPickEvent.h"
#include "RendererCommands/SetClearColor.h"
#include "RendererCommands/SetSkippingOfUnmodifiedBuffers.h"
#include "RendererCommands/ShowFrameProfiler.h"
#include "RendererCommands/SystemCompositorControllerListIviSurfaces.h"
#include "RendererCommands/SystemCompositorControllerSetLayerVisibility.h"
#include "RendererCommands/SystemCompositorControllerSetSurfaceVisibility.h"
#include "RendererCommands/SystemCompositorControllerSetSurfaceOpacity.h"
#include "RendererCommands/SystemCompositorControllerSetSurfaceDestRectangle.h"
#include "RendererCommands/SystemCompositorControllerScreenshot.h"
#include "RendererCommands/SystemCompositorControllerAddSurfaceToLayer.h"
#include "RendererCommands/SystemCompositorControllerRemoveSurfaceFromLayer.h"
#include "RendererCommands/SystemCompositorControllerDestroySurface.h"
#include "RendererCommands/SetFrameTimeLimits.h"

#include "Watchdog/IThreadAliveNotifier.h"
#include <memory>

namespace ramses_internal
{
    class RendererCommandBuffer;
    class IRendererSceneEventSender;
    class Ramsh;
    class IEmbeddedCompositingManager;
    class IEmbeddedCompositor;

    class DisplayDispatcher
    {
    public:
        DisplayDispatcher(
            const RendererConfig& config,
            RendererCommandBuffer& commandBuffer,
            IRendererSceneEventSender& rendererSceneSender,
            IThreadAliveNotifier& notifier);
        virtual ~DisplayDispatcher() = default;

        void dispatchCommands();
        void doOneLoop(std::chrono::microseconds sleepTime = std::chrono::microseconds{0});

        void dispatchRendererEvents(RendererEventVector& events);
        void dispatchSceneControlEvents(RendererEventVector& events);
        void injectRendererEvent(RendererEvent&& event);
        void injectSceneControlEvent(RendererEvent&& event);

        void startDisplayThreadsUpdating();
        void stopDisplayThreadsUpdating();
        void setLoopMode(ELoopMode loopMode);
        void setMinFrameDuration(std::chrono::microseconds minFrameDuration);
        void setMinFrameDuration(std::chrono::microseconds minFrameDuration, DisplayHandle display);

        void registerRamshCommands(Ramsh& ramsh);

        // needed for EC tests...
        IEmbeddedCompositingManager& getECManager(DisplayHandle display);
        IEmbeddedCompositor& getEC(DisplayHandle display);

        // needed for Renderer lifecycle tests...
        bool hasSystemCompositorController() const;

    protected:
        void preprocessCommand(const RendererCommand::Variant& cmd);
        void dispatchCommand(RendererCommand::Variant&& cmd);
        bool isSceneStateChangeEmittedFromOwningDisplay(SceneId sceneId, DisplayHandle emittingDisplay) const;

        struct Display
        {
            std::unique_ptr<IPlatform> platform;
            DisplayBundleShared displayBundle;
            std::unique_ptr<IDisplayThread> displayThread;
            RendererCommands pendingCommands;
        };
        // virtual to allow mock of display thread
        virtual Display createDisplayBundle(DisplayHandle displayHandle);

        const RendererConfig m_rendererConfig;
        RendererCommandBuffer& m_pendingCommandsToDispatch;
        IRendererSceneEventSender& m_rendererSceneSender;

        SceneDisplayTracker m_sceneDisplayTrackerForCommands;
        SceneDisplayTracker m_sceneDisplayTrackerForEvents;
        // use map to keep displays ordered
        std::map<DisplayHandle, Display> m_displays;

        std::unordered_map<DisplayHandle, RendererCommands> m_stashedCommandsForNewDisplays;
        RendererCommands m_stashedBroadcastCommandsForNewDisplays;

        // TODO vaclav collect events periodically (trigger by global thread) to avoid need of full lock in so many places (RAMSES-10227)
        std::mutex m_displaysAccessLock;

        std::mutex m_injectedEventsLock;
        RendererEventVector m_injectedRendererEvents;
        RendererEventVector m_injectedSceneControlEvents;

        bool m_threadedDisplays = false;
        bool m_displayThreadsUpdating = true;
        ELoopMode m_loopMode = ELoopMode::UpdateAndRender;
        std::chrono::microseconds m_generalMinFrameDuration {DefaultMinFrameDuration};
        std::unordered_map<DisplayHandle, std::chrono::microseconds> m_minFrameDurationsPerDisplay;

        IThreadAliveNotifier& m_notifier;

        std::shared_ptr<Screenshot>                                        m_cmdScreenshot{ std::make_shared<Screenshot>(m_pendingCommandsToDispatch) };
        std::shared_ptr<LogRendererInfo>                                   m_cmdLogRendererInfo{ std::make_shared<LogRendererInfo>(m_pendingCommandsToDispatch) };
        std::shared_ptr<ShowFrameProfiler>                                 m_cmdShowFrameProfiler{ std::make_shared<ShowFrameProfiler>(m_pendingCommandsToDispatch) };
        std::shared_ptr<PrintStatistics>                                   m_cmdPrintStatistics{ std::make_shared<PrintStatistics>(m_pendingCommandsToDispatch) };
        std::shared_ptr<TriggerPickEvent>                                  m_cmdTriggerPickEvent{ std::make_shared<TriggerPickEvent>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SetClearColor>                                     m_cmdSetClearColor{ std::make_shared<SetClearColor>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SetSkippingOfUnmodifiedBuffers>                    m_cmdSkippingOfUnmodifiedBuffers{ std::make_shared<SetSkippingOfUnmodifiedBuffers>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerListIviSurfaces>         m_cmdSystemCompositorControllerListIviSurfaces{ std::make_shared<SystemCompositorControllerListIviSurfaces>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerSetLayerVisibility>      m_cmdSystemCompositorControllerSetLayerVisibility{ std::make_shared<SystemCompositorControllerSetLayerVisibility>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerSetSurfaceVisibility>    m_cmdSystemCompositorControllerSetSurfaceVisibility{ std::make_shared<SystemCompositorControllerSetSurfaceVisibility>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerSetSurfaceOpacity>       m_cmdSystemCompositorControllerSetSurfaceOpacity{ std::make_shared<SystemCompositorControllerSetSurfaceOpacity>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerSetSurfaceDestRectangle> m_cmdSystemCompositorControllerSetSurfaceDestRectangle{ std::make_shared<SystemCompositorControllerSetSurfaceDestRectangle>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerScreenshot>              m_cmdSystemCompositorControllerScreenshot{ std::make_shared<SystemCompositorControllerScreenshot>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerAddSurfaceToLayer>       m_cmdSystemCompositorControllerAddSurfaceToLayer{ std::make_shared<SystemCompositorControllerAddSurfaceToLayer>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerRemoveSurfaceFromLayer>  m_cmdSystemCompositorControllerRemoveSurfaceFromLayer{ std::make_shared<SystemCompositorControllerRemoveSurfaceFromLayer>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SystemCompositorControllerDestroySurface>          m_cmdSystemCompositorControllerDestroySurface{ std::make_shared<SystemCompositorControllerDestroySurface>(m_pendingCommandsToDispatch) };
        std::shared_ptr<SetFrameTimeLimits>                                m_cmdSetFrametimerValues{ std::make_shared<SetFrameTimeLimits>(m_pendingCommandsToDispatch) };

        // to avoid re-allocs
        RendererCommands m_tmpCommands;
        RendererEventVector m_tmpEvents;
    };
}

#endif
