//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses/framework/ValidationReport.h"
#include "impl/ValidationReportImpl.h"

namespace ramses::internal
{
    class AValidationReport : public testing::Test
    {
    protected:
        ValidationReport report;
    };

    TEST_F(AValidationReport, IsInitializedEmpty)
    {
        EXPECT_FALSE(report.hasError());
        EXPECT_FALSE(report.hasIssue());
        EXPECT_TRUE(report.getIssues().empty());
    }

    TEST_F(AValidationReport, DistinguishesErrorsAndWarnings)
    {
        const Issue warn = {EIssueType::Warning, "fooWarn", nullptr};
        const Issue err  = {EIssueType::Error, "fooErr", nullptr};

        report.impl().add(warn.type, warn.message, warn.object);
        EXPECT_FALSE(report.hasError());
        EXPECT_TRUE(report.hasIssue());
        EXPECT_EQ(1u, report.getIssues().size());

        report.impl().add(warn.type, warn.message, warn.object);
        EXPECT_FALSE(report.hasError());
        EXPECT_TRUE(report.hasIssue());
        EXPECT_EQ(2u, report.getIssues().size());

        report.impl().add(err.type, err.message, err.object);
        EXPECT_TRUE(report.hasError());
        EXPECT_TRUE(report.hasIssue());
        EXPECT_EQ(3u, report.getIssues().size());
    }

    TEST_F(AValidationReport, addVisit)
    {
        const auto* obj1 = reinterpret_cast<const RamsesObjectImpl*>(1);
        const auto* obj2 = reinterpret_cast<const RamsesObjectImpl*>(2);

        EXPECT_TRUE(report.impl().addVisit(obj1));
        EXPECT_FALSE(report.impl().addVisit(obj1));
        EXPECT_TRUE(report.impl().addVisit(obj2));
        EXPECT_FALSE(report.impl().addVisit(obj2));

        report.clear();
        EXPECT_TRUE(report.impl().addVisit(obj1));
        EXPECT_TRUE(report.impl().addVisit(obj2));
        EXPECT_FALSE(report.impl().addVisit(obj1));
        EXPECT_FALSE(report.impl().addVisit(obj2));
    }

    TEST_F(AValidationReport, dependentObjects)
    {
        const auto* obj1 = reinterpret_cast<const RamsesObjectImpl*>(1);
        const auto* obj2 = reinterpret_cast<const RamsesObjectImpl*>(2);
        const auto* obj3 = reinterpret_cast<const RamsesObjectImpl*>(3);

        report.impl().addDependentObject(*obj1, *obj2);
        report.impl().addDependentObject(*obj1, *obj3);
        report.impl().addDependentObject(*obj2, *obj3);

        EXPECT_EQ(2u, report.impl().getDependentObjects(obj1).size());
        EXPECT_EQ(1u, report.impl().getDependentObjects(obj2).size());
        EXPECT_TRUE(report.impl().getDependentObjects(obj3).empty());
        EXPECT_TRUE(report.impl().getDependentObjects(nullptr).empty());
    }

    TEST_F(AValidationReport, clear)
    {
        const auto* obj1   = reinterpret_cast<const RamsesObjectImpl*>(1);
        const auto* obj2   = reinterpret_cast<const RamsesObjectImpl*>(2);
        const Issue issue1 = {EIssueType::Warning, "foo1", nullptr};
        const Issue issue2 = {EIssueType::Warning, "foo2", nullptr};

        report.impl().add(issue1.type, issue1.message, issue1.object);
        report.impl().addDependentObject(*obj1, *obj2);
        report.impl().add(issue2.type, issue2.message, issue2.object);

        EXPECT_EQ(2u, report.getIssues().size());
        EXPECT_EQ(1u, report.impl().getDependentObjects(obj1).size());

        report.clear();
        EXPECT_TRUE(report.getIssues().empty());
        EXPECT_TRUE(report.impl().getDependentObjects(obj1).empty());
    }

    TEST_F(AValidationReport, CanBeCopyAndMoveConstructed)
    {
        const Issue issue = {EIssueType::Warning, "foo", nullptr};
        report.impl().add(issue.type, issue.message, issue.object);

        ValidationReport reportCopy{report};
        ASSERT_TRUE(reportCopy.hasIssue());
        EXPECT_EQ(reportCopy.getIssues().back(), issue);

        ValidationReport reportMove{std::move(report)};
        ASSERT_TRUE(reportMove.hasIssue());
        EXPECT_EQ(reportMove.getIssues().back(), issue);
    }

    TEST_F(AValidationReport, CanBeCopyAndMoveAssigned)
    {
        const Issue issue = {EIssueType::Warning, "foo", nullptr};
        report.impl().add(issue.type, issue.message, issue.object);

        ValidationReport reportCopy;
        reportCopy = report;
        ASSERT_TRUE(reportCopy.hasIssue());
        EXPECT_EQ(reportCopy.getIssues().back(), issue);

        ValidationReport reportMove;
        reportMove = std::move(report);
        ASSERT_TRUE(reportMove.hasIssue());
        EXPECT_EQ(reportMove.getIssues().back(), issue);
    }

    TEST_F(AValidationReport, CanBeSelfAssigned)
    {
        const Issue issue = {EIssueType::Warning, "foo", nullptr};
        report.impl().add(issue.type, issue.message, issue.object);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        report = report;
        ASSERT_TRUE(report.hasIssue());
        EXPECT_EQ(report.getIssues().back(), issue);
        report = std::move(report);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ASSERT_TRUE(report.hasIssue());
        // NOLINTNEXTLINE(bugprone-use-after-move)
        EXPECT_EQ(report.getIssues().back(), issue);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
}
