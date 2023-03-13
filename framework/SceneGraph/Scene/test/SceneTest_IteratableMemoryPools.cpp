//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/TextureSampler.h"
#include "gtest/gtest.h"
#include "ActionTestScene.h"
#include "Scene/ResourceChangeCollectingScene.h"
#include "Scene/DataLayoutCachedScene.h"
#include <vector>

using namespace testing;

namespace ramses_internal
{
    using IteratableSceneTypes = ::testing::Types<
        Scene,
        TransformationCachedScene,
        ActionCollectingScene,
        ResourceChangeCollectingScene
    >;

    template <typename SCENE>
    class AnIteratableScene : public testing::Test
    {
    protected:
        SCENE m_scene;

        template<typename MemPoolT>
        void runTest(
            std::function<typename MemPoolT::handle_type()> allocateHandleF,
            std::function<void(typename MemPoolT::handle_type)> releaseHandleF,
            const MemPoolT& memPool)
        {
            using HandleT = typename MemPoolT::handle_type;

            const HandleT handle1 = allocateHandleF();
            const HandleT handle2 = allocateHandleF();
            const HandleT handle3 = allocateHandleF();
            const HandleT handle4 = allocateHandleF();
            const HandleT handle5 = allocateHandleF();
            const HandleT handle6 = allocateHandleF();

            releaseHandleF(handle3);
            const HandleT handle7 = allocateHandleF();
            releaseHandleF(handle1);
            const HandleT handle8 = allocateHandleF();
            releaseHandleF(handle4);
            releaseHandleF(handle5);

            ASSERT_EQ(HandleT{ 1u }, handle2);
            ASSERT_EQ(HandleT{ 5u }, handle6);
            ASSERT_EQ(HandleT{ 2u }, handle7);
            ASSERT_EQ(HandleT{ 0u }, handle8);

            std::vector<HandleT> handles;
            for (const auto& it : memPool)
            {
                handles.push_back(it.first);
            }

            std::vector<HandleT> exepctedHandles{ handle8, handle2, handle7, handle6 };
            EXPECT_EQ(handles, exepctedHandles);
        }
    };

