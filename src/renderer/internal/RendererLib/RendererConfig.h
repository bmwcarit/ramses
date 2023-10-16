//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"

#include <chrono>
#include <string>

namespace ramses::internal
{
    class RendererConfig
    {
    public:
        void setWaylandDisplayForSystemCompositorController(std::string_view wd);
        [[nodiscard]] std::string_view getWaylandDisplayForSystemCompositorController() const;

        void enableSystemCompositorControl();
        [[nodiscard]] bool getSystemCompositorControlEnabled() const;

        [[nodiscard]] std::chrono::microseconds getFrameCallbackMaxPollTime() const;
        void setFrameCallbackMaxPollTime(std::chrono::microseconds pollTime);
        void setRenderthreadLooptimingReportingPeriod(std::chrono::milliseconds period);
        [[nodiscard]] std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

    private:
        std::string m_waylandDisplayForSystemCompositorController;
        bool m_systemCompositorEnabled = false;
        std::chrono::microseconds m_frameCallbackMaxPollTime{10000u};
        std::chrono::milliseconds m_renderThreadLoopTimingReportingPeriod { 0 }; // zero deactivates reporting
    };
}
