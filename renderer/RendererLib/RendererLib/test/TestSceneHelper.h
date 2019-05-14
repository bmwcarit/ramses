//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTSCENEHELPER_H
#define RAMSES_TESTSCENEHELPER_H

#include "renderer_common_gmock_header.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/Renderable.h"
#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/TextureSampler.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererResourceManagerMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include "DeviceMock.h"
#include "ResourceProviderMock.h"
#include "SceneAllocateHelper.h"

namespace ramses_internal
{
    class TestSceneHelper
    {
    public:
        explicit TestSceneHelper(IScene& scene, bool indexArrayAvailable = true)
            : m_scene(scene)
            , m_sceneAllocator(m_scene)
            , m_sceneID(scene.getSceneId())
            , m_indexArrayAvailable(indexArrayAvailable)
        {
            if (m_indexArrayAvailable)
                ON_CALL(resourceManager, getClientResourceDeviceHandle(ResourceProviderMock::FakeIndexArrayHash)).WillByDefault(Return(DeviceMock::FakeIndexBufferDeviceHandle));
            else
                ON_CALL(resourceManager, getClientResourceDeviceHandle(ResourceProviderMock::FakeIndexArrayHash)).WillByDefault(Return(DeviceResourceHandle::Invalid()));

            DataFieldInfoVector geometryDataFields(2u);
            geometryDataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType_Indices, 1u, EFixedSemantics_Indices);
            geometryDataFields[vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType_Vector3Buffer, 1u, EFixedSemantics_VertexPositionAttribute);
            m_sceneAllocator.allocateDataLayout(geometryDataFields, testGeometryLayout);

            DataFieldInfoVector uniformDataFields(2u);
            uniformDataFields[dataField.asMemoryHandle()] = DataFieldInfo(EDataType_Float);
            uniformDataFields[samplerField.asMemoryHandle()] = DataFieldInfo(EDataType_TextureSampler);
            m_sceneAllocator.allocateDataLayout(uniformDataFields, testUniformLayout);
        }

        RenderGroupHandle createRenderGroup(RenderPassHandle pass1 = RenderPassHandle::Invalid(), RenderPassHandle pass2 = RenderPassHandle::Invalid())
        {
            const RenderGroupHandle renderGroupHandle = m_sceneAllocator.allocateRenderGroup();
            if (pass1.isValid())
            {
                m_scene.addRenderGroupToRenderPass(pass1, renderGroupHandle, 0);
            }
            if (pass2.isValid())
            {
                m_scene.addRenderGroupToRenderPass(pass2, renderGroupHandle, 0);
            }

            return renderGroupHandle;
        }

        RenderableHandle createRenderable(RenderGroupHandle group1 = RenderGroupHandle::Invalid(), RenderGroupHandle group2 = RenderGroupHandle::Invalid())
        {
            const NodeHandle nodeHandle = m_sceneAllocator.allocateNode();
            const RenderableHandle renderableHandle = m_sceneAllocator.allocateRenderable(nodeHandle);

            if (group1.isValid())
            {
                m_scene.addRenderableToRenderGroup(group1, renderableHandle, 0);
            }
            if (group2.isValid())
            {
                m_scene.addRenderableToRenderGroup(group2, renderableHandle, 0);
            }

            return renderableHandle;
        }

        void removeRenderable(RenderableHandle renderable, RenderGroupHandle group1 = RenderGroupHandle::Invalid(), RenderGroupHandle group2 = RenderGroupHandle::Invalid())
        {
            m_scene.releaseRenderable(renderable);
            if (group1.isValid())
            {
                m_scene.removeRenderableFromRenderGroup(group1, renderable);
            }
            if (group2.isValid())
            {
                m_scene.removeRenderableFromRenderGroup(group2, renderable);
            }
        }

        RenderPassHandle createRenderPassWithCamera()
        {
            const RenderPassHandle pass = m_sceneAllocator.allocateRenderPass();
            const auto dataLayout = m_sceneAllocator.allocateDataLayout({{EDataType_Vector2I}, {EDataType_Vector2I}});
            const CameraHandle camera = m_sceneAllocator.allocateCamera(ECameraProjectionType_Renderer, m_sceneAllocator.allocateNode(), m_sceneAllocator.allocateDataInstance(dataLayout));
            m_scene.setRenderPassCamera(pass, camera);
            return pass;
        }

