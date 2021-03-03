//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Wayland/WindowEventsPollingManager_Wayland.h"
#include "Window_Wayland/Window_Wayland.h"
#include "Utils/ThreadLocalLogForced.h"
#include <cerrno>

namespace ramses_internal
{
    WindowEventsPollingManager_Wayland::WindowEventsPollingManager_Wayland(std::chrono::microseconds maxFrameCallbackTime)
        : m_maxFrameCallbackTime(maxFrameCallbackTime)
    {
    }

    WindowEventsPollingManager_Wayland::~WindowEventsPollingManager_Wayland()
    {
        assert(m_windows.empty());
    }

    void WindowEventsPollingManager_Wayland::addWindow(Window_Wayland *window)
    {
        assert(m_windows.end() == std::find(m_windows.begin(), m_windows.end(), window));
        m_windows.emplace_back(window);

        resetFileDescriptors();
    }

    void WindowEventsPollingManager_Wayland::removeWindow(const Window_Wayland *window)
    {
        auto iter = std::find(m_windows.begin(), m_windows.end(), window);
        assert(m_windows.end() != iter);
        m_windows.erase(iter);

        resetFileDescriptors();
    }

    void WindowEventsPollingManager_Wayland::pollWindowsTillAnyCanRender() const
    {
        if(m_fileDescriptors.empty())
            return;

        const auto startTime = std::chrono::steady_clock::now();
        std::chrono::microseconds elapsedTime{ 0u };

        //wayland events must be dispatched before checking for windows that can render
        //otherwise it is possible to starve some displays
        pollAndDispatchAvailableWindowsEvents(std::chrono::milliseconds{0});

        while(!canAnyWindowRenderNewFrame() && elapsedTime <= m_maxFrameCallbackTime)
        {
            const auto pollTime = m_maxFrameCallbackTime - elapsedTime;
            pollAndDispatchAvailableWindowsEvents(std::chrono::duration_cast<std::chrono::milliseconds>(pollTime));

            const auto currentTime = std::chrono::steady_clock::now();
            elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime);
        }

        resetFileDescriptors();
    }

    void WindowEventsPollingManager_Wayland::resetFileDescriptors() const
    {
        m_fileDescriptors.clear();
        for(const auto& w : m_windows)
        {
            const auto windowDisplayFD = wl_display_get_fd(w->getNativeDisplayHandle());
            m_fileDescriptors.push_back(pollfd{windowDisplayFD, POLLIN,0});
        }
    }

    void WindowEventsPollingManager_Wayland::pollAndDispatchAvailableWindowsEvents(std::chrono::milliseconds pollTime) const
    {
        if (poll(m_fileDescriptors.data(), m_fileDescriptors.size(), pollTime.count()) == -1)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WindowEventsPollingManager_Wayland::pollAndDispatchAvailableWindowsEvents: error in poll :" << std::strerror(errno));
            assert(false);
            return;
        }

        for(size_t i = 0u; i < m_fileDescriptors.size(); ++i)
        {
            const Window_Wayland& window = *m_windows[i];

            //calling poll() sets the .revents field of every pollfd struct to a bitmask of
            //the received events on that fd
            const Bool dispatchNewEventsFromDisplayFD = (m_fileDescriptors[i].revents & POLLIN) != 0;
            window.dispatchWaylandDisplayEvents(dispatchNewEventsFromDisplayFD);
        }
    }

    bool WindowEventsPollingManager_Wayland::canAnyWindowRenderNewFrame() const
    {
        for(const auto window : m_windows)
            if(window->canRenderNewFrame())
                return true;
        return false;
    }
}
