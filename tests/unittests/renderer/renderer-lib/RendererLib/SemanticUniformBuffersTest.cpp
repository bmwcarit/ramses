//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SemanticUniformBuffers.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "TestSceneHelper.h"
#include "RendererResourceManagerMock.h"
#include <gtest/gtest.h>
#include <optional>

namespace ramses::internal
{
    using namespace testing;

    class ASemanticUniformBuffers : public ::testing::Test
    {
    public:
        ASemanticUniformBuffers()
            : m_rendererScenes{ m_rendererEventCollector }
            , m_scene{ m_rendererScenes.createScene(SceneInfo{}) }
            , m_sceneHelper{ m_scene, false, false }
        {
            // add some transformation to camera
            addTransformation(m_camera);
        }

    protected:
        void addTransformation(CameraHandle camera)
        {
            const auto cameraTransform = m_sceneHelper.m_sceneAllocator.allocateTransform(m_scene.getCamera(camera).node);
            m_scene.setRotation(cameraTransform, { 1, 2, 3, 4 }, ERotationType::Quaternion);
        }

        RenderableHandle createRenderable(RenderGroupHandle renderGroup, const std::unordered_set<EFixedSemantics>& semantics = {}, RenderableHandle handle = {})
        {
            const auto renderable = m_sceneHelper.createRenderable(renderGroup, {}, semantics, handle);

            // add some transformation
            const auto renderableTransform = m_sceneHelper.m_sceneAllocator.allocateTransform(m_scene.getRenderable(renderable).node);
            m_scene.setTranslation(renderableTransform, { 1, 2, 3 });

            return renderable;
        }

        void makeTransformChange(RenderableHandle renderable)
        {
            const auto node = m_scene.getRenderable(renderable).node;
            for (const auto& transform : m_scene.getTransforms())
            {
                if (transform.second->node == node)
                    m_scene.setTranslation(transform.first, m_scene.getTranslation(transform.first) + glm::vec3{ 1.f });
            }
        }

        void makeTransformChange(CameraHandle camera)
        {
            const auto node = m_scene.getCamera(camera).node;
            for (const auto& transform : m_scene.getTransforms())
            {
                if (transform.second->node == node)
                    m_scene.setTranslation(transform.first, m_scene.getTranslation(transform.first) + glm::vec3{ 1.f });
            }
        }

        void makeProjectionChange(CameraHandle camera)
        {
            const auto& cam = m_scene.getCamera(camera);
            const auto& ref = m_scene.getDataReference(cam.dataInstance, Camera::FrustumNearFarPlanesField);
            m_scene.setDataSingleVector2f(ref, DataFieldHandle{ 0 }, m_scene.getDataSingleVector2f(ref, DataFieldHandle{ 0 }) + glm::vec2{ 0.01f });
        }

        void sceneUpdaterUpdate()
        {
            // simulate order of commands done by RendererSceneUpdater
            m_scene.updateRenderablesAndResourceCache(m_resourceManagerMock);
            for (const auto& passInfo : m_scene.getSortedRenderingPasses())
            {
                const auto camera = m_scene.getRenderPass(passInfo.getRenderPassHandle()).camera;
                const auto& passRenderables = m_scene.getOrderedRenderablesForPass(passInfo.getRenderPassHandle());
                m_scene.collectDirtySemanticUniformBuffers(passRenderables, camera);
            }
            m_scene.updateRenderableWorldMatrices();
            m_scene.updateSemanticUniformBuffers();
            m_scene.uploadSemanticUniformBuffers(m_resourceManagerMock);
            Mock::VerifyAndClearExpectations(&m_resourceManagerMock);
        }

        DeviceResourceHandle expectUBOUpload(SemanticUniformBufferHandle uboHandle)
        {
            static DeviceResourceHandle deviceHandle{ 0u };
            uint32_t expectedUBOSize = 0u;
            switch (uboHandle.getType())
            {
            case SemanticUniformBufferHandle::Type::Model:
                expectedUBOSize = ExpectedModelUBOSize;
                break;
            case SemanticUniformBufferHandle::Type::Camera:
                expectedUBOSize = ExpectedCameraUBOSize;
                break;
            case SemanticUniformBufferHandle::Type::ModelCamera:
                expectedUBOSize = ExpectedModelCameraUBOSize;
                break;
            default:
                assert(false);
                break;
            }
            EXPECT_CALL(m_resourceManagerMock, uploadUniformBuffer(uboHandle, expectedUBOSize, m_scene.getSceneId()))
                .WillOnce(Return(++deviceHandle));

            return deviceHandle;
        }

