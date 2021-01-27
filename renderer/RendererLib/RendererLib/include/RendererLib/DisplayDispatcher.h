//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
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
#include "RendererAPI/ELoopMode.h"
#include "RendererAPI/IPlatform.h"
#include "RendererCommands/Screenshot.h"
#include "RendererCommands/LogRendererInfo.h"
#include "RendererCommands/PrintStatistics.h"
#include "RendererCommands/TriggerPickEvent.h"
#include "RendererCommands/SetClearColor.h"
#include "RendererCommands/SetSkippingOfUnmodifiedBuffers.h"
#include "RendererCommands/ShowFrameProfiler.h"
#include "RendererCommands/LinkSceneData.h"
#include "RendererCommands/UnlinkSceneData.h"
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
            IRendererSceneEventSender& rendererSceneSender);
        virtual ~DisplayDispatcher() = default;

        void doOneLoop(ELoopMode loopMode, std::chrono::microseconds sleepTime = std::chrono::microseconds{0});

        void dispatchRendererEvents(RendererEventVector& events);
        void dispatchSceneControlEvents(RendererEventVector& events);
        void injectRendererEvent(RendererEvent&& event);
        void injectSceneControlEvent(RendererEvent&& event);

        void registerRamshCommands(Ramsh& ramsh);

        // needed for EC tests...
        IEmbeddedCompositingManager& getECManager(DisplayHandle display);
        IEmbeddedCompositor& getEC(DisplayHandle display);

    protected:
        void preprocessCommand(const RendererCommand::Variant& cmd);
        void dispatchCommand(RendererCommand::Variant&& cmd);

        struct Display
        {
            std::unique_ptr<IPlatform> m_platform;
            std::unique_ptr<IDisplayBundle> m_displayBundle;
            RendererCommands m_pendingCommands;
        };
        // virtual to allow mock of display thread
        virtual Display createDisplayBundle();

        const RendererConfig m_rendererConfig;
        RendererCommandBuffer& m_pendingCommandsToDispatch;
        IRendererSceneEventSender& m_rendererSceneSender;

        SceneDisplayTracker m_sceneDisplayTracker;
        // use map to keep displays ordered
        std::map<DisplayHandle, Display> m_displays;

        std::unordered_map<DisplayHandle, RendererCommands> m_stashedCommandsForNewDisplays;
        RendererCommands m_stashedBroadcastCommandsForNewDisplays;

        std::mutex m_displayCreationLock; // TODO vaclav not needed when global rnd thread removed
        std::mutex m_injectedEventsLock; // TODO vaclav not needed when global rnd thread removed
        RendererEventVector m_injectedRendererEvents;
        RendererEventVector m_injectedSceneControlEvents;

        Screenshot                                        m_cmdScreenshot{ m_pendingCommandsToDispatch };
        LogRendererInfo                                   m_cmdLogRendererInfo{ m_pendingCommandsToDispatch };
        ShowFrameProfiler                                 m_cmdShowFrameProfiler{ m_pendingCommandsToDispatch };
        PrintStatistics                                   m_cmdPrintStatistics{ m_pendingCommandsToDispatch };
        TriggerPickEvent                                  m_cmdTriggerPickEvent{ m_pendingCommandsToDispatch };
        SetClearColor                                     m_cmdSetClearColor{ m_pendingCommandsToDispatch };
        SetSkippingOfUnmodifiedBuffers                    m_cmdSkippingOfUnmodifiedBuffers{ m_pendingCommandsToDispatch };
        LinkSceneData                                     m_cmdLinkSceneData{ m_pendingCommandsToDispatch };
        UnlinkSceneData                                   m_cmdUnlinkSceneData{ m_pendingCommandsToDispatch };
        SystemCompositorControllerListIviSurfaces         m_cmdSystemCompositorControllerListIviSurfaces{ m_pendingCommandsToDispatch };
        SystemCompositorControllerSetLayerVisibility      m_cmdSystemCompositorControllerSetLayerVisibility{ m_pendingCommandsToDispatch };
        SystemCompositorControllerSetSurfaceVisibility    m_cmdSystemCompositorControllerSetSurfaceVisibility{ m_pendingCommandsToDispatch };
        SystemCompositorControllerSetSurfaceOpacity       m_cmdSystemCompositorControllerSetSurfaceOpacity{ m_pendingCommandsToDispatch };
        SystemCompositorControllerSetSurfaceDestRectangle m_cmdSystemCompositorControllerSetSurfaceDestRectangle{ m_pendingCommandsToDispatch };
        SystemCompositorControllerScreenshot              m_cmdSystemCompositorControllerScreenshot{ m_pendingCommandsToDispatch };
        SystemCompositorControllerAddSurfaceToLayer       m_cmdSystemCompositorControllerAddSurfaceToLayer{ m_pendingCommandsToDispatch };
        SystemCompositorControllerRemoveSurfaceFromLayer  m_cmdSystemCompositorControllerRemoveSurfaceFromLayer{ m_pendingCommandsToDispatch };
        SystemCompositorControllerDestroySurface          m_cmdSystemCompositorControllerDestroySurface{ m_pendingCommandsToDispatch };
        SetFrameTimeLimits                                m_cmdSetFrametimerValues{ m_pendingCommandsToDispatch };

        // to avoid re-allocs
        RendererCommands m_tmpCommands;
        RendererEventVector m_tmpEvents;
    };
}

#endif
