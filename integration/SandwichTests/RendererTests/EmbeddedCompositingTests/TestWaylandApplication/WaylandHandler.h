//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDHANDLER_H
#define RAMSES_WAYLANDHANDLER_H

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#include "SHMBuffer.h"
#include "TestApplicationSurfaceId.h"
#include "TestApplicationShellSurfaceId.h"
#include "WaylandOutputTestParams.h"
#include "RendererAPI/Types.h"
#include "Collections/HashMap.h"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <ivi-application-client-protocol.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <signal.h>
#include <poll.h>
#include <getopt.h>

namespace ramses_internal
{
    class TestWaylandDisplay
    {
    public:
        wl_display*      display         = nullptr;
        wl_registry*     registry        = nullptr;
        wl_compositor*   compositor      = nullptr;
        wl_shell*        shell           = nullptr;
        wl_seat*         seat            = nullptr;
        wl_pointer*      pointer         = nullptr;
        ivi_application* ivi_app         = nullptr;
        wl_shm*          shm             = nullptr;
        wl_output*       output          = nullptr;
        int              fd              = 0;
    };

    class TestWaylandWindow
    {
    public:
        uint32_t                              width          = 0;
        uint32_t                              height         = 0;
        wl_egl_window*                        native         = nullptr;
        wl_surface*                           surface        = nullptr;
        // Not allowed by a compositor, but a client can create multiple ivi_surface's belonging to the same wl_surface, so
        // store them in a vector for proper clean-up.
        std::vector<ivi_surface*> iviSurfaces;
        EGLSurface                            eglsurface     = EGL_NO_SURFACE;
        EGLContext                            eglcontext     = EGL_NO_CONTEXT;
        wl_callback*                          frameCallback  = nullptr;
        uint32_t                              swapInterval   = 0;
    };

    class WaylandHandler
    {
    public:
        bool init();
        bool initWithSharedDisplayConnection(WaylandHandler& handlerToShareDisplay);
        void deinit();

        void setRequiredWaylandOutputVersion(uint32_t protocolVersion);
        bool createWindow(TestApplicationSurfaceId surfaceId, uint32_t windowWidth, uint32_t windowHeight, uint32_t swapInterval, bool useEGL);
        void createShellSurface(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId);
        void setShellSurfaceTitle(TestApplicationShellSurfaceId shellSurfaceId, const String& title);
        void setShellSurfaceDummyValues(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId);
        void destroyShellSurface(TestApplicationShellSurfaceId shellSurfaceId);
        void destroyWindow(TestApplicationSurfaceId surfaceId);
        void destroyIVISurface(TestApplicationSurfaceId surfaceId);
        void createIVISurface(TestApplicationSurfaceId surfaceId, uint32_t iviSurfaceId);
        void enableContextForSurface(TestApplicationSurfaceId surfaceId);
        void disableContextForSurface();
        void resizeWindow(TestApplicationSurfaceId surfaceId, uint32_t width, uint32_t height);
        void swapBuffersAndProcessEvents(TestApplicationSurfaceId surfaceId, bool useCallback);
        void swapBuffersAndProcessEvents(TestApplicationSurfaceId surfaceId, SHMBuffer& buffer, bool useCallback);
        void attachBuffer(TestApplicationSurfaceId surfaceId, const SHMBuffer& buffer, bool commit);
        void detachBuffer(TestApplicationSurfaceId surfaceId);
        void deleteSHMBuffers();
        void waitOnFrameCallback(TestApplicationSurfaceId surfaceId);
        void getWindowSize(TestApplicationSurfaceId surfaceId, uint32_t& width, uint32_t& height) const;

        SHMBuffer* getFreeSHMBuffer(uint32_t width, uint32_t height);
        uint32_t getNumberOfAllocatedSHMBuffer() const;
        bool getIsSHMBufferFree(uint32_t buffer) const;
        void getWaylandOutputTestParams(bool& errorsFound, WaylandOutputTestParams& waylandOutputParams) const;

    private:
        bool checkAndHandleEvents();
        void addWaylandOutputListener();

        static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
        static void registry_handle_global_remove(void* data, wl_registry* wl_registry, uint32_t name);

        static void window_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
        static void window_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface);
        static void window_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
        static void window_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        static void window_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
        static void seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);

        void frameCallback(wl_callback* callback);
        static void FrameCallback(void* userData, wl_callback* callback, uint32_t);

    static void wayland_output_handle_geometry(void *data,
             struct wl_output *wl_output,
             int32_t x,
             int32_t y,
             int32_t physical_width,
             int32_t physical_height,
             int32_t subpixel,
             const char *make,
             const char *model,
             int32_t transform);

    static void wayland_output_handle_mode(void *data,
             struct wl_output *wl_output,
             uint32_t flags,
             int32_t width,
             int32_t height,
             int32_t refresh);

    static void wayland_output_handle_done(void *data,
             struct wl_output *wl_output);

    static void wayland_output_handle_scale(void *data,
              struct wl_output *wl_output,
              int32_t factor);

        bool setupEGL();
        bool setupWayland();
        void closeWayland();
        bool createSurface(TestWaylandWindow& window);
        bool createEGLWindow(TestWaylandWindow& window);
        void terminateEGL();

        TestWaylandWindow& getWindow(TestApplicationSurfaceId surfaceId) const;
        wl_shell_surface& getShellSurface(TestApplicationShellSurfaceId shellSurfaceId) const;

        typedef HashMap<TestApplicationSurfaceId, TestWaylandWindow*> WindowsHashMap;
        typedef HashMap<TestApplicationShellSurfaceId, wl_shell_surface*> ShellSurfacesHashMap;

        EGLDisplay egldisplay = EGL_NO_DISPLAY;
        EGLConfig eglconfig;

        TestWaylandDisplay wayland;

        WindowsHashMap m_windows;
        ShellSurfacesHashMap m_shellSurfaces;

        bool m_usesSharedDisplay = false;
        std::vector<SHMBuffer*> m_shmBuffer;

        const struct Registry_Listener : public wl_registry_listener
        {
            Registry_Listener()
            {
                global        = registry_handle_global;
                global_remove = registry_handle_global_remove;
            }
        } m_registryListener;

        const struct FrameRenderingDoneCallback_Listener : public wl_callback_listener
        {
            FrameRenderingDoneCallback_Listener()
            {
                done = FrameCallback;
            }
        } m_frameRenderingDoneCallbackListener;

        const struct WaylandOutput_ListenerV1 : public wl_output_listener {
            WaylandOutput_ListenerV1()
            {
                geometry    = wayland_output_handle_geometry;
                mode        = wayland_output_handle_mode;
                done        = nullptr;
                scale       = nullptr;
            }
        } m_waylandOutputListenerV1;

        const struct WaylandOutput_ListenerV2 : public wl_output_listener {
            WaylandOutput_ListenerV2()
            {
                geometry    = wayland_output_handle_geometry;
                mode        = wayland_output_handle_mode;
                done        = wayland_output_handle_done;
                scale       = wayland_output_handle_scale;
            }
        } m_waylandOutputListenerV2;

        const struct WaylandOutput_ListenerV3 : public wl_output_listener {
            WaylandOutput_ListenerV3()
            {
                geometry    = wayland_output_handle_geometry;
                mode        = wayland_output_handle_mode;
                done        = wayland_output_handle_done;
                scale       = wayland_output_handle_scale;
            }
        } m_waylandOutputListenerV3;

        uint32_t m_requiredWaylandOutputVersion = 0u;
        bool m_errorFoundInWaylandOutput = false;
        WaylandOutputTestParams m_waylandOutputParams;
    };
}

#endif