        void expectUBOUnload(SemanticUniformBufferHandle uboHandle)
        {
            EXPECT_CALL(m_resourceManagerMock, unloadUniformBuffer(uboHandle, m_scene.getSceneId()));
        }

        void expectUBOUpdate(RenderableHandle renderable, std::optional<glm::mat4> expectModelMat = {})
        {
            EXPECT_CALL(m_resourceManagerMock, updateUniformBuffer(SemanticUniformBufferHandle{ renderable }, ExpectedModelUBOSize, _, m_scene.getSceneId()))
                .WillOnce([=](auto /*handle*/, auto /*size*/, const std::byte* data, auto /*sceneId*/)
                    {
                        if (expectModelMat)
                        {
                            const auto* floatData = reinterpret_cast<const float*>(data);
                            const auto mMat = glm::make_mat4(floatData);
                            for (size_t i = 0; i < 16; ++i)
                                EXPECT_FLOAT_EQ(glm::value_ptr(*expectModelMat)[i], glm::value_ptr(mMat)[i]) << i;
                        }
                    });
        }

        void expectUBOUpdate(CameraHandle camera,
            std::optional<glm::mat4> expectPMat = {},
            std::optional<glm::mat4> expectVMat = {},
            std::optional<glm::vec3> expectPos = {})
        {
            EXPECT_CALL(m_resourceManagerMock, updateUniformBuffer(SemanticUniformBufferHandle{ camera }, ExpectedCameraUBOSize, _, m_scene.getSceneId()))
                .WillOnce([=](auto /*handle*/, auto /*size*/, const std::byte* data, auto /*sceneId*/)
                    {
                        const auto* floatData = reinterpret_cast<const float*>(data);
                        const auto pMat = glm::make_mat4(floatData);
                        const auto vMat = glm::make_mat4(floatData + 16);
                        const auto pos = glm::make_vec3(floatData + 32);

                        if (expectPMat)
                        {
                            for (size_t i = 0; i < 16; ++i)
                                EXPECT_FLOAT_EQ(glm::value_ptr(*expectPMat)[i], glm::value_ptr(pMat)[i]) << i;
                        }

                        if (expectVMat)
                        {
                            for (size_t i = 0; i < 16; ++i)
                                EXPECT_FLOAT_EQ(glm::value_ptr(*expectVMat)[i], glm::value_ptr(vMat)[i]) << i;
                        }

                        if (expectPos)
                        {
                            for (size_t i = 0; i < 3; ++i)
                                EXPECT_FLOAT_EQ(glm::value_ptr(*expectPos)[i], glm::value_ptr(pos)[i]) << i;
                        }
                    });
        }

        void expectUBOUpdate(RenderableHandle renderable, CameraHandle camera,
            std::optional<glm::mat4> expectMVPMat = {},
            std::optional<glm::mat4> expectMVMat = {},
            std::optional<glm::mat4> expectNormalMat = {})
        {
            EXPECT_CALL(m_resourceManagerMock, updateUniformBuffer(SemanticUniformBufferHandle{ renderable, camera }, ExpectedModelCameraUBOSize, _, m_scene.getSceneId()))
                .WillOnce([=](auto /*handle*/, auto /*size*/, const std::byte* data, auto /*sceneId*/)
                    {
                        const auto* floatData = reinterpret_cast<const float*>(data);
                        const auto mvpMat = glm::make_mat4(floatData);
                        const auto mvMat = glm::make_mat4(floatData + 16);
                        const auto normalMat = glm::make_mat4(floatData + 32);

                        if (expectMVPMat)
                        {
                            for (size_t i = 0; i < 16; ++i)
                                EXPECT_FLOAT_EQ(glm::value_ptr(*expectMVPMat)[i], glm::value_ptr(mvpMat)[i]) << i;
                        }

                        if (expectMVMat)
                        {
                            for (size_t i = 0; i < 16; ++i)
                                EXPECT_FLOAT_EQ(glm::value_ptr(*expectMVMat)[i], glm::value_ptr(mvMat)[i]) << i;
                        }

                        if (expectNormalMat)
                        {
                            for (size_t i = 0; i < 16; ++i)
                                EXPECT_FLOAT_EQ(glm::value_ptr(*expectNormalMat)[i], glm::value_ptr(normalMat)[i]) << i;
                        }
                    });
        }

        RendererEventCollector m_rendererEventCollector;
        RendererScenes m_rendererScenes;
        RendererCachedScene& m_scene;
        TestSceneHelper m_sceneHelper;
        StrictMock<RendererResourceManagerMock> m_resourceManagerMock;

