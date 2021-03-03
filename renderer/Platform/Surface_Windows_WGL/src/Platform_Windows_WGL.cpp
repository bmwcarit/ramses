//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Surface_Windows_WGL/Platform_Windows_WGL.h"
#include "Context_WGL/Context_WGL.h"
#include "Window_Windows/Window_Windows.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{
    Platform_Windows_WGL::Platform_Windows_WGL(const RendererConfig& rendererConfig)
        : Platform_Base(rendererConfig)
        , m_wglExtensions()
    {
    }

    ISystemCompositorController* Platform_Windows_WGL::createSystemCompositorController()
    {
        return nullptr;
    }

    IWindow* Platform_Windows_WGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        Window_Windows* platformWindow = new Window_Windows(displayConfig, windowEventHandler, static_cast<UInt32>(m_windows.size()));
        return addPlatformWindow(platformWindow);
    }

    IContext* Platform_Windows_WGL::createContext(const DisplayConfig& displayConfig, IWindow& window, IContext* sharedContext)
    {
        Window_Windows* platformWindow = getPlatformWindow<Window_Windows>(window);
        assert(0 != platformWindow);

        Context_WGL* context = nullptr;
        if (sharedContext)
        {
            Context_WGL* const platformSharedContext = getPlatformContext<Context_WGL>(*sharedContext);
            assert(platformSharedContext);

            context = new Context_WGL(*platformSharedContext,
                platformWindow->getNativeDisplayHandle(),
                m_wglExtensions, getContextAttributes(),
                platformWindow->getMSAASampleCount());
        }
        else
        {
            context = new Context_WGL(displayConfig.getDepthStencilBufferType(),
                platformWindow->getNativeDisplayHandle(),
                m_wglExtensions, getContextAttributes(),
                platformWindow->getMSAASampleCount());
        }

        return addPlatformContext(context);
    }

    IEmbeddedCompositor* Platform_Windows_WGL::createEmbeddedCompositor(const DisplayConfig& displayConfig, IContext& context)
    {
        UNUSED(displayConfig);
        UNUSED(context);
        EmbeddedCompositor_Dummy* compositor = new EmbeddedCompositor_Dummy();
        return addEmbeddedCompositor(compositor);
    }
}
