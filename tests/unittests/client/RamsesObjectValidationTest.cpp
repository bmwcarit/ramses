//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/framework/ValidationReport.h"
#include "ramses/framework/RamsesObject.h"
#include "impl/RamsesObjectImpl.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    class TestRamsesObjectImpl : public RamsesObjectImpl
    {
    public:
        explicit TestRamsesObjectImpl(std::string_view name, std::vector<const RamsesObject*> dependentObjs, EIssueType type)
            : RamsesObjectImpl(ERamsesObjectType::Invalid, name)
            , m_depObjs(std::move(dependentObjs))
            , m_type(type)
        {
        }

        void deinitializeFrameworkData() override {}

        void onValidate(ValidationReportImpl& report) const override
        {
            report.add(m_type, getName() + "msgA", &getRamsesObject());

            // dependent objects will be validated after onValidate() returns
            for (const auto depObj : m_depObjs)
                report.addDependentObject(*this, depObj->impl());

            report.add(m_type, getName() + "msgB", &getRamsesObject());
        }

        std::vector<const RamsesObject*> m_depObjs;
        EIssueType                       m_type;
    };

    class TestRamsesObject : public RamsesObject
    {
    public:
        explicit TestRamsesObject(std::string_view name, std::vector<const RamsesObject*> dependencies = {}, EIssueType t = EIssueType::Warning)
            : RamsesObject(std::make_unique<TestRamsesObjectImpl>(name, std::move(dependencies), t))
        {}

    };

    class ARamsesObjectValidation : public ::testing::Test
    {
    };

    TEST_F(ARamsesObjectValidation, noDuplicatesInReport)
    {
        const TestRamsesObject obj1{"obj1", {}, EIssueType::Warning};
        const TestRamsesObject obj2{"obj2", {}, EIssueType::Warning};

        ValidationReport report;
        obj1.validate(report);
        obj1.validate(report);
        obj1.validate(report);
        EXPECT_EQ(2u, report.getIssues().size());
        obj2.validate(report);
        obj2.validate(report);
        obj2.validate(report);
        ASSERT_EQ(4u, report.getIssues().size());
        EXPECT_EQ("obj1msgA", report.getIssues()[0].message);
        EXPECT_EQ("obj1msgB", report.getIssues()[1].message);
        EXPECT_EQ("obj2msgA", report.getIssues()[2].message);
        EXPECT_EQ("obj2msgB", report.getIssues()[3].message);
    }

    TEST_F(ARamsesObjectValidation, validateTree)
    {
        // hierarchy of dependent object to validate
        // obj1 - obj3
        //      \ obj2 - obj3
        // obj3 exists twice in the tree
        const TestRamsesObject objLvl3{"3"};
        const TestRamsesObject objLvl2{"2", {&objLvl3}};
        const TestRamsesObject objLvl1{"1", {&objLvl2, &objLvl3}};

        {
            ValidationReport report;
            objLvl1.validate(report);
            EXPECT_FALSE(report.hasError());
            EXPECT_TRUE(report.hasIssue());
            ASSERT_EQ(6u, report.getIssues().size());

            const auto& issues = report.getIssues();
            EXPECT_EQ("1msgA", issues[0].message);
            EXPECT_EQ(EIssueType::Warning, issues[0].type);
            EXPECT_EQ(&objLvl1, issues[0].object);
            EXPECT_EQ("2msgA", issues[2].message);
            EXPECT_EQ(EIssueType::Warning, issues[2].type);
            EXPECT_EQ(&objLvl2, issues[2].object);
            EXPECT_EQ("3msgA", issues[4].message);
            EXPECT_EQ(EIssueType::Warning, issues[4].type);
            EXPECT_EQ(&objLvl3, issues[4].object);
        }
        {
            ValidationReport report;
            objLvl2.validate(report);
            EXPECT_FALSE(report.hasError());
            EXPECT_TRUE(report.hasIssue());
            ASSERT_EQ(4u, report.getIssues().size());

            const auto& issues = report.getIssues();
            EXPECT_EQ("2msgA", issues[0].message);
            EXPECT_EQ(EIssueType::Warning, issues[0].type);
            EXPECT_EQ(&objLvl2, issues[0].object);
            EXPECT_EQ("3msgA", issues[2].message);
            EXPECT_EQ(EIssueType::Warning, issues[2].type);
            EXPECT_EQ(&objLvl3, issues[2].object);
        }
        {
            ValidationReport report;
            objLvl3.validate(report);
            EXPECT_FALSE(report.hasError());
            EXPECT_TRUE(report.hasIssue());
            ASSERT_EQ(2u, report.getIssues().size());

            const auto& issues = report.getIssues();
            EXPECT_EQ("3msgA", issues[0].message);
            EXPECT_EQ(EIssueType::Warning, issues[0].type);
            EXPECT_EQ(&objLvl3, issues[0].object);
        }
    }

    TEST_F(ARamsesObjectValidation, reportsStatusBasedSeverityOfValidation)
    {
        const TestRamsesObject objWarn{"warn", {}, EIssueType::Warning};
        const TestRamsesObject objErr{"err", {}, EIssueType::Error};

        ValidationReport report;
        EXPECT_FALSE(report.hasError());
        EXPECT_FALSE(report.hasIssue());

        objWarn.validate(report);
        EXPECT_FALSE(report.hasError());
        EXPECT_TRUE(report.hasIssue());

        objErr.validate(report);
        EXPECT_TRUE(report.hasError());
        EXPECT_TRUE(report.hasIssue());

        EXPECT_EQ(4u, report.getIssues().size());
    }
}