        static constexpr size_t ExpectedModelUBOSize = 1 * 16 * 4;
        static constexpr size_t ExpectedCameraUBOSize = 2 * 16 * 4 + 1 * 3 * 4;
        static constexpr size_t ExpectedModelCameraUBOSize = 3 * 16 * 4;

        // These match default transformations in this test.
        // Intentionally hardcoded so there is no dependency on internal code to calculate them
        const glm::mat4 ExpectedMMat{
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            1.f, 2.f, 3.f, 1.f };
        const glm::mat4 ExpectedPMat{
            0.1f, 0.f, 0.f, 0.f,
            0.f, 0.1f, 0.f, 0.f,
            0.f, 0.f, -1.222222f, -1.f,
            0.f, 0.f, -0.2222222f, 0.f };
        const glm::mat4 ExpectedVMat{
            -25.f, -20.f, 22.f, 0.f,
            28.f, -19.f, 4.f, 0.f,
            -10.f, 20.f, -9.f, 0.f,
            0.f, 0.f, 0.f, 1.f };
        const glm::vec3 ExpectedPos{ 0.f, 0.f, 0.f };
        const glm::mat4 ExpectedMVPMat{
            -2.5f, -2.f, -26.888891f, -22.f,
            2.8f, -1.9f, -4.888889f, -4.f,
            -1.f, 2.f, 11.000001f, 9.f,
            0.1f, 0.2f, -3.8888893f, -3.f };
        const glm::mat4 ExpectedMVMat{
            -25.f, -20.f, 22.f, 0.f,
            28.f, -19.f, 4.f, 0.f,
            -10.f, 20.f, -9.f, 0.f,
            1.f, 2.f, 3.f, 1.f };
        const glm::mat4 ExpectedNormalMat{
            0.056f, 0.13046152889728546f, 0.22769230604171753f, -1.f,
            0.16f, 0.27384614944458008f, 0.43076923489570618f, -2.f,
            0.208f, 0.44061538577079773f, 0.63692307472229004f, -3.f,
            0.f, 0.f, 0.f, 1.f };

        const RenderPassHandle m_rp = m_sceneHelper.createRenderPassWithCamera();
        const CameraHandle m_camera = m_scene.getRenderPass(m_rp).camera;
        const RenderGroupHandle m_rg = m_sceneHelper.createRenderGroup(m_rp);
    };