        BlitPassHandle createBlitPassWithDummyRenderBuffers()
        {
            const RenderBufferHandle sourceRenderBuffer = m_sceneAllocator.allocateRenderBuffer({ 16u, 12u, ERenderBufferType_ColorBuffer, ETextureFormat_R8, ERenderBufferAccessMode_ReadWrite, 0u });
            const RenderBufferHandle destinationRenderBuffer = m_sceneAllocator.allocateRenderBuffer({ 16u, 12u, ERenderBufferType_ColorBuffer, ETextureFormat_R8, ERenderBufferAccessMode_ReadWrite, 0u });
            const BlitPassHandle pass = m_sceneAllocator.allocateBlitPass(sourceRenderBuffer, destinationRenderBuffer);

            return pass;
        }

        void createRenderTarget()
        {
            m_sceneAllocator.allocateRenderTarget(renderTarget);
            m_sceneAllocator.allocateRenderBuffer({ 16u, 12u, ERenderBufferType_ColorBuffer, ETextureFormat_R8, ERenderBufferAccessMode_ReadWrite, 0u }, renderTargetColorBuffer);
            m_scene.addRenderTargetRenderBuffer(renderTarget, renderTargetColorBuffer);
        }

        template <typename TextureContentHandle>
        TextureSamplerHandle createTextureSampler(TextureContentHandle handleOrHash)
        {
            return m_sceneAllocator.allocateTextureSampler({ {}, handleOrHash });
        }

        TextureSamplerHandle createTextureSamplerWithFakeClientTexture()
        {
            return createTextureSampler(ResourceProviderMock::FakeTextureHash);
        }

        DataInstanceHandle createAndAssignUniformDataInstance(RenderableHandle renderable, TextureSamplerHandle sampler)
        {
            const DataInstanceHandle uniformData = m_sceneAllocator.allocateDataInstance(testUniformLayout);
            m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformData);
            m_scene.setDataTextureSamplerHandle(uniformData, samplerField, sampler);

            return uniformData;
        }

        DataInstanceHandle createAndAssignVertexDataInstance(RenderableHandle renderable)
        {
            const DataInstanceHandle geometryData = m_sceneAllocator.allocateDataInstance(testGeometryLayout);
            m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geometryData);

            return geometryData;
        }

        void setResourcesToRenderable(
            RenderableHandle renderable,
            bool setEffect = true,
            bool setVertices = true,
            bool setIndices = true)
        {
            const auto vertexData = m_scene.getRenderable(renderable).dataInstances[ERenderableDataSlotType_Geometry];

            if (setEffect)
                m_scene.setRenderableEffect(renderable, ResourceProviderMock::FakeEffectHash);
            if (setVertices)
                m_scene.setDataResource(vertexData, vertAttribField, ResourceProviderMock::FakeVertArrayHash, DataBufferHandle::Invalid(), 0u);
            if (setIndices)
                m_scene.setDataResource(vertexData, indicesField, ResourceProviderMock::FakeIndexArrayHash, DataBufferHandle::Invalid(), 0u);
        }

        template <typename TextureContentHandle>
        void recreateSamplerWithDifferentContent(TextureSamplerHandle handle, TextureContentHandle contentHandleOrHash)
        {
            const auto& samplerStates = m_scene.getTextureSampler(handle).states;
            m_scene.releaseTextureSampler(handle);
            m_scene.allocateTextureSampler({ samplerStates, contentHandleOrHash }, handle);
            bool hasDataSlot = false;
            for (DataSlotHandle d(0u); d < m_scene.getDataSlotCount(); ++d)
                hasDataSlot |= (m_scene.isDataSlotAllocated(d) && m_scene.getDataSlot(d).attachedTextureSampler == handle);
            assert(!hasDataSlot && "Recreating sampler that had data slot assigned, data slot must be recreated as well");
        }

    public:
        IScene& m_scene;
        SceneAllocateHelper m_sceneAllocator;
        const SceneId m_sceneID;
        StrictMock<EmbeddedCompositingManagerMock> embeddedCompositingManager;
        StrictMock<RendererResourceManagerMock>    resourceManager;

        const DataLayoutHandle testUniformLayout            { 0u };
        const DataLayoutHandle testGeometryLayout           { 2u };
        const DataFieldHandle indicesField                  { 0u };
        const DataFieldHandle vertAttribField               { 1u };
        const DataFieldHandle dataField                     { 0u };
        const DataFieldHandle samplerField                  { 1u };
        const RenderTargetHandle renderTarget               { 13u };
        const RenderBufferHandle renderTargetColorBuffer    { 5u };
        const StreamTextureHandle streamTexture             { 6u };

    private:
        bool m_indexArrayAvailable;
    };
}

#endif
