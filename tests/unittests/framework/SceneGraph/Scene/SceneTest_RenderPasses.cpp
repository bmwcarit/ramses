//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"
#include "internal/SceneGraph/SceneAPI/RenderGroupUtils.h"

using namespace testing;

namespace ramses::internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, RenderPassCreated)
    {
        EXPECT_EQ(0u, this->m_scene.getRenderPassCount());

        RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});

        EXPECT_EQ(1u, this->m_scene.getRenderPassCount());
        EXPECT_TRUE(this->m_scene.isRenderPassAllocated(pass));
    }

    TYPED_TEST(AScene, RenderPassHasDefaultValues)
    {
        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        const RenderPass& rp = this->m_scene.getRenderPass(pass);

        EXPECT_TRUE(rp.isEnabled);
        EXPECT_EQ(glm::vec4(0.f, 0.f, 0.f, 1.f), rp.clearColor);
        EXPECT_EQ(EClearFlag::All, rp.clearFlags);
        EXPECT_FALSE(rp.camera.isValid());
        EXPECT_FALSE(rp.renderTarget.isValid());
        EXPECT_EQ(0, rp.renderOrder);
        EXPECT_FALSE(rp.isRenderOnce);
    }

    TYPED_TEST(AScene, RenderPassReleased)
    {
        RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        this->m_scene.releaseRenderPass(pass);

        EXPECT_FALSE(this->m_scene.isRenderPassAllocated(pass));
    }

    TYPED_TEST(AScene, setsCameraToRenderPass)
    {
        const RenderPassHandle renderPass = this->m_scene.allocateRenderPass(0, {});
        const auto             dataLayout = this->m_scene.allocateDataLayout({DataFieldInfo{ramses::internal::EDataType::Vector2I}, DataFieldInfo{ramses::internal::EDataType::Vector2I}}, ResourceContentHash(123u, 0u), {});
        const CameraHandle camera = this->m_scene.allocateCamera(ECameraProjectionType::Perspective, this->m_scene.allocateNode(0, {}), this->m_scene.allocateDataInstance(dataLayout, {}), {});
        EXPECT_FALSE(this->m_scene.getRenderPass(renderPass).camera.isValid());

        this->m_scene.setRenderPassCamera(renderPass, camera);
        EXPECT_TRUE(this->m_scene.getRenderPass(renderPass).camera.isValid());
    }

    TYPED_TEST(AScene, DoesNotContainRenderPassWhichWasNotCreated)
    {
        EXPECT_FALSE(this->m_scene.isRenderPassAllocated(RenderPassHandle(1)));
    }

    TYPED_TEST(AScene, RenderPassRenderTarget)
    {
        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        EXPECT_FALSE(this->m_scene.getRenderPass(pass).renderTarget.isValid());

        const RenderTargetHandle targetHandle = this->m_scene.allocateRenderTarget({});
        this->m_scene.setRenderPassRenderTarget(pass, targetHandle);
        EXPECT_TRUE(this->m_scene.getRenderPass(pass).renderTarget.isValid());
    }

    TYPED_TEST(AScene, DisableRenderPass)
    {
        RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        EXPECT_TRUE(this->m_scene.getRenderPass(pass).isEnabled);

        this->m_scene.setRenderPassEnabled(pass, false);
        EXPECT_FALSE(this->m_scene.getRenderPass(pass).isEnabled);
    }

    TYPED_TEST(AScene, createsRenderPassWithOrderZeroAndSetsOrderTwice)
    {
        const RenderPassHandle renderPass = this->m_scene.allocateRenderPass(0, {});
        const RenderPass& rp = this->m_scene.getRenderPass(renderPass);
        EXPECT_EQ(0, rp.renderOrder);

        this->m_scene.setRenderPassRenderOrder(renderPass, 42);
        EXPECT_EQ(42, rp.renderOrder);

        this->m_scene.setRenderPassRenderOrder(renderPass, 0);
        EXPECT_EQ(0, rp.renderOrder);
    }

    TYPED_TEST(AScene, AssignsDefaultClearValuesToRenderPass)
    {
        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});

        const RenderPass& rp = this->m_scene.getRenderPass(pass);
        EXPECT_EQ(glm::vec4(0.f, 0.f, 0.f, 1.f), rp.clearColor);
        EXPECT_EQ(EClearFlag::All, rp.clearFlags);
    }

    TYPED_TEST(AScene, ReturnsClearParameterOfRenderPassWhichWereSetBefore)
    {
        const glm::vec4 clearColor(0.5f, 0.0f, 1.f, 0.25f);

        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        this->m_scene.setRenderPassClearFlag(pass, EClearFlag::None);
        this->m_scene.setRenderPassClearColor(pass, clearColor);

        const RenderPass& rp = this->m_scene.getRenderPass(pass);
        EXPECT_EQ(clearColor, rp.clearColor);
        EXPECT_EQ(EClearFlag::None, rp.clearFlags);
    }

    TYPED_TEST(AScene, canAddRenderGroupToRenderPass)
    {
        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        const RenderGroupHandle renderGroup = this->m_scene.allocateRenderGroup(0, 0, {});
        const RenderPass& rp = this->m_scene.getRenderPass(pass);
        EXPECT_FALSE(RenderGroupUtils::ContainsRenderGroup(renderGroup, rp));

        this->m_scene.addRenderGroupToRenderPass(pass, renderGroup, 3);
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroup, rp));
        EXPECT_EQ(renderGroup, rp.renderGroups[0].renderGroup);
        EXPECT_EQ(3, rp.renderGroups[0].order);
    }

    TYPED_TEST(AScene, canRemoveRenderGroupFromRenderPass)
    {
        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        const RenderGroupHandle renderGroup = this->m_scene.allocateRenderGroup(0, 0, {});
        const RenderGroupHandle renderGroup2 = this->m_scene.allocateRenderGroup(0, 0, {});
        this->m_scene.addRenderGroupToRenderPass(pass, renderGroup, 1);
        this->m_scene.addRenderGroupToRenderPass(pass, renderGroup2, 2);

        const RenderPass& rp = this->m_scene.getRenderPass(pass);
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroup, rp));
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroup2, rp));

        this->m_scene.removeRenderGroupFromRenderPass(pass, renderGroup);
        EXPECT_FALSE(RenderGroupUtils::ContainsRenderGroup(renderGroup, rp));
        EXPECT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroup2, rp));
    }

    TYPED_TEST(AScene, canReaddRenderGroupWithDifferentOrder)
    {
        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        const RenderGroupHandle renderGroup = this->m_scene.allocateRenderGroup(0, 0, {});

        this->m_scene.addRenderGroupToRenderPass(pass, renderGroup, 3);
        this->m_scene.removeRenderGroupFromRenderPass(pass, renderGroup);
        this->m_scene.addRenderGroupToRenderPass(pass, renderGroup, 15);

        const RenderPass& rp = this->m_scene.getRenderPass(pass);
        ASSERT_TRUE(RenderGroupUtils::ContainsRenderGroup(renderGroup, rp));
        EXPECT_EQ(renderGroup, rp.renderGroups[0].renderGroup);
        EXPECT_EQ(15, rp.renderGroups[0].order);
    }

    TYPED_TEST(AScene, canSetRenderOnce)
    {
        const RenderPassHandle pass = this->m_scene.allocateRenderPass(0, {});
        this->m_scene.setRenderPassRenderOnce(pass, true);
        EXPECT_TRUE(this->m_scene.getRenderPass(pass).isRenderOnce);
        this->m_scene.setRenderPassRenderOnce(pass, false);
        EXPECT_FALSE(this->m_scene.getRenderPass(pass).isRenderOnce);
    }
}
