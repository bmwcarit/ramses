//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    using Clock     = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    class UpdateReport;

    class LogicNodeUpdateStatistics
    {
        public:
            LogicNodeUpdateStatistics();
            void nodeExecuted();
            void collect(const UpdateReport& report, size_t totalNodesCount);
            void calculateAndLog();
            void setLoggingRate(size_t loggingRate);
            [[nodiscard]] bool checkUpdateFrameFinished() const;

        private:
            using LogicNodeTimed = UpdateReport::LogicNodeTimed;

            struct StatisticProperty
            {
                int64_t min = std::numeric_limits<int64_t>::max();
                int64_t max = 0;
                int64_t acc = 0;

                void add(int64_t value)
                {
                    min = std::min(min, value);
                    max = std::max(max, value);
                    acc += value;
                }

                void clear()
                {
                    min = std::numeric_limits<int64_t>::max();
                    max = 0;
                    acc = 0;
                }
            };

            void clear();
            void collectTimeSinceLastUpdate();

            void logTimeSinceLastLog();
            void logUpdateExecutionTime();
            void logTimeBetweenUpdates();
            void logNodesExecuted();
            void logActivatedLinks();
            void logSlowestNodes();

            size_t m_loggingRate                = 60u;
            size_t m_currentStatisticsFrame     =  0u;
            size_t m_nodesExecutedCurrentUpdate =  0u;
            size_t m_totalNodesCount            =  0u;
            std::optional<TimePoint> m_lastTimeLogged = std::nullopt;
            std::optional<TimePoint> m_lastTimeUpdateDataAdded = std::nullopt;

            StatisticProperty m_timeSinceLastUpdate;
            StatisticProperty m_updateExecutionTime;
            StatisticProperty m_nodesExecuted;
            StatisticProperty m_activatedLinks;

            std::array<LogicNodeTimed, 5> m_slowestNodes;
    };
}
