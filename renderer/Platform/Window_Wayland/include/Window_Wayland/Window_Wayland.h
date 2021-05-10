//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_WAYLAND_H
#define RAMSES_WINDOW_WAYLAND_H

#include "Platform_Base/Window_Base.h"
#include "Window_Wayland/WlContext.h"
#include "InputHandling_Wayland.h"

namespace ramses_internal
{
    class Window_Wayland : public Window_Base
    {
    public:
        Window_Wayland(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);
        ~Window_Wayland() override;

        virtual bool init() override final;

        Bool canRenderNewFrame() const override final;
        void dispatchWaylandDisplayEvents(Bool dispatchNewEventsFromDisplayFD) const;
        void handleEvents() override final;
        void frameRendered() override final;

        wl_display* getNativeDisplayHandle() const;
        wl_egl_window* getNativeWindowHandle() const;

        bool hasTitle() const override final
        {
            return false;
        }

    protected:

        virtual void registryGlobalCreated(wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version);
        virtual bool createSurface() = 0;

    private:

        void registerFrameRenderingDoneCallback();
        Bool setFullscreen(Bool fullscreen) override final;

        static void RegistryGlobalCreated(void* data, wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version);
        static void RegistryGlobalRemoved(void* data, wl_registry* wl_registry, uint32_t name);
        static void FrameRenderingDoneCallback(void* data, wl_callback* callback, uint32_t time);

    protected:
        WlContext m_wlContext;

    private:
        const String m_waylandDisplay;
        InputHandling_Wayland m_inputHandling;

        const struct FrameRenderingDoneCallback_Listener : public wl_callback_listener
        {
            FrameRenderingDoneCallback_Listener()
            {
                done = FrameRenderingDoneCallback;
            }
        } m_frameRenderingDoneCallbackListener;

        const struct Registry_Listener : public wl_registry_listener
        {
            Registry_Listener()
            {
                global        = RegistryGlobalCreated;
                global_remove = RegistryGlobalRemoved;
            }
        } m_registryListener;
    };
}

#endif
