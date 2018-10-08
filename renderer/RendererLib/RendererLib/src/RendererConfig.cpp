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
    void RendererConfig::setWaylandSocketEmbedded(const String& socket)
    {
        m_waylandSocketEmbedded = socket;
    }

    void RendererConfig::setWaylandSocketEmbeddedFD(int fd)
    {
        m_waylandSocketEmbeddedFD = fd;
    }

    const String& RendererConfig::getWaylandSocketEmbedded() const
    {
        return m_waylandSocketEmbedded;
    }

    const String& RendererConfig::getWaylandSocketEmbeddedGroup() const
    {
        return m_waylandSocketEmbeddedGroupName;
    }

    int RendererConfig::getWaylandSocketEmbeddedFD() const
    {
        return m_waylandSocketEmbeddedFD;
    }

    void RendererConfig::setWaylandSocketEmbeddedGroup(const String& groupNameForSocketPermissions)
    {
        m_waylandSocketEmbeddedGroupName = groupNameForSocketPermissions;
    }

    void RendererConfig::setKPIFileName(const String& filename)
    {
        m_kpiFilename = filename;
    }

    const String& RendererConfig::getKPIFileName() const
    {
        return m_kpiFilename;
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
}
