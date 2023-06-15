//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererResourceManagerMock.h"
#include "RenderBackendMock.h"
#include "RenderExecutor.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/RendererScenes.h"
#include "SceneUtils/DataLayoutCreationHelper.h"
#include "RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Components/EffectUniformTime.h"
#include "MockResourceHash.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace ramses_internal {
using namespace testing;

namespace
{
    const float fakeFieldOfView = 19.f;
    const float fakeAspectRatio = 0.5f;
    const float fakeNearPlane = 0.1f;
    const float fakeFarPlane = 1500.f;

    const int32_t fakeViewportX = 15;
    const int32_t fakeViewportY = 16;
    const uint32_t fakeViewportWidth = 17u;
    const uint32_t fakeViewportHeight = 18u;

    const uint32_t startIndex = 12u;
    const uint32_t indexCount = 13;
    const uint32_t startVertex = 14u;

    const ExternalBufferHandle fakeExternalBuffer{ 1240u };
    const DeviceResourceHandle fakeExternalTextureDeviceHandle{ 1050u };
}

using DataInstances = std::pair<DataInstanceHandle, DataInstanceHandle>;

// This Matrix comparison matcher is needed to compare the MVP etc matrices. It must allow
// a relatively high error because heavy optimizations on some platforms may lead to significant
// differences in precision (likely because values are kept in 80bit FP registers)
MATCHER_P(PermissiveMatrixEq, other, "")
{
    UNUSED(result_listener);
    for (glm::length_t i = 0; i < arg.length(); ++i)
    {
        for (glm::length_t k = 0; k < arg[i].length(); ++k)
        {
            const auto a = arg[i][k];
            const auto b = other[i][k];

            const float fDelta = std::abs(a - b);
            if (fDelta > 1e-5f)
            {
                float relativeError = 0.f;
                if (std::abs(b) > std::abs(a))
                {
                    relativeError = std::abs((a - b) / b);
                }
                else
                {
                    relativeError = std::abs((a - b) / a);
                }
                if (relativeError > 1.e-7f)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

static int32_t GetElapsed(int32_t start, int32_t end)
{
    if (start == end)
        return 0;
    if (start < end)
    {
        return end - start;
    }
    else
    {
        constexpr auto limit = std::numeric_limits<int32_t>::max();
        return (limit - start) + end + 1;
    }
}

MATCHER_P(TimeEq, expectedTime, "")
{
    UNUSED(result_listener);
    const int32_t tolerance = 10000; // tolerate stuck during test execution
    return GetElapsed(expectedTime, arg) < tolerance;
}

class FakeEffectInputs
{
public:
    explicit FakeEffectInputs(bool withTimeMs)
    {
        uniformInputs.push_back(EffectInputInformation("dataRefField1", 1, EDataType::Float, EFixedSemantics::Invalid));
        dataRefField1 = DataFieldHandle(0);
        uniformInputs.push_back(EffectInputInformation("fieldModelMatrix", 1, EDataType::Matrix44F, EFixedSemantics::ModelMatrix));
        fieldModelMatrix = DataFieldHandle(1);
        uniformInputs.push_back(EffectInputInformation("fieldViewMatrix", 1, EDataType::Matrix44F, EFixedSemantics::ViewMatrix));
        fieldViewMatrix  = DataFieldHandle(2);
        uniformInputs.push_back(EffectInputInformation("fieldProjMatrix", 1, EDataType::Matrix44F, EFixedSemantics::ProjectionMatrix));
        fieldProjMatrix  = DataFieldHandle(3);
        uniformInputs.push_back(EffectInputInformation("textureField", 1, EDataType::TextureSampler2D, EFixedSemantics::Invalid));
        textureField  = DataFieldHandle(4);
        uniformInputs.push_back(EffectInputInformation("textureFieldMS", 1, EDataType::TextureSampler2DMS, EFixedSemantics::Invalid));
        textureFieldMS = DataFieldHandle(5);
        uniformInputs.push_back(EffectInputInformation("dataRefField2", 1, EDataType::Float, EFixedSemantics::Invalid));
        dataRefField2  = DataFieldHandle(6);
        uniformInputs.push_back(EffectInputInformation("dataRefFieldMatrix22f", 1, EDataType::Matrix22F, EFixedSemantics::Invalid));
        dataRefFieldMatrix22f  = DataFieldHandle(7);
        uniformInputs.push_back(EffectInputInformation("textureFieldExternal", 1, EDataType::TextureSamplerExternal, EFixedSemantics::Invalid));
        textureFieldExternal = DataFieldHandle(8);

        if (withTimeMs)
        {
            uniformInputs.push_back(EffectInputInformation("timeMs", 1, EDataType::Int32, EFixedSemantics::TimeMs));
            dataRefTimeMs  = DataFieldHandle(9);
        }

        attributeInputs.push_back(EffectInputInformation("vertPosField", 1, EDataType::Vector3Buffer, EFixedSemantics::Invalid));
        vertPosField = DataFieldHandle(0);
        attributeInputs.push_back(EffectInputInformation("vertTexcoordField", 1, EDataType::Vector2Buffer, EFixedSemantics::TextTextureCoordinatesAttribute));
        vertTexcoordField = DataFieldHandle(1);
    }

    EffectInputInformationVector uniformInputs;
    EffectInputInformationVector attributeInputs;

    DataFieldHandle vertPosField;
    DataFieldHandle vertTexcoordField;
    DataFieldHandle dataRefField1;
    DataFieldHandle dataRefField2;
    DataFieldHandle dataRefFieldMatrix22f;
    DataFieldHandle dataRefTimeMs;
    DataFieldHandle textureField;
    DataFieldHandle textureFieldMS;
    DataFieldHandle textureFieldExternal;
    DataFieldHandle fieldModelMatrix;
    DataFieldHandle fieldViewMatrix;
    DataFieldHandle fieldProjMatrix;
};


class ARenderExecutorBase: public ::testing::Test
{
public:
    explicit ARenderExecutorBase(bool withTimeMs)
        : device(renderer.deviceMock)
        , renderContext{ DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, fakeViewportWidth, fakeViewportHeight, {}, EClearFlags_All, glm::vec4{0.f}, false }
        , rendererScenes(rendererEventCollector)
        , scene(rendererScenes.createScene(SceneInfo()))
        , sceneAllocator(scene)
        , fakeEffectInputs(withTimeMs)
        , indicesField(0u)
        , vertPosField(1u)
        , vertTexcoordField(2u)
        , textureField            (fakeEffectInputs.textureField           )
        , textureFieldMS          (fakeEffectInputs.textureFieldMS         )
        , textureFieldExternal    (fakeEffectInputs.textureFieldExternal   )
        , fieldModelMatrix        (fakeEffectInputs.fieldModelMatrix       )
        , fieldViewMatrix         (fakeEffectInputs.fieldViewMatrix        )
        , fieldProjMatrix         (fakeEffectInputs.fieldProjMatrix        )
    {
        InputIndexVector referencedInputs;
        scene.preallocateSceneSize(SceneSizeInformation(0u, 0u, 0u, 0u, 0u, 1u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
        uniformLayout = DataLayoutCreationHelper::CreateUniformDataLayoutMatchingEffectInputs(scene, fakeEffectInputs.uniformInputs, referencedInputs, MockResourceHash::EffectHash, DataLayoutHandle(0u));

        DataFieldInfoVector dataFields(3u);
        dataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType::Indices, 1u, EFixedSemantics::Indices);
        dataFields[vertPosField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector3Buffer, 1u, EFixedSemantics::Invalid);
        dataFields[vertTexcoordField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector2Buffer, 1u, EFixedSemantics::TextPositionsAttribute);
        geometryLayout = sceneAllocator.allocateDataLayout(dataFields, MockResourceHash::EffectHash, DataLayoutHandle(2u));
    }

protected:
    enum class EExpectedRenderStateChange
    {
        None,
        CausedByClear,
        All,
    };

    StrictMock<RenderBackendStrictMock> renderer;
    StrictMock<DeviceMock>& device;
    NiceMock<RendererResourceManagerMock> resourceManager;

    RenderingContext renderContext;

    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    RendererCachedScene& scene;
    SceneAllocateHelper sceneAllocator;

    FakeEffectInputs fakeEffectInputs;
    TextureSamplerHandle sampler;
    TextureSamplerHandle samplerMS;
    TextureSamplerHandle samplerExternal;
    const DataFieldHandle indicesField;
    const DataFieldHandle vertPosField;
    const DataFieldHandle vertTexcoordField;
    const DataFieldHandle textureField;
    const DataFieldHandle textureFieldMS;
    const DataFieldHandle textureFieldExternal;
    const DataFieldHandle fieldModelMatrix;
    const DataFieldHandle fieldViewMatrix;
    const DataFieldHandle fieldProjMatrix;
    DataInstanceHandle dataRef1;
    DataInstanceHandle dataRef2;
    DataInstanceHandle dataRefMatrix22f;

    DataLayoutHandle uniformLayout;
    DataLayoutHandle geometryLayout;

    Sequence deviceSequence;

    ProjectionParams getDefaultProjectionParams(ECameraProjectionType cameraProjType = ECameraProjectionType::Perspective)
    {
        if (cameraProjType == ECameraProjectionType::Perspective)
            return ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane);
        return ProjectionParams::Frustum(ECameraProjectionType::Orthographic, -1.f, 1.f, -10.f, 10.f, 1.f, 10.f);
    }

    RenderPassHandle createRenderPassWithCamera(const ProjectionParams& params, const Viewport& viewport = { fakeViewportX, fakeViewportY, fakeViewportWidth, fakeViewportHeight })
    {
        const RenderPassHandle pass = sceneAllocator.allocateRenderPass();
        const NodeHandle cameraNode = sceneAllocator.allocateNode();
        const auto dataLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference} }, {});
        const auto dataInstance = sceneAllocator.allocateDataInstance(dataLayout);
        const auto vpDataRefLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2I} }, {});
        const auto vpOffsetInstance = sceneAllocator.allocateDataInstance(vpDataRefLayout);
        const auto vpSizeInstance = sceneAllocator.allocateDataInstance(vpDataRefLayout);
        const auto frustumPlanesLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector4F} }, {});
        const auto frustumPlanes = sceneAllocator.allocateDataInstance(frustumPlanesLayout);
        const auto frustumNearFarLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2F} }, {});
        const auto frustumNearFar = sceneAllocator.allocateDataInstance(frustumNearFarLayout);
        scene.setDataReference(dataInstance, Camera::ViewportOffsetField, vpOffsetInstance);
        scene.setDataReference(dataInstance, Camera::ViewportSizeField, vpSizeInstance);
        scene.setDataReference(dataInstance, Camera::FrustumPlanesField, frustumPlanes);
        scene.setDataReference(dataInstance, Camera::FrustumNearFarPlanesField, frustumNearFar);
        const CameraHandle camera = sceneAllocator.allocateCamera(params.getProjectionType(), cameraNode, dataInstance);

        scene.setDataSingleVector4f(frustumPlanes, DataFieldHandle{ 0 }, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane });
        scene.setDataSingleVector2f(frustumNearFar, DataFieldHandle{ 0 }, { params.nearPlane, params.farPlane });

        scene.setDataSingleVector2i(vpOffsetInstance, DataFieldHandle{ 0 }, { viewport.posX, viewport.posY });
        scene.setDataSingleVector2i(vpSizeInstance, DataFieldHandle{ 0 }, { int32_t(viewport.width), int32_t(viewport.height) });

        sceneAllocator.allocateTransform(cameraNode);
        scene.setRenderPassCamera(pass, camera);
        return pass;
    }

    [[nodiscard]] TransformHandle findTransformForNode(NodeHandle  node) const
    {
        for (TransformHandle transform(0u); transform < scene.getTransformCount(); ++transform)
        {
            if (scene.isTransformAllocated(transform) && scene.getTransformNode(transform) == node)
            {
                return transform;
            }
        }

        assert(false);
        return TransformHandle::Invalid();
    }

    DataInstances createTestDataInstance(bool setTextureSampler = true, bool setIndexArray = true)
    {
        //create samplers
        sampler = sceneAllocator.allocateTextureSampler({ { EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Nearest_MipMapNearest, ESamplingMethod::Nearest, 2u }, MockResourceHash::TextureHash });
        samplerMS = sceneAllocator.allocateTextureSampler({ {}, MockResourceHash::TextureHash });
        samplerExternal = sceneAllocator.allocateTextureSampler({ {EWrapMethod::Repeat, EWrapMethod::Clamp, EWrapMethod::RepeatMirrored, ESamplingMethod::Nearest_MipMapNearest, ESamplingMethod::Nearest, 1u }, TextureSampler::ContentType::ExternalTexture, ResourceContentHash::Invalid(), fakeExternalBuffer.asMemoryHandle() });

        // create data instance
        DataInstances dataInstances;
        dataInstances.first = sceneAllocator.allocateDataInstance(uniformLayout);
        dataInstances.second = sceneAllocator.allocateDataInstance(geometryLayout);

        // create referenced data instance
        // explicit preallocation needed because here we use DataLayoutCreationHelper which allocates inside,
        // we cannot use scene allocation helper
        MemoryHandle nextHandle = std::max(scene.getDataInstanceCount(), scene.getDataLayoutCount());
        scene.preallocateSceneSize(SceneSizeInformation(0u, 0u, 0u, 0u, 0u, nextHandle + 3u, nextHandle + 3u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
        dataRef1 = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(scene, dataInstances.first, fakeEffectInputs.dataRefField1, EDataType::Float, DataLayoutHandle(nextHandle), DataInstanceHandle(nextHandle));
        dataRef2 = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(scene, dataInstances.first, fakeEffectInputs.dataRefField2, EDataType::Float, DataLayoutHandle(nextHandle + 1u), DataInstanceHandle(nextHandle + 1u));
        dataRefMatrix22f = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(scene, dataInstances.first, fakeEffectInputs.dataRefFieldMatrix22f, EDataType::Matrix22F, DataLayoutHandle(nextHandle + 2u), DataInstanceHandle(nextHandle + 2u));
        scene.setDataSingleFloat(dataRef1, DataFieldHandle(0u), 0.1f);
        scene.setDataSingleFloat(dataRef2, DataFieldHandle(0u), -666.f);
        scene.setDataSingleMatrix22f(dataRefMatrix22f, DataFieldHandle(0u), glm::mat2(1, 2, 3, 4));

        if (setIndexArray)
        {
            scene.setDataResource(dataInstances.second, indicesField, MockResourceHash::IndexArrayHash, DataBufferHandle::Invalid(), 2u, 0u, 0u);
        }
        scene.setDataResource(dataInstances.second, vertPosField, MockResourceHash::VertArrayHash, DataBufferHandle::Invalid(), 3u, 17u, 77u);
        scene.setDataResource(dataInstances.second, vertTexcoordField, MockResourceHash::VertArrayHash2, DataBufferHandle::Invalid(), 4u, 18u, 88u);
        ON_CALL(resourceManager, getResourceDeviceHandle(MockResourceHash::VertArrayHash2)).WillByDefault(Return(DeviceMock::FakeVertexBufferDeviceHandle));

        if (setTextureSampler)
        {
            scene.setDataTextureSamplerHandle(dataInstances.first, textureField, sampler);
            scene.setDataTextureSamplerHandle(dataInstances.first, textureFieldMS, samplerMS);
            scene.setDataTextureSamplerHandle(dataInstances.first, textureFieldExternal, samplerExternal);
        }
        ON_CALL(resourceManager, getExternalBufferDeviceHandle(fakeExternalBuffer)).WillByDefault(Return(fakeExternalTextureDeviceHandle));

        return dataInstances;
    }

    RenderGroupHandle createRenderGroup(RenderPassHandle pass)
    {
        const RenderGroupHandle renderGroup = sceneAllocator.allocateRenderGroup();
        scene.addRenderGroupToRenderPass(pass, renderGroup, 0);
        return renderGroup;
    }

    RenderableHandle createTestRenderable(
        DataInstances dataInstances,
        RenderGroupHandle group = RenderGroupHandle::Invalid(),
        RenderableHandle renderableHandle = RenderableHandle::Invalid())
    {
        const RenderableHandle renderable = sceneAllocator.allocateRenderable(sceneAllocator.allocateNode(), renderableHandle);

        scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, dataInstances.first);
        scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, dataInstances.second);
        scene.setRenderableRenderState(renderable, sceneAllocator.allocateRenderState());

        scene.setRenderableStartIndex(renderable, startIndex);
        scene.setRenderableIndexCount(renderable, indexCount);
        scene.setRenderableStartVertex(renderable, startVertex);

        if (group.isValid())
        {
            scene.addRenderableToRenderGroup(group, renderable, 0);
        }

        return renderable;
    }

    TransformHandle addTransformToNode(NodeHandle node)
    {
        return sceneAllocator.allocateTransform(node);
    }

    TransformHandle addTransformToRenderable(RenderableHandle renderableHandle)
    {
        const NodeHandle node = scene.getRenderable(renderableHandle).node;
        return addTransformToNode(node);
    }

    RenderTargetHandle createRenderTarget(uint32_t width = 800u, uint32_t height = 600u)
    {
        const RenderTargetHandle targetHandle = sceneAllocator.allocateRenderTarget();
        const RenderBufferHandle colorBufferHandle = sceneAllocator.allocateRenderBuffer({ width, height, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });
        const RenderBufferHandle depthBufferHandle = sceneAllocator.allocateRenderBuffer({ width, height, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_WriteOnly, 0u });
        scene.addRenderTargetRenderBuffer(targetHandle, colorBufferHandle);
        scene.addRenderTargetRenderBuffer(targetHandle, depthBufferHandle);

        return targetHandle;
    }

    RenderBufferHandle createRenderbuffer()
    {
        const RenderBufferHandle bufferHandle = sceneAllocator.allocateRenderBuffer({ 10u, 20u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });

        return bufferHandle;
    }

    BlitPassHandle createBlitPass(RenderBufferHandle sourceRenderBuffer, RenderBufferHandle destinationRenderBuffer)
    {
        const BlitPassHandle passHandle = sceneAllocator.allocateBlitPass(sourceRenderBuffer, destinationRenderBuffer);
        return passHandle;
    }

    void expectFrameWithSinglePass(
        RenderableHandle renderable,
        const ProjectionParams& projectionParams,
        glm::mat4 expectedModelMatrix = glm::mat4(1.f),
        glm::mat4 expectedViewMatrix = glm::mat4(1.f))
    {
        expectActivateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, true);
        if (renderContext.displayBufferClearPending != EClearFlags_None)
            expectClearRenderTarget(renderContext.displayBufferClearPending);
        const glm::mat4 projMatrix = CameraMatrixHelper::ProjectionMatrix(projectionParams);
        expectFrameRenderCommands(renderable, expectedModelMatrix, expectedViewMatrix, projMatrix);
    }

    void expectFrameRenderCommands(RenderableHandle /*renderable*/,
        glm::mat4 expectedModelMatrix = glm::mat4(1.f),
        glm::mat4 expectedViewMatrix = glm::mat4(1.f),
        glm::mat4 expectedProjMatrix = glm::mat4(1.f),
        bool expectShaderActivation = true,
        EExpectedRenderStateChange expectRenderStateChanges = EExpectedRenderStateChange::All,
        uint32_t instanceCount = 1u,
        bool expectIndexedRendering = true,
        int32_t expectedUniformTimeMs = 0)
    {
        // TODO violin this is not entirely needed, only need to check that draw call is at the end of the commands
        InSequence seq;

        const DeviceResourceHandle FakeShaderDeviceHandle       = DeviceMock::FakeShaderDeviceHandle      ;
        const DeviceResourceHandle FakeTextureDeviceHandle      = DeviceMock::FakeTextureDeviceHandle     ;

        // RetiresOnSaturation makes it possible to invoke this expect method multiple times without
        // the expectations override each other                                                                                      .RetiresOnSaturation();

        if (expectRenderStateChanges == EExpectedRenderStateChange::All)
        {
            EXPECT_CALL(device, scissorTest(_, _))                                                                                                .InSequence(deviceSequence);
            EXPECT_CALL(device, depthFunc(_))                                                                                                     .InSequence(deviceSequence);
            EXPECT_CALL(device, depthWrite(_))                                                                                                    .InSequence(deviceSequence);
            EXPECT_CALL(device, stencilFunc(_,_,_))                                                                                               .InSequence(deviceSequence);
            EXPECT_CALL(device, stencilOp(_,_,_))                                                                                                 .InSequence(deviceSequence);
            EXPECT_CALL(device, blendOperations(_, _))                                                                                            .InSequence(deviceSequence);
            EXPECT_CALL(device, blendFactors(_, _, _, _))                                                                                         .InSequence(deviceSequence);
            EXPECT_CALL(device, blendColor(_))                                                                                                    .InSequence(deviceSequence);
            EXPECT_CALL(device, colorMask(_, _, _, _))                                                                                            .InSequence(deviceSequence);
            EXPECT_CALL(device, cullMode(_))                                                                                                      .InSequence(deviceSequence);
        }
        else if (expectRenderStateChanges == EExpectedRenderStateChange::CausedByClear)
        {
            EXPECT_CALL(device, scissorTest(_, _)).InSequence(deviceSequence);
            EXPECT_CALL(device, depthWrite(_)).InSequence(deviceSequence);
            EXPECT_CALL(device, colorMask(_, _, _, _)).InSequence(deviceSequence);
        }
        EXPECT_CALL(device, drawMode(_)).InSequence(deviceSequence);

        if (expectShaderActivation)
        {
            EXPECT_CALL(device, activateShader(FakeShaderDeviceHandle))                                                                           .InSequence(deviceSequence);
        }
        EXPECT_CALL(device, activateVertexArray(_))                                                                               .InSequence(deviceSequence);
        EXPECT_CALL(device, setConstant(fakeEffectInputs.dataRefField1, 1, Matcher<const float*>(Pointee(Eq(0.1f)))))                                       .InSequence(deviceSequence);
        EXPECT_CALL(device, setConstant(fieldModelMatrix, 1, Matcher<const glm::mat4*>(Pointee(PermissiveMatrixEq(expectedModelMatrix)))))                  .InSequence(deviceSequence);
        EXPECT_CALL(device, setConstant(fieldViewMatrix, 1, Matcher<const glm::mat4*>(Pointee(PermissiveMatrixEq(expectedViewMatrix)))))                    .InSequence(deviceSequence);
        EXPECT_CALL(device, setConstant(fieldProjMatrix, 1, Matcher<const glm::mat4*>(Pointee(PermissiveMatrixEq(expectedProjMatrix)))))                    .InSequence(deviceSequence);
        EXPECT_CALL(device, activateTexture(FakeTextureDeviceHandle, textureField))                                                                         .InSequence(deviceSequence);
        const TextureSamplerStates expectedSamplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Nearest_MipMapNearest, ESamplingMethod::Nearest, 2u);
        EXPECT_CALL(device, activateTextureSamplerObject(Property(&TextureSamplerStates::hash, Eq(expectedSamplerStates.hash())), textureField)).InSequence(deviceSequence);
        EXPECT_CALL(device, activateTexture(FakeTextureDeviceHandle, textureFieldMS))                                                                         .InSequence(deviceSequence);

        EXPECT_CALL(device, setConstant(fakeEffectInputs.dataRefField2, 1, Matcher<const float*>(Pointee(Eq(-666.f)))))                                     .InSequence(deviceSequence);
        EXPECT_CALL(device, setConstant(fakeEffectInputs.dataRefFieldMatrix22f, 1, Matcher<const glm::mat2*>(Pointee(Eq(glm::mat2(1,2,3,4))))))             .InSequence(deviceSequence);

        EXPECT_CALL(device, activateTexture(fakeExternalTextureDeviceHandle, textureFieldExternal)).InSequence(deviceSequence);
        const TextureSamplerStates expectedSamplerExternalStates(EWrapMethod::Repeat, EWrapMethod::Clamp, EWrapMethod::RepeatMirrored, ESamplingMethod::Nearest_MipMapNearest, ESamplingMethod::Nearest, 1u);
        EXPECT_CALL(device, activateTextureSamplerObject(Property(&TextureSamplerStates::hash, Eq(expectedSamplerExternalStates.hash())), textureFieldExternal)).InSequence(deviceSequence);

        if (fakeEffectInputs.dataRefTimeMs.isValid())
        {
            EXPECT_CALL(device, setConstant(fakeEffectInputs.dataRefTimeMs, 1, Matcher<const int32_t*>(Pointee(TimeEq(expectedUniformTimeMs)))))             .InSequence(deviceSequence);
        }
        if (expectIndexedRendering)
        {
            EXPECT_CALL(device, drawIndexedTriangles(startIndex, indexCount, instanceCount)).InSequence(deviceSequence);
        }
        else
        {
            EXPECT_CALL(device, drawTriangles(startIndex, indexCount, instanceCount)).InSequence(deviceSequence);
        }
    }

    void expectActivateRenderTarget(DeviceResourceHandle rtDeviceHandle, bool expectViewport = true, const Viewport& viewport = Viewport(fakeViewportX, fakeViewportY, fakeViewportWidth, fakeViewportHeight))
    {
        EXPECT_CALL(device, activateRenderTarget(rtDeviceHandle)).InSequence(deviceSequence);
        if (expectViewport)
        {
            EXPECT_CALL(device, setViewport(viewport.posX, viewport.posY, viewport.width, viewport.height)).InSequence(deviceSequence);
        }
    }

    void expectActivateFramebufferRenderTarget(bool expectViewport = true)
    {
        EXPECT_CALL(device, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle)).InSequence(deviceSequence);
        if (expectViewport)
            EXPECT_CALL(device, setViewport(fakeViewportX, fakeViewportY, fakeViewportWidth, fakeViewportHeight)).InSequence(deviceSequence);
    }

    void expectClearRenderTarget(uint32_t clearFlags = EClearFlags_All)
    {
        if (clearFlags & EClearFlags_Color)
        {
            EXPECT_CALL(device, colorMask(true, true, true, true)).InSequence(deviceSequence);
            EXPECT_CALL(device, clearColor(_)).InSequence(deviceSequence);
        }
        if (clearFlags & EClearFlags_Depth)
        {
            EXPECT_CALL(device, depthWrite(EDepthWrite::Enabled)).InSequence(deviceSequence);
        }

        RenderState::ScissorRegion scissorRegion{};
        EXPECT_CALL(device, scissorTest(EScissorTest::Disabled, scissorRegion)).InSequence(deviceSequence);

        EXPECT_CALL(device, clear(clearFlags)).InSequence(deviceSequence);
    }

    void expectDepthStencilDiscard()
    {
        EXPECT_CALL(device, discardDepthStencil()).InSequence(deviceSequence);
    }

    void updateScenes(const RenderableVector& renderablesWithUpdatedVAOs)
    {
        scene.updateRenderablesAndResourceCache(resourceManager);
        scene.updateRenderableVertexArrays(resourceManager, renderablesWithUpdatedVAOs);
        scene.markVertexArraysClean();
        scene.updateRenderableWorldMatrices();
    }

    void expectRenderingWithProjection(RenderableHandle renderable, const glm::mat4& projMatrix, const uint32_t instanceCount = 1u)
    {
        updateScenes({renderable});

        expectActivateFramebufferRenderTarget();
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), projMatrix, true, EExpectedRenderStateChange::All, instanceCount);
        executeScene();

        Mock::VerifyAndClearExpectations(&renderer);
    }

    SceneRenderExecutionIterator executeScene(const FrameTimer* frameTimer = nullptr)
    {
        RenderExecutor executor(device, renderContext, frameTimer);
        return executor.executeScene(scene);
    }
};

