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

#include <wayland-client.h>
#include <wayland-egl.h>
#include <ivi-application-client-protocol.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <signal.h>
#include <poll.h>
#include <getopt.h>

#include "Collections/Vector.h"
#include "Collections/HashMap.h"
#include "RendererAPI/Types.h"
#include "SHMBuffer.h"
#include "TestApplicationSurfaceId.h"
#include "TestApplicationShellSurfaceId.h"

class WaylandDisplay
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
    int              fd              = 0;
};

class WaylandWindow
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
    bool                                  useEGL         = true;
    uint32_t                              swapInterval   = 0;
};

class WaylandHandler
{
public:
    bool init();
    bool initWithSharedDisplayConnection(WaylandHandler& handlerToShareDisplay);
    void deinit();

    bool createWindow(ramses_internal::TestApplicationSurfaceId surfaceId, uint32_t windowWidth, uint32_t windowHeight, uint32_t swapInterval, bool useEGL);
    void createShellSurface(ramses_internal::TestApplicationSurfaceId surfaceId, ramses_internal::TestApplicationShellSurfaceId shellSurfaceId);
    void setShellSurfaceTitle(ramses_internal::TestApplicationShellSurfaceId shellSurfaceId, const ramses_internal::String& title);
    void setShellSurfaceDummyValues(ramses_internal::TestApplicationSurfaceId surfaceId, ramses_internal::TestApplicationShellSurfaceId shellSurfaceId);
    void destroyShellSurface(ramses_internal::TestApplicationShellSurfaceId shellSurfaceId);
    void destroyWindow(ramses_internal::TestApplicationSurfaceId surfaceId);
    void destroyIVISurface(ramses_internal::TestApplicationSurfaceId surfaceId);
    void createIVISurface(ramses_internal::TestApplicationSurfaceId surfaceId, uint32_t iviSurfaceId);
    void enableContextForSurface(ramses_internal::TestApplicationSurfaceId surfaceId);
    void disableContextForSurface();
    void resizeWindow(ramses_internal::TestApplicationSurfaceId surfaceId, uint32_t width, uint32_t height);
    void swapBuffersAndProcessEvents(ramses_internal::TestApplicationSurfaceId surfaceId, bool useCallback);
    void swapBuffersAndProcessEvents(ramses_internal::TestApplicationSurfaceId surfaceId, SHMBuffer& buffer, bool useCallback);
    void attachBuffer(ramses_internal::TestApplicationSurfaceId surfaceId, const SHMBuffer& buffer);
    void detachBuffer(ramses_internal::TestApplicationSurfaceId surfaceId);
    void deleteSHMBuffers();
    void waitOnFrameCallback(ramses_internal::TestApplicationSurfaceId surfaceId);
    bool getUseEGL(ramses_internal::TestApplicationSurfaceId surfaceId) const;
    void getWindowSize(ramses_internal::TestApplicationSurfaceId surfaceId, uint32_t& width, uint32_t& height) const;

    SHMBuffer* getFreeSHMBuffer(uint32_t width, uint32_t height);
    uint32_t getNumberOfAllocatedSHMBuffer() const;
    bool getIsSHMBufferFree(uint32_t buffer) const;

private:
    bool checkAndHandleEvents();

    static void window_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
    static void window_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface);
    static void window_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
    static void window_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    static void window_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
    static void seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);
    static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void registry_handle_global_remove(void* data, wl_registry* wl_registry, uint32_t name);
    void frameCallback(wl_callback* callback);
    static void FrameCallback(void* userData, wl_callback* callback, uint32_t);

    bool setupEGL();
    bool setupWayland();
    void closeWayland();
    bool createSurface(WaylandWindow& window);
    bool createEGLWindow(WaylandWindow& window);
    void terminateEGL();

    WaylandWindow& getWindow(ramses_internal::TestApplicationSurfaceId surfaceId) const;
    wl_shell_surface& getShellSurface(ramses_internal::TestApplicationShellSurfaceId shellSurfaceId) const;

    typedef ramses_internal::HashMap<ramses_internal::TestApplicationSurfaceId, WaylandWindow*> WindowsHashMap;
    typedef ramses_internal::HashMap<ramses_internal::TestApplicationShellSurfaceId, wl_shell_surface*> ShellSurfacesHashMap;

    EGLDisplay egldisplay = EGL_NO_DISPLAY;
    EGLConfig eglconfig;

    WaylandDisplay wayland;

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
};

#endif
