//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <vector>
#include <chrono>
#include <optional>
#include <array>

namespace rlogic::internal
{
    class LogicNodeImpl;

    class UpdateReport
    {
    public:
        using ReportTimeUnits = std::chrono::microseconds;
        using LogicNodesTimed = std::vector<std::pair<LogicNodeImpl*, ReportTimeUnits>>;
        using LogicNodes = std::vector<LogicNodeImpl*>;

        enum class ETimingSection
        {
            TotalUpdate = 0,
            TopologySort
        };

        void sectionStarted(ETimingSection section);
        void sectionFinished(ETimingSection section);
        void nodeExecutionStarted(LogicNodeImpl& node);
        void nodeExecutionFinished();
        void nodeSkippedExecution(LogicNodeImpl& node);
        void linksActivated(size_t activatedLinks);
        void clear();

        [[nodiscard]] const LogicNodesTimed& getNodesExecuted() const;
        [[nodiscard]] const LogicNodes& getNodesSkippedExecution() const;
        [[nodiscard]] ReportTimeUnits getSectionExecutionTime(ETimingSection section) const;
        [[nodiscard]] size_t getLinkActivations() const;

    private:
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        LogicNodesTimed m_nodesExecuted;
        LogicNodes m_nodesSkippedExecution;
        std::array<ReportTimeUnits, 2u> m_sectionExecutionTime = { ReportTimeUnits{ 0 } };
        size_t m_activatedLinks {0u};

        std::optional<TimePoint> m_nodeExecutionStarted;
        std::array<std::optional<TimePoint>, 2u> m_sectionStarted;
    };

    inline void UpdateReport::linksActivated(size_t activatedLinks)
    {
        m_activatedLinks += activatedLinks;
    }
}
