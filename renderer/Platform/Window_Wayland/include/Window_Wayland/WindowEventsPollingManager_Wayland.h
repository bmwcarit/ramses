//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOWEVENTSPOLLINGMANAGER_WAYLAND_H
#define RAMSES_WINDOWEVENTSPOLLINGMANAGER_WAYLAND_H

#include "RendererAPI/IWindowEventsPollingManager.h"
#include <vector>
#include <chrono>
#include <poll.h>

namespace ramses_internal
{
    class Window_Wayland;

    class WindowEventsPollingManager_Wayland : public IWindowEventsPollingManager
    {
    public:
        explicit WindowEventsPollingManager_Wayland(std::chrono::microseconds maxFrameCallbackTime);
        virtual ~WindowEventsPollingManager_Wayland();

        virtual void pollWindowsTillAnyCanRender() const override final;
        void addWindow(Window_Wayland* window);
        void removeWindow(const Window_Wayland* window);

    private:
        void resetFileDescriptors() const;
        void pollAndDispatchAvailableWindowsEvents(std::chrono::milliseconds pollTime) const;
        bool canAnyWindowRenderNewFrame() const;

        std::vector<Window_Wayland*>    m_windows;
        mutable std::vector<pollfd>     m_fileDescriptors;
        const std::chrono::microseconds m_maxFrameCallbackTime;
    };
}

#endif
