//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_PROGRESSMONITOR_H
#define RAMSES_SCENE_VIEWER_PROGRESSMONITOR_H

#include <future>
#include <atomic>
#include <vector>
#include <string>

namespace ramses_internal
{
    class ProgressMonitor
    {
    public:
        using FutureList = std::vector<std::future<std::vector<std::string>>>;

        bool isRunning() const
        {
            for (auto& f : m_futures)
            {
                if (f.valid() && f.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                    return true;
            }
            return false;
        }

        void start(FutureList&& futures, uint32_t total, const std::string& description)
        {
            canceled      = false;
            current       = 0;
            m_total       = total;
            m_description = description;
            m_futures     = std::move(futures);
        }

        const std::string& getDescription() const
        {
            return m_description;
        }

        uint32_t getTotal() const
        {
            return m_total;
        }

        std::vector<std::string> getResult()
        {
            std::vector<std::string> result;
            for (auto& f : m_futures)
            {
                if (f.valid())
                {
                    auto v = f.get();
                    result.insert(result.end(), v.begin(), v.end());
                }
            }
            return result;
        }

        void stop()
        {
            m_futures.clear();
        }

        std::atomic<bool>     canceled;
        std::atomic<uint32_t> current;

    private:
        uint32_t    m_total = 0;
        std::string m_description;
        FutureList  m_futures;
    };
} // ramses_internal

#endif
