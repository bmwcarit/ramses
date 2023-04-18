//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/LogicEngineReportImpl.h"
#include "internals/ApiObjects.h"

namespace rlogic::internal
{
    LogicEngineReportImpl::LogicEngineReportImpl() = default;

    LogicEngineReportImpl::LogicEngineReportImpl(const UpdateReport& reportData, const ApiObjects& apiObjects)
        : m_totalUpdateExecutionTime{ reportData.getSectionExecutionTime(UpdateReport::ETimingSection::TotalUpdate) }
        , m_topologySortExecutionTime{ reportData.getSectionExecutionTime(UpdateReport::ETimingSection::TopologySort) }
        , m_activatedLinks{ reportData.getLinkActivations() }
    {
        m_nodesExecuted.reserve(reportData.getNodesExecuted().size());
        for (const auto& n : reportData.getNodesExecuted())
            m_nodesExecuted.push_back({ apiObjects.getApiObject(*n.first), n.second });

        m_nodesSkippedExecution.reserve(reportData.getNodesSkippedExecution().size());
        for (const auto& n : reportData.getNodesSkippedExecution())
            m_nodesSkippedExecution.push_back(apiObjects.getApiObject(*n));
    }

    const LogicEngineReportImpl::LogicNodesTimed& LogicEngineReportImpl::getNodesExecuted() const
    {
        return m_nodesExecuted;
    }

    const LogicEngineReportImpl::LogicNodes& LogicEngineReportImpl::getNodesSkippedExecution() const
    {
        return m_nodesSkippedExecution;
    }

    std::chrono::microseconds LogicEngineReportImpl::getTopologySortExecutionTime() const
    {
        return m_topologySortExecutionTime;
    }

    std::chrono::microseconds LogicEngineReportImpl::getTotalUpdateExecutionTime() const
    {
        return m_totalUpdateExecutionTime;
    }

    size_t LogicEngineReportImpl::getTotalLinkActivations() const
    {
        return m_activatedLinks;
    }

}
