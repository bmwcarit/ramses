//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandClientCredentials.h"
#include <unistd.h>
#include <string>

namespace ramses_internal
{
    WaylandClientCredentials::WaylandClientCredentials(
            pid_t processId,
            uid_t userId,
            gid_t groupId)
        : m_processId(processId)
        , m_userId(userId)
        , m_groupId(groupId)
    {
    }

    pid_t WaylandClientCredentials::getProcessId() const
    {
        return m_processId;
    }

    uid_t WaylandClientCredentials::getUserId() const
    {
        return m_userId;
    }

    gid_t WaylandClientCredentials::getGroupId() const
    {
        return m_groupId;
    }
}
