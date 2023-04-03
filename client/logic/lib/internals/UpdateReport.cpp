//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/UpdateReport.h"
#include <cassert>

namespace rlogic::internal
{
    void UpdateReport::sectionStarted(ETimingSection section)
    {
        std::optional<TimePoint>& sectionTiming = m_sectionStarted[static_cast<size_t>(section)];
        assert(!sectionTiming);
        sectionTiming = Clock::now();
    }

    void UpdateReport::sectionFinished(ETimingSection section)
    {
        const auto sectionIdx = static_cast<size_t>(section);
        std::optional<TimePoint>& sectionTiming = m_sectionStarted[sectionIdx];
        assert(sectionTiming);
        m_sectionExecutionTime[sectionIdx] = std::chrono::duration_cast<ReportTimeUnits>(Clock::now() - *sectionTiming);
        sectionTiming.reset();
    }

    void UpdateReport::nodeExecutionStarted(LogicNodeImpl& node)
    {
        assert(!m_nodeExecutionStarted);
        m_nodeExecutionStarted = Clock::now();
        m_nodesExecuted.push_back({ &node, ReportTimeUnits{ 0u } });
    }

    void UpdateReport::nodeExecutionFinished()
    {
        assert(m_nodeExecutionStarted);
        m_nodesExecuted.back().second = std::chrono::duration_cast<ReportTimeUnits>(Clock::now() - *m_nodeExecutionStarted);
        m_nodeExecutionStarted.reset();
    }

    void UpdateReport::nodeSkippedExecution(LogicNodeImpl& node)
    {
        m_nodesSkippedExecution.push_back(&node);
    }

    void UpdateReport::clear()
    {
        m_nodesExecuted.clear();
        m_nodesSkippedExecution.clear();
        for (auto& s : m_sectionExecutionTime)
            s = ReportTimeUnits{ 0u };
        m_activatedLinks = 0u;

        // clear also internals in case update/measure was interrupted due to error
        m_nodeExecutionStarted.reset();
        for (auto& s : m_sectionStarted)
            s.reset();
    }

    const UpdateReport::LogicNodesTimed& UpdateReport::getNodesExecuted() const
    {
        return m_nodesExecuted;
    }

    const UpdateReport::LogicNodes& UpdateReport::getNodesSkippedExecution() const
    {
        return m_nodesSkippedExecution;
    }

    UpdateReport::ReportTimeUnits UpdateReport::getSectionExecutionTime(ETimingSection section) const
    {
        return m_sectionExecutionTime[static_cast<size_t>(section)];
    }

    size_t UpdateReport::getLinkActivations() const
    {
        return m_activatedLinks;
    }

}