class ARenderExecutor : public ARenderExecutorBase
{
public:
    ARenderExecutor()
        : ARenderExecutorBase(false)
    {
    }
};

class ARenderExecutorTimeMs : public ARenderExecutorBase
{
public:
    ARenderExecutorTimeMs()
        : ARenderExecutorBase(true)
    {
    }
};

TEST(CustomMatrixPrinter, PrintsMatrixValuesNotAByteString)
{
    // see matrix_printer.h for more info if this test ever fails
    glm::mat4 mat = glm::translate(glm::vec3(0.6f));
    EXPECT_EQ(std::string("[1,0,0,0.6][0,1,0,0.6][0,0,1,0.6][0,0,0,1]"), ::testing::PrintToString(mat));
}

TEST_F(ARenderExecutor, RendersEmptyFrameForEmptyScene)
{
    updateScenes({});
    executeScene({});
}

TEST_F(ARenderExecutor, RenderRenderPassIntoRenderTarget)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));
    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    scene.setRenderPassClearFlag(pass, ramses_internal::EClearFlags::EClearFlags_None);
    scene.setRenderPassRenderTarget(pass, targetHandle);

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));
    updateScenes({ renderable });
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    {
        InSequence seq;

        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectFrameRenderCommands(renderable, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    }

    executeScene();
}

