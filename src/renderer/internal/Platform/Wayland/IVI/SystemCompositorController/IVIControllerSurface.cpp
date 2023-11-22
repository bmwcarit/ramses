//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/IVI/SystemCompositorController/IVIControllerSurface.h"
#include "internal/Platform/Wayland/IVI/SystemCompositorController/SystemCompositorController_Wayland_IVI.h"
#include "internal/Core/Utils/LogMacros.h"
#include <cassert>

namespace ramses::internal
{
    IVIControllerSurface::IVIControllerSurface(ivi_controller_surface*                 controllerSurface,
                                               WaylandIviSurfaceId                     iviId,
                                               SystemCompositorController_Wayland_IVI& systemCompositorController)
        : m_controllerSurface(controllerSurface)
        , m_iviId(iviId)
        , m_systemCompositorController(systemCompositorController)
    {
        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::IVIControllerSurface: {}", m_iviId);

        if (nullptr != m_controllerSurface)
        {
            ivi_controller_surface_add_listener(m_controllerSurface, &m_iviControllerSurfaceListener, this);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "IVIControllerSurface::IVIControllerSurface m_controllerSurface is nullptr!");
            assert(false);
        }
    }

    IVIControllerSurface::~IVIControllerSurface()
    {
        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::~IVIControllerSurface: {}", m_iviId);

        if (nullptr != m_controllerSurface)
        {
            ivi_controller_surface_destroy(m_controllerSurface, 0);
        }
    }

    void IVIControllerSurface::setVisibility(bool visible)
    {
        if (nullptr != m_controllerSurface)
        {
            ivi_controller_surface_set_visibility(m_controllerSurface, (visible) ? 1 : 0);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "IVIControllerSurface::IVIControllerSurface m_controllerSurface is nullptr!");
            assert(false);
        }
    }

    void IVIControllerSurface::setOpacity(float opacity)
    {
        if (nullptr != m_controllerSurface)
        {
            ivi_controller_surface_set_opacity(m_controllerSurface, wl_fixed_from_double(opacity));
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "IVIControllerSurface::setOpacity m_controllerSurface is nullptr!");
            assert(false);
        }
    }

    void IVIControllerSurface::setDestinationRectangle(int32_t x, int32_t y, int32_t width, int32_t height)
    {
        if (nullptr != m_controllerSurface)
        {
            ivi_controller_surface_set_destination_rectangle(m_controllerSurface, x, y, width, height);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "IVIControllerSurface::setDestinationRectangle m_controllerSurface is nullptr!");
            assert(false);
        }
    }

    void IVIControllerSurface::sendStats()
    {
        if (nullptr != m_controllerSurface)
        {
            ivi_controller_surface_send_stats(m_controllerSurface);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "IVIControllerSurface::sendStats m_controllerSurface is nullptr!");
            assert(false);
        }
    }

    void IVIControllerSurface::destroy()
    {
        if (nullptr != m_controllerSurface)
        {
            ivi_controller_surface_destroy(m_controllerSurface, 1);
            m_controllerSurface = nullptr;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "IVIControllerSurface::destroy m_controllerSurface is nullptr!");
            assert(false);
        }
    }

    ivi_controller_surface* IVIControllerSurface::getNativeWaylandControllerSurface()
    {
        return m_controllerSurface;
    }

    WaylandIviSurfaceId IVIControllerSurface::getIVIId() const
    {
        return m_iviId;
    }

    void IVIControllerSurface::HandleVisibilityCallback(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, int32_t visibility)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);
        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleVisibilityCallback {} visibility: {}", controllerSurface.m_iviId, visibility);
    }

    void IVIControllerSurface::HandleOpacityCallBack(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, wl_fixed_t opacity)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleOpacityCallBack {} opacity: {}", controllerSurface.m_iviId, opacity);
    }

    void IVIControllerSurface::HandleSourceRectangleCallback(
        void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleSourceRectangleCallback {} x: {} y: {} width: {} height: {}", controllerSurface.m_iviId, x, y, width, height);
    }

    void IVIControllerSurface::HandleDestinationRectangleCallback(
        void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleDestinationRectangleCallback {} x: {} y: {} width: {} height: ", controllerSurface.m_iviId, x, y, width, height);
    }

    void IVIControllerSurface::HandleConfigurationCallback(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, int32_t width, int32_t height)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleConfigurationCallback {} width: {} height: {}", controllerSurface.m_iviId, width, height);
    }

    void IVIControllerSurface::HandleOrientationCallback(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, int32_t orientation)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleOrientationCallback {} orientation: {}", controllerSurface.m_iviId, orientation);
    }

    void IVIControllerSurface::HandlePixelformatCallback(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, int32_t pixelformat)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandlePixelformatCallback {} pixelformat: {}", controllerSurface.m_iviId, pixelformat);
    }

    void IVIControllerSurface::HandleLayerCallback(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, [[maybe_unused]] ivi_controller_layer* layer)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::HandleLayerCallback: surface {} added to layer", controllerSurface.m_iviId);
    }

    void IVIControllerSurface::HandleStatsCallback(void*                                    data,
                                                   [[maybe_unused]] ivi_controller_surface* iviControllerSurface,
                                                   uint32_t                                 redrawCount,
                                                   uint32_t                                 frameCount,
                                                   uint32_t                                 updateCount,
                                                   uint32_t                                 pid,
                                                   const char*                              processName)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleStatsCallback {} redrawCount: {} frameCount: {} updateCount: {} pid: {} processName: {}",
                 controllerSurface.m_iviId, redrawCount, frameCount, updateCount, pid, processName);
    }

    void IVIControllerSurface::HandleDestroyedCallback(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::HandleDestroyedCallback {}", controllerSurface.m_iviId);

        SystemCompositorController_Wayland_IVI& systemCompositorController = controllerSurface.m_systemCompositorController;
        systemCompositorController.deleteControllerSurface(controllerSurface);
    }

    void IVIControllerSurface::HandleContentCallback(void* data, [[maybe_unused]] ivi_controller_surface* iviControllerSurface, int32_t contentState)
    {
        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleContentCallback {} contentState: {}", controllerSurface.m_iviId, contentState);
    }
}
