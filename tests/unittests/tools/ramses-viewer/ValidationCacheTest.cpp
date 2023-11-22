//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ValidationCache.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ramses/client/Scene.h"
#include "impl/ValidationReportImpl.h"

namespace ramses::internal
{
    class DummyObjectImpl : public SceneObjectImpl
    {
    public:
        explicit DummyObjectImpl(SceneImpl& scene, std::string_view name)
            : SceneObjectImpl(scene, ERamsesObjectType::MeshNode, name)
        {
        }

        void deinitializeFrameworkData() override {}

        void onValidate(ValidationReportImpl& report) const override
        {
            for (auto& issue : issuesToReport)
                report.add(issue.type, issue.message, issue.object);
        }

        std::vector<Issue> issuesToReport;
    };

    class DummyObject : public SceneObject
    {
    public:
        explicit DummyObject(SceneImpl& scene, std::string_view name = {})
            : SceneObject(std::make_unique<DummyObjectImpl>(scene, name))
            , impl(static_cast<DummyObjectImpl&>(SceneObject::impl()))
        {
        }
        DummyObjectImpl& impl;
    };


    class AValidationCache : public testing::Test
    {
    protected:
        RamsesFrameworkConfig frameworkConfig{EFeatureLevel_Latest};
        SceneConfig           sceneConfig{sceneId_t(1234)};
        RamsesFramework       framework{frameworkConfig};
        RamsesClient*         client = framework.createClient("client");
        Scene*                scene  = client->createScene(sceneConfig);
        DummyObject           objA{scene->impl(), "A"};
        DummyObject           objB{scene->impl(), "B"};
        DummyObject           objC{scene->impl(), "C"};
        ValidationReport      report;
    };

    TEST_F(AValidationCache, empty)
    {
        ValidationCache cache(report);
        EXPECT_EQ(0u, cache.getAllIssues().size());
        auto rangeA = cache.getIssues(objA.impl);
        EXPECT_EQ(0, rangeA.second - rangeA.first);
        EXPECT_FALSE(cache.getIssueType(objA.impl).has_value());
        auto rangeB = cache.getIssues(objB.impl);
        EXPECT_EQ(0, rangeB.second - rangeB.first);
        EXPECT_FALSE(cache.getIssueType(objB.impl).has_value());
    }

    TEST_F(AValidationCache, singleWarning)
    {
        report.impl().add(EIssueType::Warning, "msg1", &objA);
        ValidationCache cache(report);
        EXPECT_EQ(1u, cache.getAllIssues().size());
        auto rangeA = cache.getIssues(objA.impl);
        EXPECT_EQ(1, rangeA.second - rangeA.first);
        EXPECT_EQ(EIssueType::Warning, *cache.getIssueType(objA.impl));
        auto rangeB = cache.getIssues(objB.impl);
        EXPECT_EQ(0, rangeB.second - rangeB.first);
        EXPECT_FALSE(cache.getIssueType(objB.impl).has_value());
    }

    TEST_F(AValidationCache, reordersUnsortedReport)
    {
        report.impl().add(EIssueType::Warning, "msgC1", &objC);
        report.impl().add(EIssueType::Warning, "msgA1", &objA);
        report.impl().add(EIssueType::Warning, "msgB1", &objB);
        report.impl().add(EIssueType::Error, "msgA2", &objA);
        report.impl().add(EIssueType::Warning, "msgA3", &objA);
        report.impl().add(EIssueType::Warning, "msgB2", &objB);

        ValidationCache cache(report);
        EXPECT_EQ(6u, cache.getAllIssues().size());
        auto rangeA = cache.getIssues(objA.impl);
        EXPECT_EQ(3, rangeA.second - rangeA.first);
        EXPECT_EQ("msgA1", rangeA.first->message);
        EXPECT_EQ("msgA2", (++rangeA.first)->message);
        EXPECT_EQ("msgA3", (++rangeA.first)->message);
        EXPECT_EQ(EIssueType::Error, *cache.getIssueType(objA.impl));

        auto rangeB = cache.getIssues(objB.impl);
        EXPECT_EQ(2, rangeB.second - rangeB.first);
        EXPECT_EQ("msgB1", rangeB.first->message);
        EXPECT_EQ("msgB2", (++rangeB.first)->message);
        EXPECT_EQ(EIssueType::Warning, *cache.getIssueType(objB.impl));

        auto rangeC = cache.getIssues(objC.impl);
        EXPECT_EQ(1, rangeC.second - rangeC.first);
        EXPECT_EQ("msgC1", rangeC.first->message);
        EXPECT_EQ(EIssueType::Warning, *cache.getIssueType(objC.impl));
    }

    TEST_F(AValidationCache, usesValidationToFindDependentIssues)
    {
        // objA depends on objC
        report.impl().add(EIssueType::Warning, "msgC1", &objC);
        objA.impl.issuesToReport = report.impl().getIssues();

        ValidationCache cache(report);
        EXPECT_EQ(EIssueType::Warning, *cache.getIssueType(objA.impl));
        EXPECT_EQ(EIssueType::Warning, *cache.getIssueType(objA.impl)); // from cache
        auto rangeA = cache.getIssues(objA.impl);
        EXPECT_EQ(0, rangeA.second - rangeA.first); // only dependent issues

        EXPECT_EQ(EIssueType::Warning, *cache.getIssueType(objC.impl));
        EXPECT_EQ(EIssueType::Warning, *cache.getIssueType(objC.impl)); // from cache
        auto rangeC = cache.getIssues(objC.impl);
        EXPECT_EQ(1, rangeC.second - rangeC.first);
    }
}
