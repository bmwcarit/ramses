//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCONFIG_H
#define RAMSES_RENDERERCONFIG_H

#include "RendererAPI/Types.h"
#include "Collections/String.h"
#include <chrono>

namespace ramses_internal
{
    class RendererConfig
    {
    public:
        RendererConfig() {}

        const String& getWaylandSocketEmbedded() const;
        const String& getWaylandSocketEmbeddedGroup() const;
        int getWaylandSocketEmbeddedFD() const;
        void setWaylandEmbeddedCompositingSocketName(const String& socket);
        void setWaylandEmbeddedCompositingSocketGroup(const String& groupNameForSocketPermissions);
        void setWaylandEmbeddedCompositingSocketFD(int fd);
        void setWaylandDisplayForSystemCompositorController(const String& wd);
        const String& getWaylandDisplayForSystemCompositorController() const;
        bool setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        uint32_t getWaylandSocketEmbeddedPermissions() const;

        void enableSystemCompositorControl();
        Bool getSystemCompositorControlEnabled() const;

        const String& getKPIFileName() const;
        void setKPIFileName(const String& filename);

        std::chrono::microseconds getFrameCallbackMaxPollTime() const;
        void setFrameCallbackMaxPollTime(std::chrono::microseconds pollTime);
        void setRenderthreadLooptimingReportingPeriod(std::chrono::milliseconds period);
        std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;
    private:
        String m_waylandSocketEmbedded;
        String m_waylandSocketEmbeddedGroupName;
        uint32_t m_waylandSocketEmbeddedPermissions = 0;
        int m_waylandSocketEmbeddedFD = -1;
        String m_waylandDisplayForSystemCompositorController;
        Bool m_systemCompositorEnabled = false;
        String m_kpiFilename;
        std::chrono::microseconds m_frameCallbackMaxPollTime{10000u};
        std::chrono::milliseconds m_renderThreadLoopTimingReportingPeriod { 0 }; // zero deactivates reporting
    };
}

#endif
