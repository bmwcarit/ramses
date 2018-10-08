//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformFactory_Wayland_Shell_EGL/PlatformFactory_Wayland_Shell_EGL.h"
#include "Platform_Base/TextureUploadingAdapter_Base.h"
#include "Window_Wayland_Shell/Window_Wayland_Shell.h"
#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    PlatformFactory_Wayland_Shell_EGL::PlatformFactory_Wayland_Shell_EGL(const RendererConfig& rendererConfig)
        : PlatformFactory_Wayland_EGL(rendererConfig)
    {
    }

    ISystemCompositorController* PlatformFactory_Wayland_Shell_EGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* PlatformFactory_Wayland_Shell_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Wayland_Shell* platformWindow = new Window_Wayland_Shell(displayConfig, windowEventHandler, m_windows.size());
        if(nullptr != addPlatformWindow(platformWindow))
            m_windowEventsPollingManager.addWindow(platformWindow);

        return platformWindow;
    }

    Bool PlatformFactory_Wayland_Shell_EGL::destroyWindow(IWindow &window)
    {
        m_windowEventsPollingManager.removeWindow(&static_cast<Window_Wayland&>(window));

        return PlatformFactory_Wayland_EGL::destroyWindow(window);
    }

    IEmbeddedCompositor* PlatformFactory_Wayland_Shell_EGL::createEmbeddedCompositor()
    {
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }

    ITextureUploadingAdapter* PlatformFactory_Wayland_Shell_EGL::createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window)
    {
        UNUSED(device)
        UNUSED(window)
        UNUSED(embeddedCompositor)
        ITextureUploadingAdapter* textureUploadingAdapter = new TextureUploadingAdapter_Base(device);
        return addTextureUploadingAdapter(textureUploadingAdapter);
    }
}
