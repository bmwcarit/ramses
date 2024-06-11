//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererConfigData.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"

namespace ramses::internal
{
    void RendererConfigData::enableSystemCompositorControl()
    {
        m_systemCompositorEnabled = true;
    }

    bool RendererConfigData::getSystemCompositorControlEnabled() const
    {
        return m_systemCompositorEnabled;
    }

    std::chrono::microseconds RendererConfigData::getFrameCallbackMaxPollTime() const
    {
        return m_frameCallbackMaxPollTime;
    }

    void RendererConfigData::setFrameCallbackMaxPollTime(std::chrono::microseconds pollTime)
    {
        m_frameCallbackMaxPollTime = pollTime;
    }

    void RendererConfigData::setWaylandDisplayForSystemCompositorController(std::string_view wd)
    {
        m_waylandDisplayForSystemCompositorController = wd;
    }

    std::string_view RendererConfigData::getWaylandDisplayForSystemCompositorController() const
    {
        return m_waylandDisplayForSystemCompositorController;
    }

    void RendererConfigData::setRenderthreadLooptimingReportingPeriod(std::chrono::milliseconds period)
    {
        m_renderThreadLoopTimingReportingPeriod = period;
    }

    std::chrono::milliseconds RendererConfigData::getRenderThreadLoopTimingReportingPeriod() const
    {
        return m_renderThreadLoopTimingReportingPeriod;
    }
}