TEST_F(ARenderExecutor, RenderRenderableWithoutIndexArray)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(true, false), createRenderGroup(pass));
    scene.setRenderPassClearFlag(pass, ramses_internal::EClearFlags::EClearFlags_None);
    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    updateScenes({ renderable });

    {
        InSequence seq;

        expectActivateFramebufferRenderTarget();
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, true, EExpectedRenderStateChange::All, 1, false);
    }

    executeScene();
}

TEST_F(ARenderExecutor, RenderMultipleConsecutiveRenderPassesIntoOneRenderTarget)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));

    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(pass1, targetHandle);

    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass2, EClearFlags_None);
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass2));
    scene.setRenderPassRenderTarget(pass2, targetHandle);

    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);

    updateScenes({ renderable1, renderable2 });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    {
        InSequence seq;

        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
        expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
        expectDepthStencilDiscard();
    }

    executeScene();
}

TEST_F(ARenderExecutor, RenderMultipleRenderPassesIntoMultipleRenderTargets)
{
    const auto projParams1 = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const Viewport fakeVp1(1, 2, 3, 4);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams1, fakeVp1);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));
    const RenderTargetHandle targetHandle1 = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(pass1, targetHandle1);

    const auto projParams2 = getDefaultProjectionParams(ECameraProjectionType::Orthographic);
    const Viewport fakeVp2(5, 6, 7, 8);
    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams2, fakeVp2);
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass2));
    const RenderTargetHandle targetHandle2 = createRenderTarget(17, 21);
    scene.setRenderPassRenderTarget(pass2, targetHandle2);

    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);

    updateScenes({ renderable1, renderable2 });

    const auto expectedProjectionMatrix1 = CameraMatrixHelper::ProjectionMatrix(projParams1);
    const auto expectedProjectionMatrix2 = CameraMatrixHelper::ProjectionMatrix(projParams2);

    const DeviceResourceHandle renderTargetDeviceHandle1 = resourceManager.getRenderTargetDeviceHandle(targetHandle1, scene.getSceneId());
    const DeviceResourceHandle renderTargetDeviceHandle2 = resourceManager.getRenderTargetDeviceHandle(targetHandle2, scene.getSceneId());

    {
        InSequence seq;

        expectActivateRenderTarget(renderTargetDeviceHandle1, true, fakeVp1);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix1);
        expectDepthStencilDiscard();
        expectActivateRenderTarget(renderTargetDeviceHandle2, true, fakeVp2);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix2, false, EExpectedRenderStateChange::CausedByClear);
        expectDepthStencilDiscard();
    }

    executeScene();
}

