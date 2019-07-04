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
#include "ResourceProviderMock.h"
#include "RendererEventCollector.h"
#include "Math3d/Matrix44f.h"
#include "EmbeddedCompositingManagerMock.h"
#include "SceneAllocateHelper.h"
#include <limits>

namespace ramses_internal
{
    using namespace testing;

    namespace
    {
        const UInt32 FakeVpWidth = 800;
        const UInt32 FakeVpHeight = 600;
        const Float FakeAspectRatio = 0.5f;
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
            : m_fbProjParams(ProjectionParams::Perspective(19.f, static_cast<Float>(FakeVpWidth) / FakeVpHeight, 0.1f, 1500.f))
            , m_fbProjMat(CameraMatrixHelper::ProjectionMatrix(m_fbProjParams))
            , m_fbInfo(DeviceResourceHandle(0u), m_fbProjParams, Viewport(0, 0, FakeVpWidth, FakeVpHeight))
            , m_executorState(m_device, m_fbInfo)
            , m_executorStateWithTimer(m_device, m_fbInfo, {}, &m_frameTimer)
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
        const ProjectionParams             m_fbProjParams;
        const Matrix44f                    m_fbProjMat;
        FrameBufferInfo                    m_fbInfo;
        FrameTimer                         m_frameTimer;
        RenderExecutorInternalState        m_executorState;
        RenderExecutorInternalState        m_executorStateWithTimer;
        RendererEventCollector             m_rendererEventCollector;
        RendererScenes                     m_rendererScenes;
        SceneLinksManager&                 m_sceneLinksManager;
        SceneId                            m_sceneId;
        RendererCachedScene&               m_scene;
        SceneAllocateHelper                m_sceneAllocator;

        CameraHandle createTestCamera(const Vector3& translation = Vector3(0.0f), ECameraProjectionType camProjType = ECameraProjectionType_Renderer)
        {
            const NodeHandle cameraNode = m_sceneAllocator.allocateNode();
            const auto vpDataLayout = m_sceneAllocator.allocateDataLayout({ {EDataType_DataReference}, {EDataType_DataReference} });
            const auto vpDataRefLayout = m_sceneAllocator.allocateDataLayout({ {EDataType_Vector2I} });
            const auto vpDataInstance = m_sceneAllocator.allocateDataInstance(vpDataLayout);
            const auto vpOffsetInstance = m_sceneAllocator.allocateDataInstance(vpDataRefLayout);
            const auto vpSizeInstance = m_sceneAllocator.allocateDataInstance(vpDataRefLayout);
            m_scene.setDataReference(vpDataInstance, Camera::ViewportOffsetField, vpOffsetInstance);
            m_scene.setDataReference(vpDataInstance, Camera::ViewportSizeField, vpSizeInstance);
            const CameraHandle camera = m_sceneAllocator.allocateCamera(camProjType, cameraNode, vpDataInstance);

            if (ECameraProjectionType_Perspective == camProjType)
            {
                const ramses_internal::ProjectionParams params = ramses_internal::ProjectionParams::Perspective(FakeFoV, FakeAspectRatio, FakeNearPlane, FakeFarPlane);
                m_scene.setCameraFrustum(camera, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane, params.nearPlane, params.farPlane });
            }
            else if (ECameraProjectionType_Orthographic == camProjType)
            {
                m_scene.setCameraFrustum(camera, { FakeLeftPlane, FakeRightPlane, FakeBottomPlane, FakeTopPlane, FakeNearPlane, FakeFarPlane });
            }

            if (ECameraProjectionType_Renderer != camProjType)
            {
                m_scene.setDataSingleVector2i(vpOffsetInstance, DataFieldHandle{ 0 }, { 0, 0 });
                m_scene.setDataSingleVector2i(vpSizeInstance, DataFieldHandle{ 0 }, { Int32(FakeVpWidth), Int32(FakeVpHeight) });
            }

            const TransformHandle cameraTransform = m_sceneAllocator.allocateTransform(cameraNode);
            m_scene.setTranslation(cameraTransform, translation);

