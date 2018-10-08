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
        void setWaylandSocketEmbedded(const String& socket);
        void setWaylandSocketEmbeddedGroup(const String& groupNameForSocketPermissions);
        void setWaylandSocketEmbeddedFD(int fd);

        const String& getKPIFileName() const;
        void setKPIFileName(const String& filename);

        void enableSystemCompositorControl();
        Bool getSystemCompositorControlEnabled() const;

        std::chrono::microseconds getFrameCallbackMaxPollTime() const;
        void setFrameCallbackMaxPollTime(std::chrono::microseconds pollTime);

    private:
        String m_waylandSocketEmbedded;
        String m_waylandSocketEmbeddedGroupName;
        int m_waylandSocketEmbeddedFD = -1;
        String m_kpiFilename;
        Bool m_systemCompositorEnabled = false;
        std::chrono::microseconds m_frameCallbackMaxPollTime{10000u};
    };
}

#endif
