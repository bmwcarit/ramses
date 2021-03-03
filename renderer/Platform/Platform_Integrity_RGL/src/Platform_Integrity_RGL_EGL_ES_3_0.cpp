//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Integrity_RGL/Platform_Integrity_RGL_EGL_ES_3_0.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"
#include "RendererLib/DisplayConfig.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Integrity_RGL_EGL_ES_3_0(rendererConfig);
    }

    Platform_Integrity_RGL_EGL_ES_3_0::Platform_Integrity_RGL_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_EGL<Window_Integrity_RGL>(rendererConfig)
    {
    }

    uint32_t Platform_Integrity_RGL_EGL_ES_3_0::getSwapInterval() const
    {
        return 1u;
    }

    ISystemCompositorController* Platform_Integrity_RGL_EGL_ES_3_0::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* Platform_Integrity_RGL_EGL_ES_3_0::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Integrity_RGL* platformWindow = new Window_Integrity_RGL(displayConfig, windowEventHandler);
        return addPlatformWindow(platformWindow);
    }

    IEmbeddedCompositor* Platform_Integrity_RGL_EGL_ES_3_0::createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context)
    {
        UNUSED(displayConfig);
        UNUSED(context);
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }
}
