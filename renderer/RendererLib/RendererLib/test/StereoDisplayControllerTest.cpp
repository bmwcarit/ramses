//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayControllerTestBase.h"
#include "RendererLib/StereoDisplayController.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/RendererScenes.h"
#include "ResourceProviderMock.h"
#include "RendererEventCollector.h"
#include "EmbeddedCompositingManagerMock.h"
#include "SceneAllocateHelper.h"

namespace ramses_internal
{
    using namespace testing;

    class StereoDisplayControllerFacade : public StereoDisplayController
    {
    public:
        explicit StereoDisplayControllerFacade(IRenderBackend& renderer)
            : StereoDisplayController(renderer)
        {
        }

        bool framebufferDeviceHandleEqualTo(DeviceResourceHandle handle)
        {
            // test on correct scene render target
            return getDisplayBuffer() == handle;
        }

        bool framebufferDeviceHandleEqualTo(DeviceResourceHandle handleLeft, DeviceResourceHandle handleRight)
        {
            return m_viewInfo[0].m_fbInfo.deviceHandle == handleLeft && m_viewInfo[1].m_fbInfo.deviceHandle == handleRight;
        }

        bool eyePositionEqualTo(UInt32 eye, const Vector3& position)
        {
            return position == m_viewInfo[eye].m_eyePosition;
        }

        bool viewportEqualTo(UInt32 eye, const Viewport& viewport)
        {
            return m_viewInfo[eye].m_fbInfo.viewport.posX      == viewport.posX
                && m_viewInfo[eye].m_fbInfo.viewport.posY      == viewport.posY
                && m_viewInfo[eye].m_fbInfo.viewport.width     == viewport.width
                && m_viewInfo[eye].m_fbInfo.viewport.height    == viewport.height;
        }

        bool projectionParamsEqualTo(UInt32 eye, const ProjectionParams& params)
        {
            return m_viewInfo[eye].m_fbInfo.projectionParams == params;
        }
    };

    class AStereoDisplayController : public DisplayControllerTestBase, public ::testing::Test
    {
    protected:
        StereoDisplayControllerFacade& createStereoDisplayController()
        {
            expectCreationSequence();
            // init two targets for explicit stereo rendering on seperated targets
            EXPECT_CALL(m_renderBackend.deviceMock, uploadRenderBuffer(_)).Times(4);
            EXPECT_CALL(m_renderBackend.deviceMock, uploadRenderTarget(_)).Times(2);
            return *new StereoDisplayControllerFacade(m_renderBackend);
        }
    };

    TEST_F(AStereoDisplayController, InitializesEyePositionsToZero)
    {
        StereoDisplayControllerFacade& stereoController = createStereoDisplayController();
        EXPECT_TRUE(stereoController.eyePositionEqualTo(0, Vector3(0.0f)));
        destroyDisplayController(stereoController);
    }

    TEST_F(AStereoDisplayController, InitializesRenderTargetDeviceHandlesForBothEyesToFramebuffer)
    {
        StereoDisplayControllerFacade& stereoController = createStereoDisplayController();
        EXPECT_TRUE(stereoController.framebufferDeviceHandleEqualTo(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle));
        EXPECT_TRUE(stereoController.framebufferDeviceHandleEqualTo(DeviceMock::FakeRenderTargetDeviceHandle, DeviceMock::FakeRenderTargetDeviceHandle));
        destroyDisplayController(stereoController);
    }

    TEST_F(AStereoDisplayController, InitializesViewportsToHalfScreenSplittingVertically)
    {
        StereoDisplayControllerFacade& stereoController = createStereoDisplayController();

        // We use separated frame buffers for rendering so view point is the same for both
        Viewport vp(0, 0, WindowMock::FakeWidth / 2, WindowMock::FakeHeight);
        EXPECT_TRUE(stereoController.viewportEqualTo(0, vp));
        EXPECT_TRUE(stereoController.viewportEqualTo(1, vp));

        destroyDisplayController(stereoController);
    }

    TEST_F(AStereoDisplayController, InitializesProjectionToHalfScreenSplittingVertically)
    {
        StereoDisplayControllerFacade& stereoController = createStereoDisplayController();

        const ProjectionParams params = ProjectionParams::Perspective(2.0f, static_cast<float>(WindowMock::FakeWidth) / WindowMock::FakeHeight, 1.0f, 10.0f);
        stereoController.setProjectionParams(params);

        ProjectionParams paramsAdjustedForStereoRendering(params);
        paramsAdjustedForStereoRendering = ProjectionParams::Perspective(
            ProjectionParams::GetPerspectiveFovY(paramsAdjustedForStereoRendering),
            ProjectionParams::GetAspectRatio(paramsAdjustedForStereoRendering) * 0.5f,
            paramsAdjustedForStereoRendering.nearPlane,
            paramsAdjustedForStereoRendering.farPlane);
        EXPECT_TRUE(stereoController.projectionParamsEqualTo(0, paramsAdjustedForStereoRendering));
        EXPECT_TRUE(stereoController.projectionParamsEqualTo(1, paramsAdjustedForStereoRendering));

        destroyDisplayController(stereoController);
    }

    TEST_F(AStereoDisplayController, ExecutesSceneForBothEyesWithProperViewport)
    {
        StereoDisplayControllerFacade& stereoController = createStereoDisplayController();
        stereoController.setProjectionParams(ProjectionParams::Perspective(1.f, 1.f, 1.f, 2.f)); // dummy but valid params

        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes(rendererEventCollector);
        RendererCachedScene& scene = rendererScenes.createScene(SceneInfo());
        SceneAllocateHelper sceneAllocator(scene);
        const RenderPassHandle pass = sceneAllocator.allocateRenderPass();
        const NodeHandle cameraNode = sceneAllocator.allocateNode();
        const auto dataLayout = sceneAllocator.allocateDataLayout({ {EDataType_Vector2I}, {EDataType_Vector2I} });
        const CameraHandle camera = sceneAllocator.allocateCamera(ECameraProjectionType_Renderer, cameraNode, sceneAllocator.allocateDataInstance(dataLayout));
        sceneAllocator.allocateTransform(cameraNode);
        scene.setRenderPassCamera(pass, camera);

        NiceMock<ResourceDeviceHandleAccessorMock> resourceAccessor;
        NiceMock<EmbeddedCompositingManagerMock> embeddedCompositingManager;
        scene.updateRenderablesAndResourceCache(resourceAccessor, embeddedCompositingManager);

        Viewport left(0, 0, WindowMock::FakeWidth / 2, WindowMock::FakeHeight);
        Viewport right(0, 0, WindowMock::FakeWidth / 2, WindowMock::FakeHeight);

        InSequence seq;
        EXPECT_CALL(m_renderBackend.deviceMock, activateRenderTarget(DeviceMock::FakeRenderTargetDeviceHandle));
        EXPECT_CALL(m_renderBackend.deviceMock, setViewport(left.posX, left.posY, left.width, left.height));
        EXPECT_CALL(m_renderBackend.deviceMock, activateRenderTarget(DeviceMock::FakeRenderTargetDeviceHandle));
        EXPECT_CALL(m_renderBackend.deviceMock, setViewport(right.posX, right.posY, right.width, right.height));

        const DeviceResourceHandle unusedBuffer(0);
        const Viewport unusedViewport(1, 2, 3, 4);
        stereoController.renderScene(scene, unusedBuffer, unusedViewport);

        destroyDisplayController(stereoController);
    }

}
