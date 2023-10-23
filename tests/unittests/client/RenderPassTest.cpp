//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Camera.h"
#include "ramses/client/RenderTargetDescription.h"
#include "impl/SceneImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RenderTargetImpl.h"
#include "ramses/client/ramses-utils.h"
#include "internal/SceneGraph/SceneAPI/RenderGroupUtils.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

using namespace testing;

namespace ramses::internal
{
    class ARenderPass : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        ARenderPass()
            : renderpass(*m_scene.createRenderPass("RenderPass"))
            , renderpass2(*m_scene.createRenderPass("RenderPass2"))
        {
        }

        OrthographicCamera* createDummyOrthoCamera()
        {
            OrthographicCamera* orthoCam = m_scene.createOrthographicCamera();
            orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
            orthoCam->setViewport(0, 0, 100, 200);

            return orthoCam;
        }

        PerspectiveCamera* createDummyPerspectiveCamera()
        {
            PerspectiveCamera* perspCam = m_scene.createPerspectiveCamera();
            perspCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
            perspCam->setViewport(0, 0, 100, 200);
            return perspCam;
        }

        ramses::RenderTarget* createRenderTarget()
        {
            const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(16u, 12u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
            RenderTargetDescription rtDesc;
            rtDesc.addRenderBuffer(*renderBuffer);
            return m_scene.createRenderTarget(rtDesc);
        }

        static void ExpectNumGroupsContained(uint32_t numGroups, const ramses::RenderPass& pass)
        {
            EXPECT_EQ(numGroups, static_cast<uint32_t>(pass.impl().getAllRenderGroups().size()));
        }

        void expectGroupContained(const ramses::RenderGroup& group, int32_t order, const ramses::RenderPass& pass)
        {
            EXPECT_TRUE(pass.containsRenderGroup(group));
            int32_t actualOrder = 0;
            EXPECT_TRUE(pass.getRenderGroupOrder(group, actualOrder));
            EXPECT_EQ(order, actualOrder);

            const ramses::internal::RenderPass& internalRP = m_internalScene.getRenderPass(pass.impl().getRenderPassHandle());
            ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderGroup(group.impl().getRenderGroupHandle(), internalRP));
            EXPECT_EQ(group.impl().getRenderGroupHandle(), ramses::internal::RenderGroupUtils::FindRenderGroupEntry(group.impl().getRenderGroupHandle(), internalRP)->renderGroup);
            EXPECT_EQ(order, ramses::internal::RenderGroupUtils::FindRenderGroupEntry(group.impl().getRenderGroupHandle(), internalRP)->order);
        }

        void expectGroupNotContained(const ramses::RenderGroup& group, const ramses::RenderPass& pass)
        {
            EXPECT_FALSE(pass.containsRenderGroup(group));
            const ramses::internal::RenderPass& internalRP = m_internalScene.getRenderPass(pass.impl().getRenderPassHandle());
            EXPECT_FALSE(ramses::internal::RenderGroupUtils::ContainsRenderGroup(group.impl().getRenderGroupHandle(), internalRP));
        }

