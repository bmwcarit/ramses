//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientTestUtils.h"
#include "CreationHelper.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/RenderPassBinding.h"
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

    class ASceneObjectBindingValidation : public LocalTestClientWithScene, public ::testing::Test
    {
    protected:
        LogicEngine* le{ m_scene.createLogicEngine() };
    };

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfAppearanceBindingBoundMoreThanOnce)
    {
        auto& obj = createObject<Appearance>("");
        le->createAppearanceBinding(obj);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidBinding = le->createAppearanceBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfCameraBindingBoundMoreThanOnce)
    {
        auto& obj = createObject<PerspectiveCamera>("");
        SetValidPerspectiveCameraParameters(obj);
        le->createCameraBinding(obj);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidBinding = le->createCameraBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfMeshNodeBindingBoundMoreThanOnce)
    {
        auto& obj = createValidMeshNode();
        le->createMeshNodeBinding(obj);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidBinding = le->createMeshNodeBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfNodeBindingBoundMoreThanOnce)
    {
        auto& obj = createObject<Node>("");
        le->createNodeBinding(obj);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidBinding = le->createNodeBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfRenderBufferBindingBoundMoreThanOnce)
    {
        auto& obj = createObject<ramses::RenderBuffer>("");
        le->createRenderBufferBinding(obj);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidBinding = le->createRenderBufferBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfRenderGroupBindingBoundMoreThanOnce)
    {
        auto& renderGroup = createObject<ramses::RenderGroup>("");
        MeshNode& mesh = createValidMeshNode();
        EXPECT_TRUE(renderGroup.addMeshNode(mesh, 3));
        RenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(mesh, "Mesh Node"));

        le->createRenderGroupBinding(renderGroup, elements);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidBinding = le->createRenderGroupBinding(renderGroup, elements);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfRenderPassBindingBoundMoreThanOnce)
    {
        auto& renderPass = createObject<ramses::RenderPass>("");
        le->createRenderPassBinding(renderPass);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidBinding = le->createRenderPassBinding(renderPass);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, reportsErrorIfBindingBoundMoreThanOnceInDifferentLogicEngines)
    {
        auto& meshnode = createValidMeshNode();
        auto* binding = le->createMeshNodeBinding(meshnode);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        LogicEngine* le2{ m_scene.createLogicEngine() };
        le2->createMeshNodeBinding(meshnode);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*binding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, validateCameraBindingAndNodeBindingToSameCameraObject)
    {
        auto& obj = createObject<PerspectiveCamera>("");
        SetValidPerspectiveCameraParameters(obj);
        le->createCameraBinding(obj);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        le->createNodeBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidNodeBinding = le->createNodeBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidNodeBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidCamBinding = le->createCameraBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidCamBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }

    TEST_F(ASceneObjectBindingValidation, validateMeshNodeBindingAndNodeBindingToSameMeshNodeObject)
    {
        auto& obj = createValidMeshNode();
        le->createMeshNodeBinding(obj);
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        le->createNodeBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidNodeBinding = le->createNodeBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidNodeBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());

        auto* invalidMeshNodeBinding = le->createMeshNodeBinding(obj);
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        le->destroy(*invalidMeshNodeBinding);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasError());
    }
}
