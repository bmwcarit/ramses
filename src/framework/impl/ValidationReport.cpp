//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/framework/ValidationReport.h"
// impl
#include "impl/ValidationReportImpl.h"

namespace ramses
{
    ValidationReport::ValidationReport()
        : m_impl(std::make_unique<internal::ValidationReportImpl>())
    {
    }

    ValidationReport::~ValidationReport() = default;

    ValidationReport::ValidationReport(ValidationReport&& other) noexcept = default;

    ValidationReport& ValidationReport::operator=(ValidationReport&& other) noexcept = default;

    ValidationReport::ValidationReport(const ValidationReport& other)
        : m_impl(std::make_unique<internal::ValidationReportImpl>(*other.m_impl))
    {
    }

    ValidationReport& ValidationReport::operator=(const ValidationReport& other)
    {
        if (this != &other)
            m_impl = std::make_unique<internal::ValidationReportImpl>(*other.m_impl);
        return *this;
    }

    void ValidationReport::clear()
    {
        m_impl->clear();
    }

    bool ValidationReport::hasError() const
    {
        return m_impl->hasError();
    }

    bool ValidationReport::hasIssue() const
    {
        return m_impl->hasIssue();
    }

    const std::vector<Issue>& ValidationReport::getIssues() const
    {
        return m_impl->getIssues();
    }

    internal::ValidationReportImpl& ValidationReport::impl()
    {
        return *m_impl;
    }

    const internal::ValidationReportImpl& ValidationReport::impl() const
    {
        return *m_impl;
    }
}
