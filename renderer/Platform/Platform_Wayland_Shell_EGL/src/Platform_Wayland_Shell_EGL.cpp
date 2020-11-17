//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Wayland_Shell_EGL/Platform_Wayland_Shell_EGL.h"
#include "Window_Wayland_Shell/Window_Wayland_Shell.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    Platform_Wayland_Shell_EGL::Platform_Wayland_Shell_EGL(const RendererConfig& rendererConfig)
        : Platform_Wayland_EGL(rendererConfig)
    {
    }

    ISystemCompositorController* Platform_Wayland_Shell_EGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* Platform_Wayland_Shell_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Wayland_Shell* platformWindow = new Window_Wayland_Shell(displayConfig, windowEventHandler, m_windows.size());
        if(nullptr == addPlatformWindow(platformWindow))
            return nullptr;

        m_windowEventsPollingManager.addWindow(platformWindow);
        return platformWindow;
    }

    Bool Platform_Wayland_Shell_EGL::destroyWindow(IWindow &window)
    {
        m_windowEventsPollingManager.removeWindow(&static_cast<Window_Wayland&>(window));

        return Platform_Wayland_EGL::destroyWindow(window);
    }
}
