//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/IVI/Platform_Wayland_IVI_EGL_ES_3_0.h"
#include "internal/Platform/Wayland/IVI/Window_Wayland_IVI.h"
#include "internal/Platform/Wayland/IVI/SystemCompositorController/SystemCompositorController_Wayland_IVI.h"
#include "internal/RendererLib/RendererConfig.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"

namespace ramses::internal
{

    Platform_Wayland_IVI_EGL_ES_3_0::Platform_Wayland_IVI_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_Wayland_EGL(rendererConfig)
    {
    }

    bool Platform_Wayland_IVI_EGL_ES_3_0::createSystemCompositorController()
    {
        assert(!m_systemCompositorController);
        auto systemCompositorController = std::make_unique<SystemCompositorController_Wayland_IVI>(m_rendererConfig.getWaylandDisplayForSystemCompositorController());
        if (systemCompositorController->init())
        {
            m_systemCompositorController = std::move(systemCompositorController);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Wayland_IVI_EGL_ES_3_0:createSystemCompositorController: failed to initialize system compositor controller");
        }

        return m_systemCompositorController != nullptr;
    }


    bool Platform_Wayland_IVI_EGL_ES_3_0::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        if(!displayConfig.getWaylandIviLayerID().isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Can not create Wayland IVI window because IVI layer ID was not set in display config!");
            return false;
        }

        if(!displayConfig.getWaylandIviSurfaceID().isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Can not create Wayland IVI window because IVI surface ID was not set in display config!");
            return false;
        }

        auto window = std::make_unique<Window_Wayland_IVI>(displayConfig, windowEventHandler, 0u, m_frameCallbackMaxPollTime);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }
}
