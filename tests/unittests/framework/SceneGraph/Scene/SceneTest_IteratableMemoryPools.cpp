//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "gtest/gtest.h"
#include "ActionTestScene.h"
#include "internal/SceneGraph/Scene/ResourceChangeCollectingScene.h"
#include "internal/SceneGraph/Scene/DataLayoutCachedScene.h"
#include <vector>

using namespace testing;

namespace ramses::internal
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
        auto allocateF = [scene = &this->m_scene]{ return scene->allocateRenderable(NodeHandle{}, RenderableHandle{}); };
        auto releaseF = [scene = &this->m_scene](RenderableHandle renderable) { scene->releaseRenderable(renderable); };
        this->runTest(allocateF, releaseF, this->m_scene.getRenderables());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderStates)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateRenderState(RenderStateHandle{}); };
        auto releaseF = [scene = &this->m_scene](RenderStateHandle renderState) { scene->releaseRenderState(renderState); };
        this->runTest(allocateF, releaseF, this->m_scene.getRenderStates());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverCameras)
    {
        auto allocateF = [scene = &this->m_scene]{ return scene->allocateCamera(ECameraProjectionType::Orthographic, NodeHandle{}, DataInstanceHandle{}, CameraHandle{}); };
        auto releaseF = [scene = &this->m_scene](CameraHandle camera) { scene->releaseCamera(camera); };
        this->runTest(allocateF, releaseF, this->m_scene.getCameras());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverNodes)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateNode(0u, NodeHandle{}); };
        auto releaseF = [scene = &this->m_scene](NodeHandle node) { scene->releaseNode(node); };
        this->runTest(allocateF, releaseF, this->m_scene.getNodes());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverTransforms)
    {
        const auto nodeHandle = this->m_scene.allocateNode(0, {});

        auto allocateF = [scene = &this->m_scene, nodeHandle] { return scene->allocateTransform(nodeHandle, TransformHandle{}); };
        auto releaseF = [scene = &this->m_scene](TransformHandle transform) { scene->releaseTransform(transform); };
        this->runTest(allocateF, releaseF, this->m_scene.getTransforms());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataLayouts)
    {
        auto allocateF = [&]() {
            static uint64_t resourceHash = 1u;
            return this->m_scene.allocateDataLayout(DataFieldInfoVector{}, ResourceContentHash{ resourceHash++, 0u }, {});
        };

        auto releaseF = [scene = &this->m_scene](DataLayoutHandle dataLayout) { scene->releaseDataLayout(dataLayout); };
        this->runTest(allocateF, releaseF, this->m_scene.getDataLayouts());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataInstances)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({DataFieldInfo(EDataType::Float), DataFieldInfo(EDataType::Float)}, ResourceContentHash(123u, 0u), {});

        auto allocateF = [scene = &this->m_scene, dataLayout] { return scene->allocateDataInstance(dataLayout, DataInstanceHandle{}); };
        auto releaseF = [scene = &this->m_scene](DataInstanceHandle dataInstance) { scene->releaseDataInstance(dataInstance); };
        this->runTest(allocateF, releaseF, this->m_scene.getDataInstances());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverTextureSamplers)
    {
        const TextureSampler sampler({}, ResourceContentHash{ 123u, 0u });

        auto allocateF = [scene = &this->m_scene, sampler] { return scene->allocateTextureSampler(sampler, TextureSamplerHandle{}); };
        auto releaseF = [scene = &this->m_scene](TextureSamplerHandle textureSampler) { scene->releaseTextureSampler(textureSampler); };
        this->runTest(allocateF, releaseF, this->m_scene.getTextureSamplers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderGroups)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateRenderGroup(0u, 0u, RenderGroupHandle{}); };
        auto releaseF = [scene = &this->m_scene](RenderGroupHandle renderGroup) { scene->releaseRenderGroup(renderGroup); };
        this->runTest(allocateF, releaseF, this->m_scene.getRenderGroups());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderPasses)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateRenderPass(0u, RenderPassHandle{}); };
        auto releaseF = [scene = &this->m_scene](RenderPassHandle renderPass) { scene->releaseRenderPass(renderPass); };
        this->runTest(allocateF, releaseF, this->m_scene.getRenderPasses());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverBlitPasses)
    {
        const auto srcRenderBufferHandle = this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, {});
        const auto dstRenderBufferHandle = this->m_scene.allocateRenderBuffer({ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u }, {});

        auto allocateF = [scene = &this->m_scene, srcRenderBufferHandle, dstRenderBufferHandle] { return scene->allocateBlitPass(srcRenderBufferHandle, dstRenderBufferHandle, BlitPassHandle{}); };
        auto releaseF = [scene = &this->m_scene](BlitPassHandle blitPass) { scene->releaseBlitPass(blitPass); };
        this->runTest(allocateF, releaseF, this->m_scene.getBlitPasses());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverPickableObjects)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocatePickableObject(DataBufferHandle{}, NodeHandle{}, PickableObjectId{ 1u }, PickableObjectHandle{}); };
        auto releaseF = [scene = &this->m_scene](PickableObjectHandle pickable) { scene->releasePickableObject(pickable); };
        this->runTest(allocateF, releaseF, this->m_scene.getPickableObjects());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderTargets)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateRenderTarget(RenderTargetHandle{}); };
        auto releaseF = [scene = &this->m_scene](RenderTargetHandle renderTarget) { scene->releaseRenderTarget(renderTarget); };
        this->runTest(allocateF, releaseF, this->m_scene.getRenderTargets());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverRenderBuffers)
    {
        const RenderBuffer renderBuffer{ 1u, 2u, EPixelStorageFormat::Depth16, ERenderBufferAccessMode::ReadWrite, 0u };
        auto allocateF = [scene = &this->m_scene, renderBuffer] { return scene->allocateRenderBuffer(renderBuffer, RenderBufferHandle{}); };
        auto releaseF = [scene = &this->m_scene](RenderBufferHandle handle) { scene->releaseRenderBuffer(handle); };
        this->runTest(allocateF, releaseF, this->m_scene.getRenderBuffers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataBuffers)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::Int32, 4u, DataBufferHandle{}); };
        auto releaseF = [scene = &this->m_scene](DataBufferHandle dataBuffer) { scene->releaseDataBuffer(dataBuffer); };
        this->runTest(allocateF, releaseF, this->m_scene.getDataBuffers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverTextureBuffers)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateTextureBuffer(EPixelStorageFormat::R16F, MipMapDimensions{ {1, 1} }, TextureBufferHandle{}); };
        auto releaseF = [scene = &this->m_scene](TextureBufferHandle textureBuffer) { scene->releaseTextureBuffer(textureBuffer); };
        this->runTest(allocateF, releaseF, this->m_scene.getTextureBuffers());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverDataSlots)
    {
        const DataSlot dataSlot{ EDataSlotType::TransformationConsumer, DataSlotId{6u}, NodeHandle{5u}, DataInstanceHandle{1u}, ResourceContentHash{1u, 0u}, TextureSamplerHandle{1u} };

        auto allocateF = [scene = &this->m_scene, dataSlot] { return scene->allocateDataSlot(dataSlot, DataSlotHandle{}); };
        auto releaseF = [scene = &this->m_scene](DataSlotHandle handle) { scene->releaseDataSlot(handle); };
        this->runTest(allocateF, releaseF, this->m_scene.getDataSlots());
    }

    TYPED_TEST(AnIteratableScene, CanIterateOverSceneReferences)
    {
        auto allocateF = [scene = &this->m_scene] { return scene->allocateSceneReference(SceneId{123u}, SceneReferenceHandle{}); };
        auto releaseF = [scene = &this->m_scene](SceneReferenceHandle sceneReference) { scene->releaseSceneReference(sceneReference); };
        this->runTest(allocateF, releaseF, this->m_scene.getSceneReferences());
    }
}