TEST_F(ARenderExecutor, ResetsCachedRenderStatesAfterClearingRenderTargets)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));

    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass2));
    const RenderTargetHandle targetHandle2 = createRenderTarget(17, 21);
    scene.setRenderPassRenderTarget(pass2, targetHandle2);

    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);

    updateScenes({ renderable1, renderable2 });

    const DeviceResourceHandle renderTargetDeviceHandle2 = resourceManager.getRenderTargetDeviceHandle(targetHandle2, scene.getSceneId());
    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    {
        InSequence seq;

        expectActivateFramebufferRenderTarget();
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), projMatrix);
        expectActivateRenderTarget(renderTargetDeviceHandle2, false);
        expectClearRenderTarget();
        //render states are set again but shader and index buffer do not have to be set again
        expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), projMatrix, false, EExpectedRenderStateChange::CausedByClear);
        expectDepthStencilDiscard();
    }

    executeScene();
}

TEST_F(ARenderExecutor, RenderMultipleRenderPassesIntoOneRenderTargetAndEachClearsWithDifferentFlags)
{
    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);

    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass1, EClearFlags_All);
    scene.setRenderPassRenderTarget(pass1, targetHandle);

    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass2, EClearFlags_Color);
    scene.setRenderPassRenderTarget(pass2, targetHandle);

    const RenderPassHandle pass3 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass3, EClearFlags_Depth);
    scene.setRenderPassRenderTarget(pass3, targetHandle);

    const RenderPassHandle pass4 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass4, EClearFlags_Stencil);
    scene.setRenderPassRenderTarget(pass4, targetHandle);

    const RenderPassHandle pass5 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass5, EClearFlags_Color | EClearFlags_Depth);
    scene.setRenderPassRenderTarget(pass5, targetHandle);

    const RenderPassHandle pass6 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass6, EClearFlags_Color | EClearFlags_Stencil);
    scene.setRenderPassRenderTarget(pass6, targetHandle);

    const RenderPassHandle pass7 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass7, EClearFlags_Depth | EClearFlags_Stencil);
    scene.setRenderPassRenderTarget(pass7, targetHandle);

    const RenderPassHandle pass8 = createRenderPassWithCamera(projParams);
    scene.setRenderPassClearFlag(pass8, EClearFlags_None);
    scene.setRenderPassRenderTarget(pass8, targetHandle);

    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);
    scene.setRenderPassRenderOrder(pass3, 2);
    scene.setRenderPassRenderOrder(pass4, 3);
    scene.setRenderPassRenderOrder(pass5, 4);
    scene.setRenderPassRenderOrder(pass6, 5);
    scene.setRenderPassRenderOrder(pass7, 6);
    scene.setRenderPassRenderOrder(pass8, 7);

    updateScenes({});

    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());
    {
        InSequence seq;
        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectClearRenderTarget(EClearFlags_All);                         // pass 1
        expectClearRenderTarget(EClearFlags_Color);                       // pass 2
        expectClearRenderTarget(EClearFlags_Depth);                       // pass 3
        expectClearRenderTarget(EClearFlags_Stencil);                     // pass 4
        expectClearRenderTarget(EClearFlags_Color | EClearFlags_Depth);   // pass 5
        expectClearRenderTarget(EClearFlags_Color | EClearFlags_Stencil); // pass 6
        expectDepthStencilDiscard();
        expectClearRenderTarget(EClearFlags_Depth | EClearFlags_Stencil); // pass 7
        // pass 8 no clear expectation
        expectDepthStencilDiscard();
    }

    executeScene();
}

