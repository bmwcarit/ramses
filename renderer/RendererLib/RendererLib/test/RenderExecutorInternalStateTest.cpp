//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RenderExecutorInternalState.h"
#include "RenderBackendMock.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/RendererScenes.h"
#include "TestEqualHelper.h"
#include "ResourceDeviceHandleAccessorMock.h"
#include "RendererEventCollector.h"
#include "Math3d/Matrix44f.h"
#include "SceneAllocateHelper.h"
#include <limits>

namespace ramses_internal
{
    using namespace testing;

    namespace
    {
        const UInt32 FakeVpWidth = 800;
        const UInt32 FakeVpHeight = 600;
        const Float FakeAspectRatio = float(FakeVpWidth) / FakeVpHeight;
        const Float FakeNearPlane = 0.1f;
        const Float FakeFarPlane = 0.2f;
        const Float FakeLeftPlane = 0.3f;
        const Float FakeRightPlane = 0.4f;
        const Float FakeBottomPlane = 0.5f;
        const Float FakeTopPlane = 0.6f;
        const Float FakeFoV = 30.0f;
    }

    class ARenderExecutorInternalState : public ::testing::Test
    {
    public:
        ARenderExecutorInternalState()
            : m_renderContext{ DeviceResourceHandle(0u), FakeVpWidth, FakeVpHeight, SceneRenderExecutionIterator{}, EClearFlags_All, Vector4{1.f}, false }
            , m_executorState(m_device, m_renderContext)
            , m_executorStateWithTimer(m_device, m_renderContext, &m_frameTimer)
            , m_rendererScenes(m_rendererEventCollector)
            , m_sceneLinksManager(m_rendererScenes.getSceneLinksManager())
            , m_sceneId(666u)
            , m_scene(m_rendererScenes.createScene(SceneInfo(m_sceneId)))
            , m_sceneAllocator(m_scene)
        {
            m_executorState.setScene(m_scene);
            m_frameTimer.startFrame();
        }

    protected:
        StrictMock<DeviceMock>             m_device;
        RenderingContext                   m_renderContext;
        FrameTimer                         m_frameTimer;
        RenderExecutorInternalState        m_executorState;
        RenderExecutorInternalState        m_executorStateWithTimer;
        RendererEventCollector             m_rendererEventCollector;
        RendererScenes                     m_rendererScenes;
        SceneLinksManager&                 m_sceneLinksManager;
        SceneId                            m_sceneId;
        RendererCachedScene&               m_scene;
        SceneAllocateHelper                m_sceneAllocator;

