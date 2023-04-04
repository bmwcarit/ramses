//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicEngineReport.h"
#include <chrono>
#include <algorithm>
#include <vector>

namespace rlogic
{
    class UpdateReportSummary
    {
    public:
        template<typename T>
        struct StatisticEntry
        {
            T minValue = std::numeric_limits<T>::max();
            T maxValue = std::numeric_limits<T>::min();
            T sum = T();

            void add(const T& value)
            {
                sum += value;
                minValue = std::min(minValue, value);
                maxValue = std::max(maxValue, value);
            }

            void reset()
            {
                minValue = std::numeric_limits<T>::max();
                maxValue = std::numeric_limits<T>::min();
                sum = T();
            }
        };

        template<typename T>
        struct Summary
        {
            Summary()
                : minValue()
                , maxValue()
                , average()
            {
            }

            Summary(const StatisticEntry<T>& stat, size_t count)
                : minValue(stat.minValue)
                , maxValue(stat.maxValue)
                , average((count != 0u) ? T(stat.sum / count) : T())
            {
            }

            T minValue;
            T maxValue;
            T average;
        };

        [[nodiscard]] const auto& getTotalTime() const
        {
            return m_totalTimeSummary;
        }

        [[nodiscard]] const auto& getSortTime() const
        {
            return m_sortTimeSummary;
        }

        [[nodiscard]] const auto& getLinkActivations() const
        {
            return m_linkActivationsSummary;
        }

        [[nodiscard]] const auto& getNodesExecuted() const
        {
            return m_nodesExecuted;
        }

        [[nodiscard]] const auto& getNodesSkippedExecution() const
        {
            return m_nodesSkipped;
        }

        void add(rlogic::LogicEngineReport&& report)
        {
            m_sortTime.add(report.getTopologySortExecutionTime());
            m_linkActivations.add(report.getTotalLinkActivations());
            const auto totalTime = report.getTotalUpdateExecutionTime();
            if (totalTime > m_totalTime.maxValue)
            {
                m_report = std::move(report);
            }
            m_totalTime.add(totalTime);
            ++m_measureCount;
            if (m_measureCount == m_measureInterval)
            {
                applySummary();
            }
        }

        void setInterval(size_t interval)
        {
            if (interval >= 1u)
            {
                if (interval <= m_measureCount)
                {
                    applySummary();
                }
                m_measureInterval = interval;
            }
        }

        [[nodiscard]] size_t getInterval() const
        {
            return m_measureInterval;
        }

    private:
        void applySummary()
        {
            m_nodesExecuted = m_report.getNodesExecuted();
            std::sort(m_nodesExecuted.begin(), m_nodesExecuted.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
            m_nodesSkipped     = m_report.getNodesSkippedExecution();
            m_totalTimeSummary = {m_totalTime, m_measureCount};
            m_sortTimeSummary  = {m_sortTime, m_measureCount};
            m_linkActivationsSummary = {m_linkActivations, m_measureCount};
            m_totalTime.reset();
            m_sortTime.reset();
            m_linkActivations.reset();
            m_measureCount = 0;
        }

        rlogic::LogicEngineReport                 m_report;
        StatisticEntry<std::chrono::microseconds> m_totalTime;
        StatisticEntry<std::chrono::microseconds> m_sortTime;
        StatisticEntry<size_t>                    m_linkActivations;
        Summary<std::chrono::microseconds>        m_totalTimeSummary;
        Summary<std::chrono::microseconds>        m_sortTimeSummary;
        Summary<size_t>                           m_linkActivationsSummary;
        std::vector<LogicEngineReport::LogicNodeTimed> m_nodesExecuted;
        std::vector<LogicNode*>                        m_nodesSkipped;
        size_t m_measureInterval = 60u;
        size_t m_measureCount = 0u;
    };
}

