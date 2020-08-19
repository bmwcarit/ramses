//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandHandler.h"
#include "SHMBuffer.h"
#include "Utils/LogMacros.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include <GLES3/gl3.h>
#include <cassert>

namespace ramses_internal
{
    bool WaylandHandler::init()
    {
        if (!setupWayland())
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupWaylandEGLWindow(): could not setup Wayland");
            return false;
        }

        if (!setupEGL())
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupWaylandEGLWindow(): could not setup EGL context");
            return false;
        }

        return true;
    }

    bool WaylandHandler::initWithSharedDisplayConnection(WaylandHandler& handlerToShareDisplay)
    {
        m_usesSharedDisplay = true;

        wayland.display = handlerToShareDisplay.wayland.display;

        if (wayland.display == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupWaylandEGLWithSharedDisplayConnection: no display to share");
            return false;
        }

        wayland.registry = wl_display_get_registry(wayland.display);

        wl_registry_add_listener(wayland.registry, &m_registryListener, this);

        wayland.fd = wl_display_get_fd(wayland.display);

        wl_display_dispatch(wayland.display);
        wl_display_roundtrip(wayland.display);
        LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::setupWaylandWithSharedDisplayConnection(): successful");

        if (!setupEGL())
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupWaylandEGLWindow(): could not setup EGL context");
            return false;
        }
        return true;
    }

    void WaylandHandler::deinit()
    {
        WindowsHashMap windowsToDelete = m_windows;
        for (auto i: windowsToDelete)
        {
            destroyWindow(i.key);
        }

        for (auto i : m_shellSurfaces)
        {
            wl_shell_surface_destroy(i.value);
        }

        deleteSHMBuffers();
        terminateEGL();
        closeWayland();

        m_requiredWaylandOutputVersion = 0u;
        m_errorFoundInWaylandOutput = false;
        m_waylandOutputParams = {};
    }

    void WaylandHandler::setRequiredWaylandOutputVersion(uint32_t protocolVersion)
    {
        if(wayland.output)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setRequiredWaylandOutputVersion(): wl_output is already bound!");
            assert(false);
            return;
        }

        m_requiredWaylandOutputVersion = protocolVersion;
    }

    bool WaylandHandler::createWindow(TestApplicationSurfaceId surfaceId, uint32_t windowWidth, uint32_t windowHeight, uint32_t swapInterval, bool useEGL)
    {
        TestWaylandWindow* window = new TestWaylandWindow;
        m_windows.put(surfaceId, window);

        window->width = windowWidth;
        window->height = windowHeight;
        window->useEGL = useEGL;
        window->swapInterval = swapInterval;
        if (!createSurface(*window))
        {
            return false;
        }

        if (useEGL)
        {
            if (!createEGLWindow(*window))
            {
                return false;
            }
        }

        return true;
    }

    void WaylandHandler::swapBuffersAndProcessEvents(TestApplicationSurfaceId surfaceId, bool useCallback)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        if (useCallback)
        {
            window.frameCallback = wl_surface_frame(window.surface);
            wl_callback_add_listener(window.frameCallback, &m_frameRenderingDoneCallbackListener, this);
        }

        eglSwapBuffers(egldisplay, window.eglsurface);
        checkAndHandleEvents();

        while (window.frameCallback != nullptr)
        {
            wl_display_dispatch(wayland.display);
        }
    }

    void WaylandHandler::swapBuffersAndProcessEvents(TestApplicationSurfaceId surfaceId, SHMBuffer& buffer, bool useCallback)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        if (useCallback)
        {
            window.frameCallback = wl_surface_frame(window.surface);
            wl_callback_add_listener(window.frameCallback, &m_frameRenderingDoneCallbackListener, this);
        }

        buffer.attachAndCommitToSurface(window.surface);
        checkAndHandleEvents();
    }

    void WaylandHandler::attachBuffer(TestApplicationSurfaceId surfaceId, const SHMBuffer& buffer)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        wl_surface_attach(window.surface, buffer.getWaylandBuffer(), 0, 0);
        wl_display_flush(wayland.display);
    }

    void WaylandHandler::detachBuffer(TestApplicationSurfaceId surfaceId)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        wl_surface_attach(window.surface, nullptr, 0, 0);
        wl_surface_damage(window.surface, 0, 0, window.width, window.height);
        wl_surface_commit(window.surface);
        //make roundtrip to wait for buffer release event:
        //roundtrip waits for done event from EC. EC should send buffer release event not later
        //than the done event. If the buffer release event is not received after the roundtrip
        //the tests should fail
        wl_display_roundtrip(wayland.display);
    }

    void WaylandHandler::waitOnFrameCallback(TestApplicationSurfaceId surfaceId)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        while (window.frameCallback != nullptr)
        {
            wl_display_dispatch(wayland.display);
        }
    }

    bool WaylandHandler::checkAndHandleEvents()
    {
        pollfd pfd;

        pfd.fd = wayland.fd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        wl_display_dispatch_pending(wayland.display);
        wl_display_flush(wayland.display);

        if (poll(&pfd, 1, 0) == -1)
        {
            return false;
        }

        if (pfd.revents & POLLIN)
        {
            return wl_display_dispatch(wayland.display);
        }

        return true;
    }

    bool WaylandHandler::setupEGL()
    {
        static const EGLint ConfigAttribs[] =
        {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,   // add this, otherwise EGL might not get any configs
            EGL_RED_SIZE, 1,
            EGL_GREEN_SIZE, 1,
            EGL_BLUE_SIZE, 1,
            EGL_ALPHA_SIZE, 1,
            EGL_DEPTH_SIZE, 24, // 16 bit does not work on the Intel NUC VDT for unknown reason
            EGL_NONE
        };

        assert(nullptr != wayland.display);
        egldisplay = eglGetDisplay(static_cast<EGLNativeDisplayType>(wayland.display));
        LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::setupEGL(): Display: " << egldisplay);

        if (!m_usesSharedDisplay)
        {
            EGLint major;
            EGLint minor;
            if (eglInitialize(egldisplay, &major, &minor) != EGL_TRUE)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupEGL(): eglInitialize failed with code :" << eglGetError());
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::setupEGL(): EGL version: " << major << "." << minor);
            if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupEGL(): eglBindAPI with code :" << eglGetError());
                return false;
            }
        }

        EGLint numconfigs;
        if (eglChooseConfig(egldisplay, ConfigAttribs, &eglconfig, 1, &numconfigs) != EGL_TRUE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupEGL(): eglChooseConfig failed !");
            return false;
        }

        if (numconfigs == 0)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupEGL(): got no EGL configs");
            return false;
        }

        return true;
    }

    void WaylandHandler::registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t versionProvidedByCompositor)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): binding interface: " << interface << " version: " << versionProvidedByCompositor);

        WaylandHandler* waylandHandler = static_cast<WaylandHandler*>(data);

        if (strcmp(interface, "wl_compositor") == 0)
        {
            if(waylandHandler->wayland.compositor)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): interface: " << interface << " already bound!");
                assert(false);
            }

            waylandHandler->wayland.compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, versionProvidedByCompositor));
        }
        else if (strcmp(interface, "wl_shell") == 0)
        {
            if(waylandHandler->wayland.shell)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): interface: " << interface << " already bound!");
                assert(false);
            }

            waylandHandler->wayland.shell = static_cast<wl_shell*>(wl_registry_bind(registry, name, &wl_shell_interface, versionProvidedByCompositor));
        }
        else if (strcmp(interface, "wl_seat") == 0)
        {
            if(waylandHandler->wayland.seat)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): interface: " << interface << " already bound!");
                assert(false);
            }

            waylandHandler->wayland.seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, versionProvidedByCompositor));
        }
        else if (strcmp(interface, "ivi_application") == 0)
        {
            if(waylandHandler->wayland.ivi_app)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): interface: " << interface << " already bound!");
                assert(false);
            }

            waylandHandler->wayland.ivi_app = static_cast<ivi_application*>(wl_registry_bind(registry, name, &ivi_application_interface, versionProvidedByCompositor));
        }
        else if (strcmp(interface, "wl_shm") == 0)
        {
            if(waylandHandler->wayland.shm)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): interface: " << interface << " already bound!");
                assert(false);
            }

            waylandHandler->wayland.shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, versionProvidedByCompositor));
        }
        else if (strcmp(interface, "wl_output") == 0)
        {
            if(waylandHandler->wayland.output)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): interface: " << interface << " already bound!");
                abort();
            }

            if(waylandHandler->m_requiredWaylandOutputVersion)
            {
                const uint32_t maximumExpectedProtocolVersion = 3u;
                if(versionProvidedByCompositor > maximumExpectedProtocolVersion)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): wl_output version provied by compositor: " << versionProvidedByCompositor
                              << " while maximum expected version is: " << maximumExpectedProtocolVersion <<"!");
                    abort();
                }

                waylandHandler->wayland.output = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, waylandHandler->m_requiredWaylandOutputVersion));
                waylandHandler->addWaylandOutputListener();
            }
        }
        else if (strcmp(interface, "wl_drm") == 0)
        {
        }
        else if (strcmp(interface, "zwp_linux_dmabuf_v1") == 0)
        {
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::registry_handle_global(): unhandled interface: " << interface << " !");
            assert(false);
        }
    }

    void WaylandHandler::registry_handle_global_remove(void* data, wl_registry* wl_registry, uint32_t name)
    {
        UNUSED(data)
        UNUSED(wl_registry)
        UNUSED(name)
    }

    bool WaylandHandler::setupWayland()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::setupWayland(): will connect to display");

        WaylandEnvironmentUtils::LogEnvironmentState("");
        wayland.display = wl_display_connect(nullptr);
        if (wayland.display == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::setupWayland(): wl_display_connect() failed");
            return false;
        }

        wayland.registry = wl_display_get_registry(wayland.display);

        wl_registry_add_listener(wayland.registry, &m_registryListener, this);

        wayland.fd = wl_display_get_fd(wayland.display);

        wl_display_dispatch(wayland.display);
        wl_display_roundtrip(wayland.display);
        LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::setupWayland(): successful");

        return true;
    }

    void WaylandHandler::closeWayland()
    {
        if (wayland.ivi_app != nullptr)
        {
            ivi_application_destroy(wayland.ivi_app);
            wayland.ivi_app = nullptr;
        }
        if (wayland.pointer != nullptr)
        {
            wl_pointer_destroy(wayland.pointer);
            wayland.pointer = nullptr;
        }
        if (wayland.seat != nullptr)
        {
            wl_seat_destroy(wayland.seat);
            wayland.seat = nullptr;
        }
        if (wayland.shell != nullptr)
        {
            wl_shell_destroy(wayland.shell);
            wayland.shell = nullptr;
        }
        if (wayland.compositor != nullptr)
        {
            wl_compositor_destroy(wayland.compositor);
            wayland.compositor = nullptr;
        }
        if (wayland.registry != nullptr)
        {
            wl_registry_destroy(wayland.registry);
            wayland.registry = nullptr;
        }
        if (wayland.display != nullptr)
        {
            wl_display_roundtrip(wayland.display);

            if (!m_usesSharedDisplay)
            {
                wl_display_disconnect(wayland.display);
            }
            wayland.display = nullptr;
        }
        wayland.fd = 0;
    }

    bool WaylandHandler::createSurface(TestWaylandWindow& window)
    {
        window.surface = wl_compositor_create_surface(wayland.compositor);
        if (window.surface == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::createSurface wl_compositor_create_surface failed!");
            return false;
        }

        // These calls don't have any impact on the EC tests, but are invoked in order to increase the test code coverage of
        // the EC tests. wl_surface_set_opaque_region has no effect because RAMSES ignores this property (stream textures
        // are always shown, regardless if opaque or transparent). set_input_region has no effect because RAMSES doesn't
        // support wayland input propagation for stream textures. wl_surface_set_buffer_transform and
        // wl_surface_set_buffer_scale are also ignored in RAMSES, because they are mentioned for a 2D compositor and don't
        // make sense for a compositor which shows the content on a 3D scene. For a 2D compositor and for example a rotated
        // output, a client can render already rotated and tell the compositor that buffer content is rotated. That saves an
        // additional extra buffer rotation on the compositor side, when showing the buffer on the output. Doing the same in the
        // RAMSES world, would mean that the transform values from the wayland client would have to be propagated back to the
        // RAMSES client, which creates the 3D scene. However, no optimization can be achieved in this way, since rotation
        // and scaling does not cost extra, when rendering with OpenGL. wl_surface_damage_buffer has no effect because
        // RAMSES ignores the damage area, instead RAMSES always repaints the whole scene, and not only the damaged parts of
        // a client. Regions are used for the damage and input area, which are both not used in RAMSES, so the regions
        // implementation is also empty in RAMSES.
        wl_region* region = wl_compositor_create_region(wayland.compositor);
        wl_region_add(region, 0, 0, window.width, window.height);
        wl_region_subtract(region, 0, 0, 0, 0);
        wl_surface_set_opaque_region(window.surface, region);
        wl_surface_set_input_region(window.surface, region);
        wl_region_destroy(region);
        wl_surface_set_buffer_transform(window.surface, WL_OUTPUT_TRANSFORM_NORMAL);
        wl_surface_set_buffer_scale(window.surface, 1);
    #ifdef WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION
        wl_surface_damage_buffer(window.surface, 0, 0, 0, 0);
    #endif

        wl_display_flush(wayland.display);

        LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::createSurface successful");
        return true;
    }

    bool WaylandHandler::createEGLWindow(TestWaylandWindow& window)
    {
        window.native = wl_egl_window_create(window.surface, window.width, window.height);
        window.eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, static_cast<NativeWindowType>(window.native), nullptr);
        if (window.eglsurface == EGL_NO_SURFACE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::createEGLWindow eglCreateWindowSurface failed !");
            return false;
        }
        EGLint ContextAttribList[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        window.eglcontext = eglCreateContext(egldisplay, eglconfig, EGL_NO_CONTEXT, ContextAttribList);
        if (window.eglcontext == EGL_NO_CONTEXT)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::createEGLWindow eglCreateContext failed !");
            return false;
        }

        return true;
    }

    void WaylandHandler::terminateEGL()
    {
        if (!m_usesSharedDisplay)
        {
            if (egldisplay != EGL_NO_DISPLAY)
            {
                if (eglTerminate(egldisplay) != EGL_TRUE)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::terminateEGL failed !");
                }
            }
        }
    }

    void WaylandHandler::destroyWindow(TestApplicationSurfaceId surfaceId)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        if (window.surface != nullptr)
        {
            if (window.useEGL)
            {
                //TODO Mohamed: figure out what is the real issue and remove this roundtrip
                //when real issue fixed
                wl_display_roundtrip(wayland.display);
                if (eglDestroySurface(egldisplay, window.eglsurface) != EGL_TRUE)
                {
                    LOG_ERROR(CONTEXT_RENDERER,
                              "WaylandHandler::destroyWindow eglDestroySurface failed !");
                }
                if (eglDestroyContext(egldisplay, window.eglcontext) != EGL_TRUE)
                {
                    LOG_ERROR(CONTEXT_RENDERER,
                              "WaylandHandler::destroyWindow eglDestroyContext failed !");
                }
                window.eglcontext = EGL_NO_CONTEXT;

                wl_egl_window_destroy(window.native);
                window.native = nullptr;
            }

            destroyIVISurface(surfaceId);

            wl_surface_destroy(window.surface);
            window.surface = nullptr;

            wl_display_roundtrip(wayland.display);
        }

        delete &window;
        m_windows.remove(surfaceId);
    }

    void WaylandHandler::destroyIVISurface(TestApplicationSurfaceId surfaceId)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        for (auto iviSurface : window.iviSurfaces)
        {
            ivi_surface_destroy(iviSurface);
        }
        window.iviSurfaces.clear();
        wl_display_flush(wayland.display);
    }

    void WaylandHandler::createIVISurface(TestApplicationSurfaceId surfaceId, uint32_t iviSurfaceId)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        if (wayland.ivi_app != nullptr)
        {
            ivi_surface* iviSurface = ivi_application_surface_create(wayland.ivi_app, iviSurfaceId, window.surface);

            if (iviSurface == nullptr)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandHandler::createIVISurface(): Failed to create ivi-surface");
                assert(false);
            }
            else
            {
                window.iviSurfaces.push_back(iviSurface);
            }
            wl_display_flush(wayland.display);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::createIVISurface(): wayland.ivi_app is 0!");
            assert(false);
        }
    }

    void WaylandHandler::createShellSurface(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        if (wayland.shell != nullptr)
        {
            wl_shell_surface* shellSurface = wl_shell_get_shell_surface(wayland.shell, window.surface);
            assert(shellSurface != nullptr);
            m_shellSurfaces.put(shellSurfaceId, shellSurface);
            wl_shell_surface_set_toplevel(shellSurface);

            wl_display_flush(wayland.display);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandHandler::createShellSurface wl_shell interface not available!");
        }
    }

    void WaylandHandler::enableContextForSurface(TestApplicationSurfaceId surfaceId)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        if (eglMakeCurrent(egldisplay, window.eglsurface, window.eglsurface, window.eglcontext) != EGL_TRUE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::makeCurrent eglMakeCurrent failed !");
        }
        if (eglSwapInterval(egldisplay, window.swapInterval) != EGL_TRUE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::makeCurrent eglSwapInterval failed !");
        }
        glViewport(0, 0, window.width, window.height);
    }

    void WaylandHandler::disableContextForSurface()
    {
        if (eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::makeUncurrent eglMakeCurrent failed !");
        }
    }

    void WaylandHandler::resizeWindow(TestApplicationSurfaceId surfaceId, uint32_t width, uint32_t height)
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        if (nullptr != window.surface)
        {
            wl_egl_window_resize(window.native, width, height, 0, 0);
        }
        window.width  = width;
        window.height = height;
    }

    void WaylandHandler::frameCallback(wl_callback* callback)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandHandler::frameCallback");

        for (auto i: m_windows)
        {
            TestWaylandWindow* window = i.value;
            if (window->frameCallback == callback)
            {
                window->frameCallback = nullptr;
            }
        }

        wl_callback_destroy(callback);

    }

    void WaylandHandler::FrameCallback(void* userData, wl_callback* callback, uint32_t)
    {
        WaylandHandler* waylandHandler = static_cast<WaylandHandler*>(userData);
        waylandHandler->frameCallback(callback);
    }

    void WaylandHandler::wayland_output_handle_geometry(void* data, wl_output*, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char*, const char*, int32_t transform)
    {
        WaylandHandler* waylandHandler = static_cast<WaylandHandler*>(data);

        if(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_DoneReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_geometry: geometry is received AFTER done callback!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        if(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_GeometryReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_geometry: geometry was already received!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags |= WaylandOutputTestParams::WaylandOutput_GeometryReceived;

        waylandHandler->m_waylandOutputParams.x = x;
        waylandHandler->m_waylandOutputParams.y = y;
        waylandHandler->m_waylandOutputParams.physicalWidth = physical_width;
        waylandHandler->m_waylandOutputParams.physicalHeight = physical_height;
        waylandHandler->m_waylandOutputParams.subpixel = subpixel;
        waylandHandler->m_waylandOutputParams.transform = transform;
    }

    void WaylandHandler::wayland_output_handle_mode(void* data, wl_output*, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
    {
        WaylandHandler* waylandHandler = static_cast<WaylandHandler*>(data);

        if(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_DoneReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_mode: mode is received AFTER done callback!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        if(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ModeReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_mode: mode was already received!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags |= WaylandOutputTestParams::WaylandOutput_ModeReceived;

        waylandHandler->m_waylandOutputParams.modeFlags = flags;
        waylandHandler->m_waylandOutputParams.width = width;
        waylandHandler->m_waylandOutputParams.height = height;
        waylandHandler->m_waylandOutputParams.refreshRate = refresh;
    }

    void WaylandHandler::wayland_output_handle_done(void *data, wl_output*)
    {
        WaylandHandler* waylandHandler = static_cast<WaylandHandler*>(data);

        if(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_DoneReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_done: done was already received!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        if(!(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_GeometryReceived))
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_done: geometry was NOT received!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        if(!(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ModeReceived))
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_done: mode was NOT received!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        if(!(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ScaleReceived))
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_done: scale was NOT received!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags |= WaylandOutputTestParams::WaylandOutput_DoneReceived;
    }

    void WaylandHandler::wayland_output_handle_scale(void* data, wl_output*, int32_t factor)
    {
        WaylandHandler* waylandHandler = static_cast<WaylandHandler*>(data);

        if(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_DoneReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_scale: scale is received AFTER done callback!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        if(waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ScaleReceived)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::wayland_output_handle_scale: scale was already received!");
            waylandHandler->m_errorFoundInWaylandOutput = true;
        }

        waylandHandler->m_waylandOutputParams.m_waylandOutputReceivedFlags |= WaylandOutputTestParams::WaylandOutput_ScaleReceived;

        waylandHandler->m_waylandOutputParams.factor = factor;
    }

    bool WaylandHandler::getUseEGL(TestApplicationSurfaceId surfaceId) const
    {
        TestWaylandWindow& window = getWindow(surfaceId);
        return window.useEGL;
    }

    void WaylandHandler::getWindowSize(TestApplicationSurfaceId surfaceId, uint32_t& width, uint32_t& height) const
    {
        TestWaylandWindow window = getWindow(surfaceId);
        width = window.width;
        height = window.height;
    }

    SHMBuffer* WaylandHandler::getFreeSHMBuffer(uint32_t width, uint32_t height)
    {
        for(const auto shBuffer : m_shmBuffer)
        {
            if (shBuffer->isFree() && shBuffer->getWidth() == width && shBuffer->getHeight() == height)
            {
                return shBuffer;
            }
        }

        SHMBuffer* buffer = new SHMBuffer(wayland.shm, width, height, m_shmBuffer.size());
        m_shmBuffer.push_back(buffer);
        return buffer;
    }

    void WaylandHandler::deleteSHMBuffers()
    {
        for(const auto shBuffer : m_shmBuffer)
        {
            delete shBuffer;
        }
        m_shmBuffer.clear();
        if (wayland.display)
        {
            wl_display_flush(wayland.display);
        }
    }

    void WaylandHandler::addWaylandOutputListener()
    {
        if(!wayland.output)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::addWaylandOutputListener: wl_output is not registered (yet)!");
            abort();
        }

        switch(m_requiredWaylandOutputVersion)
        {
        case 1:
            wl_output_add_listener(wayland.output, &m_waylandOutputListenerV1, this);
            break;
        case 2:
            wl_output_add_listener(wayland.output, &m_waylandOutputListenerV2, this);
            break;
        case 3:
            wl_output_add_listener(wayland.output, &m_waylandOutputListenerV3, this);
            break;
        default:
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::addWaylandOutputListener: unsupported protocol version :" << m_requiredWaylandOutputVersion);
            abort();
        }
    }

    uint32_t WaylandHandler::getNumberOfAllocatedSHMBuffer() const
    {
        return static_cast<uint32_t>(m_shmBuffer.size());
    }

    bool WaylandHandler::getIsSHMBufferFree(uint32_t buffer) const
    {
        if (buffer < getNumberOfAllocatedSHMBuffer())
        {
            return m_shmBuffer[buffer]->isFree();
        }
        else
        {
            return false;
        }
    }

    void WaylandHandler::getWaylandOutputTestParams(bool& errorsFound, WaylandOutputTestParams& waylandOutputParams) const
    {
        errorsFound = m_errorFoundInWaylandOutput;
        waylandOutputParams = m_waylandOutputParams;
    }

    TestWaylandWindow& WaylandHandler::getWindow(TestApplicationSurfaceId surfaceId) const
    {
        auto i = m_windows.find(surfaceId);
        if (i != m_windows.end())
        {
            return *i->value;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::getWindow Window with id: " << surfaceId.getValue() << " not found !");
            assert(false);
            static TestWaylandWindow dummyWindow;
            return dummyWindow;
        }
    }

    wl_shell_surface& WaylandHandler::getShellSurface(TestApplicationShellSurfaceId shellSurfaceId) const
    {
        auto i = m_shellSurfaces.find(shellSurfaceId);
        if (i != m_shellSurfaces.end())
        {
            return *(i->value);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandHandler::getShellSurface Shell surface with id: " << shellSurfaceId.getValue() << " not found !");
            assert(false);

        }
        wl_shell_surface* nullShellSurface(nullptr);
        return static_cast<wl_shell_surface&>(*nullShellSurface);
    }


    void WaylandHandler::setShellSurfaceTitle(TestApplicationShellSurfaceId shellSurfaceId, const String& title)
    {
        wl_shell_surface& shellSurface = getShellSurface(shellSurfaceId);
        wl_shell_surface_set_title(&shellSurface, title.c_str());
    }

    void WaylandHandler::setShellSurfaceDummyValues(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId)
    {
        wl_shell_surface& shellSurface = getShellSurface(shellSurfaceId);
        TestWaylandWindow& window = getWindow(surfaceId);

        // Additional calls just to improve test code coverage in EC, the implementation of this functions in the EC is empty.
        wl_shell_surface_set_toplevel(&shellSurface);
        wl_shell_surface_pong(&shellSurface, 0);
        wl_shell_surface_set_transient(&shellSurface, window.surface, 0, 0, 0);
        wl_shell_surface_set_class(&shellSurface, "ClassName");
    }

    void WaylandHandler::destroyShellSurface(TestApplicationShellSurfaceId shellSurfaceId)
    {
        wl_shell_surface& shellSurface = getShellSurface(shellSurfaceId);
        wl_shell_surface_destroy(&shellSurface);
        m_shellSurfaces.remove(shellSurfaceId);
        wl_display_flush(wayland.display);
    }
}
