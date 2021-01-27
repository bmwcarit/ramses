//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Android/Platform_Android_EGL.h"
#include "RendererLib/RendererConfig.h"
#include "Context_EGL/Context_EGL.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{
    Platform_Android_EGL::Platform_Android_EGL(const RendererConfig& rendererConfig)
        : Platform_EGL<Window_Android>(rendererConfig)
    {
    }

    ISystemCompositorController* Platform_Android_EGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* Platform_Android_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Android* platformWindow = new Window_Android(displayConfig, windowEventHandler, m_windows.size());
        return addPlatformWindow(platformWindow);
    }

    IEmbeddedCompositor* Platform_Android_EGL::createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context)
    {
        UNUSED(displayConfig);
        UNUSED(context);
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }

    uint32_t Platform_Android_EGL::getSwapInterval() const
    {
        return 1u;
    }
}