TEST_F(ARenderExecutor, DoesNotRenderRenderableIfResourcesInvalid)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams());
    DataInstances InvalidDataInstances;
    createTestRenderable(InvalidDataInstances, createRenderGroup(pass));

    updateScenes({});
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();
    // empty frame

    executeScene();
}

TEST_F(ARenderExecutor, DoesNotRenderRenderableWithNoResourcesAssigned)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams());
    const RenderGroupHandle group = createRenderGroup(pass);
    const RenderableHandle renderable = sceneAllocator.allocateRenderable(sceneAllocator.allocateNode());
    scene.addRenderableToRenderGroup(group, renderable, 0);

    updateScenes({ renderable });
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();
    // empty frame

    executeScene();
}

TEST_F(ARenderExecutor, DoesNotRenderRenderableWithoutDataInstance)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams());

    DataInstances InvalidDataInstances;
    createTestRenderable(InvalidDataInstances, createRenderGroup(pass), RenderableHandle::Invalid());

    updateScenes({});
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();

    executeScene();
}

TEST_F(ARenderExecutor, expectUpdateSceneDefaultMatricesIdentity)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));

    updateScenes({ renderable });
    expectFrameWithSinglePass(renderable, projParams);
    executeScene();

    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, SceneViewMatrixChangeViaCameraTransformation)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));

    updateScenes({ renderable });
    expectFrameWithSinglePass(renderable, projParams);
    executeScene();
    Mock::VerifyAndClearExpectations(&device);

    const NodeHandle cameraNode = scene.getCamera(scene.getRenderPass(pass).camera).node;
    const TransformHandle cameraTransform = findTransformForNode(cameraNode);
    scene.setTranslation(cameraTransform, glm::vec3(2.f, 3.f, 4.f));

    updateScenes({});
    expectFrameWithSinglePass(renderable, projParams, glm::identity<glm::mat4>(), glm::translate(glm::vec3(-2.f, -3.f, -4.f)));
    executeScene();
    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, RendersRenderableWithRendererProjection)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle renderPass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass));

    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    expectRenderingWithProjection(renderable, projMatrix);
}

TEST_F(ARenderExecutor, RendersMultipleRenderableInstancesWithRendererProjection)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle renderPass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass));
    const uint32_t instanceCount = 23u;
    scene.setRenderableInstanceCount(renderable, instanceCount);

    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    expectRenderingWithProjection(renderable, projMatrix, instanceCount);
}

TEST_F(ARenderExecutor, RendersRenderableWithOrthographicProjection)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Orthographic);
    const RenderPassHandle renderPass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass));

    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    expectRenderingWithProjection(renderable, projMatrix);
}

TEST_F(ARenderExecutor, RendersRenderableInTwoPassesUsingTheSameCamera)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle renderPass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle renderPass2 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass1));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass2));
    const CameraHandle camera1 = scene.getRenderPass(renderPass1).camera;
    const CameraHandle camera2 = scene.getRenderPass(renderPass2).camera;
    const NodeHandle cameraNode1 = scene.getCamera(camera1).node;
    const NodeHandle cameraNode2 = scene.getCamera(camera2).node;
    const TransformHandle cameraTransform1 = findTransformForNode(cameraNode1);
    const TransformHandle cameraTransform2 = findTransformForNode(cameraNode2);

    scene.setRenderPassRenderOrder(renderPass1, 0);
    scene.setRenderPassRenderOrder(renderPass2, 1);

    scene.setTranslation(cameraTransform1, glm::vec3(1.f, 2.f, 3.f));
    scene.setTranslation(cameraTransform2, glm::vec3(4.f, 5.f, 6.f));

    updateScenes({ renderable1, renderable2 });
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();

    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::translate(glm::vec3(-1.f, -2.f, -3.f)), projMatrix);
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::translate(glm::vec3(-4.f, -5.f, -6.f)), projMatrix, false, EExpectedRenderStateChange::None);
    executeScene();
    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, RenderStatesAppliedOnceIfSameForMultipleRenderables)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle renderPass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle renderPass2 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass1));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass2));

    updateScenes({ renderable1, renderable2 });

    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();

    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), projMatrix);
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), projMatrix, false, EExpectedRenderStateChange::None);

    executeScene();
    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, RenderStatesAppliedForEachRenderableIfDifferent)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle renderPass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle renderPass2 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass1));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass2));

    const RenderStateHandle state1 = scene.getRenderable(renderable1).renderState;
    const RenderStateHandle state2 = scene.getRenderable(renderable2).renderState;

    scene.setRenderStateScissorTest(state1, EScissorTest::Disabled, {});
    scene.setRenderStateScissorTest(state2, EScissorTest::Enabled, {});
    scene.setRenderStateDepthWrite(state1, EDepthWrite::Enabled);
    scene.setRenderStateDepthWrite(state2, EDepthWrite::Disabled);
    scene.setRenderStateDepthFunc(state1, EDepthFunc::Always);
    scene.setRenderStateDepthFunc(state2, EDepthFunc::Disabled);
    scene.setRenderStateStencilFunc(state1, EStencilFunc::Always, 0u, 0u);
    scene.setRenderStateStencilFunc(state2, EStencilFunc::Equal, 0u, 0u);
    scene.setRenderStateBlendFactors(state1, EBlendFactor::ConstColor, EBlendFactor::ConstColor, EBlendFactor::ConstColor, EBlendFactor::ConstColor);
    scene.setRenderStateBlendFactors(state2, EBlendFactor::DstColor, EBlendFactor::ConstColor, EBlendFactor::ConstColor, EBlendFactor::ConstColor);
    scene.setRenderStateBlendOperations(state1, EBlendOperation::Add, EBlendOperation::Add);
    scene.setRenderStateBlendOperations(state2, EBlendOperation::Subtract, EBlendOperation::Add);
    scene.setRenderStateBlendColor(state1, glm::vec4(1.f));
    scene.setRenderStateBlendColor(state2, glm::vec4(.5f));
    scene.setRenderStateColorWriteMask(state1, EColorWriteFlag_Red);
    scene.setRenderStateColorWriteMask(state2, EColorWriteFlag_Blue);
    scene.setRenderStateCullMode(state1, ECullMode::BackFacing);
    scene.setRenderStateCullMode(state2, ECullMode::Disabled);

    updateScenes({ renderable1, renderable2 });

    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();

    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), projMatrix);
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), projMatrix, false, EExpectedRenderStateChange::All);

    executeScene();
    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, UpdatesModelMatrixWhenChangingTranslationRotationOrScalingOfNode)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));
    const TransformHandle transform = addTransformToRenderable(renderable);

    scene.setTranslation(transform, glm::vec3(0.5f));
    updateScenes({ renderable });
    expectFrameWithSinglePass(renderable, projParams, glm::translate(glm::vec3(0.5f)));
    executeScene();

    Mock::VerifyAndClearExpectations(&device);

    scene.setTranslation(transform, glm::vec3(0.15f));
    scene.setScaling(transform, glm::vec3(0.25f));
    const auto rotation = glm::normalize(glm::quat(1.f, 2.f, 3.f, 4.f));
    scene.setRotation(transform, glm::vec4(rotation.x, rotation.y, rotation.z, rotation.w), ERotationType::Quaternion);
    updateScenes({});
    expectFrameWithSinglePass(renderable, projParams,
        glm::translate(glm::vec3(0.15f)) * glm::scale(glm::vec3(0.25f)) * glm::toMat4(rotation));
    executeScene();
}

