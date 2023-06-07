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
#include "Utils/Warnings.h"
PUSH_DISABLE_C_STYLE_CAST_WARNING
#include "ivi-controller-client-protocol.h"
POP_DISABLE_C_STYLE_CAST_WARNING
#include <string_view>
#include <string>

namespace ramses_internal
{
    class IVIControllerSurface;
    class IVIControllerScreen;
    class WaylandOutput;

    class SystemCompositorController_Wayland_IVI : public ISystemCompositorController
    {
    public:
        explicit SystemCompositorController_Wayland_IVI(std::string_view waylandDisplay = {});
        ~SystemCompositorController_Wayland_IVI() override;

        virtual bool init();
        void update() override;
        void listIVISurfaces() const override;
        bool setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility) override;
        bool setSurfaceOpacity(WaylandIviSurfaceId surfaceId, float opacity) override;
        bool setSurfaceDestinationRectangle(
            WaylandIviSurfaceId surfaceId, int32_t x, int32_t y, int32_t width, int32_t height) override;
        bool doScreenshot(std::string_view fileName, int32_t screenIviId) override;
        bool addSurfaceToLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) override;
        bool removeSurfaceFromLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) override;
        bool destroySurface(WaylandIviSurfaceId surfaceId) override;
        bool setLayerVisibility(WaylandIviLayerId layerId, bool visibility) override;
        void         deleteControllerSurface(IVIControllerSurface& controllerSurface);

    private:
        [[nodiscard]] IVIControllerSurface* getControllerSurface(WaylandIviSurfaceId iviId) const;
        [[nodiscard]] IVIControllerScreen*  getControllerScreen(uint32_t screenId) const;
        IVIControllerSurface& getOrCreateControllerSurface(WaylandIviSurfaceId iviId);
        void                  commitAndFlushControllerChanges();

        void registryHandleGlobal(wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
        void iviControllerHandleScreen(ivi_controller* controller, uint32_t id_screen, ivi_controller_screen* nativeControllerScreen);
        void iviControllerHandleLayer(ivi_controller* controller, uint32_t id_layer);
        void iviControllerHandleSurface(ivi_controller* controller, uint32_t iviID);

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

        const std::string m_waylandDisplay;
        wl_display*       m_display    = nullptr;
        wl_registry*      m_registry   = nullptr;
        ivi_controller*   m_controller = nullptr;

        std::vector<std::unique_ptr<IVIControllerScreen>>   m_controllerScreens;
        std::vector<std::unique_ptr<IVIControllerSurface>>  m_controllerSurfaces;
        std::vector<std::unique_ptr<WaylandOutput>>         m_waylandOutputs;

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