            return camera;
        }
    };

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

    TEST_F(ARenderExecutorInternalState, setsRendererViewMatrix)
    {
        const Matrix44f mat(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
        m_executorState.setRendererViewMatrix(mat);
        EXPECT_EQ(mat, m_executorState.getRendererViewMatrix());
    }

    TEST_F(ARenderExecutorInternalState, updatesCameraRelatedMatricesWhenSettingCamera)
    {
        const Matrix44f mat(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
        m_executorState.setRendererViewMatrix(mat);

        const CameraHandle camera = createTestCamera(Vector3(3, 5, 6), ECameraProjectionType_Perspective);
        m_executorState.setCamera(camera);

        const NodeHandle cameraNode = m_scene.getCamera(camera).node;
        const Matrix44f camViewMat = m_scene.updateMatrixCache(ETransformationMatrixType_Object, cameraNode);

        EXPECT_EQ(camViewMat, m_executorState.getCameraViewMatrix());
        EXPECT_TRUE(matrixFloatEquals(mat * camViewMat, m_executorState.getViewMatrix()));
    }

    TEST_F(ARenderExecutorInternalState, updatesCameraPositionSemanticValueWhenSettingCamera)
    {
        const Matrix44f mat(Matrix44f::Translation(Vector3(0.5f, 1.5f, 3.5f)));
        m_executorState.setRendererViewMatrix(mat);

        const CameraHandle camera = createTestCamera(Vector3(3, 5, 6), ECameraProjectionType_Perspective);
        m_executorState.setCamera(camera);

        EXPECT_EQ(Vector3(2.5f, 3.5f, 2.5f), m_executorState.getCameraWorldPosition());
    }

    TEST_F(ARenderExecutorInternalState, DISABLED_setsFramebufferProjectionMatrixWhenSettingCameraOfTypeRenderer)
    {
        const Matrix44f mat(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
        m_executorState.setRendererViewMatrix(mat);

        const CameraHandle camera = createTestCamera(Vector3(3, 5, 6));
        m_executorState.setCamera(camera);
        EXPECT_TRUE(matrixFloatEquals(m_fbProjMat, m_executorState.getProjectionMatrix()));
    }

    TEST_F(ARenderExecutorInternalState, updatesProjectionMatrixWhenSettingPerspectiveCamera)
    {
        const CameraHandle camera = createTestCamera(Vector3(0.0f), ECameraProjectionType_Perspective);

        m_executorState.setCamera(camera);

        const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(
            ProjectionParams::Perspective(FakeFoV, FakeAspectRatio, FakeNearPlane, FakeFarPlane));

        EXPECT_EQ(projMatrix, m_executorState.getProjectionMatrix());
    }

    TEST_F(ARenderExecutorInternalState, updatesProjectionMatrixWhenSettingOrthographicCamera)
    {
        const CameraHandle camera = createTestCamera(Vector3(0.0f), ECameraProjectionType_Orthographic);

        const Matrix44f projMatrix = CameraMatrixHelper::ProjectionMatrix(
            ProjectionParams::Frustum(ECameraProjectionType_Orthographic,
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
        const Matrix44f mat(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
        m_executorState.setRendererViewMatrix(mat);

        const CameraHandle camera = createTestCamera(Vector3(3, 5, 6), ECameraProjectionType_Perspective);
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
        m_scene.setRotation(renderableTransform, Vector3(1, 2, 3));

        NiceMock<ResourceDeviceHandleAccessorMock> resourceAccessor;
        NiceMock<EmbeddedCompositingManagerMock> embeddedCompositingManager;
        m_scene.updateRenderablesAndResourceCache(resourceAccessor, embeddedCompositingManager);
        m_scene.updateRenderableWorldMatrices();

        m_executorState.setRenderable(renderable);

        const Matrix44f modelMat = m_scene.updateMatrixCache(ETransformationMatrixType_World, renderableNode);
        const Matrix44f expectedViewMat = mat * camViewMat;
        const Matrix44f expectedProjectionMat = CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(FakeFoV, static_cast<Float>(FakeVpWidth) / FakeVpHeight, FakeNearPlane, FakeFarPlane));

        EXPECT_TRUE(matrixFloatEquals(modelMat, m_executorState.getModelMatrix()));
        EXPECT_TRUE(matrixFloatEquals(expectedViewMat * modelMat, m_executorState.getModelViewMatrix()));
        EXPECT_TRUE(matrixFloatEquals(expectedProjectionMat * expectedViewMat * modelMat, m_executorState.getModelViewProjectionMatrix()));
    }

    TEST_F(ARenderExecutorInternalState, initialSetToDepthRenderStateTriggersChange)
    {
        DepthStencilState depthState;
        depthState.m_depthFunc = EDepthFunc::Disabled;
        m_executorState.depthStencilState.setState(depthState);
        EXPECT_TRUE(m_executorState.depthStencilState.hasChanged());
        EXPECT_FALSE(depthState != m_executorState.depthStencilState.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToBlendRenderStateTriggersChange)
    {
        BlendState blendState;
        blendState.m_blendFactorSrcColor = EBlendFactor::OneMinusDstAlpha;
        m_executorState.blendState.setState(blendState);
        EXPECT_TRUE(m_executorState.blendState.hasChanged());
        EXPECT_FALSE(blendState != m_executorState.blendState.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToRasterizerRenderStateTriggersChange)
    {
        RasterizerState rasterState;
        rasterState.m_cullMode = ECullMode::Disabled;
        m_executorState.rasterizerState.setState(rasterState);
        EXPECT_TRUE(m_executorState.rasterizerState.hasChanged());
        EXPECT_FALSE(rasterState != m_executorState.rasterizerState.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToDefaultDeviceHandlesTriggersChange)
    {
        m_executorState.shaderDeviceHandle.setState(DeviceResourceHandle(0u));
        EXPECT_TRUE(m_executorState.shaderDeviceHandle.hasChanged());
        EXPECT_EQ(0u, m_executorState.shaderDeviceHandle.getState());

        m_executorState.indexBufferDeviceHandle.setState(DeviceResourceHandle(0u));
        EXPECT_TRUE(m_executorState.indexBufferDeviceHandle.hasChanged());
        EXPECT_EQ(0u, m_executorState.indexBufferDeviceHandle.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToRenderTargetTriggersChange)
    {
        m_executorState.renderTargetState.setState(RenderTargetHandle::Invalid());
        EXPECT_TRUE(m_executorState.renderTargetState.hasChanged());
        EXPECT_EQ(RenderTargetHandle::Invalid(), m_executorState.renderTargetState.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToRenderPassTriggersChange)
    {
        m_executorState.renderPassState.setState(RenderPassHandle(0u));
        EXPECT_TRUE(m_executorState.renderPassState.hasChanged());
        EXPECT_EQ(0u, m_executorState.renderPassState.getState());
    }

    TEST_F(ARenderExecutorInternalState, initialSetToViewportTriggersChange)
    {
        m_executorState.viewportState.setState(Viewport());
        EXPECT_TRUE(m_executorState.viewportState.hasChanged());
        EXPECT_EQ(Viewport(), m_executorState.viewportState.getState());
    }

    TEST_F(ARenderExecutorInternalState, canResetCachedStates)
    {
        m_executorState.blendState.setState(BlendState());
        m_executorState.depthStencilState.setState(DepthStencilState());
        m_executorState.rasterizerState.setState(RasterizerState());

        ASSERT_FALSE(m_executorState.blendState.hasChanged());
        ASSERT_FALSE(m_executorState.depthStencilState.hasChanged());
        ASSERT_FALSE(m_executorState.rasterizerState.hasChanged());

        m_executorState.blendState.reset();
        m_executorState.depthStencilState.reset();
        m_executorState.rasterizerState.reset();

        EXPECT_TRUE(m_executorState.blendState.hasChanged());
        EXPECT_TRUE(m_executorState.depthStencilState.hasChanged());
        EXPECT_TRUE(m_executorState.rasterizerState.hasChanged());

        EXPECT_FALSE(m_executorState.blendState.getState()          != BlendState());
        EXPECT_FALSE(m_executorState.depthStencilState.getState()   != DepthStencilState());
        EXPECT_FALSE(m_executorState.rasterizerState.getState()     != RasterizerState());
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