TEST_F(ARenderExecutor, AppliesParentTransformationToBothChildNodes)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderGroupHandle group = createRenderGroup(pass);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), group);
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), group);
    const NodeHandle childNode1 = scene.getRenderable(renderable1).node;
    const NodeHandle childNode2 = scene.getRenderable(renderable2).node;
    const NodeHandle parentNode = sceneAllocator.allocateNode();
    scene.addChildToNode(parentNode, childNode1);
    scene.addChildToNode(parentNode, childNode2);
    const TransformHandle transformChild1 = addTransformToNode(childNode1);
    const TransformHandle transformChild2 = addTransformToNode(childNode2);
    const TransformHandle transformParent = addTransformToNode(parentNode);

    scene.setTranslation(transformParent, glm::vec3(0.25f));
    scene.setTranslation(transformChild1, glm::vec3(0.15f));
    scene.setTranslation(transformChild2, glm::vec3(0.05f));
    updateScenes({ renderable1, renderable2 });

    const auto projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();

    expectFrameRenderCommands(renderable1, glm::translate(glm::vec3(0.40f)), glm::identity<glm::mat4>(), projMatrix);
    expectFrameRenderCommands(renderable2, glm::translate(glm::vec3(0.30f)), glm::identity<glm::mat4>(), projMatrix, false, EExpectedRenderStateChange::None);
    executeScene();

    Mock::VerifyAndClearExpectations(&device);

    scene.setTranslation(transformParent, glm::vec3(0.0f));
    updateScenes({});

    expectActivateFramebufferRenderTarget();
    expectFrameRenderCommands(renderable1, glm::translate(glm::vec3(0.15f)), glm::identity<glm::mat4>(), projMatrix);
    expectFrameRenderCommands(renderable2, glm::translate(glm::vec3(0.05f)), glm::identity<glm::mat4>(), projMatrix, false, EExpectedRenderStateChange::None);
    executeScene();

    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, ExecutesBlitPass)
{
    const RenderBufferHandle sourceRenderBuffer = createRenderbuffer();
    const RenderBufferHandle destinationRenderBuffer = createRenderbuffer();
    const BlitPassHandle blitPassHandle = createBlitPass(sourceRenderBuffer, destinationRenderBuffer);
    updateScenes({});

    EXPECT_CALL(device, blitRenderTargets(_, _, _, _, _)).InSequence(deviceSequence);
    executeScene();

    scene.releaseRenderBuffer(sourceRenderBuffer);
    scene.releaseRenderBuffer(destinationRenderBuffer);
    scene.releaseBlitPass(blitPassHandle);
    updateScenes({});
}

TEST_F(ARenderExecutor, ActivatesRenderTargetForRenderPassAfterExecutingBlitPass)
{
    const RenderBufferHandle sourceRenderBuffer = createRenderbuffer();
    const RenderBufferHandle destinationRenderBuffer = createRenderbuffer();
    const BlitPassHandle blitPass = createBlitPass(sourceRenderBuffer, destinationRenderBuffer);
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle firstRenderPass = createRenderPassWithCamera(projParams);
    const RenderPassHandle secondRenderPass = createRenderPassWithCamera(projParams);

    scene.setRenderPassRenderOrder(firstRenderPass  , 0);
    scene.setBlitPassRenderOrder(blitPass           , 1);
    scene.setRenderPassRenderOrder(secondRenderPass , 2);
    updateScenes({});

    {
        //first render pass
        expectActivateFramebufferRenderTarget();
        expectClearRenderTarget();

        //blit pass
        EXPECT_CALL(device, blitRenderTargets(_, _, _, _, _)).InSequence(deviceSequence);

        //second render pass
        expectActivateFramebufferRenderTarget(false);
    }

    executeScene();

    scene.releaseRenderBuffer(sourceRenderBuffer);
    scene.releaseRenderBuffer(destinationRenderBuffer);
    scene.releaseBlitPass(blitPass);
    updateScenes({});
}

TEST_F(ARenderExecutor, willRenderAllRenderablesIfWithinTimeBudget)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    const DataInstances dataInstances = createTestDataInstance();
    const RenderableHandle renderable1 = createTestRenderable(dataInstances, createRenderGroup(pass1));
    const RenderableHandle renderable2 = createTestRenderable(dataInstances, createRenderGroup(pass1));
    const RenderableHandle renderable3 = createTestRenderable(dataInstances, createRenderGroup(pass2));
    const RenderableHandle renderable4 = createTestRenderable(dataInstances, createRenderGroup(pass2));
    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(pass1, targetHandle);
    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);

    updateScenes({ renderable1 ,renderable2, renderable3, renderable4 });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    {
        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
        expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
        expectDepthStencilDiscard();

        expectActivateFramebufferRenderTarget(false);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable3, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::CausedByClear);
        expectFrameRenderCommands(renderable4, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    }

    FrameTimer frameTimer;
    frameTimer.startFrame();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, std::numeric_limits<uint64_t>::max());

    const auto renderIterator = executeScene(&frameTimer);
    EXPECT_EQ(SceneRenderExecutionIterator(), renderIterator); // finished
}

TEST_F(ARenderExecutor, willRenderAtLeastOneRenderableBatchIfExceededTimeBudget)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const DataInstances dataInstances = createTestDataInstance();
    const RenderGroupHandle renderGroup = createRenderGroup(pass1);
    std::array<RenderableHandle, RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks> batchRenderables;
    for (auto& renderable : batchRenderables)
        renderable = createTestRenderable(dataInstances, renderGroup);
    const RenderableHandle renderableOutOfBudget = createTestRenderable(dataInstances, renderGroup);

    updateScenes({ batchRenderables.cbegin(), batchRenderables.cend() });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));

    {
        InSequence s;

        expectActivateFramebufferRenderTarget();
        expectClearRenderTarget();

        // one batch of renderables is rendered, first sets states
        expectFrameRenderCommands(batchRenderables.front(), glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
        for (auto it = batchRenderables.cbegin() + 1; it != batchRenderables.cend(); ++it)
            expectFrameRenderCommands(*it, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);

        // otherRenderable is not rendered
        UNUSED(renderableOutOfBudget);
    }

    FrameTimer frameTimer;
    frameTimer.startFrame();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, 0u);

    const auto renderIterator = executeScene(&frameTimer);
    EXPECT_NE(SceneRenderExecutionIterator(), renderIterator); // not finished
}

