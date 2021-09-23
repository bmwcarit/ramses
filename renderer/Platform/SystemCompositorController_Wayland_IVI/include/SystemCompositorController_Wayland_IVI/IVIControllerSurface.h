//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IVICONTROLLERSURFACE_H
#define RAMSES_IVICONTROLLERSURFACE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "RendererAPI/Types.h"
#include "SceneAPI/WaylandIviSurfaceId.h"
#include "Utils/Warnings.h"
PUSH_DISABLE_C_STYLE_CAST_WARNING
#include "ivi-controller-client-protocol.h"
POP_DISABLE_C_STYLE_CAST_WARNING

namespace ramses_internal
{
    class SystemCompositorController_Wayland_IVI;

    class IVIControllerSurface
    {
    public:
        IVIControllerSurface(ivi_controller_surface*                 controllerSurface,
                             WaylandIviSurfaceId                     iviId,
                             SystemCompositorController_Wayland_IVI& systemCompositorController);
        ~IVIControllerSurface();

        void                      setVisibility(bool visible);
        void                      setOpacity(float opacity);
        void                      setDestinationRectangle(int32_t x, int32_t y, int32_t width, int32_t height);
        void                      sendStats();
        void                      destroy();
        ivi_controller_surface*   getNativeWaylandControllerSurface();
        const WaylandIviSurfaceId getIVIId() const;

    private:
        static void HandleVisibilityCallback(void*                   data,
                                             ivi_controller_surface* iviControllerSurface,
                                             int32_t                 visibility);

        static void HandleOpacityCallBack(void*                   data,
                                          ivi_controller_surface* iviControllerSurface,
                                          wl_fixed_t              opacity);

        static void HandleSourceRectangleCallback(void*                   data,
                                                  ivi_controller_surface* iviControllerSurface,
                                                  int32_t                 x,
                                                  int32_t                 y,
                                                  int32_t                 width,
                                                  int32_t                 height);

        static void HandleDestinationRectangleCallback(void*                   data,
                                                       ivi_controller_surface* iviControllerSurface,
                                                       int32_t                 x,
                                                       int32_t                 y,
                                                       int32_t                 width,
                                                       int32_t                 height);

        static void HandleConfigurationCallback(void*                   data,
                                                ivi_controller_surface* iviControllerSurface,
                                                int32_t                 width,
                                                int32_t                 height);

        static void HandleOrientationCallback(void*                   data,
                                              ivi_controller_surface* iviControllerSurface,
                                              int32_t                 orientation);

        static void HandlePixelformatCallback(void*                   data,
                                              ivi_controller_surface* iviControllerSurface,
                                              int32_t                 pixelformat);

        static void HandleLayerCallback(void*                   data,
                                        ivi_controller_surface* iviControllerSurface,
                                        ivi_controller_layer*   layer);

        static void HandleStatsCallback(void*                   data,
                                        ivi_controller_surface* iviControllerSurface,
                                        uint32_t                redrawCount,
                                        uint32_t                frameCount,
                                        uint32_t                updateCount,
                                        uint32_t                pid,
                                        const char*             processName);

        static void HandleDestroyedCallback(void* data, ivi_controller_surface* iviControllerSurface);

        static void HandleContentCallback(void*                   data,
                                          ivi_controller_surface* iviControllerSurface,
                                          int32_t                 contentState);

        ivi_controller_surface*                 m_controllerSurface;
        WaylandIviSurfaceId                     m_iviId;
        SystemCompositorController_Wayland_IVI& m_systemCompositorController;

        const struct IVIControllerSurface_Listener : public ivi_controller_surface_listener
        {
            IVIControllerSurface_Listener()
            {
                visibility            = HandleVisibilityCallback;
                opacity               = HandleOpacityCallBack;
                source_rectangle      = HandleSourceRectangleCallback;
                destination_rectangle = HandleDestinationRectangleCallback;
                configuration         = HandleConfigurationCallback;
                orientation           = HandleOrientationCallback;
                pixelformat           = HandlePixelformatCallback;
                layer                 = HandleLayerCallback;
                stats                 = HandleStatsCallback;
                destroyed             = HandleDestroyedCallback;
                content               = HandleContentCallback;
            }
        } m_iviControllerSurfaceListener;
    };
}

#endif
