//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Wayland/Window_Wayland.h"
#include "Utils/ThreadLocalLogForced.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "RendererLib/DisplayConfig.h"
#include <poll.h>

namespace ramses_internal
{
    Window_Wayland::Window_Wayland(const DisplayConfig& displayConfig,
                                   IWindowEventHandler& windowEventHandler,
                                   uint32_t               id,
                                   std::chrono::microseconds frameCallbackMaxPollTime)
        : Window_Base(displayConfig, windowEventHandler, id)
        , m_waylandDisplay(displayConfig.getWaylandDisplay())
        , m_inputHandling(windowEventHandler)
        , m_frameCallbackMaxPollTime(frameCallbackMaxPollTime)
    {
    }

    bool Window_Wayland::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_Wayland::init Opening Wayland window");

        WaylandEnvironmentUtils::LogEnvironmentState(m_waylandDisplay);
        m_wlContext.display = wl_display_connect(m_waylandDisplay.empty()? nullptr : m_waylandDisplay.c_str());
        if (!m_wlContext.display)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::init Could not connect to system compositor (compositor running and or correct socket set?)");
            return false;
        }

        m_wlContext.displayFD = wl_display_get_fd(m_wlContext.display);

        m_wlContext.registry = wl_display_get_registry(m_wlContext.display);

        if (0 != wl_registry_add_listener(m_wlContext.registry, &m_registryListener, this))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::init Error creating wayland registry listener");
            return false;
        }

        wl_display_dispatch(m_wlContext.display);

        // make sure all pending requests are processed
        // and compositor is created (see callbacks above)
        wl_display_roundtrip(m_wlContext.display);

        // Creates the Wayland window
        m_wlContext.surface = wl_compositor_create_surface(m_wlContext.compositor);
        if (!m_wlContext.surface)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::init Error creating wayland surface");
            return false;
        }

        m_wlContext.native_window = wl_egl_window_create(m_wlContext.surface, m_width, m_height);
        if (!m_wlContext.native_window)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::init Error: wl_egl_window_create() failed");
            return false;
        }

        if (!createSurface())
        {
            return false;
        }

        LOG_TRACE(CONTEXT_RENDERER, "Window_Wayland::init Flushing wayland display");
        wl_display_flush(m_wlContext.display);
        LOG_TRACE(CONTEXT_RENDERER, "Window_Wayland::init Flushed wayland display");
        wl_display_roundtrip(m_wlContext.display);

        registerFrameRenderingDoneCallback();

        return true;
    }

    Window_Wayland::~Window_Wayland()
    {
        m_inputHandling.deinit();

        if (m_wlContext.frameRenderingDoneWaylandCallbacObject)
        {
            wl_callback_destroy(m_wlContext.frameRenderingDoneWaylandCallbacObject);
        }

        if (m_wlContext.native_window)
        {
            wl_egl_window_destroy(m_wlContext.native_window);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::~Window_Wayland Failed destroying native egl window");
        }

        if (m_wlContext.surface)
        {
            wl_surface_destroy(m_wlContext.surface);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::~Window_Wayland Failed destroying wayland surface");
        }

        if (m_wlContext.compositor)
        {
            wl_compositor_destroy(m_wlContext.compositor);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::~Window_Wayland Failed destroying compositor object");
        }

        if (m_wlContext.registry)
        {
            wl_registry_destroy(m_wlContext.registry);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::~Window_Wayland Failed destroying registry object");
        }

        if (m_wlContext.display)
        {
            wl_display_disconnect(m_wlContext.display);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::~Window_Wayland Failed disconnecting from display");
        }
    }

    void Window_Wayland::handleEvents()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Window_Wayland::handleEvents Updating Wayland window");
        dispatchWaylandDisplayEvents(std::chrono::milliseconds{ 0u });
    }

    void Window_Wayland::frameRendered()
    {
        assert(m_wlContext.previousFrameRenderingDone);
        m_wlContext.previousFrameRenderingDone = false;
    }

    bool Window_Wayland::canRenderNewFrame() const
    {
        const auto startTime = std::chrono::steady_clock::now();
        std::chrono::microseconds elapsedTime{ 0u };

        while(!m_wlContext.previousFrameRenderingDone && elapsedTime < m_frameCallbackMaxPollTime)
        {
            const auto pollTime = m_frameCallbackMaxPollTime - elapsedTime;
            dispatchWaylandDisplayEvents(std::chrono::duration_cast<std::chrono::milliseconds>(pollTime));

            const auto currentTime = std::chrono::steady_clock::now();
            elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime);
        }

        return m_wlContext.previousFrameRenderingDone;
    }

    wl_display* Window_Wayland::getNativeDisplayHandle() const
    {
        return m_wlContext.display;
    }

    wl_egl_window* Window_Wayland::getNativeWindowHandle() const
    {
        return m_wlContext.native_window;
    }

    void Window_Wayland::RegistryGlobalCreated(void* data, wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version)
    {
        UNUSED(version);
        Window_Wayland& window = *static_cast<Window_Wayland*>(data);
        window.registryGlobalCreated(wl_registry, name, interface, version);
    }

    void Window_Wayland::RegistryGlobalRemoved(void* data, wl_registry* wl_registry, uint32_t name)
    {
        UNUSED(data);
        UNUSED(wl_registry);
        UNUSED(name);
        //callback to destroy globals not implemented in neither winston or weston, globals are destroyed in destructor.
    }

    void Window_Wayland::FrameRenderingDoneCallback(void* userData, wl_callback* callbackWaylandObject, uint32_t)
    {
        Window_Wayland* window = static_cast<Window_Wayland*>(userData);
        //assert that it is a "done" event for the last registered callback
        assert(callbackWaylandObject == window->m_wlContext.frameRenderingDoneWaylandCallbacObject);

        wl_callback_destroy(callbackWaylandObject);
        window->m_wlContext.frameRenderingDoneWaylandCallbacObject = nullptr;
        window->m_wlContext.previousFrameRenderingDone = true;

        window->registerFrameRenderingDoneCallback();
    }

    void Window_Wayland::registerFrameRenderingDoneCallback()
    {
        assert(m_wlContext.previousFrameRenderingDone);
        assert(nullptr == m_wlContext.frameRenderingDoneWaylandCallbacObject);
        assert(m_wlContext.compositor);

        m_wlContext.frameRenderingDoneWaylandCallbacObject = wl_surface_frame(m_wlContext.surface);
        wl_callback_add_listener(m_wlContext.frameRenderingDoneWaylandCallbacObject, &m_frameRenderingDoneCallbackListener, this);
    }

    bool Window_Wayland::setFullscreen(bool fullscreen)
    {
        UNUSED(fullscreen);
        return true;
    }

    void Window_Wayland::dispatchWaylandDisplayEvents(std::chrono::milliseconds pollTime) const
    {
        // dispatch enqueued events, this does not read the socket so we could still miss some events here...
        wl_display_dispatch_pending(m_wlContext.display);


        pollfd pollFileDescriptor = {m_wlContext.displayFD, POLLIN, 0};
        if (poll(&pollFileDescriptor, 1u, static_cast<int>(pollTime.count())) == -1)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Window_Wayland::dispatchWaylandDisplayEvents: error in poll :" << std::strerror(errno));
            assert(false);
            return;
        }

        //calling poll() sets the .revents field of every pollfd struct to a bitmask of
        //the received events on that fd
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const bool dispatchNewEventsFromDisplayFD = (pollFileDescriptor.revents & POLLIN) != 0;
        if(dispatchNewEventsFromDisplayFD)
            wl_display_dispatch(m_wlContext.display);
    }

    void Window_Wayland::registryGlobalCreated(wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version)
    {
        UNUSED(version);
        if (0 == strcmp(interface, "wl_compositor"))
        {
            m_wlContext.compositor =
                reinterpret_cast<wl_compositor*>(wl_registry_bind(wl_registry, name, &wl_compositor_interface, 1));
            LOG_DEBUG(CONTEXT_RENDERER, "Window_Wayland::registryGlobalCreated Bound wl_compositor");
        }

        if (0 == strcmp(interface, "wl_seat"))
        {
            m_inputHandling.registerSeat(wl_registry, name);
        }
    }
}
