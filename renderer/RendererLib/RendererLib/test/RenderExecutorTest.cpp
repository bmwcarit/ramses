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
#include "EmbeddedCompositingManagerMock.h"
#include "RenderExecutor.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/RendererScenes.h"
#include "SceneUtils/DataLayoutCreationHelper.h"
#include "RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "MockResourceHash.h"

namespace ramses_internal {
using namespace testing;

namespace
{
    const Float fakeFieldOfView = 19.f;
    const Float fakeAspectRatio = 0.5f;
    const Float fakeNearPlane = 0.1f;
    const Float fakeFarPlane = 1500.f;

    const Int32 fakeViewportX = 15;
    const Int32 fakeViewportY = 16;
    const UInt32 fakeViewportWidth = 17u;
    const UInt32 fakeViewportHeight = 18u;

    const UInt32 startIndex = 12u;
    const UInt32 indexCount = 13;
    const UInt32 startVertex = 14u;
}

typedef std::pair<DataInstanceHandle, DataInstanceHandle> DataInstances;

// This Matrix comparison matcher is needed to compare the MVP etc matrices. It must allow
// a relatively high error because heavy optimizations on some platforms may lead to significant
// differences in precision (likely because values are kept in 80bit FP registers)
MATCHER_P(PermissiveMatrixEq, other, "")
{
    UNUSED(result_listener);
    const UInt32 numElements = sizeof(arg_type) / sizeof(arg.m11);
    for (UInt32 index = 0; index < numElements; index++)
    {
        const Float a = arg.data[index];
        const Float b = other.data[index];

        const Float fDelta = std::abs(a - b);
        if (fDelta > 1e-5f)
        {
            Float relativeError = 0.f;
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
    return true;
}

class FakeEffectInputs
{
public:
    FakeEffectInputs()
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
    DataFieldHandle textureField;
    DataFieldHandle textureFieldMS;
    DataFieldHandle fieldModelMatrix;
    DataFieldHandle fieldViewMatrix;
    DataFieldHandle fieldProjMatrix;
};


class ARenderExecutor: public ::testing::Test
{
public:
    ARenderExecutor()
        : device(renderer.deviceMock)
        , rendererScenes(rendererEventCollector)
        , scene(rendererScenes.createScene(SceneInfo()))
        , sceneAllocator(scene)
        , indicesField(0u)
        , vertPosField(1u)
        , vertTexcoordField(2u)
        , textureField            (fakeEffectInputs.textureField           )
        , textureFieldMS          (fakeEffectInputs.textureFieldMS         )
        , fieldModelMatrix        (fakeEffectInputs.fieldModelMatrix       )
        , fieldViewMatrix         (fakeEffectInputs.fieldViewMatrix        )
        , fieldProjMatrix         (fakeEffectInputs.fieldProjMatrix        )
    {
        InputIndexVector referencedInputs;
        scene.preallocateSceneSize(SceneSizeInformation(0u, 0u, 0u, 0u, 0u, 1u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
        uniformLayout = DataLayoutCreationHelper::CreateUniformDataLayoutMatchingEffectInputs(scene, fakeEffectInputs.uniformInputs, referencedInputs, MockResourceHash::EffectHash, DataLayoutHandle(0u));

        DataFieldInfoVector dataFields(3u);
        dataFields[indicesField.asMemoryHandle()] = DataFieldInfo(EDataType::Indices, 1u, EFixedSemantics::Indices);
        dataFields[vertPosField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector3Buffer, 1u, EFixedSemantics::Invalid);
        dataFields[vertTexcoordField.asMemoryHandle()] = DataFieldInfo(EDataType::Vector2Buffer, 1u, EFixedSemantics::TextPositionsAttribute);
        geometryLayout = sceneAllocator.allocateDataLayout(dataFields, MockResourceHash::EffectHash, DataLayoutHandle(2u));
    }

protected:
    StrictMock<RenderBackendStrictMock> renderer;
    NiceMock<EmbeddedCompositingManagerMock> embeddedCompositingManager;
    StrictMock<DeviceMock>& device;

    NiceMock<RendererResourceManagerMock> resourceManager;

    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    RendererCachedScene& scene;
    SceneAllocateHelper sceneAllocator;

    FakeEffectInputs fakeEffectInputs;
    TextureSamplerHandle sampler;
    TextureSamplerHandle samplerMS;
    const DataFieldHandle indicesField;
    const DataFieldHandle vertPosField;
    const DataFieldHandle vertTexcoordField;
    const DataFieldHandle textureField;
    const DataFieldHandle textureFieldMS;
    const DataFieldHandle fieldModelMatrix;
    const DataFieldHandle fieldViewMatrix;
    const DataFieldHandle fieldProjMatrix;
    DataInstanceHandle dataRef1;
    DataInstanceHandle dataRef2;
    DataInstanceHandle dataRefMatrix22f;

    DataLayoutHandle uniformLayout;
    DataLayoutHandle geometryLayout;

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
        scene.setDataSingleVector2i(vpSizeInstance, DataFieldHandle{ 0 }, { Int32(viewport.width), Int32(viewport.height) });

        sceneAllocator.allocateTransform(cameraNode);
        scene.setRenderPassCamera(pass, camera);
        return pass;
    }

    TransformHandle findTransformForNode(NodeHandle node) const
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

        // create data instance
        DataInstances dataInstances;
        dataInstances.first = sceneAllocator.allocateDataInstance(uniformLayout);
        dataInstances.second = sceneAllocator.allocateDataInstance(geometryLayout);

        // create referenced data instance
        // explicit preallocation needed because here we use DataLayoutCreationHelper which allocates inside,
        // we cannot use scene allocation helper
        MemoryHandle nextHandle = std::max(scene.getDataInstanceCount(), scene.getDataLayoutCount());
        scene.preallocateSceneSize(SceneSizeInformation(0u, 0u, 0u, 0u, 0u, nextHandle + 3u, nextHandle + 3u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
        dataRef1 = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(scene, dataInstances.first, fakeEffectInputs.dataRefField1, EDataType::Float, DataLayoutHandle(nextHandle), DataInstanceHandle(nextHandle));
        dataRef2 = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(scene, dataInstances.first, fakeEffectInputs.dataRefField2, EDataType::Float, DataLayoutHandle(nextHandle + 1u), DataInstanceHandle(nextHandle + 1u));
        dataRefMatrix22f = ramses_internal::DataLayoutCreationHelper::CreateAndBindDataReference(scene, dataInstances.first, fakeEffectInputs.dataRefFieldMatrix22f, EDataType::Matrix22F, DataLayoutHandle(nextHandle + 2u), DataInstanceHandle(nextHandle + 2u));
        scene.setDataSingleFloat(dataRef1, DataFieldHandle(0u), 0.1f);
        scene.setDataSingleFloat(dataRef2, DataFieldHandle(0u), -666.f);
        scene.setDataSingleMatrix22f(dataRefMatrix22f, DataFieldHandle(0u), Matrix22f(1, 2, 3, 4));

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
            scene.setDataTextureSamplerHandle(dataInstances.first, textureFieldMS, sampler);
        }

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

    RenderTargetHandle createRenderTarget(UInt32 width = 800u, UInt32 height = 600u)
    {
        const RenderTargetHandle targetHandle = sceneAllocator.allocateRenderTarget();
        const RenderBufferHandle bufferHandle = sceneAllocator.allocateRenderBuffer({ width, height, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });
        scene.addRenderTargetRenderBuffer(targetHandle, bufferHandle);

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
        Matrix44f expectedModelMatrix = Matrix44f::Identity,
        Matrix44f expectedViewMatrix = Matrix44f::Identity)
    {
        expectActivateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, true);
        const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projectionParams);
        expectFrameRenderCommands(renderable, expectedModelMatrix, expectedViewMatrix, projMatrix);
    }

    void expectFrameRenderCommands(RenderableHandle /*renderable*/,
        Matrix44f expectedModelMatrix = Matrix44f::Identity,
        Matrix44f expectedViewMatrix = Matrix44f::Identity,
        Matrix44f expectedProjMatrix = Matrix44f::Identity,
        bool expectShaderActivation = true,
        bool expectRenderStateChanges = true,
        bool expectIndexBufferActivation = true,
        UInt32 instanceCount = 1u,
        bool expectIndexedRendering = true)
    {
        // TODO violin this is not entirely needed, only need to check that draw call is at the end of the commands
        InSequence seq;

        const DeviceResourceHandle FakeShaderDeviceHandle       = DeviceMock::FakeShaderDeviceHandle      ;
        const DeviceResourceHandle FakeVertexBufferDeviceHandle = DeviceMock::FakeVertexBufferDeviceHandle;
        const DeviceResourceHandle FakeIndexBufferDeviceHandle  = DeviceMock::FakeIndexBufferDeviceHandle ;
        const DeviceResourceHandle FakeTextureDeviceHandle      = DeviceMock::FakeTextureDeviceHandle     ;

        // RetiresOnSaturation makes it possible to invoke this expect method multiple times without
        // the expectations override each other                                                                                      .RetiresOnSaturation();

        //TODO Mohamed: move those expectations in the if-clause after the caching issue of render states is clarified
        EXPECT_CALL(device, scissorTest(_, _)).RetiresOnSaturation();
        if (expectRenderStateChanges)
        {
            EXPECT_CALL(device, depthFunc(_))                                                                                                     .RetiresOnSaturation();
            EXPECT_CALL(device, depthWrite(_))                                                                                                    .RetiresOnSaturation();
            EXPECT_CALL(device, stencilFunc(_,_,_))                                                                                               .RetiresOnSaturation();
            EXPECT_CALL(device, stencilOp(_,_,_))                                                                                                 .RetiresOnSaturation();
            EXPECT_CALL(device, blendOperations(_, _))                                                                                            .RetiresOnSaturation();
            EXPECT_CALL(device, blendFactors(_, _, _, _))                                                                                         .RetiresOnSaturation();
            EXPECT_CALL(device, blendColor(_))                                                                                                    .RetiresOnSaturation();
            EXPECT_CALL(device, colorMask(_, _, _, _))                                                                                            .RetiresOnSaturation();
            EXPECT_CALL(device, cullMode(_))                                                                                                      .RetiresOnSaturation();
            EXPECT_CALL(device, drawMode(_))                                                                                                      .RetiresOnSaturation();
        }
        if (expectShaderActivation)
        {
            EXPECT_CALL(device, activateShader(FakeShaderDeviceHandle))                                                                           .RetiresOnSaturation();
        }
        EXPECT_CALL(device, activateVertexBuffer(FakeVertexBufferDeviceHandle, fakeEffectInputs.vertPosField, 3u, startVertex, EDataType::Vector3Buffer, 17u, 77u)).RetiresOnSaturation();
        EXPECT_CALL(device, activateVertexBuffer(FakeVertexBufferDeviceHandle, fakeEffectInputs.vertTexcoordField, 4u, startVertex, EDataType::Vector2Buffer, 18u, 88u)).RetiresOnSaturation();
        EXPECT_CALL(device, setConstant(fakeEffectInputs.dataRefField1, 1, Matcher<const Float*>(Pointee(Eq(0.1f)))))                                       .RetiresOnSaturation();
        EXPECT_CALL(device, setConstant(fieldModelMatrix, 1, Matcher<const Matrix44f*>(Pointee(PermissiveMatrixEq(expectedModelMatrix)))))                  .RetiresOnSaturation();
        EXPECT_CALL(device, setConstant(fieldViewMatrix, 1, Matcher<const Matrix44f*>(Pointee(PermissiveMatrixEq(expectedViewMatrix)))))                    .RetiresOnSaturation();
        EXPECT_CALL(device, setConstant(fieldProjMatrix, 1, Matcher<const Matrix44f*>(Pointee(PermissiveMatrixEq(expectedProjMatrix)))))                    .RetiresOnSaturation();
        EXPECT_CALL(device, activateTexture(FakeTextureDeviceHandle, textureField))                                                                         .RetiresOnSaturation();
        EXPECT_CALL(device, setTextureSampling(textureField, EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Nearest_MipMapNearest, ESamplingMethod::Nearest, 2u)).RetiresOnSaturation();
        EXPECT_CALL(device, activateTexture(FakeTextureDeviceHandle, textureFieldMS))                                                                         .RetiresOnSaturation();
        EXPECT_CALL(device, setConstant(fakeEffectInputs.dataRefField2, 1, Matcher<const Float*>(Pointee(Eq(-666.f)))))                                     .RetiresOnSaturation();
        EXPECT_CALL(device, setConstant(fakeEffectInputs.dataRefFieldMatrix22f, 1, Matcher<const Matrix22f*>(Pointee(Eq(Matrix22f(1,2,3,4))))))             .RetiresOnSaturation();
        if (expectIndexBufferActivation)
        {
            EXPECT_CALL(device, activateIndexBuffer(FakeIndexBufferDeviceHandle))                                                                           .RetiresOnSaturation();
        }
        if (expectIndexedRendering)
        {
            EXPECT_CALL(device, drawIndexedTriangles(startIndex, indexCount, instanceCount)).RetiresOnSaturation();
        }
        else
        {
            EXPECT_CALL(device, drawTriangles(startIndex, indexCount, instanceCount)).RetiresOnSaturation();
        }
    }

    void expectActivateRenderTarget(DeviceResourceHandle rtDeviceHandle, bool expectViewport = true, const Viewport& viewport = Viewport(fakeViewportX, fakeViewportY, fakeViewportWidth, fakeViewportHeight))
    {
        EXPECT_CALL(device, activateRenderTarget(rtDeviceHandle)).Times(1);
        if (expectViewport)
        {
            EXPECT_CALL(device, setViewport(viewport.posX, viewport.posY, viewport.width, viewport.height)).Times(1);
        }
    }

    void expectActivateFramebufferRenderTarget(bool expectViewport = true)
    {
        EXPECT_CALL(device, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle)).Times(1);
        if (expectViewport)
            EXPECT_CALL(device, setViewport(fakeViewportX, fakeViewportY, fakeViewportWidth, fakeViewportHeight)).Times(1);
    }

    void expectClearRenderTarget(UInt32 clearFlags = EClearFlags_All)
    {
        if (clearFlags & EClearFlags_Color)
        {
            EXPECT_CALL(device, colorMask(true, true, true, true));
            EXPECT_CALL(device, clearColor(_));
        }
        if (clearFlags & EClearFlags_Depth)
        {
            EXPECT_CALL(device, depthWrite(EDepthWrite::Enabled));
        }

        RenderState::ScissorRegion scissorRegion{};
        EXPECT_CALL(device, scissorTest(EScissorTest::Disabled, scissorRegion));

        EXPECT_CALL(device, clear(clearFlags));
    }

    void updateScenes()
    {
        scene.updateRenderablesAndResourceCache(resourceManager, embeddedCompositingManager);
        scene.updateRenderableWorldMatrices();
    }

    void expectRenderingWithProjection(RenderableHandle renderable, const Matrix44f& projMatrix, const UInt32 instanceCount = 1u)
    {
        updateScenes();

        expectActivateFramebufferRenderTarget();
        expectFrameRenderCommands(renderable, Matrix44f::Identity, Matrix44f::Identity, projMatrix, true, true, true, instanceCount);
        executeScene();

        Mock::VerifyAndClearExpectations(&renderer);
    }

    SceneRenderExecutionIterator executeScene(SceneRenderExecutionIterator renderFrom = {}, const FrameTimer* frameTimer = nullptr)
    {
        const Viewport vp(fakeViewportX, fakeViewportY, fakeViewportWidth, fakeViewportHeight);
        const TargetBufferInfo bufferInfo{ DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, vp.width, vp.height };
        RenderExecutor executor(device, bufferInfo, renderFrom, frameTimer);

        return executor.executeScene(scene);
    }
};

TEST(CustomMatrixPrinter, PrintsMatrixValuesNotAByteString)
{
    // see matrix_printer.h for more info if this test ever fails
    Matrix44f mat = Matrix44f::Translation(Vector3(0.6f));
    EXPECT_EQ(std::string("[1,0,0,0.6][0,1,0,0.6][0,0,1,0.6][0,0,0,1]"), ::testing::PrintToString(mat));
}

TEST_F(ARenderExecutor, RendersEmptyFrameForEmptyScene)
{
    updateScenes();
    executeScene();
}

TEST_F(ARenderExecutor, RenderRenderPassIntoRenderTarget)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams(ECameraProjectionType::Perspective));
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));
    const RenderTargetHandle targetHandle = createRenderTarget(16, 20);
    scene.setRenderPassClearFlag(pass, ramses_internal::EClearFlags::EClearFlags_None);
    scene.setRenderPassRenderTarget(pass, targetHandle);

    const Matrix44f expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));
    updateScenes();
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    {
        InSequence seq;

        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectFrameRenderCommands(renderable, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
    }

    executeScene();
}