    TEST_F(ASemanticUniformBuffers, noUBOUploadIfNoRenderables)
    {
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, uploadsAndUpdatesUBOWhenAddedRenderable_model)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock });

        const auto deviceHandleC = expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        const auto deviceHandleMC = expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(renderable, ExpectedMMat);
        sceneUpdaterUpdate();
        EXPECT_EQ(deviceHandleC, m_scene.getDeviceHandle(m_camera));
        EXPECT_EQ(deviceHandleMC, m_scene.getDeviceHandle(renderable));

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, uploadsAndUpdatesUBOWhenAddedRenderable_modelCamera)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelCameraBlock });

        const auto deviceHandleC = expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        const auto deviceHandleMC = expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();
        EXPECT_EQ(deviceHandleC, m_scene.getDeviceHandle(m_camera));
        EXPECT_EQ(deviceHandleMC, m_scene.getDeviceHandle(renderable, m_camera));

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, uploadsAndUpdatesUBOWhenAddedRenderable_modelAndModelCamera)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });

        const auto deviceHandleC = expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        const auto deviceHandle1 = expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        const auto deviceHandle2 = expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();
        EXPECT_EQ(deviceHandleC, m_scene.getDeviceHandle(m_camera));
        EXPECT_EQ(deviceHandle1, m_scene.getDeviceHandle(renderable));
        EXPECT_EQ(deviceHandle2, m_scene.getDeviceHandle(renderable, m_camera));

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, doesNotUploadUBOWhenAddedRenderablesWithoutSemantic)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelCameraBlock });

        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // renderable with no semantic
        createRenderable(m_rg, {});
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, uploadsAndUpdatesUBOsForAllCameraRenderables)
    {
        sceneUpdaterUpdate();

        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto renderable2 = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });

        const auto deviceHandleC = expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        const auto deviceHandleM1 = expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        const auto deviceHandleM2 = expectUBOUpload(SemanticUniformBufferHandle{ renderable2 });
        const auto deviceHandleMC1 = expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        const auto deviceHandleMC2 = expectUBOUpload(SemanticUniformBufferHandle{ renderable2, m_camera });
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable2, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderable2, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        sceneUpdaterUpdate();

        EXPECT_EQ(deviceHandleC, m_scene.getDeviceHandle(m_camera));
        EXPECT_EQ(deviceHandleM1, m_scene.getDeviceHandle(renderable));
        EXPECT_EQ(deviceHandleM2, m_scene.getDeviceHandle(renderable2));
        EXPECT_EQ(deviceHandleMC1, m_scene.getDeviceHandle(renderable, m_camera));
        EXPECT_EQ(deviceHandleMC2, m_scene.getDeviceHandle(renderable2, m_camera));

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, uploadsAndUpdatesUBOsForAllCamerasRenderable)
    {
        // create another render pass with camera
        const auto rp2 = m_sceneHelper.createRenderPassWithCamera();
        const auto camera2 = m_scene.getRenderPass(rp2).camera;
        addTransformation(camera2);
        const auto rg2 = m_sceneHelper.createRenderGroup(rp2);
        sceneUpdaterUpdate();

        // renderable in RP1
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        // renderable in RP2
        m_scene.addRenderableToRenderGroup(rg2, renderable, 0);

        const auto deviceHandleC1 = expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        const auto deviceHandleC2 = expectUBOUpload(SemanticUniformBufferHandle{ camera2 });
        const auto deviceHandleM1 = expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        const auto deviceHandleMC1 = expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        const auto deviceHandleMC2 = expectUBOUpload(SemanticUniformBufferHandle{ renderable, camera2 });
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(camera2, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderable, camera2, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        sceneUpdaterUpdate();

        EXPECT_EQ(deviceHandleC1, m_scene.getDeviceHandle(m_camera));
        EXPECT_EQ(deviceHandleC2, m_scene.getDeviceHandle(camera2));
        EXPECT_EQ(deviceHandleM1, m_scene.getDeviceHandle(renderable));
        EXPECT_EQ(deviceHandleMC1, m_scene.getDeviceHandle(renderable, m_camera));
        EXPECT_EQ(deviceHandleMC2, m_scene.getDeviceHandle(renderable, camera2));

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, doesNotUpdateAgainUBOsForAdditionalCamerasRenderablWhichAlreadyExists)
    {
        // renderable in RP1
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });

        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        sceneUpdaterUpdate();

        // create another render pass with the SAME camera and the SAME renderable
        const auto rp2 = m_sceneHelper.m_sceneAllocator.allocateRenderPass();
        m_scene.setRenderPassCamera(rp2, m_camera);
        const auto rg2 = m_sceneHelper.createRenderGroup(rp2);
        m_scene.addRenderableToRenderGroup(rg2, renderable, 0);

        // UBOs for renderable and renderable/camera pair already exist and are up-to-date -> expect nothing
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, uploadsAndUpdatesUBOForNewCameraRenderablePairButBothCameraAndRenderableNotDirty)
    {
        // camera1 + renderable1 in RP1
        const auto renderable1 = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        // camera2 + renderable2 in RP2
        const auto rp2 = m_sceneHelper.createRenderPassWithCamera();
        const auto camera2 = m_scene.getRenderPass(rp2).camera;
        addTransformation(camera2);
        const auto rg2 = m_sceneHelper.createRenderGroup(rp2);
        const auto renderable2 = createRenderable(rg2, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });

        // upload all, all objects not dirty afterwards
        {
            expectUBOUpload(SemanticUniformBufferHandle{ camera2 });
            expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
            expectUBOUpload(SemanticUniformBufferHandle{ renderable1 });
            expectUBOUpload(SemanticUniformBufferHandle{ renderable2 });
            expectUBOUpload(SemanticUniformBufferHandle{ renderable1, m_camera });
            expectUBOUpload(SemanticUniformBufferHandle{ renderable2, camera2 });
            expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
            expectUBOUpdate(camera2, ExpectedPMat, ExpectedVMat, ExpectedPos);
            expectUBOUpdate(renderable1, ExpectedMMat);
            expectUBOUpdate(renderable2, ExpectedMMat);
            expectUBOUpdate(renderable1, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
            expectUBOUpdate(renderable2, camera2, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
            sceneUpdaterUpdate();
        }

        // create third render pass with camera1 + renderable2
        const auto rp3 = m_sceneHelper.m_sceneAllocator.allocateRenderPass();
        m_scene.setRenderPassCamera(rp3, m_camera);
        const auto rg3 = m_sceneHelper.createRenderGroup(rp3);
        m_scene.addRenderableToRenderGroup(rg3, renderable2, 0);

        // a new UBO for camera1 + renderable2, both NOT dirty
        {
            expectUBOUpload(SemanticUniformBufferHandle{ renderable2, m_camera });
            expectUBOUpdate(renderable2, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
            sceneUpdaterUpdate();
        }

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, updatesOnlyRelevantUBOOnChangeOfEitherCameraOrRenderable)
    {
        // setup 2 RPs, each with camera and renderable
        const auto rp2 = m_sceneHelper.createRenderPassWithCamera();
        const auto camera2 = m_scene.getRenderPass(rp2).camera;
        addTransformation(camera2);
        const auto rg2 = m_sceneHelper.createRenderGroup(rp2);
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto renderable2 = createRenderable(rg2, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });

        // third RP uses existing camera and renderable2
        const auto rp3 = m_sceneHelper.m_sceneAllocator.allocateRenderPass();
        m_scene.setRenderPassCamera(rp3, m_camera);
        const auto rg3 = m_sceneHelper.createRenderGroup(rp3);
        m_scene.addRenderableToRenderGroup(rg3, renderable2, 0);

        // and another renderable with no MVP
        const auto renderable3 = createRenderable(m_rg, {});

        const auto deviceHandleC1 = expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        const auto deviceHandleC2 = expectUBOUpload(SemanticUniformBufferHandle{ camera2 });
        const auto deviceHandleM1 = expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        const auto deviceHandleM2 = expectUBOUpload(SemanticUniformBufferHandle{ renderable2 });
        const auto deviceHandleMC1 = expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        const auto deviceHandleMC2 = expectUBOUpload(SemanticUniformBufferHandle{ renderable2, camera2 });
        const auto deviceHandleMC3 = expectUBOUpload(SemanticUniformBufferHandle{ renderable2, m_camera });
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(camera2, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable2, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderable2, camera2, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderable2, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        // always updating twice, making sure additional update with no other change is noop
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        // modify renderable
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        makeTransformChange(renderable);
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        // modify renderable2
        expectUBOUpdate(renderable2);
        expectUBOUpdate(renderable2, camera2);
        expectUBOUpdate(renderable2, m_camera);
        makeTransformChange(renderable2);
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        // modify renderable3 (no MVP)
        makeTransformChange(renderable3);
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        // modify camera transformation
        expectUBOUpdate(m_camera);
        expectUBOUpdate(renderable, m_camera);
        expectUBOUpdate(renderable2, m_camera);
        makeTransformChange(m_camera);
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        // modify camera2 transformation
        expectUBOUpdate(camera2);
        expectUBOUpdate(renderable2, camera2);
        makeTransformChange(camera2);
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        // modify camera projection
        expectUBOUpdate(m_camera);
        expectUBOUpdate(renderable, m_camera);
        expectUBOUpdate(renderable2, m_camera);
        makeProjectionChange(m_camera);
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        // modify camera2 projection
        expectUBOUpdate(camera2);
        expectUBOUpdate(renderable2, camera2);
        makeProjectionChange(camera2);
        sceneUpdaterUpdate();
        sceneUpdaterUpdate();

        EXPECT_EQ(deviceHandleC1, m_scene.getDeviceHandle(m_camera));
        EXPECT_EQ(deviceHandleC2, m_scene.getDeviceHandle(camera2));
        EXPECT_EQ(deviceHandleM1, m_scene.getDeviceHandle(renderable));
        EXPECT_EQ(deviceHandleM2, m_scene.getDeviceHandle(renderable2));
        EXPECT_EQ(deviceHandleMC1, m_scene.getDeviceHandle(renderable, m_camera));
        EXPECT_EQ(deviceHandleMC2, m_scene.getDeviceHandle(renderable2, camera2));
        EXPECT_EQ(deviceHandleMC3, m_scene.getDeviceHandle(renderable2, m_camera));
    }

    TEST_F(ASemanticUniformBuffers, updatesOnlyRelevantUBOOnChangeDependingOnWhichRenderableSemanticIsUsed)
    {
        // prepare data instances with various usages of renderable ubo semantics
        ResourceContentHash dummy;
        const auto dataLayoutWithModelSemantics = m_sceneHelper.m_sceneAllocator.allocateDataLayout({
            DataFieldInfo{ EDataType::Matrix44F, 1u, EFixedSemantics::ModelBlock }}, dummy, {});
        const auto dataLayoutWithModelAndModelCameraSemantics = m_sceneHelper.m_sceneAllocator.allocateDataLayout({
            DataFieldInfo{ EDataType::Matrix44F, 1u, EFixedSemantics::ModelBlock },
            DataFieldInfo{ EDataType::Matrix44F, 1u, EFixedSemantics::ModelCameraBlock } }, dummy, {});
        const auto uniformsWithModelSemantics = m_sceneHelper.m_sceneAllocator.allocateDataInstance(dataLayoutWithModelSemantics);
        const auto uniformsWithModelAndModelCameraSemantics = m_sceneHelper.m_sceneAllocator.allocateDataInstance(dataLayoutWithModelAndModelCameraSemantics);
        const auto uniformsWithNoSemantics = DataInstanceHandle::Invalid();

        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });

        // note that camera ubo is currently always processed regardless of uniform semantics
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);

        // no semantics no ubo
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformsWithNoSemantics);
        sceneUpdaterUpdate();

        // model semantics
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformsWithModelSemantics);
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpdate(renderable, ExpectedMMat);
        sceneUpdaterUpdate();

        // model and model/camera semantics
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformsWithModelAndModelCameraSemantics);
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        sceneUpdaterUpdate();

        // modify renderable -> both ubos updated
        makeTransformChange(renderable);
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        sceneUpdaterUpdate();

        // change to model only and modify -> only model ubo updated
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformsWithModelSemantics);
        makeTransformChange(renderable);
        expectUBOUpdate(renderable);
        sceneUpdaterUpdate();

        // change to no semantics and modify -> no ubo updated
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformsWithNoSemantics);
        makeTransformChange(renderable);
        sceneUpdaterUpdate();

        // change back to both semantics -> both ubos updated
        m_scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformsWithModelAndModelCameraSemantics);
        makeTransformChange(renderable);
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, unloadsUBOOnlyWhenNotUsedLongEnough_byReleasingIt)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto renderableUnused = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableUnused });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableUnused, m_camera });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderableUnused, ExpectedMMat);
        expectUBOUpdate(renderableUnused, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // make renderable unused by removing it
        m_scene.removeRenderableFromRenderGroup(m_rg, renderableUnused);
        m_scene.releaseRenderable(renderableUnused);

        // no unload, decays not even processed if no update
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate; ++decay)
            sceneUpdaterUpdate();

        // decays/unloads are processed only if there is a change (dirty caused by modification or new MVP)
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate - 1; ++decay)
        {
            makeTransformChange(renderable);
            expectUBOUpdate(renderable);
            expectUBOUpdate(renderable, m_camera);
            sceneUpdaterUpdate();
        }

        // decay of unused renderable reached threshold to release its UBOs
        makeTransformChange(renderable);
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        expectUBOUnload(SemanticUniformBufferHandle{ renderableUnused });
        expectUBOUnload(SemanticUniformBufferHandle{ renderableUnused, m_camera });
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, unloadsUBOOnlyWhenNotUsedLongEnough_byRemovingItsSemanticUsage)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto renderableUnused = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableUnused });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableUnused, m_camera });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderableUnused, ExpectedMMat);
        expectUBOUpdate(renderableUnused, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // make renderable unused by removing its semantic usage
        m_scene.setRenderableDataInstance(renderableUnused, ERenderableDataSlotType::ERenderableDataSlotType_Uniforms, DataInstanceHandle::Invalid());

        // no unload, decays not even processed if no update
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate; ++decay)
            sceneUpdaterUpdate();

        // decays/unloads are processed only if there is a change (dirty caused by modification or new MVP)
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate - 1; ++decay)
        {
            makeTransformChange(renderable);
            expectUBOUpdate(renderable);
            expectUBOUpdate(renderable, m_camera);
            sceneUpdaterUpdate();
        }

        // decay of unused renderable reached threshold to release its UBOs
        makeTransformChange(renderable);
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        expectUBOUnload(SemanticUniformBufferHandle{ renderableUnused });
        expectUBOUnload(SemanticUniformBufferHandle{ renderableUnused, m_camera });
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, reallocatingRenderableTriggersNewUploadAndUpdate)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // remove renderable
        m_scene.removeRenderableFromRenderGroup(m_rg, renderable);
        m_scene.releaseRenderable(renderable);
        sceneUpdaterUpdate();

        // recreate and expect new upload and update
        const auto renderableRenewed = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        EXPECT_NE(renderable, renderableRenewed);
        expectUBOUpload(SemanticUniformBufferHandle{ renderableRenewed });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableRenewed, m_camera });
        expectUBOUpdate(renderableRenewed, ExpectedMMat);
        expectUBOUpdate(renderableRenewed, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, reallocatingRenderableTriggersNewUpdateNotUpload_reusingSameHandle)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // remove renderable
        m_scene.removeRenderableFromRenderGroup(m_rg, renderable);
        m_scene.releaseRenderable(renderable);
        sceneUpdaterUpdate();

        // recreate and expect update (UBOs kept uploaded)
        const auto renderableRenewed = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock }, renderable);
        EXPECT_EQ(renderable, renderableRenewed);
        expectUBOUpdate(renderableRenewed, ExpectedMMat);
        expectUBOUpdate(renderableRenewed, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, reallocatingRenderableTriggersNewUploadAndUpdate_withDecay)
    {
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto renderableToReallocate = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableToReallocate });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableToReallocate, m_camera });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderableToReallocate, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderableToReallocate, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // remove renderable
        m_scene.removeRenderableFromRenderGroup(m_rg, renderableToReallocate);
        m_scene.releaseRenderable(renderableToReallocate);
        sceneUpdaterUpdate();

        // decay and let MVP unload
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate - 1; ++decay)
        {
            makeTransformChange(renderable);
            expectUBOUpdate(renderable);
            expectUBOUpdate(renderable, m_camera);
            sceneUpdaterUpdate();
        }
        makeTransformChange(renderable);
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        expectUBOUnload(SemanticUniformBufferHandle{ renderableToReallocate });
        expectUBOUnload(SemanticUniformBufferHandle{ renderableToReallocate, m_camera });
        sceneUpdaterUpdate();

        // recreate and expect new upload and update
        const auto renderableRenewed = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock }, renderableToReallocate);
        EXPECT_EQ(renderableToReallocate, renderableRenewed);
        expectUBOUpload(SemanticUniformBufferHandle{ renderableRenewed });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableRenewed, m_camera });
        expectUBOUpdate(renderableRenewed, ExpectedMMat);
        expectUBOUpdate(renderableRenewed, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, decaysAndRefreshesUBOsOfDisabledAndReenabledRenderPass)
    {
        // setup 2 RPs, each with camera and renderable
        const auto rp2 = m_sceneHelper.createRenderPassWithCamera();
        const auto camera2 = m_scene.getRenderPass(rp2).camera;
        addTransformation(camera2);
        const auto rg2 = m_sceneHelper.createRenderGroup(rp2);
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto renderable2 = createRenderable(rg2, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ camera2 });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable2 });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable2, camera2 });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable2, ExpectedMMat);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderable2, camera2, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(camera2, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // disable RP1
        m_scene.setRenderPassEnabled(m_rp, false);
        sceneUpdaterUpdate();

        // let its UBOs decay
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate - 1; ++decay)
        {
            // updating both renderable and camera so their decays are tracked (decay only tracked when something changed)
            makeTransformChange(renderable2);
            makeTransformChange(camera2);
            expectUBOUpdate(renderable2);
            expectUBOUpdate(renderable2, camera2);
            expectUBOUpdate(camera2);
            sceneUpdaterUpdate();
        }
        makeTransformChange(renderable2);
        makeTransformChange(camera2);
        expectUBOUpdate(renderable2);
        expectUBOUpdate(renderable2, camera2);
        expectUBOUpdate(camera2);
        expectUBOUnload(SemanticUniformBufferHandle{ renderable });
        expectUBOUnload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUnload(SemanticUniformBufferHandle{ m_camera });
        sceneUpdaterUpdate();

        // reenable RP1 and expect reupload+update
        m_scene.setRenderPassEnabled(m_rp, true);
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(m_camera);
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        sceneUpdaterUpdate();

        // disable RP2
        m_scene.setRenderPassEnabled(rp2, false);
        sceneUpdaterUpdate();

        // let RP2 UBOs decay
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate - 1; ++decay)
        {
            makeTransformChange(renderable);
            makeTransformChange(m_camera);
            expectUBOUpdate(renderable);
            expectUBOUpdate(renderable, m_camera);
            expectUBOUpdate(m_camera);
            sceneUpdaterUpdate();
        }
        makeTransformChange(renderable);
        makeTransformChange(m_camera);
        expectUBOUpdate(renderable);
        expectUBOUpdate(renderable, m_camera);
        expectUBOUpdate(m_camera);
        expectUBOUnload(SemanticUniformBufferHandle{ renderable2 });
        expectUBOUnload(SemanticUniformBufferHandle{ renderable2, camera2 });
        expectUBOUnload(SemanticUniformBufferHandle{ camera2 });
        sceneUpdaterUpdate();

        // reenable RP2 and expect reupload+update
        m_scene.setRenderPassEnabled(rp2, true);
        expectUBOUpload(SemanticUniformBufferHandle{ camera2 });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable2 });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable2, camera2 });
        expectUBOUpdate(renderable2);
        expectUBOUpdate(renderable2, camera2);
        expectUBOUpdate(camera2);
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, decaysAllUBOsOfDisabledRenderPass)
    {
        // setup 2 RPs, first with 2 renderables
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto renderable2 = createRenderable(m_rg, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        const auto rp2 = m_sceneHelper.createRenderPassWithCamera();
        const auto camera2 = m_scene.getRenderPass(rp2).camera;
        addTransformation(camera2);
        const auto rg2 = m_sceneHelper.createRenderGroup(rp2);
        const auto renderableDummy = createRenderable(rg2, { EFixedSemantics::ModelBlock, EFixedSemantics::ModelCameraBlock });
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ camera2 });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable2 });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableDummy });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable2, m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderableDummy, camera2 });
        expectUBOUpdate(renderable, ExpectedMMat);
        expectUBOUpdate(renderable2, ExpectedMMat);
        expectUBOUpdate(renderableDummy);
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderable2, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderableDummy, camera2);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(camera2, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // disable RP1
        m_scene.setRenderPassEnabled(m_rp, false);
        sceneUpdaterUpdate();

        // let its UBOs decay
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate - 1; ++decay)
        {
            makeTransformChange(renderableDummy);
            makeTransformChange(camera2);
            expectUBOUpdate(renderableDummy);
            expectUBOUpdate(renderableDummy, camera2);
            expectUBOUpdate(camera2);
            sceneUpdaterUpdate();
        }

        // expect all RP1 UBOs unloaded
        makeTransformChange(renderableDummy);
        makeTransformChange(camera2);
        expectUBOUpdate(renderableDummy);
        expectUBOUpdate(renderableDummy, camera2);
        expectUBOUpdate(camera2);
        expectUBOUnload(SemanticUniformBufferHandle{ renderable });
        expectUBOUnload(SemanticUniformBufferHandle{ renderable2 });
        expectUBOUnload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUnload(SemanticUniformBufferHandle{ renderable2, m_camera });
        expectUBOUnload(SemanticUniformBufferHandle{ m_camera });
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, decaysOnlyUnusedMVPOfTwoMVPsUsingSameRenderable)
    {
        // setup 2 RPs, each with its camera but using same renderable
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelCameraBlock });
        const auto rp2 = m_sceneHelper.createRenderPassWithCamera();
        const auto camera2 = m_scene.getRenderPass(rp2).camera;
        const auto rg2 = m_sceneHelper.createRenderGroup(rp2);
        m_scene.addRenderableToRenderGroup(rg2, renderable, 0);
        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ camera2 });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, camera2 });
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(renderable, camera2);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        expectUBOUpdate(camera2);
        sceneUpdaterUpdate();

        // disable RP2
        m_scene.setRenderPassEnabled(rp2, false);
        sceneUpdaterUpdate();

        // let its MVPs decay
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate - 1; ++decay)
        {
            makeTransformChange(renderable);
            expectUBOUpdate(renderable, m_camera);
            sceneUpdaterUpdate();
        }

        // expect only the MVP from disabled RP2 unloaded
        makeTransformChange(renderable);
        expectUBOUpdate(renderable, m_camera);
        expectUBOUnload(SemanticUniformBufferHandle{ renderable, camera2 });
        sceneUpdaterUpdate();

        // another update with no change
        sceneUpdaterUpdate();
    }

    TEST_F(ASemanticUniformBuffers, decaysNoMVPIfCameraRenderablePairUsedInTwoRPsAndOneDisabled)
    {
        // setup 2 RPs, using same camera and same renderable
        const auto renderable = createRenderable(m_rg, { EFixedSemantics::ModelCameraBlock });
        const auto rp2 = m_sceneHelper.m_sceneAllocator.allocateRenderPass();
        m_scene.setRenderPassCamera(rp2, m_camera);
        m_scene.addRenderGroupToRenderPass(rp2, m_rg, 0);

        expectUBOUpload(SemanticUniformBufferHandle{ m_camera });
        expectUBOUpload(SemanticUniformBufferHandle{ renderable, m_camera });
        expectUBOUpdate(renderable, m_camera, ExpectedMVPMat, ExpectedMVMat, ExpectedNormalMat);
        expectUBOUpdate(m_camera, ExpectedPMat, ExpectedVMat, ExpectedPos);
        sceneUpdaterUpdate();

        // disable RP2
        m_scene.setRenderPassEnabled(rp2, false);
        sceneUpdaterUpdate();

        // expect no unload because the MVP from disabled RP2 is also used actively in RP1
        for (uint32_t decay = 0u; decay < SemanticUniformBuffers_ModelCamera::DecayCountToDeallocate + 10; ++decay)
        {
            makeTransformChange(renderable);
            expectUBOUpdate(renderable, m_camera);
            sceneUpdaterUpdate();
        }
    }
}
