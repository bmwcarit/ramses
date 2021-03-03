//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_X11/Platform_X11_EGL.h"
#include "RendererLib/RendererConfig.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{
    Platform_X11_EGL::Platform_X11_EGL(const RendererConfig& rendererConfig)
        : Platform_EGL<Window_X11>(rendererConfig)
    {
    }

    ISystemCompositorController* Platform_X11_EGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* Platform_X11_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_X11* platformWindow = new Window_X11(displayConfig, windowEventHandler, m_windows.size());
        return addPlatformWindow(platformWindow);
    }

    IEmbeddedCompositor* Platform_X11_EGL::createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context)
    {
        UNUSED(displayConfig);
        UNUSED(context);
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }
}