TEST_F(ARenderExecutor, RenderRenderableWithoutIndexArray)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(true, false), createRenderGroup(pass));
    scene.setRenderPassClearFlag(pass, ramses_internal::EClearFlags::EClearFlags_None);
    const Matrix44f expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    updateScenes();

    {
        InSequence seq;

        expectActivateFramebufferRenderTarget();
        expectFrameRenderCommands(renderable, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, true, true, false, 1, false);
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

    updateScenes();

    const Matrix44f expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    {
        InSequence seq;

        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable1, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
        expectFrameRenderCommands(renderable2, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);
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

    updateScenes();

    const Matrix44f expectedProjectionMatrix1 = CameraMatrixHelper::ProjectionMatrix(projParams1);
    const Matrix44f expectedProjectionMatrix2 = CameraMatrixHelper::ProjectionMatrix(projParams2);

    const DeviceResourceHandle renderTargetDeviceHandle1 = resourceManager.getRenderTargetDeviceHandle(targetHandle1, scene.getSceneId());
    const DeviceResourceHandle renderTargetDeviceHandle2 = resourceManager.getRenderTargetDeviceHandle(targetHandle2, scene.getSceneId());

    {
        InSequence seq;

        expectActivateRenderTarget(renderTargetDeviceHandle1, true, fakeVp1);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable1, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix1);
        expectActivateRenderTarget(renderTargetDeviceHandle2, true, fakeVp2);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable2, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix2, false, true, false);
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

    updateScenes();

    const DeviceResourceHandle renderTargetDeviceHandle2 = resourceManager.getRenderTargetDeviceHandle(targetHandle2, scene.getSceneId());
    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    {
        InSequence seq;

        expectActivateFramebufferRenderTarget();
        expectFrameRenderCommands(renderable1, Matrix44f::Identity, Matrix44f::Identity, projMatrix);
        expectActivateRenderTarget(renderTargetDeviceHandle2, false);
        expectClearRenderTarget();
        //render states are set again but shader and index buffer do not have to be set again
        expectFrameRenderCommands(renderable2, Matrix44f::Identity, Matrix44f::Identity, projMatrix, false, true, false);
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

    updateScenes();

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
        expectClearRenderTarget(EClearFlags_Depth | EClearFlags_Stencil); // pass 7
        // pass 8 no clear expectation
    }

    executeScene();
}

