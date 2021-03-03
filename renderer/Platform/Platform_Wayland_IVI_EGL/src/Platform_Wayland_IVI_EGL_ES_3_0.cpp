//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Wayland_IVI_EGL/Platform_Wayland_IVI_EGL_ES_3_0.h"

#include "Window_Wayland_IVI/Window_Wayland_IVI.h"
#include "SystemCompositorController_Wayland_IVI/SystemCompositorController_Wayland_IVI.h"
#include "Window_Wayland/WindowEventsPollingManager_Wayland.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{

    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Wayland_IVI_EGL_ES_3_0(rendererConfig);
    }

    Platform_Wayland_IVI_EGL_ES_3_0::Platform_Wayland_IVI_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_Wayland_EGL(rendererConfig)
    {
    }

    ISystemCompositorController* Platform_Wayland_IVI_EGL_ES_3_0::createSystemCompositorController()
    {
        SystemCompositorController_Wayland_IVI* waylandSystemCompositorController = new SystemCompositorController_Wayland_IVI(m_rendererConfig.getWaylandDisplayForSystemCompositorController());
        return setPlatformSystemCompositorController(waylandSystemCompositorController);
    }


    IWindow* Platform_Wayland_IVI_EGL_ES_3_0::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        if(!displayConfig.getWaylandIviLayerID().isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Can not create Wayland IVI window because IVI layer ID was not set in display config!");
            return nullptr;
        }

        if(!displayConfig.getWaylandIviSurfaceID().isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Can not create Wayland IVI window because IVI surface ID was not set in display config!");
            return nullptr;
        }

        Window_Wayland_IVI* platformWindow = new Window_Wayland_IVI(displayConfig, windowEventHandler, m_windows.size());
        if(nullptr != addPlatformWindow(platformWindow))
        {
            m_windowEventsPollingManager.addWindow(platformWindow);
            return platformWindow;
        }
        return nullptr;
    }

    Bool Platform_Wayland_IVI_EGL_ES_3_0::destroyWindow(IWindow &window)
    {
        m_windowEventsPollingManager.removeWindow(&static_cast<Window_Wayland&>(window));

        return Platform_Wayland_EGL::destroyWindow(window);
    }
}
