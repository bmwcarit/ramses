//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngineReport.h"
#include "impl/LogicEngineReportImpl.h"

namespace ramses
{
    LogicEngineReport::LogicEngineReport() noexcept
        : m_impl{ std::make_unique<internal::LogicEngineReportImpl>() }
    {
    }

    LogicEngineReport::LogicEngineReport(std::unique_ptr<internal::LogicEngineReportImpl> impl) noexcept
        : m_impl{ std::move(impl) }
    {
    }

    LogicEngineReport::LogicEngineReport(LogicEngineReport&& other) noexcept = default;
    LogicEngineReport& LogicEngineReport::operator=(LogicEngineReport&& other) noexcept = default;
    LogicEngineReport::~LogicEngineReport() = default;

    const std::vector<LogicEngineReport::LogicNodeTimed>& LogicEngineReport::getNodesExecuted() const
    {
        return m_impl->getNodesExecuted();
    }

    const std::vector<LogicNode*>& LogicEngineReport::getNodesSkippedExecution() const
    {
        return m_impl->getNodesSkippedExecution();
    }

    std::chrono::microseconds LogicEngineReport::getTopologySortExecutionTime() const
    {
        return m_impl->getTopologySortExecutionTime();
    }

    std::chrono::microseconds LogicEngineReport::getTotalUpdateExecutionTime() const
    {
        return m_impl->getTotalUpdateExecutionTime();
    }

    RAMSES_API size_t LogicEngineReport::getTotalLinkActivations() const
    {
        return m_impl->getTotalLinkActivations();
    }

}