TEST_F(ARenderExecutor, DoesNotRenderRenderableIfResourcesInvalid)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams());
    DataInstances InvalidDataInstances;
    createTestRenderable(InvalidDataInstances, createRenderGroup(pass));

    updateScenes();
    // empty frame
    expectActivateFramebufferRenderTarget();

    executeScene();
}

TEST_F(ARenderExecutor, DoesNotRenderRenderableWithNoResourcesAssigned)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams());
    const RenderGroupHandle group = createRenderGroup(pass);
    const RenderableHandle renderable = sceneAllocator.allocateRenderable(sceneAllocator.allocateNode());
    scene.addRenderableToRenderGroup(group, renderable, 0);

    updateScenes();
    // empty frame
    expectActivateFramebufferRenderTarget();

    executeScene();
}

TEST_F(ARenderExecutor, DoesNotRenderRenderableWithoutDataInstance)
{
    const RenderPassHandle pass = createRenderPassWithCamera(getDefaultProjectionParams());

    DataInstances InvalidDataInstances;
    createTestRenderable(InvalidDataInstances, createRenderGroup(pass), RenderableHandle::Invalid());

    updateScenes();
    expectActivateFramebufferRenderTarget();

    executeScene();
}

TEST_F(ARenderExecutor, expectUpdateSceneDefaultMatricesIdentity)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));

    updateScenes();
    expectFrameWithSinglePass(renderable, projParams);
    executeScene();

    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, SceneViewMatrixChangeViaCameraTransformation)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));

    updateScenes();
    expectFrameWithSinglePass(renderable, projParams);
    executeScene();
    Mock::VerifyAndClearExpectations(&device);

    const NodeHandle cameraNode = scene.getCamera(scene.getRenderPass(pass).camera).node;
    const TransformHandle cameraTransform = findTransformForNode(cameraNode);
    scene.setTranslation(cameraTransform, Vector3(2.f, 3.f, 4.f));

    updateScenes();
    expectFrameWithSinglePass(renderable, projParams, Matrix44f::Identity, Matrix44f::Translation(Vector3(-2.f, -3.f, -4.f)));
    executeScene();
    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, RendersRenderableWithRendererProjection)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle renderPass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass));

    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    expectRenderingWithProjection(renderable, projMatrix);
}