TEST_F(ARenderExecutor, willContinueRenderingWhereLeftOffLastRenderWhenInterrupted)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    const DataInstances dataInstances = createTestDataInstance();
    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(pass1, targetHandle);
    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);

    const RenderGroupHandle renderGroup1 = createRenderGroup(pass1);
    const RenderGroupHandle renderGroup2 = createRenderGroup(pass2);
    std::array<RenderableHandle, RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks> batchRenderables1;
    std::array<RenderableHandle, RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks> batchRenderables2;
    std::array<RenderableHandle, RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks> batchRenderables3;
    std::array<RenderableHandle, RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks> batchRenderables4;
    for (auto& renderable : batchRenderables1)
        renderable = createTestRenderable(dataInstances, renderGroup1);
    for (auto& renderable : batchRenderables2)
        renderable = createTestRenderable(dataInstances, renderGroup1);
    for (auto& renderable : batchRenderables3)
        renderable = createTestRenderable(dataInstances, renderGroup2);
    for (auto& renderable : batchRenderables4)
        renderable = createTestRenderable(dataInstances, renderGroup2);

    RenderableVector allRenderables;
    allRenderables.insert(allRenderables.end(), batchRenderables1.cbegin(), batchRenderables1.cend());
    allRenderables.insert(allRenderables.end(), batchRenderables2.cbegin(), batchRenderables2.cend());
    allRenderables.insert(allRenderables.end(), batchRenderables3.cbegin(), batchRenderables3.cend());
    allRenderables.insert(allRenderables.end(), batchRenderables4.cbegin(), batchRenderables4.cend());

    updateScenes(allRenderables);

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    FrameTimer frameTimer;
    frameTimer.startFrame();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, 0u);

    {
        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectClearRenderTarget();
        expectFrameRenderCommands(batchRenderables1.front(), glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
        for (auto it = batchRenderables1.cbegin() + 1; it != batchRenderables1.cend(); ++it)
            expectFrameRenderCommands(*it, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    }
    SceneRenderExecutionIterator renderIterator = executeScene(&frameTimer);
    EXPECT_EQ(0u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(1u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(1u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        expectActivateRenderTarget(renderTargetDeviceHandle); // no clear
        expectFrameRenderCommands(batchRenderables2.front(), glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
        for (auto it = batchRenderables2.cbegin() + 1; it != batchRenderables2.cend(); ++it)
            expectFrameRenderCommands(*it, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    }
    renderContext.renderFrom = renderIterator;
    renderIterator = executeScene(&frameTimer);
    EXPECT_EQ(0u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(2u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(2u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        // last render interrupted at last renderable from pass1, so it continues from there including activating of pass1's RT
        // even though there are no more renderables to render
        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectDepthStencilDiscard();

        expectActivateFramebufferRenderTarget(false);
        expectClearRenderTarget();
        expectFrameRenderCommands(batchRenderables3.front(), glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
        for (auto it = batchRenderables3.cbegin() + 1; it != batchRenderables3.cend(); ++it)
            expectFrameRenderCommands(*it, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    }
    renderContext.renderFrom = renderIterator;
    renderIterator = executeScene(&frameTimer);
    EXPECT_EQ(1u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(1u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(3u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        expectActivateFramebufferRenderTarget();
        expectFrameRenderCommands(batchRenderables4.front(), glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
        for (auto it = batchRenderables4.cbegin() + 1; it != batchRenderables4.cend(); ++it)
            expectFrameRenderCommands(*it, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    }
    renderContext.renderFrom = renderIterator;
    renderIterator = executeScene(&frameTimer);
    EXPECT_EQ(1u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(2u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(4u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        // last render interrupted at last renderable from pass2, so it continues from there including activating of pass2's RT being FB
        // even though there are no more renderables to render
        expectActivateFramebufferRenderTarget();
        // no additional render operation (eg. blit passes)
    }
    renderContext.renderFrom = renderIterator;
    renderIterator = executeScene(&frameTimer);
    EXPECT_EQ(SceneRenderExecutionIterator(), renderIterator); // finished
}

TEST_F(ARenderExecutor, ClearsDispBufferBeforeRenderingIntoIt_MainOnly)
{
    const RenderPassHandle passMain = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(passMain));

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));
    updateScenes({ renderable1 });

    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget(EClearFlags_All);
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);

    executeScene();

    // main disp buffer was cleared, no pending clear in rendering context
    EXPECT_EQ(EClearFlags_None, renderContext.displayBufferClearPending);
}

TEST_F(ARenderExecutor, ClearsDispBufferBeforeRenderingIntoIt_RenderTargetThenMain)
{
    const RenderPassHandle passRT = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderPassHandle passMain = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(passRT));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(passMain));
    const RenderTargetHandle rt = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(passRT, rt);
    scene.setRenderPassRenderOrder(passRT, 1);
    scene.setRenderPassRenderOrder(passMain, 2);

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));
    updateScenes({ renderable1, renderable2 });
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(rt, scene.getSceneId());

    expectActivateRenderTarget(renderTargetDeviceHandle);
    expectClearRenderTarget(EClearFlags_All);
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    expectDepthStencilDiscard();

    expectActivateFramebufferRenderTarget(false);
    expectClearRenderTarget(EClearFlags_All);
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, ARenderExecutor::EExpectedRenderStateChange::CausedByClear);

    executeScene();

    // main disp buffer was cleared, no pending clear in rendering context
    EXPECT_EQ(EClearFlags_None, renderContext.displayBufferClearPending);
}

TEST_F(ARenderExecutor, ClearsDispBufferBeforeRenderingIntoIt_MainThenRenderTarget)
{
    const RenderPassHandle passMain = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderPassHandle passRT = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(passMain));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(passRT));
    const RenderTargetHandle rt = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(passRT, rt);
    scene.setRenderPassRenderOrder(passMain, 1);
    scene.setRenderPassRenderOrder(passRT, 2);

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));
    updateScenes({ renderable1, renderable2 });
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(rt, scene.getSceneId());

    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget(EClearFlags_All);
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    expectActivateRenderTarget(renderTargetDeviceHandle, false);
    expectClearRenderTarget(EClearFlags_All);
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, ARenderExecutor::EExpectedRenderStateChange::CausedByClear);
    expectDepthStencilDiscard();

    executeScene();

    // main disp buffer was cleared, no pending clear in rendering context
    EXPECT_EQ(EClearFlags_None, renderContext.displayBufferClearPending);
}

TEST_F(ARenderExecutor, DoesNotClearDispBufferWhenNotRenderingIntoIt_RenderTargetOnly)
{
    const RenderPassHandle passRT = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(passRT));
    const RenderTargetHandle rt = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(passRT, rt);

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));
    updateScenes({ renderable1 });
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(rt, scene.getSceneId());

    expectActivateRenderTarget(renderTargetDeviceHandle);
    expectClearRenderTarget(EClearFlags_All);
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    expectDepthStencilDiscard();

    executeScene();

    // main disp buffer was NOT cleared, pending clear still set in rendering context
    EXPECT_EQ(EClearFlags_All, renderContext.displayBufferClearPending);
}

TEST_F(ARenderExecutor, ClearsDispBufferBeforeRenderingIntoIt_InterruptedRenderTargetThenMain)
{
    const RenderPassHandle passRT = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const DataInstances dataInstances = createTestDataInstance();
    const RenderTargetHandle rt = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(passRT, rt);

    const RenderGroupHandle renderGroup = createRenderGroup(passRT);
    RenderableVector batchRenderables;
    batchRenderables.resize(RenderExecutor::DefaultNumRenderablesToRenderInBetweenTimeBudgetChecks);
    for (auto& renderable : batchRenderables)
        renderable = createTestRenderable(dataInstances, renderGroup);

    const RenderPassHandle passMain = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderableHandle renderableMain = createTestRenderable(createTestDataInstance(), createRenderGroup(passMain));

    scene.setRenderPassRenderOrder(passRT, 1);
    scene.setRenderPassRenderOrder(passMain, 2);

    auto allRenderables = batchRenderables;
    allRenderables.push_back(renderableMain);
    updateScenes(allRenderables);

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(rt, scene.getSceneId());

    expectActivateRenderTarget(renderTargetDeviceHandle);
    expectClearRenderTarget(EClearFlags_All);

    // one batch of renderables is rendered, first sets states
    expectFrameRenderCommands(batchRenderables.front(), glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    for (auto it = batchRenderables.cbegin() + 1; it != batchRenderables.cend(); ++it)
        expectFrameRenderCommands(*it, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);

    FrameTimer frameTimer;
    frameTimer.startFrame();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, 0u);

    const auto renderIterator = executeScene(&frameTimer);
    EXPECT_NE(SceneRenderExecutionIterator(), renderIterator); // not finished

    // main disp buffer was NOT cleared due to interrupt, pending clear kept in rendering context
    EXPECT_EQ(EClearFlags_All, renderContext.displayBufferClearPending);

    expectActivateRenderTarget(renderTargetDeviceHandle); // will be activated to finish previous pass (although no more renderables to render in it)
    expectDepthStencilDiscard();

    expectActivateFramebufferRenderTarget(false);
    expectClearRenderTarget(EClearFlags_All);
    expectFrameRenderCommands(renderableMain, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);

    renderContext.renderFrom = renderIterator;
    executeScene();

    // main disp buffer was cleared, no pending clear in rendering context
    EXPECT_EQ(EClearFlags_None, renderContext.displayBufferClearPending);
}

TEST_F(ARenderExecutor, discardsDepthStencilAfterLastPassRenderingToFB_ifAllowedByRenderContext)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass2));
    scene.setRenderPassClearFlag(pass2, EClearFlags_None);
    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);

    updateScenes({ renderable1, renderable2 });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    expectDepthStencilDiscard();

    renderContext.displayBufferDepthDiscard = true;
    executeScene();
}

TEST_F(ARenderExecutor, doesNotDiscardDepthStencil_ifNotAllowedByRenderContext)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass2));
    scene.setRenderPassClearFlag(pass2, EClearFlags_None);
    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);

    updateScenes({ renderable1, renderable2 });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    // no discard

    renderContext.displayBufferDepthDiscard = false;
    executeScene();
}