    TYPED_TEST_SUITE(AnIteratableScene, IteratableSceneTypes);

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderables)
    {
        auto allocateF = std::bind(&TypeParam::allocateRenderable, &this->m_scene, NodeHandle{}, RenderableHandle{});
        auto releaseF = std::bind(&TypeParam::releaseRenderable, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getRenderables());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderStates)
    {
        auto allocateF = std::bind(&TypeParam::allocateRenderState, &this->m_scene, RenderStateHandle{});
        auto releaseF = std::bind(&TypeParam::releaseRenderState, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getRenderStates());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverCameras)
    {
        auto allocateF = std::bind(&TypeParam::allocateCamera, &this->m_scene, ECameraProjectionType::Orthographic, NodeHandle{}, DataInstanceHandle{}, CameraHandle{});
        auto releaseF = std::bind(&TypeParam::releaseCamera, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getCameras());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverNodes)
    {
        auto allocateF = std::bind(&TypeParam::allocateNode, &this->m_scene, 0u, NodeHandle{});
        auto releaseF = std::bind(&TypeParam::releaseNode, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getNodes());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverTransforms)
    {
        const auto nodeHandle = this->m_scene.allocateNode();

        auto allocateF = std::bind(&TypeParam::allocateTransform, &this->m_scene, nodeHandle, TransformHandle{});
        auto releaseF = std::bind(&TypeParam::releaseTransform, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getTransforms());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataLayouts)
    {
        auto allocateF = [&]() {
            static UInt64 resourceHash = 1u;
            return this->m_scene.allocateDataLayout(DataFieldInfoVector{}, ResourceContentHash{ resourceHash++, 0u });
        };

        auto releaseF = std::bind(&TypeParam::releaseDataLayout, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getDataLayouts());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataInstances)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType::Float), DataFieldInfo(EDataType::Float) }, ResourceContentHash(123u, 0u));

        auto allocateF = std::bind(&TypeParam::allocateDataInstance, &this->m_scene, dataLayout, DataInstanceHandle{});
        auto releaseF = std::bind(&TypeParam::releaseDataInstance, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getDataInstances());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverTextureSamplers)
    {
        const TextureSampler sampler({}, ResourceContentHash{ 123u, 0u });

        auto allocateF = std::bind(&TypeParam::allocateTextureSampler, &this->m_scene, sampler, TextureSamplerHandle{});
        auto releaseF = std::bind(&TypeParam::releaseTextureSampler, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getTextureSamplers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderGroups)
    {
        auto allocateF = std::bind(&TypeParam::allocateRenderGroup, &this->m_scene, 0u, 0u, RenderGroupHandle{});
        auto releaseF = std::bind(&TypeParam::releaseRenderGroup, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getRenderGroups());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderPasses)
    {
        auto allocateF = std::bind(&TypeParam::allocateRenderPass, &this->m_scene, 0u, RenderPassHandle{});
        auto releaseF = std::bind(&TypeParam::releaseRenderPass, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getRenderPasses());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverBlitPasses)
    {
        const auto srcRenderBufferHandle = this->m_scene.allocateRenderBuffer({ 1u, 2u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth16, ERenderBufferAccessMode_ReadWrite, 0u });
        const auto dstRenderBufferHandle = this->m_scene.allocateRenderBuffer({ 1u, 2u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth16, ERenderBufferAccessMode_ReadWrite, 0u });

        auto allocateF = std::bind(&TypeParam::allocateBlitPass, &this->m_scene, srcRenderBufferHandle, dstRenderBufferHandle, BlitPassHandle{});
        auto releaseF = std::bind(&TypeParam::releaseBlitPass, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getBlitPasses());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverPickableObjects)
    {
        auto allocateF = std::bind(&TypeParam::allocatePickableObject, &this->m_scene, DataBufferHandle{}, NodeHandle{}, PickableObjectId{ 1u }, PickableObjectHandle{});
        auto releaseF = std::bind(&TypeParam::releasePickableObject, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getPickableObjects());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderTargets)
    {
        auto allocateF = std::bind(&TypeParam::allocateRenderTarget, &this->m_scene, RenderTargetHandle{});
        auto releaseF = std::bind(&TypeParam::releaseRenderTarget, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getRenderTargets());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderBuffers)
    {
        const RenderBuffer renderBuffer{ 1u, 2u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth16, ERenderBufferAccessMode_ReadWrite, 0u };
        auto allocateF = std::bind(&TypeParam::allocateRenderBuffer, &this->m_scene, renderBuffer, RenderBufferHandle{});
        auto releaseF = std::bind(&TypeParam::releaseRenderBuffer, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getRenderBuffers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverStreamTextures)
    {
        auto allocateF = std::bind(&TypeParam::allocateStreamTexture, &this->m_scene, WaylandIviSurfaceId{1u}, ResourceContentHash{}, StreamTextureHandle{});
        auto releaseF = std::bind(&TypeParam::releaseStreamTexture, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getStreamTextures());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataBuffers)
    {
        auto allocateF = std::bind(&TypeParam::allocateDataBuffer, &this->m_scene, EDataBufferType::IndexBuffer, EDataType::Int32, 4u, DataBufferHandle{});
        auto releaseF = std::bind(&TypeParam::releaseDataBuffer, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getDataBuffers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverTextureBuffers)
    {
        auto allocateF = std::bind(&TypeParam::allocateTextureBuffer, &this->m_scene, ETextureFormat::R16F, MipMapDimensions{ {1, 1} }, TextureBufferHandle{});
        auto releaseF = std::bind(&TypeParam::releaseTextureBuffer, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getTextureBuffers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataSlots)
    {
        const DataSlot dataSlot{ EDataSlotType_TransformationConsumer, DataSlotId{6u}, NodeHandle{5u}, DataInstanceHandle{1u}, ResourceContentHash{1u, 0u}, TextureSamplerHandle{1u} };

        auto allocateF = std::bind(&TypeParam::allocateDataSlot, &this->m_scene, dataSlot, DataSlotHandle{});
        auto releaseF = std::bind(&TypeParam::releaseDataSlot, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getDataSlots());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverSceneReferences)
    {
        auto allocateF = std::bind(&TypeParam::allocateSceneReference, &this->m_scene, SceneId{123u}, SceneReferenceHandle{});
        auto releaseF = std::bind(&TypeParam::releaseSceneReference, &this->m_scene, std::placeholders::_1);
        this->runTest(allocateF, releaseF, this->m_scene.getSceneReferences());
    }
}
