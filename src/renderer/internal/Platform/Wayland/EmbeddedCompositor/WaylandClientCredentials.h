//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/FmtBase.h"
#include <unistd.h>

namespace ramses::internal
{
    class WaylandClientCredentials
    {
    public:
        WaylandClientCredentials() = default;
        WaylandClientCredentials(
                pid_t processId,
                uid_t userId,
                gid_t groupId);

        [[nodiscard]] pid_t getProcessId() const;
        [[nodiscard]] uid_t getUserId() const;
        [[nodiscard]] gid_t getGroupId() const;

    private:
        pid_t m_processId = -1;
        uid_t m_userId = 0u;
        gid_t m_groupId = 0u;
    };
}

template <>
struct fmt::formatter<ramses::internal::WaylandClientCredentials> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::internal::WaylandClientCredentials& cred, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[pid:{} uid:{} gid:{}]",
                              cred.getProcessId(),
                              cred.getUserId(),
                              cred.getGroupId());
    }
};