TEST_F(ARenderExecutor, discardsDepthStencilAfterLastPassRenderingToFB_withRTPassInBetween)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass3 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));
    const RenderableHandle renderable2 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass2));
    const RenderableHandle renderable3 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass3));
    scene.setRenderPassClearFlag(pass3, EClearFlags_None);
    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);
    scene.setRenderPassRenderOrder(pass3, 2);

    // second pass uses RT
    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(pass2, targetHandle);

    updateScenes({ renderable1, renderable2, renderable3 });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    // pass1
    expectActivateFramebufferRenderTarget();
    expectClearRenderTarget();
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    // no discard

    // pass2
    expectActivateRenderTarget(renderTargetDeviceHandle, false);
    expectClearRenderTarget();
    expectFrameRenderCommands(renderable2, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::CausedByClear);
    expectDepthStencilDiscard(); // discard for RT

    // pass3
    expectActivateFramebufferRenderTarget(false);
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix, false, EExpectedRenderStateChange::None);
    expectDepthStencilDiscard(); // discard for FB

    renderContext.displayBufferDepthDiscard = true;
    executeScene();
}

TEST_F(ARenderExecutor, discardsDepthStencilWhenRenderToRTWithClear)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));
    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    scene.setRenderPassRenderTarget(pass1, targetHandle);

    updateScenes({ renderable1 });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    expectActivateRenderTarget(renderTargetDeviceHandle);
    expectClearRenderTarget();
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    expectDepthStencilDiscard();

    executeScene();
}

TEST_F(ARenderExecutor, doesNotDiscardDepthStencilWhenRenderToRTWithNoDepthComponent)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable1 = createTestRenderable(createTestDataInstance(), createRenderGroup(pass1));
    const RenderTargetHandle targetHandle = sceneAllocator.allocateRenderTarget();
    const RenderBufferHandle colorBufferHandle = sceneAllocator.allocateRenderBuffer({ 1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });
    scene.addRenderTargetRenderBuffer(targetHandle, colorBufferHandle);
    scene.setRenderPassRenderTarget(pass1, targetHandle);

    updateScenes({ renderable1 });

    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    expectActivateRenderTarget(renderTargetDeviceHandle);
    expectClearRenderTarget();
    expectFrameRenderCommands(renderable1, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), expectedProjectionMatrix);
    // dicard would be here if there was a depth component to discard

    executeScene();
}

TEST_F(ARenderExecutor, discardsDepthStencilWhenRenderToRT_onlyIfNextPassClearsDepthAndStencil)
{
    const RenderTargetHandle targetHandle = createRenderTarget(1, 1);
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);

    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass2 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass3 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass4 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass5 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass6 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass7 = createRenderPassWithCamera(projParams);
    const RenderPassHandle pass8 = createRenderPassWithCamera(projParams);

    scene.setRenderPassRenderTarget(pass1, targetHandle);
    scene.setRenderPassRenderTarget(pass2, targetHandle);
    scene.setRenderPassRenderTarget(pass3, targetHandle);
    scene.setRenderPassRenderTarget(pass4, targetHandle);
    scene.setRenderPassRenderTarget(pass5, targetHandle);
    scene.setRenderPassRenderTarget(pass6, targetHandle);
    // pass7 is FB pass
    scene.setRenderPassRenderTarget(pass8, targetHandle);

    scene.setRenderPassClearFlag(pass1, EClearFlags_None);
    scene.setRenderPassClearFlag(pass2, EClearFlags_Color);
    scene.setRenderPassClearFlag(pass3, EClearFlags_Depth);
    scene.setRenderPassClearFlag(pass4, EClearFlags_Stencil);
    scene.setRenderPassClearFlag(pass5, EClearFlags_Color | EClearFlags_Depth);
    scene.setRenderPassClearFlag(pass6, EClearFlags_Color | EClearFlags_Stencil);
    scene.setRenderPassClearFlag(pass7, EClearFlags_All);
    scene.setRenderPassClearFlag(pass8, EClearFlags_Depth | EClearFlags_Stencil);

    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setRenderPassRenderOrder(pass2, 1);
    scene.setRenderPassRenderOrder(pass3, 2);
    scene.setRenderPassRenderOrder(pass4, 3);
    scene.setRenderPassRenderOrder(pass5, 4);
    scene.setRenderPassRenderOrder(pass6, 5);
    scene.setRenderPassRenderOrder(pass7, 6);
    scene.setRenderPassRenderOrder(pass8, 7);

    updateScenes({});

    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());
    expectActivateRenderTarget(renderTargetDeviceHandle);
    // pass 1, no clear
    expectClearRenderTarget(EClearFlags_Color);                       // pass 2
    expectClearRenderTarget(EClearFlags_Depth);                       // pass 3
    expectClearRenderTarget(EClearFlags_Stencil);                     // pass 4
    expectClearRenderTarget(EClearFlags_Color | EClearFlags_Depth);   // pass 5
    expectClearRenderTarget(EClearFlags_Color | EClearFlags_Stencil); // pass 6
    expectDepthStencilDiscard(); // next pass (pass8) rendering to RT clears depth+stencil, can discard

    // pass 7 to FB
    expectActivateFramebufferRenderTarget(false);
    expectClearRenderTarget();
    expectDepthStencilDiscard(); // no other pass rendering to FB, can discard

    // pass 8
    expectActivateRenderTarget(renderTargetDeviceHandle, false);
    expectClearRenderTarget(EClearFlags_Depth | EClearFlags_Stencil);
    // next pass is pass1 next frame which does not clear, therefore cannot discard here even though at end of frame

    renderContext.displayBufferDepthDiscard = true;
    executeScene();
}

TEST_F(ARenderExecutor, doesNotDiscardDepthStencilIfUpcomingPassUsesItAsBlitPassSource)
{
    const RenderTargetHandle targetHandle = createRenderTarget(1, 1);
    const RenderBufferHandle depthBuffer = scene.getRenderTargetRenderBuffer(targetHandle, 1u);
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);

    const RenderPassHandle pass1 = createRenderPassWithCamera(projParams);
    const BlitPassHandle pass2 = createBlitPass(depthBuffer, createRenderbuffer());
    const RenderPassHandle pass3 = createRenderPassWithCamera(projParams);

    scene.setRenderPassRenderTarget(pass1, targetHandle);
    scene.setRenderPassRenderTarget(pass3, targetHandle);

    scene.setRenderPassClearFlag(pass1, EClearFlags_All);
    scene.setRenderPassClearFlag(pass3, EClearFlags_All);

    scene.setRenderPassRenderOrder(pass1, 0);
    scene.setBlitPassRenderOrder(pass2, 1);
    scene.setRenderPassRenderOrder(pass3, 2);

    updateScenes({});

    // pass1
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());
    expectActivateRenderTarget(renderTargetDeviceHandle);
    expectClearRenderTarget();
    // no discard due to blit using depth as source next

    // pass2 blit
    EXPECT_CALL(device, blitRenderTargets(_, _, _, _, _)).InSequence(deviceSequence);

    // pass3
    expectActivateRenderTarget(renderTargetDeviceHandle, false);
    expectClearRenderTarget();
    expectDepthStencilDiscard();

    executeScene();
}

TEST_F(ARenderExecutorTimeMs, SetsActiveShaderAnimationFlagOnRenderedScene)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(true, false), createRenderGroup(pass));
    scene.setRenderPassClearFlag(pass, ramses_internal::EClearFlags::EClearFlags_None);
    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    updateScenes({ renderable });

    {
        InSequence seq;

        expectActivateFramebufferRenderTarget();
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable,
                                  glm::identity<glm::mat4>(),
                                  glm::identity<glm::mat4>(),
                                  expectedProjectionMatrix,
                                  true,
                                  EExpectedRenderStateChange::All,
                                  1,
                                  false,
                                  EffectUniformTime::GetMilliseconds({}));
    }

    EXPECT_FALSE(scene.hasActiveShaderAnimation());
    executeScene();
    EXPECT_TRUE(scene.hasActiveShaderAnimation());
}

TEST_F(ARenderExecutorTimeMs, UniformTimeMsStartsAtZeroAfterEffectTimeSync)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(true, false), createRenderGroup(pass));
    scene.setRenderPassClearFlag(pass, ramses_internal::EClearFlags::EClearFlags_None);
    const auto expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    scene.setEffectTimeSync(FlushTime::Clock::now());

    updateScenes({ renderable });

    {
        InSequence seq;

        expectActivateFramebufferRenderTarget();
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable,
                                  glm::identity<glm::mat4>(),
                                  glm::identity<glm::mat4>(),
                                  expectedProjectionMatrix,
                                  true,
                                  EExpectedRenderStateChange::All,
                                  1,
                                  false,
                                  0);
    }

    executeScene();
}

}
