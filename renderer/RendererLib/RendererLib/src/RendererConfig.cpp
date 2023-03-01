//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererConfig.h"
#include "Collections/StringOutputStream.h"
#include "CLI/CLI.hpp"

namespace ramses_internal
{
    void RendererConfig::registerOptions(CLI::App& cli)
    {
        auto* grp = cli.add_option_group("Renderer Options");
        grp->add_flag("--ivi-control,!--no-ivi-control", m_systemCompositorEnabled, "enable system compositor IVI controller");
    }

    void RendererConfig::enableSystemCompositorControl()
    {
        m_systemCompositorEnabled = true;
    }

    Bool RendererConfig::getSystemCompositorControlEnabled() const
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

    void RendererConfig::setWaylandDisplayForSystemCompositorController(const String& wd)
    {
        m_waylandDisplayForSystemCompositorController = wd;
    }

    const String &RendererConfig::getWaylandDisplayForSystemCompositorController() const
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
