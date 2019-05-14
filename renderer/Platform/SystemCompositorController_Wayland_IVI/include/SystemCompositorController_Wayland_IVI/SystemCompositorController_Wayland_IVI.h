//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLER_WAYLAND_IVI_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLER_WAYLAND_IVI_H

#include "RendererAPI/ISystemCompositorController.h"
#include "wayland-client-protocol.h"
#include "Collections/HashMap.h"
#include "Collections/HashSet.h"
#include "Utils/Warnings.h"
PUSH_DISABLE_C_STYLE_CAST_WARNING
#include "ivi-controller-client-protocol.h"
POP_DISABLE_C_STYLE_CAST_WARNING

namespace ramses_internal
{
    class IVIControllerSurface;
    class IVIControllerScreen;
    class WaylandOutput;

    class SystemCompositorController_Wayland_IVI : public ISystemCompositorController
    {
    public:
        explicit SystemCompositorController_Wayland_IVI(const String& waylandDisplay = "");
        virtual ~SystemCompositorController_Wayland_IVI();

        virtual Bool init();
        virtual void update() override;
        virtual void listIVISurfaces() const override;
        virtual Bool setSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility) override;
        virtual Bool setSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity) override;
        virtual Bool setSurfaceDestinationRectangle(
            WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height) override;
        virtual Bool doScreenshot(const String& fileName, int32_t screenIviId) override;
        virtual Bool addSurfaceToLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) override;
        virtual Bool removeSurfaceFromLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) override;
        virtual Bool destroySurface(WaylandIviSurfaceId surfaceId) override;
        virtual Bool setLayerVisibility(WaylandIviLayerId layerId, Bool visibility) override;
        void         deleteControllerSurface(IVIControllerSurface& controllerSurface);

    private:
        IVIControllerSurface* getControllerSurface(WaylandIviSurfaceId iviId) const;
        IVIControllerScreen*  getControllerScreen(uint32_t screenId) const;
        IVIControllerSurface& getOrCreateControllerSurface(WaylandIviSurfaceId iviId);
        void                  commitAndFlushControllerChanges();

        void registryHandleGlobal(wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
        void iviControllerHandleScreen(ivi_controller* controller, uint32_t id_screen, ivi_controller_screen* nativeControllerScreen);
        void iviControllerHandleLayer(ivi_controller* controller, uint32_t id_layer);
        void iviControllerHandleSurface(ivi_controller* controller, uint32_t id_surface);

        static void RegistryHandleGlobalCallback(
            void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
        static void RegistryHandleGlobalRemoveCallback(void* data, wl_registry* wl_registry, uint32_t name);
        static void IVIControllerHandleScreenCallback(void*                  data,
                                                      ivi_controller*        controller,
                                                      uint32_t               id_screen,
                                                      ivi_controller_screen* screen);
        static void IVIControllerHandleLayerCallback(void* data, ivi_controller* controller, uint32_t id_layer);
        static void IVIControllerHandleSurfaceCallback(void* data, ivi_controller* controller, uint32_t id_surface);
        static void IVIControllerHandleErrorCallback(void*           data,
                                                     ivi_controller* controller,
                                                     int32_t         objectId,
                                                     int32_t         objectType,
                                                     int32_t         errorCode,
                                                     const char*     errorText);

        const String    m_waylandDisplay;
        wl_display*     m_display    = nullptr;
        wl_registry*    m_registry   = nullptr;
        ivi_controller* m_controller = nullptr;

        typedef HashSet<IVIControllerScreen*>  ControllerScreens;
        typedef HashSet<IVIControllerSurface*> ControllerSurfaces;
        typedef HashSet<WaylandOutput*> WaylandOutputs;

        ControllerScreens  m_controllerScreens;
        ControllerSurfaces m_controllerSurfaces;
        WaylandOutputs     m_waylandOutputs;

        const struct Registry_Listener : public wl_registry_listener
        {
            Registry_Listener()
            {
                global        = RegistryHandleGlobalCallback;
                global_remove = RegistryHandleGlobalRemoveCallback;
            }
        } m_registryListener;

        const struct IVIController_Listener : public ivi_controller_listener
        {
            IVIController_Listener()
            {
                screen  = IVIControllerHandleScreenCallback;
                layer   = IVIControllerHandleLayerCallback;
                surface = IVIControllerHandleSurfaceCallback;
                error   = IVIControllerHandleErrorCallback;
            }
        } m_iviControllerListener;
    };
}

#endif