        ramses::RenderPass& renderpass;
        ramses::RenderPass& renderpass2;
    };

    TEST_F(ARenderPass, canValidate)
    {
        renderpass.setCamera(*createDummyPerspectiveCamera());

        MeshNode& mesh = createValidMeshNode();
        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderGroup->addMeshNode(mesh);
        renderpass.addRenderGroup(*renderGroup);
        renderpass.setClearFlags(EClearFlag::None);

        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderPass, failsValidationIfNoCameraWasSet)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        renderpass.addRenderGroup(*group);

        ValidationReport report;
        renderpass.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderPass, failsValidationIfNoRenderGroupWasAdded)
    {
        renderpass.setCamera(*createDummyOrthoCamera());

        ValidationReport report;
        renderpass.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderPass, validatesIfClearAllAndRenderTargetIsSet)
    {
        ramses::RenderTarget*    renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::All);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderPass, validatesIfClearStencilAndRenderTargetIsSet)
    {
        ramses::RenderTarget*    renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::Stencil);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderPass, validatesIfClearDepthAndRenderTargetIsSet)
    {
        ramses::RenderTarget*    renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::Depth);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderPass, validatesIfClearColorAndRenderTargetIsSet)
    {
        ramses::RenderTarget*    renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::Color);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderPass, validatesIfClearNoneAndRenderTargetIsSet)
    {
        ramses::RenderTarget*    renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::None);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderPass, validatesIfClearNoneIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::None);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderPass, failsValidationIfClearStencilIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::Stencil);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasError());
        EXPECT_TRUE(report.hasIssue());
        EXPECT_THAT(report.getIssues()[0].message, ::testing::HasSubstr("clear flags will have no effect"));
        EXPECT_EQ(report.getIssues()[0].type, EIssueType::Warning);
    }

    TEST_F(ARenderPass, failsValidationIfClearDepthIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::Depth);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_FALSE(report.hasError());
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderPass, failsValidationIfClearColorIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::Color);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderPass, failsValidationIfClearAllIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        ramses::RenderGroup*     renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(EClearFlag::All);
        ValidationReport report;
        renderpass.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderPass, hasNoCameraByDefaultWhichIsInterpretedAsIdentityViewMatrix)
    {
        EXPECT_EQ(nullptr, renderpass.getCamera());
    }

    TEST_F(ARenderPass, returnsTheSameCameraWhichWasSet)
    {
        ramses::Camera* camera = createDummyPerspectiveCamera();

        EXPECT_TRUE(renderpass.setCamera(*camera));

        EXPECT_EQ(camera, renderpass.getCamera());
    }

    TEST_F(ARenderPass, reportsErrorWhenSetCameraFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(sceneId_t(12u));
        // create valid camera from another scene
        auto camera = anotherScene.createOrthographicCamera();
        camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        camera->setViewport(0, 0, 100, 200);

        EXPECT_FALSE(renderpass.setCamera(*camera));
    }

    TEST_F(ARenderPass, canBeAssignedCustomOrthoCameraOnlyWhenAllParametersAreValid)
    {
        OrthographicCamera* orthoCam = m_scene.createOrthographicCamera();

        EXPECT_FALSE(renderpass.setCamera(*orthoCam));
        EXPECT_NE(orthoCam, renderpass.getCamera());

        // Viewport is correct, this is enough because initialized correctly
        orthoCam->setViewport(0, 0, 5, 10);
        EXPECT_FALSE(renderpass.setCamera(*orthoCam));
        EXPECT_NE(orthoCam, renderpass.getCamera());

        // Clipping planes set -> camera is valid now
        orthoCam->setFrustum(0.f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);

        EXPECT_TRUE(renderpass.setCamera(*orthoCam));
        EXPECT_EQ(orthoCam, renderpass.getCamera());
    }

    TEST_F(ARenderPass, canBeAssignedCustomPerspectiveCameraOnlyWhenAllParametersAreValid)
    {
        PerspectiveCamera* perspCam = m_scene.createPerspectiveCamera();

        EXPECT_FALSE(renderpass.setCamera(*perspCam));
        EXPECT_NE(perspCam, renderpass.getCamera());

        // Viewport is correct, but still not enough
        perspCam->setViewport(0, 0, 5, 10);
        EXPECT_FALSE(renderpass.setCamera(*perspCam));
        EXPECT_NE(perspCam, renderpass.getCamera());

        // Frustum set -> camera is valid now
        perspCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        EXPECT_TRUE(renderpass.setCamera(*perspCam));
        EXPECT_EQ(perspCam, renderpass.getCamera());
    }

    TEST_F(ARenderPass, confidence_setsCameraMulitpleTimesAndChecksInScene)
    {
        ramses::Camera* camera = createDummyPerspectiveCamera();
        ramses::Camera* camera2 = createDummyOrthoCamera();

        EXPECT_TRUE(renderpass.setCamera(*camera));
        EXPECT_EQ(camera, renderpass.getCamera());

        EXPECT_TRUE(renderpass.setCamera(*camera2));
        EXPECT_EQ(camera2, renderpass.getCamera());
    }

    TEST_F(ARenderPass, hasFrameBufferAsDefaultRenderTarget)
    {
        EXPECT_EQ(nullptr, renderpass.getRenderTarget());
        EXPECT_FALSE(m_scene.impl().getIScene().getRenderPass(renderpass.impl().getRenderPassHandle()).renderTarget.isValid());
    }

    TEST_F(ARenderPass, canBeAssignedFrameBufferAsRenderTarget)
    {
        EXPECT_TRUE(renderpass.setRenderTarget(nullptr));
        EXPECT_EQ(nullptr, renderpass.getRenderTarget());
        EXPECT_FALSE(m_scene.impl().getIScene().getRenderPass(renderpass.impl().getRenderPassHandle()).renderTarget.isValid());
    }

    TEST_F(ARenderPass, canSwitchFromRenderTargetBackToFramebuffer)
    {
        // first set valid rendertarget
        PerspectiveCamera* camera = m_scene.createPerspectiveCamera();
        SetValidPerspectiveCameraParameters(*camera);
        EXPECT_TRUE(renderpass.setCamera(*camera));
        ramses::RenderTarget* renderTarget = createRenderTarget();
        renderpass.setRenderTarget(renderTarget);

        // then set framebuffer rendering
        EXPECT_TRUE(renderpass.setRenderTarget(nullptr));
        EXPECT_EQ(nullptr, renderpass.getRenderTarget());
        EXPECT_FALSE(m_scene.impl().getIScene().getRenderPass(renderpass.impl().getRenderPassHandle()).renderTarget.isValid());
    }

    TEST_F(ARenderPass, reportsErrorWhenSetRenderTargetFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(ramses::sceneId_t(12u));

        const ramses::RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(16u, 12u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*renderBuffer);
        ramses::RenderTarget* renderTarget = anotherScene.createRenderTarget(rtDesc);
        ASSERT_TRUE(nullptr != renderTarget);

        PerspectiveCamera* camera = createDummyPerspectiveCamera();
        ASSERT_TRUE(nullptr != camera);
        EXPECT_TRUE(renderpass.setCamera(*camera));

        EXPECT_FALSE(renderpass.setRenderTarget(renderTarget));
    }

    TEST_F(ARenderPass, canNotBeAssignedRenderTargetIfCameraIsNotSetFirst)
    {
        ramses::RenderTarget* renderTarget = createRenderTarget();
        EXPECT_FALSE(renderpass.setRenderTarget(renderTarget));
        EXPECT_NE(renderTarget->impl().getRenderTargetHandle(), m_scene.impl().getIScene().getRenderPass(renderpass.impl().getRenderPassHandle()).renderTarget);
        EXPECT_NE(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, canBeAssignedRenderTargetWithOrthoCamera)
    {
        ramses::RenderTarget* renderTarget = createRenderTarget();
        const OrthographicCamera* orthoCamera = createDummyOrthoCamera();

        ASSERT_TRUE(renderpass.setCamera(*orthoCamera));
        EXPECT_TRUE(renderpass.setRenderTarget(renderTarget));

        EXPECT_EQ(renderTarget->impl().getRenderTargetHandle(), m_scene.impl().getIScene().getRenderPass(renderpass.impl().getRenderPassHandle()).renderTarget);
        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, canBeAssignedRenderTargetWithPerspectiveCamera)
    {
        ramses::RenderTarget* renderTarget = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();

        ASSERT_TRUE(renderpass.setCamera(*perspectiveCamera));
        EXPECT_TRUE(renderpass.setRenderTarget(renderTarget));

        EXPECT_EQ(renderTarget->impl().getRenderTargetHandle(), m_scene.impl().getIScene().getRenderPass(renderpass.impl().getRenderPassHandle()).renderTarget);
        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, reportsErrorWhenDestroyingRenderTargetIfAssignedToRenderPass)
    {
        ramses::RenderTarget* renderTarget = createRenderTarget();
        ASSERT_TRUE(renderpass.setCamera(*createDummyPerspectiveCamera()));

        EXPECT_TRUE(renderpass.setRenderTarget(renderTarget));
        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());

        EXPECT_FALSE(m_scene.destroy(*renderTarget));

        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, isEnabledByDefault)
    {
        EXPECT_TRUE(renderpass.isEnabled());
    }

    TEST_F(ARenderPass, canBeDisabled)
    {
        EXPECT_TRUE(renderpass.setEnabled(false));
        EXPECT_FALSE(renderpass.isEnabled());
    }

    TEST_F(ARenderPass, setsItsClearColor)
    {
        auto c = renderpass.getClearColor();
        EXPECT_EQ(0.f, c[0]);
        EXPECT_EQ(0.f, c[1]);
        EXPECT_EQ(0.f, c[2]);
        EXPECT_EQ(1.f, c[3]);

        EXPECT_TRUE(renderpass.setClearColor({0.1f, 0.2f, 0.3f, 0.4f}));
        c = renderpass.getClearColor();
        EXPECT_EQ(0.1f, c[0]);
        EXPECT_EQ(0.2f, c[1]);
        EXPECT_EQ(0.3f, c[2]);
        EXPECT_EQ(0.4f, c[3]);
    }

    TEST_F(ARenderPass, setsItsClearFlag)
    {
        EXPECT_EQ(EClearFlag::All, renderpass.getClearFlags());

        EXPECT_TRUE(renderpass.setClearFlags(EClearFlag::None));
        EXPECT_EQ(EClearFlag::None, renderpass.getClearFlags());

        EXPECT_TRUE(renderpass.setClearFlags(EClearFlag::Depth));
        EXPECT_EQ(EClearFlag::Depth, renderpass.getClearFlags());

        EXPECT_TRUE(renderpass.setClearFlags(EClearFlag::Color | EClearFlag::Stencil));
        EXPECT_EQ(EClearFlag::Color | EClearFlag::Stencil, renderpass.getClearFlags());
    }

    TEST_F(ARenderPass, canAddRenderGroups)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        ramses::RenderGroup* group2 = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 3));
        EXPECT_TRUE(renderpass.addRenderGroup(*group2, -7));

        ExpectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 3, renderpass);
        expectGroupContained(*group2, -7, renderpass);
    }

    TEST_F(ARenderPass, reportsErrorWhenAddRenderGroupFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(ramses::sceneId_t(12u));
        ramses::RenderGroup* group = anotherScene.createRenderGroup();

        EXPECT_FALSE(renderpass.addRenderGroup(*group, 3));
    }

    TEST_F(ARenderPass, canCheckContainmentOfRenderGroups)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        ramses::RenderGroup* group2 = m_scene.createRenderGroup();
        ramses::RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 1));
        EXPECT_TRUE(renderpass.addRenderGroup(*group3, 3));

        ExpectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 1, renderpass);
        expectGroupNotContained(*group2, renderpass);
        expectGroupContained(*group3, 3, renderpass);
    }

    TEST_F(ARenderPass, canRemoveRenderGroup)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        renderpass.addRenderGroup(*group, 1);
        renderpass.removeRenderGroup(*group);

        ExpectNumGroupsContained(0u, renderpass);
        expectGroupNotContained(*group, renderpass);
    }

    TEST_F(ARenderPass, removingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        ramses::RenderGroup* group2 = m_scene.createRenderGroup();
        ramses::RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 1));
        EXPECT_TRUE(renderpass.addRenderGroup(*group2, 2));
        EXPECT_TRUE(renderpass.addRenderGroup(*group3, 3));

        EXPECT_TRUE(renderpass.removeRenderGroup(*group2));

        ExpectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 1, renderpass);
        expectGroupNotContained(*group2, renderpass);
        expectGroupContained(*group3, 3, renderpass);
    }

    TEST_F(ARenderPass, doesNotContainRenderGroupWhichWasDestroyed)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        renderpass.addRenderGroup(*group);
        m_scene.destroy(*group);

        ExpectNumGroupsContained(0u, renderpass);
    }

    TEST_F(ARenderPass, destroyingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        ramses::RenderGroup* group2 = m_scene.createRenderGroup();
        ramses::RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 1));
        EXPECT_TRUE(renderpass.addRenderGroup(*group2, 2));
        EXPECT_TRUE(renderpass.addRenderGroup(*group3, 3));

        EXPECT_TRUE(m_scene.destroy(*group2));

        ExpectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 1, renderpass);
        expectGroupContained(*group3, 3, renderpass);
    }

    TEST_F(ARenderPass, reportsErrorWhenTryingToRemoveRenderGroupWhichWasNotAddedToPass)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        bool status = renderpass.removeRenderGroup(*group);

        EXPECT_FALSE(status);
    }

    TEST_F(ARenderPass, canChangeRenderGroupOrderByReaddingIt)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 1));
        EXPECT_TRUE(renderpass.removeRenderGroup(*group));
        EXPECT_TRUE(renderpass.addRenderGroup(*group, 999));

        expectGroupContained(*group, 999, renderpass);
    }

    TEST_F(ARenderPass, groupCanBeAddedToTwoPassesWithDifferentOrder)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 3));
        EXPECT_TRUE(renderpass2.addRenderGroup(*group, 999));

        expectGroupContained(*group, 3, renderpass);
        expectGroupContained(*group, 999, renderpass2);
    }

    TEST_F(ARenderPass, removingRenderGroupFromPassDoesNotAffectItsPlaceInAnotherPass)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 3));
        EXPECT_TRUE(renderpass2.addRenderGroup(*group, 999));

        EXPECT_TRUE(renderpass.removeRenderGroup(*group));

        expectGroupNotContained(*group, renderpass);
        expectGroupContained(*group, 999, renderpass2);
    }

    TEST_F(ARenderPass, destroyedRenderGroupIsRemovedFromAllItsPasses)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 3));
        EXPECT_TRUE(renderpass2.addRenderGroup(*group, 999));

        EXPECT_TRUE(m_scene.destroy(*group));

        EXPECT_TRUE(renderpass.impl().getAllRenderGroups().empty());
        EXPECT_TRUE(renderpass2.impl().getAllRenderGroups().empty());
    }

    TEST_F(ARenderPass, canRemoveAllRenderGroups)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        ramses::RenderGroup* group2 = m_scene.createRenderGroup();
        ramses::RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_TRUE(renderpass.addRenderGroup(*group, 1));
        EXPECT_TRUE(renderpass.addRenderGroup(*group2, 2));
        EXPECT_TRUE(renderpass.addRenderGroup(*group3, 3));

        EXPECT_TRUE(renderpass.removeAllRenderGroups());

        ExpectNumGroupsContained(0u, renderpass);
        expectGroupNotContained(*group, renderpass);
        expectGroupNotContained(*group2, renderpass);
        expectGroupNotContained(*group3, renderpass);
    }

    TEST_F(ARenderPass, isNotRenderOnceInitially)
    {
        EXPECT_FALSE(renderpass.isRenderOnce());
    }

    TEST_F(ARenderPass, canBeSetAndUnsetAsRenderOnce)
    {
        EXPECT_TRUE(renderpass.setRenderOnce(true));
        EXPECT_TRUE(renderpass.isRenderOnce());
        EXPECT_TRUE(renderpass.setRenderOnce(false));
        EXPECT_FALSE(renderpass.isRenderOnce());
    }

    TEST_F(ARenderPass, canRetriggerRenderOnce)
    {
        EXPECT_TRUE(renderpass.setRenderOnce(true));
        EXPECT_TRUE(renderpass.retriggerRenderOnce());
    }

    TEST_F(ARenderPass, failsToRetriggerRenderOnceIfNotRenderOnce)
    {
        EXPECT_FALSE(renderpass.retriggerRenderOnce());
    }
}
