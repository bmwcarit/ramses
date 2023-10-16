//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/logic/UpdateReport.h"
#include "internal/logic/LogicNodeUpdateStatistics.h"
#include "impl/RamsesObjectImpl.h"
#include "impl/logic/LogicNodeImpl.h"
#include <cassert>
#include <fmt/format.h>

namespace ramses::internal
{
    LogicNodeUpdateStatistics::LogicNodeUpdateStatistics()
    {
        m_slowestNodes.fill({ nullptr, std::chrono::microseconds(-1) });
    }

    void LogicNodeUpdateStatistics::clear()
    {
        m_updateExecutionTime.clear();
        m_timeSinceLastUpdate.clear();
        m_nodesExecuted.clear();
        m_activatedLinks.clear();
        m_currentStatisticsFrame = 0u;
        m_totalNodesCount = 0u;
        m_lastTimeUpdateDataAdded = std::nullopt;
        m_slowestNodes.fill({nullptr, std::chrono::microseconds(-1)});
    }

    void LogicNodeUpdateStatistics::collect(const UpdateReport& report, size_t totalNodesCount)
    {
        m_totalNodesCount = totalNodesCount;

        collectTimeSinceLastUpdate();
        m_updateExecutionTime.add(report.getSectionExecutionTime(UpdateReport::ETimingSection::TotalUpdate).count());
        m_nodesExecuted.add(static_cast<int64_t>(m_nodesExecutedCurrentUpdate));
        m_nodesExecutedCurrentUpdate = 0u;
        m_activatedLinks.add(static_cast<int64_t>(report.getLinkActivations()));

        auto isLongerTime = [](const LogicNodeTimed& a, const LogicNodeTimed& b) { return a.second > b.second; };
        for (auto& newNode : report.getNodesExecuted())
        {
            if (isLongerTime(newNode, m_slowestNodes.back()))
            {
                auto it = std::find_if(m_slowestNodes.begin(), m_slowestNodes.end(), [&newNode](const auto& slowNode) { return newNode.first == slowNode.first; });
                if (it == m_slowestNodes.end())
                {
                    m_slowestNodes.back() = newNode;
                }
                else
                {
                    it->second = std::max(newNode.second, it->second);
                }

                std::sort(m_slowestNodes.begin(), m_slowestNodes.end(), isLongerTime);
            }
        }

        m_currentStatisticsFrame++;
    }

    bool LogicNodeUpdateStatistics::checkUpdateFrameFinished() const
    {
        return m_currentStatisticsFrame == m_loggingRate;
    }

    void LogicNodeUpdateStatistics::nodeExecuted()
    {
        m_nodesExecutedCurrentUpdate++;
    }

    void LogicNodeUpdateStatistics::collectTimeSinceLastUpdate()
    {
        const auto now = Clock::now();
        if (m_lastTimeUpdateDataAdded.has_value())
        {
            const auto timeSinceLastFrame = std::chrono::duration_cast<std::chrono::microseconds>((now - m_lastTimeUpdateDataAdded.value())).count();
            m_timeSinceLastUpdate.add(timeSinceLastFrame);
        }
        m_lastTimeUpdateDataAdded = now;
    }

    void LogicNodeUpdateStatistics::setLoggingRate(size_t loggingRate)
    {
        m_loggingRate = loggingRate;
    }

    void LogicNodeUpdateStatistics::logTimeSinceLastLog()
    {
        const auto now = Clock::now();
        if (!m_lastTimeLogged.has_value())
        {
            LOG_INFO(CONTEXT_PERIODIC, "First Statistics Log");
        }
        else
        {
            const auto timeSinceLastLog = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastTimeLogged.value());
            LOG_INFO_P(CONTEXT_PERIODIC, "Time since last log: {:.2f} sec", static_cast<float>(timeSinceLastLog.count()) / 1000.f);
        }
        m_lastTimeLogged = now;
    }

    void LogicNodeUpdateStatistics::logUpdateExecutionTime()
    {
        LOG_INFO_P(CONTEXT_PERIODIC, "Update Execution time (min/max/avg): {}/{}/{} [u]sec",
            m_updateExecutionTime.min,
            m_updateExecutionTime.max,
            m_updateExecutionTime.acc / m_currentStatisticsFrame);
    }

    void LogicNodeUpdateStatistics::logTimeBetweenUpdates()
    {
        if (m_currentStatisticsFrame == 1)
        {
            LOG_INFO_P(CONTEXT_PERIODIC, "Time between Update calls cannot be measured with loggingRate = 1");
        }
        else
        {
            LOG_INFO_P(CONTEXT_PERIODIC, "Time between Update calls (min/max/avg): {:.2f}/{:.2f}/{:.2f} [m]sec",
                static_cast<float>(m_timeSinceLastUpdate.min) / 1000.f,
                static_cast<float>(m_timeSinceLastUpdate.max) / 1000.f,
                (static_cast<float>(m_timeSinceLastUpdate.acc) / static_cast<float>(m_currentStatisticsFrame - 1)) / 1000.f);
        }
    }

    void LogicNodeUpdateStatistics::logNodesExecuted()
    {
        const size_t totalNodesNotZero = std::max(size_t(1), m_totalNodesCount);
        LOG_INFO_P(CONTEXT_PERIODIC, "Nodes Executed (min/max/avg): {}%/{}%/{}% ({}/{}/{}) of {} nodes total",
            static_cast<size_t>(static_cast<float>(m_nodesExecuted.min) / totalNodesNotZero * 100.f),
            static_cast<size_t>(static_cast<float>(m_nodesExecuted.max) / totalNodesNotZero * 100.f),
            static_cast<size_t>(static_cast<float>(m_nodesExecuted.acc) / m_currentStatisticsFrame / totalNodesNotZero * 100.f),
            m_nodesExecuted.min,
            m_nodesExecuted.max,
            m_nodesExecuted.acc / m_currentStatisticsFrame,
            m_totalNodesCount);
    }

    void LogicNodeUpdateStatistics::logActivatedLinks()
    {
        LOG_INFO_P(CONTEXT_PERIODIC, "Activated links (min/max/avg): {}/{}/{}",
            m_activatedLinks.min,
            m_activatedLinks.max,
            m_activatedLinks.acc / m_currentStatisticsFrame);
    }

    void LogicNodeUpdateStatistics::logSlowestNodes()
    {
        if (m_slowestNodes[0].first == nullptr)
            return;

        std::string nodes;
        for (auto& node : m_slowestNodes)
        {
            if (node.first != nullptr)
                nodes += fmt::format(" [{}:{}]", node.first->getName(), node.second.count());
        }
        LOG_INFO_P(CONTEXT_PERIODIC, "Slowest nodes [name:time_us]:{}", nodes);
    }

    void LogicNodeUpdateStatistics::calculateAndLog()
    {
        assert(m_currentStatisticsFrame != 0u);

        logTimeSinceLastLog();

        logUpdateExecutionTime();

        logTimeBetweenUpdates();

        logNodesExecuted();

        logActivatedLinks();

        logSlowestNodes();

        clear();
    }
}
