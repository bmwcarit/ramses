//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestSceneHelper.h"

#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "internal/SceneGraph/SceneAPI/Camera.h"
#include "DeviceMock.h"
#include "MockResourceHash.h"

namespace ramses::internal
{
    TestSceneHelper::TestSceneHelper(IScene& scene, bool indexArrayAvailable, bool createDefaultLayouts)
        : m_scene(scene)
        , m_sceneAllocator(m_scene)
        , m_sceneID(scene.getSceneId())
        , m_indexArrayAvailable(indexArrayAvailable)
    {
        if (m_indexArrayAvailable)
        {
            ON_CALL(resourceManager, getResourceDeviceHandle(MockResourceHash::IndexArrayHash)).WillByDefault(Return(DeviceMock::FakeIndexBufferDeviceHandle));
        }
        else
        {
            ON_CALL(resourceManager, getResourceDeviceHandle(MockResourceHash::IndexArrayHash)).WillByDefault(Return(DeviceResourceHandle::Invalid()));
        }

        if (createDefaultLayouts)
        {
            DataFieldInfoVector geometryDataFields(2u);
            geometryDataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType::Indices, 1u, EFixedSemantics::Indices);
            geometryDataFields[vertAttribField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector3Buffer, 1u, EFixedSemantics::Invalid);
            m_sceneAllocator.allocateDataLayout(geometryDataFields, MockResourceHash::EffectHash, testGeometryLayout);

            DataFieldInfoVector uniformDataFields(2u);
            uniformDataFields[dataField.asMemoryHandle()] = DataFieldInfo(EDataType::Float);
            uniformDataFields[samplerField.asMemoryHandle()] = DataFieldInfo(EDataType::TextureSampler2D);
            m_sceneAllocator.allocateDataLayout(uniformDataFields, MockResourceHash::EffectHash, testUniformLayout);
        }
    }

    RenderGroupHandle TestSceneHelper::createRenderGroup(RenderPassHandle pass1, RenderPassHandle pass2)
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

    RenderableHandle TestSceneHelper::createRenderable(RenderGroupHandle group1, RenderGroupHandle group2, const std::unordered_set<EFixedSemantics>& semantics, RenderableHandle handle)
    {
        const NodeHandle nodeHandle = m_sceneAllocator.allocateNode();
        const RenderableHandle renderableHandle = m_sceneAllocator.allocateRenderable(nodeHandle, handle);

        if (group1.isValid())
        {
            m_scene.addRenderableToRenderGroup(group1, renderableHandle, 0);
        }
        if (group2.isValid())
        {
            m_scene.addRenderableToRenderGroup(group2, renderableHandle, 0);
        }

        if (!semantics.empty())
        {
            DataFieldInfoVector uniformDataFields;
            for (const auto s : semantics)
                uniformDataFields.push_back(DataFieldInfo{ EDataType::Matrix44F, 1u, s });
            const auto uniformDataLayoutHandle = m_sceneAllocator.allocateDataLayout(uniformDataFields, MockResourceHash::EffectHash, {});
            const auto uniformDataInstanceHandle = m_sceneAllocator.allocateDataInstance(uniformDataLayoutHandle);
            m_scene.setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, uniformDataInstanceHandle);
        }

        return renderableHandle;
    }

    void TestSceneHelper::removeRenderable(RenderableHandle renderable, RenderGroupHandle group1, RenderGroupHandle group2)
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

    CameraHandle TestSceneHelper::createCamera(ECameraProjectionType projectionType, vec2f frustumNearFar, vec4f frustumPlanes, vec2f viewportOffset, vec2f viewportSize, CameraHandle handle)
    {
        const DataFieldInfoVector dataRefFiels(4u, DataFieldInfo{ EDataType::DataReference });
        const auto dataLayout = m_sceneAllocator.allocateDataLayout(dataRefFiels, {}, {});
        const auto dataInstance = m_sceneAllocator.allocateDataInstance(dataLayout, {});
        const auto viewportDataReferenceLayout = m_sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2I} }, {}, {});
        const auto viewportOffsetDataReference = m_sceneAllocator.allocateDataInstance(viewportDataReferenceLayout, {});
        const auto viewportSizeDataReference = m_sceneAllocator.allocateDataInstance(viewportDataReferenceLayout, {});
        const auto frustumPlanesDataReferenceLayout = m_sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector4F} }, {}, {});
        const auto frustumPlanesDataReference = m_sceneAllocator.allocateDataInstance(frustumPlanesDataReferenceLayout, {});
        const auto frustumNearFarDataReferenceLayout = m_sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2F} }, {}, {});
        const auto frustumNearFarDataReference = m_sceneAllocator.allocateDataInstance(frustumNearFarDataReferenceLayout, {});

        m_scene.setDataReference(dataInstance, Camera::ViewportOffsetField, viewportOffsetDataReference);
        m_scene.setDataReference(dataInstance, Camera::ViewportSizeField, viewportSizeDataReference);
        m_scene.setDataReference(dataInstance, Camera::FrustumPlanesField, frustumPlanesDataReference);
        m_scene.setDataReference(dataInstance, Camera::FrustumNearFarPlanesField, frustumNearFarDataReference);

        m_scene.setDataSingleVector2f(frustumNearFarDataReference, DataFieldHandle{ 0 }, frustumNearFar);
        m_scene.setDataSingleVector4f(frustumPlanesDataReference, DataFieldHandle{ 0 }, frustumPlanes);
        m_scene.setDataSingleVector2i(viewportOffsetDataReference, DataFieldHandle{ 0 }, viewportOffset);
        m_scene.setDataSingleVector2i(viewportSizeDataReference, DataFieldHandle{ 0 }, viewportSize);

        const auto nodeHandle = m_sceneAllocator.allocateNode(0u, {});

        return m_sceneAllocator.allocateCamera(projectionType, nodeHandle, dataInstance, handle);
    }

    CameraHandle TestSceneHelper::createCamera(const ProjectionParams& params, vec2f viewportOffset, vec2f viewportSize, CameraHandle handle)
    {
        return createCamera(params.getProjectionType(),
            { params.nearPlane, params.farPlane },
            { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane },
            viewportOffset,
            viewportSize, handle);
    }

    RenderPassHandle TestSceneHelper::createRenderPassWithCamera()
    {
        const RenderPassHandle pass = m_sceneAllocator.allocateRenderPass();
        const CameraHandle camera = createCamera();
        m_scene.setRenderPassCamera(pass, camera);
        return pass;
    }

    BlitPassHandle TestSceneHelper::createBlitPassWithDummyRenderBuffers()
    {
        const RenderBufferHandle sourceRenderBuffer = m_sceneAllocator.allocateRenderBuffer({ 16u, 12u, EPixelStorageFormat::R8, ERenderBufferAccessMode::ReadWrite, 0u });
        const RenderBufferHandle destinationRenderBuffer = m_sceneAllocator.allocateRenderBuffer({ 16u, 12u, EPixelStorageFormat::R8, ERenderBufferAccessMode::ReadWrite, 0u });
        const BlitPassHandle pass = m_sceneAllocator.allocateBlitPass(sourceRenderBuffer, destinationRenderBuffer);

        return pass;
    }

    void TestSceneHelper::createRenderTarget()
    {
        m_sceneAllocator.allocateRenderTarget(renderTarget);
        m_sceneAllocator.allocateRenderBuffer({ 16u, 12u, EPixelStorageFormat::R8, ERenderBufferAccessMode::ReadWrite, 0u }, renderTargetColorBuffer);
        m_scene.addRenderTargetRenderBuffer(renderTarget, renderTargetColorBuffer);
    }

    template <typename TextureContentHandle>
    TextureSamplerHandle TestSceneHelper::createTextureSampler(TextureContentHandle handleOrHash)
    {
        return m_sceneAllocator.allocateTextureSampler({ {}, handleOrHash });
    }
    template TextureSamplerHandle TestSceneHelper::createTextureSampler<TextureBufferHandle>(TextureBufferHandle handleOrHash);
    template TextureSamplerHandle TestSceneHelper::createTextureSampler<RenderBufferHandle>(RenderBufferHandle handleOrHash);
    template TextureSamplerHandle TestSceneHelper::createTextureSampler<ResourceContentHash>(ResourceContentHash handleOrHash);

    TextureSamplerHandle TestSceneHelper::createTextureSamplerWithFakeTexture()
    {
        return createTextureSampler(MockResourceHash::TextureHash);
    }

    DataInstanceHandle TestSceneHelper::createAndAssignUniformDataInstance(RenderableHandle renderable, TextureSamplerHandle sampler)
    {
        const DataInstanceHandle uniformData = m_sceneAllocator.allocateDataInstance(testUniformLayout);
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformData);
        m_scene.setDataTextureSamplerHandle(uniformData, samplerField, sampler);

        return uniformData;
    }

    DataInstanceHandle TestSceneHelper::createAndAssignVertexDataInstance(RenderableHandle renderable)
    {
        const DataInstanceHandle geometryData = m_sceneAllocator.allocateDataInstance(testGeometryLayout);
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geometryData);

        return geometryData;
    }

    void TestSceneHelper::setResourcesToRenderable(
        RenderableHandle renderable,
        bool setVertices,
        bool setIndices)
    {
        auto vertexData = m_scene.getRenderable(renderable).dataInstances[ERenderableDataSlotType_Geometry];
        if (setVertices)
            m_scene.setDataResource(vertexData, vertAttribField, MockResourceHash::VertArrayHash, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        if (setIndices)
            m_scene.setDataResource(vertexData, indicesField, MockResourceHash::IndexArrayHash, DataBufferHandle::Invalid(), 0u, 0u, 0u);
    }

    template <typename TextureContentHandle>
    void TestSceneHelper::recreateSamplerWithDifferentContent(TextureSamplerHandle handle, TextureContentHandle contentHandleOrHash)
    {
        const auto& samplerStates = m_scene.getTextureSampler(handle).states;
        m_scene.releaseTextureSampler(handle);
        m_scene.allocateTextureSampler({ samplerStates, contentHandleOrHash }, handle);
        [[maybe_unused]] bool hasDataSlot = false;
        for (DataSlotHandle d(0u); d < m_scene.getDataSlotCount(); ++d)
            hasDataSlot |= (m_scene.isDataSlotAllocated(d) && m_scene.getDataSlot(d).attachedTextureSampler == handle);
        assert(!hasDataSlot && "Recreating sampler that had data slot assigned, data slot must be recreated as well");
    }

    template void TestSceneHelper::recreateSamplerWithDifferentContent<RenderBufferHandle>(TextureSamplerHandle handle, RenderBufferHandle contentHandleOrHash);
    template void TestSceneHelper::recreateSamplerWithDifferentContent<ResourceContentHash>(TextureSamplerHandle handle, ResourceContentHash contentHandleOrHash);
}