TEST_F(ARenderExecutor, RendersMultipleRenderableInstancesWithRendererProjection)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle renderPass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass));
    const UInt32 instanceCount = 23u;
    scene.setRenderableInstanceCount(renderable, instanceCount);

    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

    expectRenderingWithProjection(renderable, projMatrix, instanceCount);
}

TEST_F(ARenderExecutor, RendersRenderableWithOrthographicProjection)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Orthographic);
    const RenderPassHandle renderPass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(renderPass));

    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);

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

    scene.setTranslation(cameraTransform1, Vector3(1.f, 2.f, 3.f));
    scene.setTranslation(cameraTransform2, Vector3(4.f, 5.f, 6.f));

    updateScenes();
    // reversed order because of google mock convention
    expectActivateFramebufferRenderTarget();
    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    expectFrameRenderCommands(renderable2, Matrix44f::Identity, Matrix44f::Translation(Vector3(-4.f, -5.f, -6.f)), projMatrix, false, false, false);
    expectFrameRenderCommands(renderable1, Matrix44f::Identity, Matrix44f::Translation(Vector3(-1.f, -2.f, -3.f)), projMatrix);
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

    updateScenes();

    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    // reversed order because of google mock convention
    expectActivateFramebufferRenderTarget();
    expectFrameRenderCommands(renderable2, Matrix44f::Identity, Matrix44f::Identity, projMatrix, false, false, false);
    expectFrameRenderCommands(renderable1, Matrix44f::Identity, Matrix44f::Identity, projMatrix);

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
    scene.setRenderStateColorWriteMask(state1, EColorWriteFlag_Red);
    scene.setRenderStateColorWriteMask(state2, EColorWriteFlag_Blue);
    scene.setRenderStateCullMode(state1, ECullMode::BackFacing);
    scene.setRenderStateCullMode(state2, ECullMode::Disabled);

    updateScenes();

    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    // reversed order because of google mock convention
    expectActivateFramebufferRenderTarget();
    expectFrameRenderCommands(renderable2, Matrix44f::Identity, Matrix44f::Identity, projMatrix, false, true, false);
    expectFrameRenderCommands(renderable1, Matrix44f::Identity, Matrix44f::Identity, projMatrix);

    executeScene();
    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, UpdatesModelMatrixWhenChangingTranslationRotationOrScalingOfNode)
{
    const auto projParams = getDefaultProjectionParams(ECameraProjectionType::Perspective);
    const RenderPassHandle pass = createRenderPassWithCamera(projParams);
    const RenderableHandle renderable = createTestRenderable(createTestDataInstance(), createRenderGroup(pass));
    const TransformHandle transform = addTransformToRenderable(renderable);

    scene.setTranslation(transform, Vector3(0.5f));
    updateScenes();
    expectFrameWithSinglePass(renderable, projParams, Matrix44f::Translation(Vector3(0.5f)));
    executeScene();

    Mock::VerifyAndClearExpectations(&device);

    scene.setTranslation(transform, Vector3(0.15f));
    scene.setScaling(transform, Vector3(0.25f));
    scene.setRotation(transform, Vector3(0.35f), ERotationConvention::Legacy_ZYX);
    updateScenes();
    expectFrameWithSinglePass(renderable, projParams,
        Matrix44f::Translation(Vector3(0.15f)) * Matrix44f::Scaling(Vector3(0.25f)) * Matrix44f::RotationEuler(Vector3(0.35f), ERotationConvention::Legacy_ZYX));
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

    scene.setTranslation(transformParent, Vector3(0.25f));
    scene.setTranslation(transformChild1, Vector3(0.15f));
    scene.setTranslation(transformChild2, Vector3(0.05f));
    updateScenes();

    const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    // reversed order because of google mock convention
    expectActivateFramebufferRenderTarget();
    expectFrameRenderCommands(renderable2, Matrix44f::Translation(Vector3(0.30f)), Matrix44f::Identity, projMatrix, false, false, false);
    expectFrameRenderCommands(renderable1, Matrix44f::Translation(Vector3(0.40f)), Matrix44f::Identity, projMatrix);
    executeScene();

    Mock::VerifyAndClearExpectations(&device);

    scene.setTranslation(transformParent, Vector3(0.0f));
    updateScenes();

    // reversed order because of google mock convention
    expectActivateFramebufferRenderTarget();
    expectFrameRenderCommands(renderable2, Matrix44f::Translation(Vector3(0.05f)), Matrix44f::Identity, projMatrix, false, false, false);
    expectFrameRenderCommands(renderable1, Matrix44f::Translation(Vector3(0.15f)), Matrix44f::Identity, projMatrix);
    executeScene();

    Mock::VerifyAndClearExpectations(&device);
}

