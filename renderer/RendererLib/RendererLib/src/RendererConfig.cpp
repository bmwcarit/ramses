//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererConfig.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    void RendererConfig::enableSystemCompositorControl()
    {
        m_systemCompositorEnabled = true;
    }

    bool RendererConfig::getSystemCompositorControlEnabled() const
    {
        return m_systemCompositorEnabled;
    }

    std::chrono::microseconds RendererConfig::getFrameCallbackMaxPollTime() const
    {
        return m_frameCallbackMaxPollTime;
    }

    void RendererConfig::setFrameCallbackMaxPollTime(std::chrono::microseconds pollTime)
    {
        m_frameCallbackMaxPollTime = pollTime;
    }

    void RendererConfig::setWaylandDisplayForSystemCompositorController(std::string_view wd)
    {
        m_waylandDisplayForSystemCompositorController = wd;
    }

    std::string_view RendererConfig::getWaylandDisplayForSystemCompositorController() const
    {
        return m_waylandDisplayForSystemCompositorController;
    }

    void RendererConfig::setRenderthreadLooptimingReportingPeriod(std::chrono::milliseconds period)
    {
        m_renderThreadLoopTimingReportingPeriod = period;
    }

    std::chrono::milliseconds RendererConfig::getRenderThreadLoopTimingReportingPeriod() const
    {
        return m_renderThreadLoopTimingReportingPeriod;
    }
}
