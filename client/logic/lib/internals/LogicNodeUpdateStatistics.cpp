//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicNode.h"
#include "internals/UpdateReport.h"
#include "internals/LogicNodeUpdateStatistics.h"
#include "impl/LoggerImpl.h"

namespace rlogic::internal
{
    void LogicNodeUpdateStatistics::clear()
    {
        m_updateExecutionTime.clear();
        m_timeSinceLastUpdate.clear();
        m_nodesExecuted.clear();
        m_activatedLinks.clear();
        m_currentStatisticsFrame = 0u;
        m_totalNodesCount = 0u;
        m_lastTimeUpdateDataAdded = std::nullopt;
    }

    void LogicNodeUpdateStatistics::setLogLevel(ELogMessageType logLevel)
    {
        m_logLevel = logLevel;
    }

    void LogicNodeUpdateStatistics::collect(const UpdateReport& report, size_t totalNodesCount)
    {
        m_totalNodesCount = totalNodesCount;

        collectTimeSinceLastUpdate();
        m_updateExecutionTime.add(report.getSectionExecutionTime(UpdateReport::ETimingSection::TotalUpdate).count());
        m_nodesExecuted.add(static_cast<int64_t>(m_nodesExecutedCurrentUpdate));
        m_nodesExecutedCurrentUpdate = 0u;
        m_activatedLinks.add(static_cast<int64_t>(report.getLinkActivations()));

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
            log("First Statistics Log");
        }
        else
        {
            const auto timeSinceLastLog = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastTimeLogged.value());
            log("Time since last log: {:.2f} sec", static_cast<float>(timeSinceLastLog.count()) / 1000.f);
        }
        m_lastTimeLogged = now;
    }

    void LogicNodeUpdateStatistics::logUpdateExecutionTime()
    {
        log("Update Execution time (min/max/avg): {}/{}/{} [u]sec",
            m_updateExecutionTime.min,
            m_updateExecutionTime.max,
            m_updateExecutionTime.acc / m_currentStatisticsFrame);
    }

    void LogicNodeUpdateStatistics::logTimeBetweenUpdates()
    {
        if (m_currentStatisticsFrame == 1)
        {
            log("Time between Update calls cannot be measured with loggingRate = 1");
        }
        else
        {
            log("Time between Update calls (min/max/avg): {:.2f}/{:.2f}/{:.2f} [m]sec",
                static_cast<float>(m_timeSinceLastUpdate.min) / 1000.f,
                static_cast<float>(m_timeSinceLastUpdate.max) / 1000.f,
                (static_cast<float>(m_timeSinceLastUpdate.acc) / static_cast<float>(m_currentStatisticsFrame - 1)) / 1000.f);
        }
    }

    void LogicNodeUpdateStatistics::logNodesExecuted()
    {
        const size_t totalNodesNotZero = std::max(size_t(1), m_totalNodesCount);
        log("Nodes Executed (min/max/avg): {}%/{}%/{}% of {} nodes total",
            (m_nodesExecuted.min / totalNodesNotZero) * 100,
            (m_nodesExecuted.max / totalNodesNotZero) * 100,
            ((m_nodesExecuted.acc / m_currentStatisticsFrame) / totalNodesNotZero) * 100,
            m_totalNodesCount);
    }

    void LogicNodeUpdateStatistics::logActivatedLinks()
    {
        log("Activated links (min/max/avg): {}/{}/{}",
            m_activatedLinks.min,
            m_activatedLinks.max,
            m_activatedLinks.acc / m_currentStatisticsFrame);
    }

    void LogicNodeUpdateStatistics::calculateAndLog()
    {
        assert(m_currentStatisticsFrame != 0u);

        logTimeSinceLastLog();

        logUpdateExecutionTime();

        logTimeBetweenUpdates();

        logNodesExecuted();

        logActivatedLinks();

        clear();
    }
}
