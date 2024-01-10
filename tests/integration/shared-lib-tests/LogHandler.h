//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include <mutex>
#include <list>
#include <string>

namespace ramses::internal
{
    class LogHandler
    {
    public:
        void add(ELogLevel level, std::string_view context, std::string_view msg)
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_log.push_back({level, std::string(context), std::string(msg)});
        }

        void clear()
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_log.clear();
        }

        [[nodiscard]] size_t find(const std::string& token)
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            size_t                      count = 0u;
            for (const auto& entry : m_log)
            {
                if (entry.msg.find(token) != std::string::npos)
                {
                    ++count;
                }
            }
            return count;
        }

        [[nodiscard]] bool empty() const
        {
            return m_log.empty();
        }

    private:
        struct LogEntry
        {
            ELogLevel level;
            std::string       context;
            std::string       msg;
        };
        std::mutex          m_mutex;
        std::list<LogEntry> m_log;
    };
}