        CameraHandle createTestCamera(const Vector3& translation = Vector3(0.0f), ECameraProjectionType camProjType = ECameraProjectionType::Perspective)
        {
            const NodeHandle cameraNode = m_sceneAllocator.allocateNode();
            const auto dataLayout = m_sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference} }, {});
            const auto dataInstance = m_sceneAllocator.allocateDataInstance(dataLayout);
            const auto vpDataRefLayout = m_sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2I} }, {});
            const auto vpOffsetInstance = m_sceneAllocator.allocateDataInstance(vpDataRefLayout);
            const auto vpSizeInstance = m_sceneAllocator.allocateDataInstance(vpDataRefLayout);
            const auto frustumPlanesLayout = m_sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector4F} }, {});
            const auto frustumPlanes = m_sceneAllocator.allocateDataInstance(frustumPlanesLayout);
            const auto frustumNearFarLayout = m_sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2F} }, {});
            const auto frustumNearFar = m_sceneAllocator.allocateDataInstance(frustumNearFarLayout);
            m_scene.setDataReference(dataInstance, Camera::ViewportOffsetField, vpOffsetInstance);
            m_scene.setDataReference(dataInstance, Camera::ViewportSizeField, vpSizeInstance);
            m_scene.setDataReference(dataInstance, Camera::FrustumPlanesField, frustumPlanes);
            m_scene.setDataReference(dataInstance, Camera::FrustumNearFarPlanesField, frustumNearFar);
            const CameraHandle camera = m_sceneAllocator.allocateCamera(camProjType, cameraNode, dataInstance);

            ProjectionParams params = ProjectionParams::Frustum(ECameraProjectionType::Orthographic, FakeLeftPlane, FakeRightPlane, FakeBottomPlane, FakeTopPlane, FakeNearPlane, FakeFarPlane);
            if (camProjType == ECameraProjectionType::Perspective)
                params = ProjectionParams::Perspective(FakeFoV, FakeAspectRatio, FakeNearPlane, FakeFarPlane);
            m_scene.setDataSingleVector4f(frustumPlanes, DataFieldHandle{ 0 }, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane });
            m_scene.setDataSingleVector2f(frustumNearFar, DataFieldHandle{ 0 }, { params.nearPlane, params.farPlane });

            m_scene.setDataSingleVector2i(vpOffsetInstance, DataFieldHandle{ 0 }, { 0, 0 });
            m_scene.setDataSingleVector2i(vpSizeInstance, DataFieldHandle{ 0 }, { Int32(FakeVpWidth), Int32(FakeVpHeight) });

            const TransformHandle cameraTransform = m_sceneAllocator.allocateTransform(cameraNode);
            m_scene.setTranslation(cameraTransform, translation);

            return camera;
        }
    };

    TEST_F(ARenderExecutorInternalState, canGetRenderingContext)
    {
        EXPECT_EQ(DeviceResourceHandle(0u), m_executorState.getRenderingContext().displayBufferDeviceHandle);
        EXPECT_EQ(FakeVpWidth, m_executorState.getRenderingContext().viewportWidth);
        EXPECT_EQ(FakeVpHeight, m_executorState.getRenderingContext().viewportHeight);
        EXPECT_EQ(SceneRenderExecutionIterator{}, m_executorState.getRenderingContext().renderFrom);
        EXPECT_EQ(EClearFlags_All, m_executorState.getRenderingContext().displayBufferClearPending);
        EXPECT_EQ(Vector4{ 1.f }, m_executorState.getRenderingContext().displayBufferClearColor);
    }

    TEST_F(ARenderExecutorInternalState, hasIdentityProjectionMatrixInDefaultState)
    {
        EXPECT_EQ(&m_device, &m_executorState.getDevice());
        EXPECT_EQ(Matrix44f::Identity, m_executorState.getProjectionMatrix());
    }

    TEST_F(ARenderExecutorInternalState, setsSceneState)
    {
        ASSERT_EQ(&m_scene, &m_executorState.getScene());

        RendererCachedScene otherScene(m_sceneLinksManager, SceneInfo(SceneId(33u)));
        m_executorState.setScene(otherScene);
        ASSERT_EQ(&otherScene, &m_executorState.getScene());
    }

    TEST_F(ARenderExecutorInternalState, updatesCameraRelatedMatricesWhenSettingCamera)
    {
        const CameraHandle camera = createTestCamera(Vector3(3, 5, 6), ECameraProjectionType::Perspective);
        m_executorState.setCamera(camera);

        const NodeHandle cameraNode = m_scene.getCamera(camera).node;
        const Matrix44f camViewMat = m_scene.updateMatrixCache(ETransformationMatrixType_Object, cameraNode);

        expectMatrixFloatEqual(camViewMat, m_executorState.getViewMatrix());
    }

    TEST_F(ARenderExecutorInternalState, updatesProjectionMatrixWhenSettingPerspectiveCamera)
    {
        const CameraHandle camera = createTestCamera(Vector3(0.0f), ECameraProjectionType::Perspective);

        m_executorState.setCamera(camera);

        const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(
            ProjectionParams::Perspective(FakeFoV, FakeAspectRatio, FakeNearPlane, FakeFarPlane));

        EXPECT_EQ(projMatrix, m_executorState.getProjectionMatrix());
    }

    TEST_F(ARenderExecutorInternalState, updatesProjectionMatrixWhenSettingOrthographicCamera)
    {
        const CameraHandle camera = createTestCamera(Vector3(0.0f), ECameraProjectionType::Orthographic);

        const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(
            ProjectionParams::Frustum(ECameraProjectionType::Orthographic,
                FakeLeftPlane,
                FakeRightPlane,
                FakeBottomPlane,
                FakeTopPlane,
                FakeNearPlane,
                FakeFarPlane
            ));
        m_executorState.setCamera(camera);

        EXPECT_EQ(projMatrix, m_executorState.getProjectionMatrix());
    }

    TEST_F(ARenderExecutorInternalState, setRenderableUpdatesAllModelDependentMatrices)
    {
        const CameraHandle camera = createTestCamera(Vector3(3, 5, 6), ECameraProjectionType::Perspective);
        m_executorState.setCamera(camera);
        const NodeHandle cameraNode = m_scene.getCamera(camera).node;
        const Matrix44f camViewMat = m_scene.updateMatrixCache(ETransformationMatrixType_Object, cameraNode);

        const NodeHandle renderableNode = m_sceneAllocator.allocateNode();
        const RenderableHandle renderable = m_sceneAllocator.allocateRenderable(renderableNode);

        const TransformHandle renderableTransform = m_sceneAllocator.allocateTransform(renderableNode);
        const RenderPassHandle renderPass = m_sceneAllocator.allocateRenderPass();
        m_scene.setRenderPassCamera(renderPass, camera);
        const RenderGroupHandle renderGroup = m_sceneAllocator.allocateRenderGroup();
        m_scene.addRenderGroupToRenderPass(renderPass, renderGroup, 0);
        m_scene.addRenderableToRenderGroup(renderGroup, renderable, 0);
        m_scene.setRotation(renderableTransform, Vector4(1, 2, 3, 1), ERotationType::Euler_XYZ);

        NiceMock<ResourceDeviceHandleAccessorMock> resourceAccessor;
        m_scene.updateRenderablesAndResourceCache(resourceAccessor);
        m_scene.updateRenderableWorldMatrices();

        m_executorState.setRenderable(renderable);

        const Matrix44f modelMat = m_scene.updateMatrixCache(ETransformationMatrixType_World, renderableNode);
        const Matrix44f expectedViewMat = camViewMat;
        const Matrix44f expectedProjectionMat = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(FakeFoV, static_cast<Float>(FakeVpWidth) / FakeVpHeight, FakeNearPlane, FakeFarPlane));

        expectMatrixFloatEqual(modelMat, m_executorState.getModelMatrix());
        expectMatrixFloatEqual(expectedViewMat, m_executorState.getViewMatrix());
        expectMatrixFloatEqual(expectedProjectionMat, m_executorState.getProjectionMatrix());
        expectMatrixFloatEqual(expectedViewMat * modelMat, m_executorState.getModelViewMatrix());
        expectMatrixFloatEqual(expectedProjectionMat * expectedViewMat * modelMat, m_executorState.getModelViewProjectionMatrix());
    }

    TEST_F(ARenderExecutorInternalState, depthFuncRenderStateMarkedAsUnchangedIfSetToSameValue)
    {
        const auto depthFuncState = EDepthFunc::Disabled;
        m_executorState.depthFuncState.setState(depthFuncState);
        EXPECT_TRUE(m_executorState.depthFuncState.hasChanged());
        m_executorState.depthFuncState.setState(depthFuncState);
        EXPECT_FALSE(m_executorState.depthFuncState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToDepthFuncRenderStateTriggersChange)
    {
        const auto depthState = EDepthFunc::Disabled;
        m_executorState.depthFuncState.setState(depthState);
        EXPECT_TRUE(m_executorState.depthFuncState.hasChanged());
        EXPECT_FALSE(depthState != m_executorState.depthFuncState.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToDepthWriteRenderStateTriggersChange)
    {
        const auto depthState = EDepthWrite::Disabled;
        m_executorState.depthWriteState.setState(depthState);
        EXPECT_TRUE(m_executorState.depthWriteState.hasChanged());
        EXPECT_FALSE(depthState != m_executorState.depthWriteState.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToStencilRenderStateTriggersChange)
    {
        StencilState stencilState;
        stencilState.m_stencilFunc = EStencilFunc::Disabled;
        m_executorState.stencilState.setState(stencilState);
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());
        EXPECT_FALSE(stencilState != m_executorState.stencilState.getState());
    }

    TEST_F(ARenderExecutorInternalState, depthFuncRenderStateMarkedAsChanged_IfDepthFunctionChanged)
    {
        m_executorState.depthFuncState.setState(EDepthFunc::Disabled);
        EXPECT_TRUE(m_executorState.depthFuncState.hasChanged());

        m_executorState.depthFuncState.setState(EDepthFunc::Greater);
        EXPECT_TRUE(m_executorState.depthFuncState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, depthWriteRenderStateMarkedAsChanged_IfDepthWriteChanged)
    {
        m_executorState.depthWriteState.setState(EDepthWrite::Disabled);
        EXPECT_TRUE(m_executorState.depthWriteState.hasChanged());

        m_executorState.depthWriteState.setState(EDepthWrite::Enabled);
        EXPECT_TRUE(m_executorState.depthWriteState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, stencilRenderStateMarkedAsChanged_IfStencilFunctionChanged)
    {
        StencilState stencilState;
        stencilState.m_stencilFunc = EStencilFunc::NeverPass;
        m_executorState.stencilState.setState(stencilState);
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());

        stencilState.m_stencilFunc = EStencilFunc::AlwaysPass;
        m_executorState.stencilState.setState(stencilState);
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, stencilRenderStateMarkedAsChanged_IfStencilOperationChanged)
    {
        auto runTest = [this](StencilState& depthState, EStencilOp& stencilOp) {
            stencilOp = EStencilOp::Increment;
            m_executorState.stencilState.setState(depthState);
            EXPECT_TRUE(m_executorState.stencilState.hasChanged());

            stencilOp = EStencilOp::Invert;
            m_executorState.stencilState.setState(depthState);
            EXPECT_TRUE(m_executorState.stencilState.hasChanged());
        };

        {
            StencilState stencilState;
            runTest(stencilState, stencilState.m_stencilOpDepthFail);
        }
        {
            StencilState stencilState;
            runTest(stencilState, stencilState.m_stencilOpDepthPass);
        }
        {
            StencilState stencilState;
            runTest(stencilState, stencilState.m_stencilOpFail);
        }
    }

    TEST_F(ARenderExecutorInternalState, stencilRenderStateMarkedAsChanged_IfStencilMaskChanged)
    {
        StencilState stencilState;
        stencilState.m_stencilMask = 123;
        m_executorState.stencilState.setState(stencilState);
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());

        stencilState.m_stencilMask = 124;
        m_executorState.stencilState.setState(stencilState);
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, stencilRenderStateMarkedAsChanged_IfStencilRefValueChanged)
    {
        StencilState stencilState;
        stencilState.m_stencilRefValue = 123;
        m_executorState.stencilState.setState(stencilState);
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());

        stencilState.m_stencilRefValue = 124;
        m_executorState.stencilState.setState(stencilState);
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToScissorStateTriggersChange)
    {
        ScissorState scissorState;
        scissorState.m_scissorTest = EScissorTest::Disabled;
        m_executorState.scissorState.setState(scissorState);
        EXPECT_TRUE(m_executorState.scissorState.hasChanged());
        EXPECT_FALSE(scissorState != m_executorState.scissorState.getState());
    }

    TEST_F(ARenderExecutorInternalState, scissorStateMarkedAsUnchangedIfSetToSameValue)
    {
        ScissorState scissorState;
        scissorState.m_scissorTest = EScissorTest::Disabled;
        m_executorState.scissorState.setState(scissorState);
        EXPECT_TRUE(m_executorState.scissorState.hasChanged());
        m_executorState.scissorState.setState(scissorState);
        EXPECT_FALSE(m_executorState.scissorState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, scissorStateMarkedAsChanged_IfScissorTestChanged)
    {
        ScissorState scissorState;
        scissorState.m_scissorTest = EScissorTest::Disabled;
        m_executorState.scissorState.setState(scissorState);
        EXPECT_TRUE(m_executorState.scissorState.hasChanged());

        scissorState.m_scissorTest = EScissorTest::Enabled;
        m_executorState.scissorState.setState(scissorState);
        EXPECT_TRUE(m_executorState.scissorState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, scissorStateMarkedAsChanged_IfScissorRegionsChanged)
    {
        ScissorState scissorState;
        scissorState.m_scissorRegion.x = 1u;
        m_executorState.scissorState.setState(scissorState);
        EXPECT_TRUE(m_executorState.scissorState.hasChanged());

        scissorState.m_scissorRegion.x = 2u;
        m_executorState.scissorState.setState(scissorState);
        EXPECT_TRUE(m_executorState.scissorState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToBlendRenderStateTriggersChange)
    {
        BlendFactorsState blendFactorsState;
        blendFactorsState.m_blendFactorSrcColor = EBlendFactor::OneMinusDstAlpha;
        m_executorState.blendFactorsState.setState(blendFactorsState);
        EXPECT_TRUE(m_executorState.blendFactorsState.hasChanged());
        EXPECT_FALSE(blendFactorsState != m_executorState.blendFactorsState.getState());

        BlendOperationsState blendOperationsState;
        blendOperationsState.m_blendOperationAlpha = EBlendOperation::Max;
        m_executorState.blendOperationsState.setState(blendOperationsState);
        EXPECT_TRUE(m_executorState.blendOperationsState.hasChanged());
        EXPECT_FALSE(blendOperationsState != m_executorState.blendOperationsState.getState());

        const Vector4 blendColorState(0.f);
        m_executorState.blendColorState.setState(blendColorState);
        EXPECT_TRUE(m_executorState.blendColorState.hasChanged());
        EXPECT_FALSE(blendColorState != m_executorState.blendColorState.getState());

        const ColorWriteMask colorMaskState = 0u;
        m_executorState.colorWriteMaskState.setState(colorMaskState);
        EXPECT_TRUE(m_executorState.colorWriteMaskState.hasChanged());
        EXPECT_FALSE(colorMaskState != m_executorState.colorWriteMaskState.getState());
    }

    TEST_F(ARenderExecutorInternalState, blendRenderStateMarkedAsUnchangedIfSetToSameValue)
    {
        BlendFactorsState blendFactorsState;
        blendFactorsState.m_blendFactorSrcColor = EBlendFactor::OneMinusConstColor;
        m_executorState.blendFactorsState.setState(blendFactorsState);
        EXPECT_TRUE(m_executorState.blendFactorsState.hasChanged());
        m_executorState.blendFactorsState.setState(blendFactorsState);
        EXPECT_FALSE(m_executorState.blendFactorsState.hasChanged());

        BlendOperationsState blendOperationsState;
        blendOperationsState.m_blendOperationAlpha = EBlendOperation::Max;
        m_executorState.blendOperationsState.setState(blendOperationsState);
        EXPECT_TRUE(m_executorState.blendOperationsState.hasChanged());
        m_executorState.blendOperationsState.setState(blendOperationsState);
        EXPECT_FALSE(m_executorState.blendOperationsState.hasChanged());

        const Vector4 blendColorState(123.f);
        m_executorState.blendColorState.setState(blendColorState);
        EXPECT_TRUE(m_executorState.blendColorState.hasChanged());
        m_executorState.blendColorState.setState(blendColorState);
        EXPECT_FALSE(m_executorState.blendColorState.hasChanged());

        const ColorWriteMask colorMaskState = 123u;
        m_executorState.colorWriteMaskState.setState(colorMaskState);
        EXPECT_TRUE(m_executorState.colorWriteMaskState.hasChanged());
        m_executorState.colorWriteMaskState.setState(colorMaskState);
        EXPECT_FALSE(m_executorState.colorWriteMaskState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, blendFactorsStateMarkedAsChanged_IfBlendFactorChanged)
    {
        auto runTest = [this](auto& blendState, EBlendFactor& blendFactor) {
            blendFactor = EBlendFactor::OneMinusConstAlpha;
            m_executorState.blendFactorsState.setState(blendState);
            EXPECT_TRUE(m_executorState.blendFactorsState.hasChanged());

            blendFactor = EBlendFactor::ConstAlpha;
            m_executorState.blendFactorsState.setState(blendState);
            EXPECT_TRUE(m_executorState.blendFactorsState.hasChanged());
        };

        {
            BlendFactorsState state;
            runTest(state, state.m_blendFactorSrcColor);
        }
        {
            BlendFactorsState state;
            runTest(state, state.m_blendFactorDstColor);
        }
        {
            BlendFactorsState state;
            runTest(state, state.m_blendFactorSrcAlpha);
        }
        {
            BlendFactorsState state;
            runTest(state, state.m_blendFactorDstAlpha);
        }
    }

    TEST_F(ARenderExecutorInternalState, blendOperationsStateMarkedAsChanged_IfBlendOperationChanged)
    {
        {
            BlendOperationsState state;
            state.m_blendOperationColor = EBlendOperation::Min;
            m_executorState.blendOperationsState.setState(state);
            EXPECT_TRUE(m_executorState.blendOperationsState.hasChanged());

            state.m_blendOperationColor = EBlendOperation::Max;
            m_executorState.blendOperationsState.setState(state);
            EXPECT_TRUE(m_executorState.blendOperationsState.hasChanged());
        }
        {
            BlendOperationsState state;
            state.m_blendOperationAlpha = EBlendOperation::Min;
            m_executorState.blendOperationsState.setState(state);
            EXPECT_TRUE(m_executorState.blendOperationsState.hasChanged());

            state.m_blendOperationAlpha = EBlendOperation::Max;
            m_executorState.blendOperationsState.setState(state);
            EXPECT_TRUE(m_executorState.blendOperationsState.hasChanged());
        }
    }

    TEST_F(ARenderExecutorInternalState, blendColorMarkedAsChanged_IfBlendColorChanged)
    {
        m_executorState.blendColorState.setState(Vector4{ .1f, .2f, .3f, .4f });
        EXPECT_TRUE(m_executorState.blendColorState.hasChanged());

        m_executorState.blendColorState.setState(Vector4{ .9f });
        EXPECT_TRUE(m_executorState.blendColorState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, colorWriteMaskMarkedAsChanged_IfColorWriteMaskChanged)
    {
        m_executorState.colorWriteMaskState.setState(EColorWriteFlag::EColorWriteFlag_Green);
        EXPECT_TRUE(m_executorState.colorWriteMaskState.hasChanged());

        m_executorState.colorWriteMaskState.setState(EColorWriteFlag::EColorWriteFlag_Blue);
        EXPECT_TRUE(m_executorState.colorWriteMaskState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToCullModeRenderStateTriggersChange)
    {
        const auto cullModeState = ECullMode::Disabled;
        m_executorState.cullModeState.setState(cullModeState);
        EXPECT_TRUE(m_executorState.cullModeState.hasChanged());
        EXPECT_FALSE(cullModeState != m_executorState.cullModeState.getState());
    }

    TEST_F(ARenderExecutorInternalState, rasterizerRenderStateMarkedAsUnchangedIfSetToSameValue)
    {
        m_executorState.cullModeState.setState(ECullMode::Disabled);
        EXPECT_TRUE(m_executorState.cullModeState.hasChanged());
        m_executorState.cullModeState.setState(ECullMode::Disabled);
        EXPECT_FALSE(m_executorState.cullModeState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, rasterizerRenderStateMarkedAsChanged_IfCullModeChanged)
    {
        m_executorState.cullModeState.setState(ECullMode::Disabled);
        EXPECT_TRUE(m_executorState.cullModeState.hasChanged());

        m_executorState.cullModeState.setState(ECullMode::BackFacing);
        EXPECT_TRUE(m_executorState.cullModeState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToDefaultDeviceHandlesTriggersChange)
    {
        m_executorState.shaderDeviceHandle.setState(DeviceResourceHandle(0u));
        EXPECT_TRUE(m_executorState.shaderDeviceHandle.hasChanged());
        EXPECT_EQ(0u, m_executorState.shaderDeviceHandle.getState());
    }

    TEST_F(ARenderExecutorInternalState, deviceHandlesMarkedAsUnchangedIfSetToSameValue)
    {
        m_executorState.shaderDeviceHandle.setState(DeviceResourceHandle(123u));
        EXPECT_TRUE(m_executorState.shaderDeviceHandle.hasChanged());
        m_executorState.shaderDeviceHandle.setState(DeviceResourceHandle(123u));
        EXPECT_FALSE(m_executorState.shaderDeviceHandle.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, deviceHandlesMarkedAsChangedIfSetToDifferentValue)
    {
        m_executorState.shaderDeviceHandle.setState(DeviceResourceHandle(123u));
        EXPECT_TRUE(m_executorState.shaderDeviceHandle.hasChanged());
        m_executorState.shaderDeviceHandle.setState(DeviceResourceHandle(567u));
        EXPECT_TRUE(m_executorState.shaderDeviceHandle.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToRenderTargetTriggersChange)
    {
        m_executorState.renderTargetState.setState(RenderTargetHandle::Invalid());
        EXPECT_TRUE(m_executorState.renderTargetState.hasChanged());
        EXPECT_EQ(RenderTargetHandle::Invalid(), m_executorState.renderTargetState.getState());
    }

    TEST_F(ARenderExecutorInternalState, renderTargetMarkedAsUnchangedIfSetToSameValue)
    {
        m_executorState.renderTargetState.setState(RenderTargetHandle(123u));
        EXPECT_TRUE(m_executorState.renderTargetState.hasChanged());

        m_executorState.renderTargetState.setState(RenderTargetHandle(123u));
        EXPECT_FALSE(m_executorState.renderTargetState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, renderTargetMarkedAsChangedIfSetToDifferentValue)
    {
        m_executorState.renderTargetState.setState(RenderTargetHandle(123u));
        EXPECT_TRUE(m_executorState.renderTargetState.hasChanged());

        m_executorState.renderTargetState.setState(RenderTargetHandle(567u));
        EXPECT_TRUE(m_executorState.renderTargetState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToRenderPassTriggersChange)
    {
        m_executorState.renderPassState.setState(RenderPassHandle(0u));
        EXPECT_TRUE(m_executorState.renderPassState.hasChanged());
        EXPECT_EQ(0u, m_executorState.renderPassState.getState());
    }

    TEST_F(ARenderExecutorInternalState, renderPassMarkedAsUnchangedIfSetToSameValue)
    {
        m_executorState.renderPassState.setState(RenderPassHandle(10u));
        EXPECT_TRUE(m_executorState.renderPassState.hasChanged());

        m_executorState.renderPassState.setState(RenderPassHandle(10u));
        EXPECT_FALSE(m_executorState.renderPassState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, renderPassMarkedAsChangedIfSetToDifferentValue)
    {
        m_executorState.renderPassState.setState(RenderPassHandle(10u));
        EXPECT_TRUE(m_executorState.renderPassState.hasChanged());

        m_executorState.renderPassState.setState(RenderPassHandle(0u));
        EXPECT_TRUE(m_executorState.renderPassState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToViewportTriggersChange)
    {
        m_executorState.viewportState.setState(Viewport());
        EXPECT_TRUE(m_executorState.viewportState.hasChanged());
        EXPECT_EQ(Viewport(), m_executorState.viewportState.getState());
    }

    TEST_F(ARenderExecutorInternalState, viewportMarkedAsUnchangedIfSetToSameValue)
    {
        m_executorState.viewportState.setState(Viewport(1, 2, 3u, 4u));
        EXPECT_TRUE(m_executorState.viewportState.hasChanged());

        m_executorState.viewportState.setState(Viewport(1, 2, 3u, 4u));
        EXPECT_FALSE(m_executorState.viewportState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, viewportMarkedAsChangedIfSetToDifferentValue)
    {
        m_executorState.viewportState.setState(Viewport(1, 2, 3u, 4u));
        EXPECT_TRUE(m_executorState.viewportState.hasChanged());

        m_executorState.viewportState.setState(Viewport(100, 200, 300u, 400u));
        EXPECT_TRUE(m_executorState.viewportState.hasChanged());
    }

    TEST_F(ARenderExecutorInternalState, canResetCachedStates)
    {
        m_executorState.blendOperationsState.setState(BlendOperationsState());
        m_executorState.blendFactorsState.setState(BlendFactorsState());
        m_executorState.blendColorState.setState(Vector4(std::numeric_limits<float>::max()));
        m_executorState.colorWriteMaskState.setState(std::numeric_limits<ColorWriteMask>::max());

        m_executorState.depthFuncState.setState(EDepthFunc::NUMBER_OF_ELEMENTS);
        m_executorState.depthWriteState.setState(EDepthWrite::NUMBER_OF_ELEMENTS);
        m_executorState.stencilState.setState(StencilState());
        m_executorState.cullModeState.setState(ECullMode::NUMBER_OF_ELEMENTS);

        ASSERT_FALSE(m_executorState.blendOperationsState.hasChanged());
        ASSERT_FALSE(m_executorState.blendFactorsState.hasChanged());
        ASSERT_FALSE(m_executorState.blendColorState.hasChanged());
        ASSERT_FALSE(m_executorState.colorWriteMaskState.hasChanged());

        ASSERT_FALSE(m_executorState.depthFuncState.hasChanged());
        ASSERT_FALSE(m_executorState.depthWriteState.hasChanged());
        ASSERT_FALSE(m_executorState.stencilState.hasChanged());
        ASSERT_FALSE(m_executorState.cullModeState.hasChanged());

        m_executorState.blendOperationsState.reset();
        m_executorState.blendFactorsState.reset();
        m_executorState.blendColorState.reset();
        m_executorState.colorWriteMaskState.reset();

        m_executorState.depthFuncState.reset();
        m_executorState.depthWriteState.reset();
        m_executorState.stencilState.reset();
        m_executorState.cullModeState.reset();

        EXPECT_TRUE(m_executorState.blendOperationsState.hasChanged());
        EXPECT_TRUE(m_executorState.blendFactorsState.hasChanged());
        EXPECT_TRUE(m_executorState.blendColorState.hasChanged());
        EXPECT_TRUE(m_executorState.colorWriteMaskState.hasChanged());
        EXPECT_TRUE(m_executorState.depthFuncState.hasChanged());
        EXPECT_TRUE(m_executorState.depthWriteState.hasChanged());
        EXPECT_TRUE(m_executorState.stencilState.hasChanged());
        EXPECT_TRUE(m_executorState.cullModeState.hasChanged());

        EXPECT_FALSE(m_executorState.blendOperationsState.getState() != BlendOperationsState());
        EXPECT_FALSE(m_executorState.blendFactorsState.getState()   != BlendFactorsState());
        EXPECT_FALSE(m_executorState.blendColorState.getState()     != Vector4(std::numeric_limits<float>::max()));
        EXPECT_FALSE(m_executorState.colorWriteMaskState.getState() != std::numeric_limits<ColorWriteMask>::max());
        EXPECT_FALSE(m_executorState.depthFuncState.getState()      != EDepthFunc::NUMBER_OF_ELEMENTS);
        EXPECT_FALSE(m_executorState.depthWriteState.getState()     != EDepthWrite::NUMBER_OF_ELEMENTS);
        EXPECT_FALSE(m_executorState.stencilState.getState()        != StencilState());
        EXPECT_FALSE(m_executorState.cullModeState.getState()       != ECullMode::NUMBER_OF_ELEMENTS);
    }

    TEST_F(ARenderExecutorInternalState, canIncrementAndGetRenderPassAndRenderableIdx)
    {
        EXPECT_EQ(0u, m_executorState.m_currentRenderIterator.getRenderPassIdx());
        m_executorState.m_currentRenderIterator.incrementRenderPassIdx();
        EXPECT_EQ(1u, m_executorState.m_currentRenderIterator.getRenderPassIdx());
        m_executorState.m_currentRenderIterator.incrementRenderPassIdx();
        EXPECT_EQ(2u, m_executorState.m_currentRenderIterator.getRenderPassIdx());

        EXPECT_EQ(0u, m_executorState.m_currentRenderIterator.getRenderableIdx());
        m_executorState.m_currentRenderIterator.incrementRenderableIdx();
        EXPECT_EQ(1u, m_executorState.m_currentRenderIterator.getRenderableIdx());
        m_executorState.m_currentRenderIterator.incrementRenderableIdx();
        EXPECT_EQ(2u, m_executorState.m_currentRenderIterator.getRenderableIdx());
    }

    TEST_F(ARenderExecutorInternalState, incrementingRenderPassIdxResetsRenderableIdxButNotFlattenedIdx)
    {
        EXPECT_EQ(0u, m_executorState.m_currentRenderIterator.getFlattenedRenderableIdx());
        m_executorState.m_currentRenderIterator.incrementRenderableIdx();
        m_executorState.m_currentRenderIterator.incrementRenderableIdx();
        EXPECT_EQ(2u, m_executorState.m_currentRenderIterator.getRenderableIdx());
        EXPECT_EQ(2u, m_executorState.m_currentRenderIterator.getFlattenedRenderableIdx());

        m_executorState.m_currentRenderIterator.incrementRenderPassIdx();
        EXPECT_EQ(0u, m_executorState.m_currentRenderIterator.getRenderableIdx());
        EXPECT_EQ(2u, m_executorState.m_currentRenderIterator.getFlattenedRenderableIdx());

        m_executorState.m_currentRenderIterator.incrementRenderableIdx();
        m_executorState.m_currentRenderIterator.incrementRenderableIdx();
        EXPECT_EQ(2u, m_executorState.m_currentRenderIterator.getRenderableIdx());
        EXPECT_EQ(4u, m_executorState.m_currentRenderIterator.getFlattenedRenderableIdx());

        m_executorState.m_currentRenderIterator.incrementRenderPassIdx();
        EXPECT_EQ(0u, m_executorState.m_currentRenderIterator.getRenderableIdx());
        EXPECT_EQ(4u, m_executorState.m_currentRenderIterator.getFlattenedRenderableIdx());
    }

    TEST_F(ARenderExecutorInternalState, reportsExceedingOfTimeBudget)
    {
        m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, 0u);
        ASSERT_TRUE(m_frameTimer.isTimeBudgetExceededForSection(EFrameTimerSectionBudget::OffscreenBufferRender));
        EXPECT_TRUE(m_executorStateWithTimer.hasExceededTimeBudgetForRendering());
    }

    TEST_F(ARenderExecutorInternalState, reportsExceedingOfTimeBudgetOnlyAfterItIsReallyExceeded)
    {
        m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, std::numeric_limits<UInt64>::max());
        ASSERT_FALSE(m_frameTimer.isTimeBudgetExceededForSection(EFrameTimerSectionBudget::OffscreenBufferRender));
        EXPECT_FALSE(m_executorStateWithTimer.hasExceededTimeBudgetForRendering());

        m_frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, 0u);
        ASSERT_TRUE(m_frameTimer.isTimeBudgetExceededForSection(EFrameTimerSectionBudget::OffscreenBufferRender));
        EXPECT_TRUE(m_executorStateWithTimer.hasExceededTimeBudgetForRendering());
    }
}
