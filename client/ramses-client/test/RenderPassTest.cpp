//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "SceneImpl.h"
#include "RenderPassImpl.h"
#include "RenderGroupImpl.h"
#include "RenderTargetImpl.h"
#include "ramses-utils.h"
#include "SceneAPI/RenderGroupUtils.h"
#include "Scene/ClientScene.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class ARenderPass : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    protected:
        ARenderPass()
            : LocalTestClientWithSceneAndAnimationSystem()
            , renderpass(*m_scene.createRenderPass("RenderPass"))
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

        RenderTarget* createRenderTarget()
        {
            const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(16u, 12u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
            RenderTargetDescription rtDesc;
            rtDesc.addRenderBuffer(*renderBuffer);
            return m_scene.createRenderTarget(rtDesc);
        }

        void expectNumGroupsContained(uint32_t numGroups, const RenderPass& pass)
        {
            EXPECT_EQ(numGroups, static_cast<uint32_t>(pass.impl.getAllRenderGroups().size()));
        }

        void expectGroupContained(const RenderGroup& group, int32_t order, const RenderPass& pass)
        {
            EXPECT_TRUE(pass.containsRenderGroup(group));
            int32_t actualOrder = 0;
            EXPECT_EQ(StatusOK, pass.getRenderGroupOrder(group, actualOrder));
            EXPECT_EQ(order, actualOrder);

            const ramses_internal::RenderPass& internalRP = m_internalScene.getRenderPass(pass.impl.getRenderPassHandle());
            ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(group.impl.getRenderGroupHandle(), internalRP));
            EXPECT_EQ(group.impl.getRenderGroupHandle(), ramses_internal::RenderGroupUtils::FindRenderGroupEntry(group.impl.getRenderGroupHandle(), internalRP)->renderGroup);
            EXPECT_EQ(order, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(group.impl.getRenderGroupHandle(), internalRP)->order);
        }

        void expectGroupNotContained(const RenderGroup& group, const RenderPass& pass)
        {
            EXPECT_FALSE(pass.containsRenderGroup(group));
            const ramses_internal::RenderPass& internalRP = m_internalScene.getRenderPass(pass.impl.getRenderPassHandle());
            EXPECT_FALSE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(group.impl.getRenderGroupHandle(), internalRP));
        }

        RenderPass& renderpass;
        RenderPass& renderpass2;
    };

    TEST_F(ARenderPass, canValidate)
    {
        Camera* camera = m_scene.createRemoteCamera();
        renderpass.setCamera(*camera);

        MeshNode& mesh = createValidMeshNode();
        RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderGroup->addMeshNode(mesh);
        renderpass.addRenderGroup(*renderGroup);
        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_None);

        EXPECT_EQ(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, failsValidationIfNoCameraWasSet)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        renderpass.addRenderGroup(*group);

        EXPECT_NE(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, failsValidationIfNoRenderGroupWasAdded)
    {
        Camera* camera = m_scene.createRemoteCamera();
        renderpass.setCamera(*camera);

        EXPECT_NE(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, validatesIfClearAllAndRenderTargetIsSet)
    {
        RenderTarget*            renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_All);
        EXPECT_EQ(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, validatesIfClearStencilAndRenderTargetIsSet)
    {
        RenderTarget*            renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Stencil);
        EXPECT_EQ(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, validatesIfClearDepthAndRenderTargetIsSet)
    {
        RenderTarget*            renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Depth);
        EXPECT_EQ(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, validatesIfClearColorAndRenderTargetIsSet)
    {
        RenderTarget*            renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Color);
        EXPECT_EQ(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, validatesIfClearNoneAndRenderTargetIsSet)
    {
        RenderTarget*            renderTarget      = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(renderTarget);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_None);
        EXPECT_EQ(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, validatesIfClearNoneIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_None);
        EXPECT_EQ(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, failsValidationIfClearStencilIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Stencil);
        EXPECT_NE(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, failsValidationIfClearDepthIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Depth);
        EXPECT_NE(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, failsValidationIfClearColorIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Color);
        EXPECT_NE(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, failsValidationIfClearAllIsSetWithoutRenderTarget)
    {
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();
        MeshNode&                mesh              = createValidMeshNode();
        RenderGroup*             renderGroup       = m_scene.createRenderGroup();

        renderGroup->addMeshNode(mesh);
        renderpass.setCamera(*perspectiveCamera);
        renderpass.setRenderTarget(nullptr);
        renderpass.addRenderGroup(*renderGroup);

        renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_All);
        EXPECT_NE(StatusOK, renderpass.validate());
    }

    TEST_F(ARenderPass, hasNoCameraByDefaultWhichIsInterpretedAsIdentityViewMatrix)
    {
        EXPECT_EQ(nullptr, renderpass.getCamera());
    }

    TEST_F(ARenderPass, returnsTheSameCameraWhichWasSet)
    {
        Camera* camera = m_scene.createRemoteCamera();

        EXPECT_EQ(StatusOK, renderpass.setCamera(*camera));

        EXPECT_EQ(camera, renderpass.getCamera());
    }

    TEST_F(ARenderPass, reportsErrorWhenSetCameraFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);
        Camera* camera = anotherScene.createRemoteCamera();

        EXPECT_NE(StatusOK, renderpass.setCamera(*camera));
    }

    TEST_F(ARenderPass, canBeAssignedCustomOrthoCameraOnlyWhenAllParametersAreValid)
    {
        OrthographicCamera* orthoCam = m_scene.createOrthographicCamera();

        EXPECT_NE(StatusOK, renderpass.setCamera(*orthoCam));
        EXPECT_NE(orthoCam, renderpass.getCamera());

        // Viewport is correct, this is enough because initialized correctly
        orthoCam->setViewport(0, 0, 5, 10);
        EXPECT_NE(StatusOK, renderpass.setCamera(*orthoCam));
        EXPECT_NE(orthoCam, renderpass.getCamera());

        // Clipping planes set -> camera is valid now
        orthoCam->setFrustum(0.f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);

        EXPECT_EQ(StatusOK, renderpass.setCamera(*orthoCam));
        EXPECT_EQ(orthoCam, renderpass.getCamera());
    }

    TEST_F(ARenderPass, canBeAssignedCustomPerspectiveCameraOnlyWhenAllParametersAreValid)
    {
        PerspectiveCamera* perspCam = m_scene.createPerspectiveCamera();

        EXPECT_NE(StatusOK, renderpass.setCamera(*perspCam));
        EXPECT_NE(perspCam, renderpass.getCamera());

        // Viewport is correct, but still not enough
        perspCam->setViewport(0, 0, 5, 10);
        EXPECT_NE(StatusOK, renderpass.setCamera(*perspCam));
        EXPECT_NE(perspCam, renderpass.getCamera());

        // Frustum set -> camera is valid now
        perspCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        EXPECT_EQ(StatusOK, renderpass.setCamera(*perspCam));
        EXPECT_EQ(perspCam, renderpass.getCamera());
    }

    TEST_F(ARenderPass, confidence_setsCameraMulitpleTimesAndChecksInScene)
    {
        Camera* camera = m_scene.createRemoteCamera();
        Camera* camera2 = m_scene.createRemoteCamera();

        EXPECT_EQ(StatusOK, renderpass.setCamera(*camera));
        EXPECT_EQ(camera, renderpass.getCamera());

        EXPECT_EQ(StatusOK, renderpass.setCamera(*camera2));
        EXPECT_EQ(camera2, renderpass.getCamera());
    }

    TEST_F(ARenderPass, hasFrameBufferAsDefaultRenderTarget)
    {
        EXPECT_EQ(nullptr, renderpass.getRenderTarget());
        EXPECT_FALSE(m_scene.impl.getIScene().getRenderPass(renderpass.impl.getRenderPassHandle()).renderTarget.isValid());
    }

    TEST_F(ARenderPass, canBeAssignedFrameBufferAsRenderTarget)
    {
        EXPECT_EQ(StatusOK, renderpass.setRenderTarget(nullptr));
        EXPECT_EQ(nullptr, renderpass.getRenderTarget());
        EXPECT_FALSE(m_scene.impl.getIScene().getRenderPass(renderpass.impl.getRenderPassHandle()).renderTarget.isValid());
    }

    TEST_F(ARenderPass, canSwitchFromRenderTargetBackToFramebuffer)
    {
        // first set valid rendertarget
        PerspectiveCamera* camera = m_scene.createPerspectiveCamera();
        setValidPerspectiveCameraParameters(*camera);
        EXPECT_EQ(StatusOK, renderpass.setCamera(*camera));
        RenderTarget* renderTarget = createRenderTarget();
        renderpass.setRenderTarget(renderTarget);

        // then set framebuffer rendering
        EXPECT_EQ(StatusOK, renderpass.setRenderTarget(nullptr));
        EXPECT_EQ(nullptr, renderpass.getRenderTarget());
        EXPECT_FALSE(m_scene.impl.getIScene().getRenderPass(renderpass.impl.getRenderPassHandle()).renderTarget.isValid());
    }

    TEST_F(ARenderPass, reportsErrorWhenSetRenderTargetFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);

        const RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(16u, 12u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*renderBuffer);
        RenderTarget* renderTarget = anotherScene.createRenderTarget(rtDesc);
        ASSERT_TRUE(nullptr != renderTarget);

        PerspectiveCamera* camera = createDummyPerspectiveCamera();
        ASSERT_TRUE(nullptr != camera);
        EXPECT_EQ(StatusOK, renderpass.setCamera(*camera));

        EXPECT_NE(StatusOK, renderpass.setRenderTarget(renderTarget));
    }

    TEST_F(ARenderPass, canNotBeAssignedRenderTargetIfCameraIsNotSetFirst)
    {
        RenderTarget* renderTarget = createRenderTarget();
        EXPECT_NE(StatusOK, renderpass.setRenderTarget(renderTarget));
        EXPECT_NE(renderTarget->impl.getRenderTargetHandle(), m_scene.impl.getIScene().getRenderPass(renderpass.impl.getRenderPassHandle()).renderTarget);
        EXPECT_NE(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, canNotBeAssignedRenderTargetIfCameraIsRemoteCamera)
    {
        RenderTarget* renderTarget = createRenderTarget();
        const RemoteCamera* remoteCamera = m_scene.createRemoteCamera();
        ASSERT_EQ(StatusOK, renderpass.setCamera(*remoteCamera));

        EXPECT_NE(StatusOK, renderpass.setRenderTarget(renderTarget));
    }

    TEST_F(ARenderPass, canBeAssignedRenderTargetWithOrthoCamera)
    {
        RenderTarget* renderTarget = createRenderTarget();
        const OrthographicCamera* orthoCamera = createDummyOrthoCamera();

        ASSERT_EQ(StatusOK, renderpass.setCamera(*orthoCamera));
        EXPECT_EQ(StatusOK, renderpass.setRenderTarget(renderTarget));

        EXPECT_EQ(renderTarget->impl.getRenderTargetHandle(), m_scene.impl.getIScene().getRenderPass(renderpass.impl.getRenderPassHandle()).renderTarget);
        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, canBeAssignedRenderTargetWithPerspectiveCamera)
    {
        RenderTarget* renderTarget = createRenderTarget();
        const PerspectiveCamera* perspectiveCamera = createDummyPerspectiveCamera();

        ASSERT_EQ(StatusOK, renderpass.setCamera(*perspectiveCamera));
        EXPECT_EQ(StatusOK, renderpass.setRenderTarget(renderTarget));

        EXPECT_EQ(renderTarget->impl.getRenderTargetHandle(), m_scene.impl.getIScene().getRenderPass(renderpass.impl.getRenderPassHandle()).renderTarget);
        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, reportsErrorWhenDestroyingRenderTargetIfAssignedToRenderPass)
    {
        RenderTarget* renderTarget = createRenderTarget();
        ASSERT_EQ(StatusOK, renderpass.setCamera(*createDummyPerspectiveCamera()));

        EXPECT_EQ(StatusOK, renderpass.setRenderTarget(renderTarget));
        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());

        EXPECT_NE(StatusOK, m_scene.destroy(*renderTarget));

        EXPECT_EQ(renderTarget, renderpass.getRenderTarget());
    }

    TEST_F(ARenderPass, isEnabledByDefault)
    {
        EXPECT_TRUE(renderpass.isEnabled());
    }

    TEST_F(ARenderPass, canBeDisabled)
    {
        EXPECT_EQ(StatusOK, renderpass.setEnabled(false));
        EXPECT_FALSE(renderpass.isEnabled());
    }

    TEST_F(ARenderPass, setsItsClearColor)
    {
        float c[4];
        renderpass.getClearColor(c[0], c[1], c[2], c[3]);
        EXPECT_EQ(0.f, c[0]);
        EXPECT_EQ(0.f, c[1]);
        EXPECT_EQ(0.f, c[2]);
        EXPECT_EQ(1.f, c[3]);

        EXPECT_EQ(StatusOK, renderpass.setClearColor(0.1f, 0.2f, 0.3f, 0.4f));
        renderpass.getClearColor(c[0], c[1], c[2], c[3]);
        EXPECT_EQ(0.1f, c[0]);
        EXPECT_EQ(0.2f, c[1]);
        EXPECT_EQ(0.3f, c[2]);
        EXPECT_EQ(0.4f, c[3]);
    }

    TEST_F(ARenderPass, setsItsClearFlag)
    {
        EXPECT_EQ(static_cast<UInt32>(ramses::EClearFlags::EClearFlags_All), renderpass.getClearFlags());

        EXPECT_EQ(StatusOK, renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_None));
        EXPECT_EQ(static_cast<UInt32>(ramses::EClearFlags::EClearFlags_None), renderpass.getClearFlags());

        EXPECT_EQ(StatusOK, renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Depth));
        EXPECT_EQ(static_cast<UInt32>(ramses::EClearFlags::EClearFlags_Depth), renderpass.getClearFlags());

        EXPECT_EQ(StatusOK, renderpass.setClearFlags(ramses::EClearFlags::EClearFlags_Color | ramses::EClearFlags::EClearFlags_Stencil));
        EXPECT_EQ(static_cast<uint32_t>(ramses::EClearFlags::EClearFlags_Color | ramses::EClearFlags::EClearFlags_Stencil), renderpass.getClearFlags());
    }

    TEST_F(ARenderPass, canAddRenderGroups)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        RenderGroup* group2 = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 3));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group2, -7));

        expectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 3, renderpass);
        expectGroupContained(*group2, -7, renderpass);
    }

    TEST_F(ARenderPass, reportsErrorWhenAddRenderGroupFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);
        RenderGroup* group = anotherScene.createRenderGroup();

        EXPECT_NE(StatusOK, renderpass.addRenderGroup(*group, 3));
    }

    TEST_F(ARenderPass, canCheckContainmentOfRenderGroups)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        RenderGroup* group2 = m_scene.createRenderGroup();
        RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 1));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group3, 3));

        expectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 1, renderpass);
        expectGroupNotContained(*group2, renderpass);
        expectGroupContained(*group3, 3, renderpass);
    }

    TEST_F(ARenderPass, canRemoveRenderGroup)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        renderpass.addRenderGroup(*group, 1);
        renderpass.removeRenderGroup(*group);

        expectNumGroupsContained(0u, renderpass);
        expectGroupNotContained(*group, renderpass);
    }

    TEST_F(ARenderPass, removingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        RenderGroup* group2 = m_scene.createRenderGroup();
        RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 1));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group2, 2));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group3, 3));

        EXPECT_EQ(StatusOK, renderpass.removeRenderGroup(*group2));

        expectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 1, renderpass);
        expectGroupNotContained(*group2, renderpass);
        expectGroupContained(*group3, 3, renderpass);
    }

    TEST_F(ARenderPass, doesNotContainRenderGroupWhichWasDestroyed)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        renderpass.addRenderGroup(*group);
        m_scene.destroy(*group);

        expectNumGroupsContained(0u, renderpass);
    }

    TEST_F(ARenderPass, destroyingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        RenderGroup* group2 = m_scene.createRenderGroup();
        RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 1));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group2, 2));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group3, 3));

        EXPECT_EQ(StatusOK, m_scene.destroy(*group2));

        expectNumGroupsContained(2u, renderpass);
        expectGroupContained(*group, 1, renderpass);
        expectGroupContained(*group3, 3, renderpass);
    }

    TEST_F(ARenderPass, reportsErrorWhenTryingToRemoveRenderGroupWhichWasNotAddedToPass)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        status_t status = renderpass.removeRenderGroup(*group);

        EXPECT_NE(StatusOK, status);
    }

    TEST_F(ARenderPass, canChangeRenderGroupOrderByReaddingIt)
    {
        RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 1));
        EXPECT_EQ(StatusOK, renderpass.removeRenderGroup(*group));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 999));

        expectGroupContained(*group, 999, renderpass);
    }

    TEST_F(ARenderPass, groupCanBeAddedToTwoPassesWithDifferentOrder)
    {
        RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 3));
        EXPECT_EQ(StatusOK, renderpass2.addRenderGroup(*group, 999));

        expectGroupContained(*group, 3, renderpass);
        expectGroupContained(*group, 999, renderpass2);
    }

    TEST_F(ARenderPass, removingRenderGroupFromPassDoesNotAffectItsPlaceInAnotherPass)
    {
        RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 3));
        EXPECT_EQ(StatusOK, renderpass2.addRenderGroup(*group, 999));

        EXPECT_EQ(StatusOK, renderpass.removeRenderGroup(*group));

        expectGroupNotContained(*group, renderpass);
        expectGroupContained(*group, 999, renderpass2);
    }

    TEST_F(ARenderPass, destroyedRenderGroupIsRemovedFromAllItsPasses)
    {
        RenderGroup* group = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 3));
        EXPECT_EQ(StatusOK, renderpass2.addRenderGroup(*group, 999));

        EXPECT_EQ(StatusOK, m_scene.destroy(*group));

        EXPECT_TRUE(renderpass.impl.getAllRenderGroups().empty());
        EXPECT_TRUE(renderpass2.impl.getAllRenderGroups().empty());
    }

    TEST_F(ARenderPass, canRemoveAllRenderGroups)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        RenderGroup* group2 = m_scene.createRenderGroup();
        RenderGroup* group3 = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group, 1));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group2, 2));
        EXPECT_EQ(StatusOK, renderpass.addRenderGroup(*group3, 3));

        EXPECT_EQ(StatusOK, renderpass.removeAllRenderGroups());

        expectNumGroupsContained(0u, renderpass);
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
        EXPECT_EQ(StatusOK, renderpass.setRenderOnce(true));
        EXPECT_TRUE(renderpass.isRenderOnce());
        EXPECT_EQ(StatusOK, renderpass.setRenderOnce(false));
        EXPECT_FALSE(renderpass.isRenderOnce());
    }

    TEST_F(ARenderPass, canRetriggerRenderOnce)
    {
        EXPECT_EQ(StatusOK, renderpass.setRenderOnce(true));
        EXPECT_EQ(StatusOK, renderpass.retriggerRenderOnce());
    }

    TEST_F(ARenderPass, failsToRetriggerRenderOnceIfNotRenderOnce)
    {
        EXPECT_NE(StatusOK, renderpass.retriggerRenderOnce());
    }
}