TEST_F(ARenderExecutor, ExecutesBlitPass)
{
    const RenderBufferHandle sourceRenderBuffer = createRenderbuffer();
    const RenderBufferHandle destinationRenderBuffer = createRenderbuffer();
    const BlitPassHandle blitPassHandle = createBlitPass(sourceRenderBuffer, destinationRenderBuffer);
    updateScenes();

    EXPECT_CALL(device, blitRenderTargets(_, _, _, _, _));
    executeScene();

    scene.releaseRenderBuffer(sourceRenderBuffer);
    scene.releaseRenderBuffer(destinationRenderBuffer);
    scene.releaseBlitPass(blitPassHandle);
    updateScenes();
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
    updateScenes();

    {
        InSequence s;
        //first render pass
        EXPECT_CALL(device, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle));
        EXPECT_CALL(device, setViewport(_, _, _, _));
        //blit pass
        EXPECT_CALL(device, blitRenderTargets(_, _, _, _, _));
        //second render pass
        EXPECT_CALL(device, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle));
    }

    executeScene();

    scene.releaseRenderBuffer(sourceRenderBuffer);
    scene.releaseRenderBuffer(destinationRenderBuffer);
    scene.releaseBlitPass(blitPass);
    updateScenes();
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

    updateScenes();

    const Matrix44f expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    {
        InSequence s;

        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectClearRenderTarget();
        expectFrameRenderCommands(renderable1, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
        expectFrameRenderCommands(renderable2, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);

        expectActivateFramebufferRenderTarget(false);
        expectFrameRenderCommands(renderable3, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);
        expectFrameRenderCommands(renderable4, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);
    }

    FrameTimer frameTimer;
    frameTimer.startFrame();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, std::numeric_limits<UInt64>::max());

    const auto renderIterator = executeScene({}, &frameTimer);
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

    updateScenes();

    const Matrix44f expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(fakeFieldOfView, fakeAspectRatio, fakeNearPlane, fakeFarPlane));

    {
        InSequence s;

        expectActivateFramebufferRenderTarget();

        // one batch of renderables is rendered, first sets states
        expectFrameRenderCommands(batchRenderables.front(), Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
        for (auto it = batchRenderables.cbegin() + 1; it != batchRenderables.cend(); ++it)
            expectFrameRenderCommands(*it, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);

        // otherRenderable is not rendered
        UNUSED(renderableOutOfBudget);
    }

    FrameTimer frameTimer;
    frameTimer.startFrame();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, 0u);

    const auto renderIterator = executeScene({}, &frameTimer);
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

    updateScenes();

    const Matrix44f expectedProjectionMatrix = CameraMatrixHelper::ProjectionMatrix(projParams);
    const DeviceResourceHandle renderTargetDeviceHandle = resourceManager.getRenderTargetDeviceHandle(targetHandle, scene.getSceneId());

    FrameTimer frameTimer;
    frameTimer.startFrame();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, 0u);

    {
        InSequence s;
        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectClearRenderTarget();
        expectFrameRenderCommands(batchRenderables1.front(), Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
        for (auto it = batchRenderables1.cbegin() + 1; it != batchRenderables1.cend(); ++it)
            expectFrameRenderCommands(*it, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);
    }
    SceneRenderExecutionIterator renderIterator = executeScene({}, &frameTimer);
    EXPECT_EQ(0u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(1u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(1u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        InSequence s;
        expectActivateRenderTarget(renderTargetDeviceHandle); // no clear
        expectFrameRenderCommands(batchRenderables2.front(), Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
        for (auto it = batchRenderables2.cbegin() + 1; it != batchRenderables2.cend(); ++it)
            expectFrameRenderCommands(*it, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);
    }
    renderIterator = executeScene(renderIterator, &frameTimer);
    EXPECT_EQ(0u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(2u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(2u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        InSequence s;
        // last render interrupted at last renderable from pass1, so it continues from there including activating of pass1's RT
        // even though there are no more renderables to render
        expectActivateRenderTarget(renderTargetDeviceHandle);
        expectActivateFramebufferRenderTarget(false);
        expectFrameRenderCommands(batchRenderables3.front(), Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
        for (auto it = batchRenderables3.cbegin() + 1; it != batchRenderables3.cend(); ++it)
            expectFrameRenderCommands(*it, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);
    }
    renderIterator = executeScene(renderIterator, &frameTimer);
    EXPECT_EQ(1u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(1u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(3u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        InSequence s;
        expectActivateFramebufferRenderTarget();
        expectFrameRenderCommands(batchRenderables4.front(), Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix);
        for (auto it = batchRenderables4.cbegin() + 1; it != batchRenderables4.cend(); ++it)
            expectFrameRenderCommands(*it, Matrix44f::Identity, Matrix44f::Identity, expectedProjectionMatrix, false, false, false);
    }
    renderIterator = executeScene(renderIterator, &frameTimer);
    EXPECT_EQ(1u, renderIterator.getRenderPassIdx());
    EXPECT_EQ(2u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getRenderableIdx());
    EXPECT_EQ(4u * RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks, renderIterator.getFlattenedRenderableIdx());

    {
        InSequence s;
        // last render interrupted at last renderable from pass2, so it continues from there including activating of pass2's RT being FB
        // even though there are no more renderables to render
        expectActivateFramebufferRenderTarget();
        // no additional render operation (eg. blit passes)
    }
    renderIterator = executeScene(renderIterator, &frameTimer);
    EXPECT_EQ(SceneRenderExecutionIterator(), renderIterator); // finished
}
}
