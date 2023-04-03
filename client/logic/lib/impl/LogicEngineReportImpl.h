//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicNode.h"
#include "internals/UpdateReport.h"

namespace rlogic::internal
{
    class ApiObjects;

    class LogicEngineReportImpl
    {
    public:
        using LogicNodesTimed = std::vector<std::pair<LogicNode*, UpdateReport::ReportTimeUnits>>;
        using LogicNodes = std::vector<LogicNode*>;

        LogicEngineReportImpl();
        explicit LogicEngineReportImpl(const UpdateReport& reportData, const ApiObjects& apiObjects);

        [[nodiscard]] const LogicNodesTimed& getNodesExecuted() const;
        [[nodiscard]] const LogicNodes& getNodesSkippedExecution() const;
        [[nodiscard]] std::chrono::microseconds getTopologySortExecutionTime() const;
        [[nodiscard]] std::chrono::microseconds getTotalUpdateExecutionTime() const;
        [[nodiscard]] size_t getTotalLinkActivations() const;

    private:
        LogicNodesTimed m_nodesExecuted;
        LogicNodes m_nodesSkippedExecution;
        UpdateReport::ReportTimeUnits m_totalUpdateExecutionTime{ 0 };
        UpdateReport::ReportTimeUnits m_topologySortExecutionTime{ 0 };
        size_t m_activatedLinks = 0u;
    };
}
