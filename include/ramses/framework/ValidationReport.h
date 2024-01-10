//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/framework/Issue.h"
#include <memory>
#include <vector>

namespace ramses
{
    namespace internal
    {
        class ValidationReportImpl;
    }

    /**
     * @brief ValidationReport contains a list of issues that is reported by #ramses::RamsesObject::validate().
     *
     * @see #ramses::RamsesObject::validate()
     * @ingroup CoreAPI
     */
    class RAMSES_API ValidationReport
    {
    public:
        /**
         * @brief Creates an empty report
         */
        ValidationReport();

        /**
         * @brief Destructor
         */
        ~ValidationReport();

        /**
         * @brief Deleted copy constructor
         */
        ValidationReport(const ValidationReport& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        ValidationReport(ValidationReport&& other) noexcept;

        /**
         * @brief Deleted copy assignment
         */
        ValidationReport& operator=(const ValidationReport& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        ValidationReport& operator=(ValidationReport&& other) noexcept;

        /**
         * @brief Erases all issues from the report. After this call getIssues() returns an empty vector
         */
        void clear();

        /**
         * @brief Checks whether the report contains an issue that is qualified as an error
         * @return true if the report contains an error, false otherwise
         */
        [[nodiscard]] bool hasError() const;

        /**
         * @brief Checks whether the report contains any issues (warnings or errors)
         * @return true if the report contains any error or warning, false otherwise
         */
        [[nodiscard]] bool hasIssue() const;

        /**
         * @brief Gets all reported issues
         * @return vector of issues
         */
        [[nodiscard]] const std::vector<Issue>& getIssues() const;

        /**
         * Get the internal implementation
         */
        [[nodiscard]] internal::ValidationReportImpl& impl();

        /**
         * Get the internal implementation
         */
        [[nodiscard]] const internal::ValidationReportImpl& impl() const;

    private:
        std::unique_ptr<internal::ValidationReportImpl> m_impl;
    };
}
