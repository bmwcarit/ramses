//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SystemCompositorController_Wayland_IVI/IVIControllerSurface.h"
#include "SystemCompositorController_Wayland_IVI/SystemCompositorController_Wayland_IVI.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    IVIControllerSurface::IVIControllerSurface(ivi_controller_surface*                 controllerSurface,
                                               WaylandIviSurfaceId                     iviId,
                                               SystemCompositorController_Wayland_IVI& systemCompositorController)
        : m_controllerSurface(controllerSurface)
        , m_iviId(iviId)
        , m_systemCompositorController(systemCompositorController)
    {
        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::IVIControllerSurface ivi-id: " << m_iviId);

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
        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::~IVIControllerSurface ivi-id: " << m_iviId);

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

    const WaylandIviSurfaceId IVIControllerSurface::getIVIId() const
    {
        return m_iviId;
    }

    void IVIControllerSurface::HandleVisibilityCallback(void*                   data,
                                                        ivi_controller_surface* iviControllerSurface,
                                                        int32_t                 visibility)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);
        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleVisibilityCallback ivi-id: " << controllerSurface.m_iviId << " visibility: " << visibility);
    }

    void IVIControllerSurface::HandleOpacityCallBack(void*                   data,
                                                     ivi_controller_surface* iviControllerSurface,
                                                     wl_fixed_t              opacity)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleOpacityCallBack ivi-id: " << controllerSurface.m_iviId << " opacity: " << opacity);

    }

    void IVIControllerSurface::HandleSourceRectangleCallback(
        void* data, ivi_controller_surface* iviControllerSurface, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleSourceRectangleCallback ivi-id: "
                     << controllerSurface.m_iviId << " x: " << x << " y: " << y << " width: " << width << " height: " << height);

    }

    void IVIControllerSurface::HandleDestinationRectangleCallback(
        void* data, ivi_controller_surface* iviControllerSurface, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleDestinationRectangleCallback ivi-id: "
                     << controllerSurface.m_iviId << " x: " << x << " y: " << y << " width: " << width << " height: " << height);

    }

    void IVIControllerSurface::HandleConfigurationCallback(void*                   data,
                                                           ivi_controller_surface* iviControllerSurface,
                                                           int32_t                 width,
                                                           int32_t                 height)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleConfigurationCallback ivi-id: " << controllerSurface.m_iviId << " width: "
                                                                              << width << " height: " << height);
    }

    void IVIControllerSurface::HandleOrientationCallback(void*                   data,
                                                         ivi_controller_surface* iviControllerSurface,
                                                         int32_t                 orientation)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleOrientationCallback ivi-id: " << controllerSurface.m_iviId
                                                                            << " orientation: " << orientation);
    }

    void IVIControllerSurface::HandlePixelformatCallback(void*                   data,
                                                         ivi_controller_surface* iviControllerSurface,
                                                         int32_t                 pixelformat)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandlePixelformatCallback ivi-id: " << controllerSurface.m_iviId
                                                                            << " pixelformat: " << pixelformat);
    }

    void IVIControllerSurface::HandleLayerCallback(void*                   data,
                                                   ivi_controller_surface* iviControllerSurface,
                                                   ivi_controller_layer*   layer)
    {
        UNUSED(iviControllerSurface)
        UNUSED(layer)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::HandleLayerCallback: surface " << controllerSurface.m_iviId << " added to layer");
    }

    void IVIControllerSurface::HandleStatsCallback(void*                   data,
                                                   ivi_controller_surface* iviControllerSurface,
                                                   uint32_t                redrawCount,
                                                   uint32_t                frameCount,
                                                   uint32_t                updateCount,
                                                   uint32_t                pid,
                                                   const char*             processName)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleStatsCallback ivi-id: "
                     << controllerSurface.m_iviId << " redrawCount: " << redrawCount << " frameCount: " << frameCount
                     << " updateCount: " << updateCount << " pid: " << pid << " processName: " << processName);
    }

    void IVIControllerSurface::HandleDestroyedCallback(void* data, ivi_controller_surface* iviControllerSurface)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::HandleDestroyedCallback ivi-id: " << controllerSurface.m_iviId);

        SystemCompositorController_Wayland_IVI& systemCompositorController = controllerSurface.m_systemCompositorController;
        systemCompositorController.deleteControllerSurface(controllerSurface);
    }

    void IVIControllerSurface::HandleContentCallback(void*                   data,
                                                     ivi_controller_surface* iviControllerSurface,
                                                     int32_t                 contentState)
    {
        UNUSED(iviControllerSurface)

        IVIControllerSurface& controllerSurface = *static_cast<IVIControllerSurface*>(data);

        LOG_INFO(CONTEXT_RENDERER,
                 "IVIControllerSurface::HandleContentCallback ivi-id: " << controllerSurface.m_iviId
                                                                        << " contentState: " << contentState);
    }
}
