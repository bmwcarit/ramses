//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include <vector>
#include <memory>
#include <chrono>

namespace rlogic::internal
{
    class LogicEngineReportImpl;
}

namespace rlogic
{
    class LogicNode;

    /**
    * A collection of results from #rlogic::LogicEngine::update which can be used
    * for debugging or profiling logic node networks.
    * Can be obtained using #rlogic::LogicEngine::getLastUpdateReport after an update with enabled reporting
    * (#rlogic::LogicEngine::enableUpdateReport).
    */
    class LogicEngineReport
    {
    public:
        /// LogicNode with measured update execution
        using LogicNodeTimed = std::pair<LogicNode*, std::chrono::microseconds>;

        /**
        * Gets list of logic nodes that were updated and the amount of time it took to execute their update logic.
        * Typically nodes have to be updated due to one of their inputs being 'dirty' (modified)
        * or some other internal trigger.
        * The order of the nodes in the list matches the order of their execution, this order is determined
        * by the logic network topology and is deterministic.
        *
        * @return list of updated nodes with update execution time, sorted by execution order
        */
        [[nodiscard]] RAMSES_API const std::vector<LogicNodeTimed>& getNodesExecuted() const;

        /**
        * List of logic nodes that were not updated because their inputs did not change therefore there was no need to.
        * Similar to #getNodesExecuted, the order of the nodes in the list matches the order of their
        * (in this case skipped) execution.
        *
        * @return list of nodes which did not need update
        */
        [[nodiscard]] RAMSES_API const std::vector<LogicNode*>& getNodesSkippedExecution() const;

        /**
        * Time it took to sort logic nodes by their topology during update.
        * Note that re-sorting is only needed if topology changed (node linked/unlinked),
        * otherwise the result is cached and this should take negligible time.
        *
        * @return time to sort logic nodes
        */
        [[nodiscard]] RAMSES_API std::chrono::microseconds getTopologySortExecutionTime() const;

        /**
        * Time it took to update the whole logic nodes network.
        * This is essentially the same as measuring how long it took to call #rlogic::LogicEngine::update
        * from application side.
        *
        * @return time to update all logic nodes
        */
        [[nodiscard]] RAMSES_API std::chrono::microseconds getTotalUpdateExecutionTime() const;

        /**
        * Obtain the number of links activated during update.
        *
        * @return the number of links activated during update
        */
        [[nodiscard]] RAMSES_API size_t getTotalLinkActivations() const;

        /**
        * Default constructor of LogicEngineReport.
        */
        RAMSES_API LogicEngineReport() noexcept;

        /**
        * Constructor of LogicEngineReport. Do not construct, use #rlogic::LogicEngine::getLastUpdateReport to obtain.
        *
        * @param impl implementation details of the LogicEngineReport
        */
        RAMSES_API explicit LogicEngineReport(std::unique_ptr<internal::LogicEngineReportImpl> impl) noexcept;

        /**
        * Class destructor
        */
        RAMSES_API ~LogicEngineReport();

        /**
        * Copying disabled, move instead.
        */
        LogicEngineReport(const LogicEngineReport&) = delete;

        /**
        * Move constructor
        *
        * @param other source
        */
        RAMSES_API LogicEngineReport(LogicEngineReport&& other) noexcept;

        /**
        * Copying disabled, move instead.
        */
        LogicEngineReport& operator=(const LogicEngineReport&) = delete;

        /**
        * Move assignment
        *
        * @param other source
        */
        RAMSES_API LogicEngineReport& operator=(LogicEngineReport&& other) noexcept;

    private:
        /**
        * Implementation detail of LogicEngineReport
        */
        std::unique_ptr<internal::LogicEngineReportImpl> m_impl; //NOLINT(modernize-use-default-member-init) fixing this would break pimpl pattern
    };
}
