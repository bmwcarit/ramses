//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses/client/ArrayResource.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/SceneObjectIterator.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/framework/EDataType.h"
#include "ramses/framework/EFeatureLevel.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

#include "impl/DataObjectImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/BlitPassImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/Texture2DImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/SceneConfigImpl.h"
#include "ClientTestUtils.h"
#include "SimpleSceneTopology.h"
#include "FileDescriptorHelper.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Components/FileInputStreamContainer.h"
#include "internal/PlatformAbstraction/PlatformTime.h"

using namespace testing;

namespace ramses::internal
{
    class AScene : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        AScene()
        {
            effectDescriptionEmpty.setVertexShader("void main(void) {gl_Position=vec4(0);}");
            effectDescriptionEmpty.setFragmentShader("void main(void) {gl_FragColor=vec4(0);}");
        }

    protected:
        ramses::EffectDescription effectDescriptionEmpty;
    };

    TEST(SceneOjects, canGiveOwningScene)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework framework{config};
        RamsesClient& client(*framework.createClient({}));
        ramses::Scene* sceneA = client.createScene(SceneConfig(sceneId_t(1u)));
        ramses::Scene* sceneB = client.createScene(SceneConfig(sceneId_t(2u)));

        ramses::Node* fromSceneA = sceneA->createNode();
        ramses::SceneObject* objectFromB = sceneB->createNode();

        EXPECT_EQ(sceneA, &fromSceneA->getScene());
        EXPECT_EQ(sceneB, &objectFromB->getScene());

        // use case to retrieve scene object itself
        EXPECT_EQ(sceneA, client.getScene(fromSceneA->getScene().getSceneId()));
    }

    TEST(ASceneConfig, CanBeCopyAndMoveConstructed)
    {
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode::LocalAndRemote);
        config.setRenderBackendCompatibility(ERenderBackendCompatibility::VulkanAndOpenGL);

        SceneConfig configCopy{ config };
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, configCopy.impl().getPublicationMode());
        EXPECT_EQ(ERenderBackendCompatibility::VulkanAndOpenGL, configCopy.impl().getRenderBackendCompatibility());

        SceneConfig configMove{ std::move(config) };
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, configMove.impl().getPublicationMode());
        EXPECT_EQ(ERenderBackendCompatibility::VulkanAndOpenGL, configCopy.impl().getRenderBackendCompatibility());
    }

    TEST(ASceneConfig, CanBeCopyAndMoveAssigned)
    {
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode::LocalAndRemote);

        SceneConfig configCopy;
        configCopy = config;
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, configCopy.impl().getPublicationMode());

        SceneConfig configMove;
        configMove = std::move(config);
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, configMove.impl().getPublicationMode());
    }

    TEST(ASceneConfig, CanBeSelfAssigned)
    {
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode::LocalAndRemote);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        config = config;
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, config.impl().getPublicationMode());
        config = std::move(config);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, config.impl().getPublicationMode());
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }

    class ASceneWithContent : public SimpleSceneTopology
    {
    };

    static int32_t GetElapsed(int32_t start, int32_t end)
    {
        if (start == end)
            return 0;
        if (start < end)
        {
            return end - start;
        }
        constexpr auto limit = std::numeric_limits<int32_t>::max();
        return (limit - start) + end + 1;
    }

    TEST(GetElapsedSelfTest, overflow)
    {
        EXPECT_EQ(0, GetElapsed(0, 0));
        EXPECT_EQ(1000, GetElapsed(0, 1000));
        EXPECT_EQ(1, GetElapsed(0x7fffffff, 0));
        EXPECT_EQ(1000, GetElapsed(0x7fffffff, 999));
        EXPECT_EQ(1000, GetElapsed(0x7fffffff - 999, 0));
    }


    TEST(DistributedSceneTest, givesErrorWhenRemotePublishingAfterEnablingLocalOnlyOptmisations)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient({}));
        framework.connect();
        SceneConfig config(sceneId_t(1u));
        config.setPublicationMode(EScenePublicationMode::LocalOnly);
        ramses::Scene* distributedScene = remoteClient.createScene(config);
        EXPECT_FALSE(distributedScene->publish(EScenePublicationMode::LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishRemotelyIfSetInConfig)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient({}));
        framework.connect();
        SceneConfig config(sceneId_t(1u));
        config.setPublicationMode(EScenePublicationMode::LocalAndRemote);
        ramses::Scene* distributedScene = remoteClient.createScene(config);
        EXPECT_TRUE(distributedScene->publish(EScenePublicationMode::LocalAndRemote));
    }

    TEST(DistributedSceneTest, cannotPublishRemotelyByDefault)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient({}));
        framework.connect();
        SceneConfig config(sceneId_t(1u));
        ramses::Scene* distributedScene = remoteClient.createScene(config);
        EXPECT_FALSE(distributedScene->publish(EScenePublicationMode::LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishLocallyWhenEnablingLocalOnlyOptimisations)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient({}));
        framework.connect();
        SceneConfig config(sceneId_t(1u));
        config.setPublicationMode(EScenePublicationMode::LocalOnly);
        ramses::Scene* distributedScene = remoteClient.createScene(config);
        EXPECT_TRUE(distributedScene->publish(EScenePublicationMode::LocalOnly));
    }

    TEST(DistributedSceneTest, isPublishedAfterDisconnect_LocalOnly)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient({}));
        framework.connect();
        SceneConfig config(sceneId_t(1u));
        config.setPublicationMode(EScenePublicationMode::LocalOnly);
        ramses::Scene* distributedScene = remoteClient.createScene(config);
        EXPECT_TRUE(distributedScene->publish(EScenePublicationMode::LocalOnly));
        framework.disconnect();
        EXPECT_TRUE(distributedScene->isPublished());
        framework.connect();
        EXPECT_TRUE(distributedScene->isPublished());
        EXPECT_FALSE(distributedScene->publish(EScenePublicationMode::LocalOnly));
    }

    TEST(DistributedSceneTest, isPublishedAfterDisconnect_Remote)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient({}));
        framework.connect();
        SceneConfig config(sceneId_t(1u));
        config.setPublicationMode(EScenePublicationMode::LocalAndRemote);
        ramses::Scene* distributedScene = remoteClient.createScene(config);
        EXPECT_TRUE(distributedScene->publish(EScenePublicationMode::LocalAndRemote));
        framework.disconnect();
        EXPECT_TRUE(distributedScene->isPublished());
        framework.connect();
        EXPECT_TRUE(distributedScene->isPublished());
        EXPECT_FALSE(distributedScene->publish(EScenePublicationMode::LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishSceneIfNotConnected)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient({}));
        ramses::Scene* distributedScene = remoteClient.createScene(SceneConfig(sceneId_t(1u)));
        EXPECT_TRUE(distributedScene->publish());
        EXPECT_TRUE(distributedScene->isPublished());
        framework.connect();
        EXPECT_TRUE(distributedScene->isPublished());
    }

    TEST_F(AScene, canValidate)
    {
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(AScene, byDefaultSceneHasOpenGLCompatibility)
    {
        EXPECT_EQ(ERenderBackendCompatibility::OpenGL, m_internalScene.getRenderBackendCompatibility());
        EXPECT_EQ(EVulkanAPIVersion::Invalid, m_internalScene.getVulkanAPIVersion());
        EXPECT_EQ(ESPIRVVersion::Invalid, m_internalScene.getSPIRVVersion());
    }

    TEST_F(AScene, canCreateSceneWithVulkanAndOpenGLCompatibility)
    {
        const SceneConfig sceneConfig{ sceneId_t{456u}, EScenePublicationMode::LocalOnly, ERenderBackendCompatibility::VulkanAndOpenGL };
        auto* scene = client.createScene(sceneConfig);
        ASSERT_NE(nullptr, scene);
        const auto& iscene = scene->impl().getIScene();
        EXPECT_EQ(ERenderBackendCompatibility::VulkanAndOpenGL, iscene.getRenderBackendCompatibility());
        EXPECT_EQ(TargetVulkanApiVersion, iscene.getVulkanAPIVersion());
        EXPECT_EQ(TargetSPIRVVersion, iscene.getSPIRVVersion());
    }

    TEST_F(AScene, canNotCreateSceneWithVulkanAndOpenGLCompatibilityWithFeatureLevel01)
    {
        RamsesFramework fl0Framework(LocalTestClient::GetDefaultFrameworkConfig(EFeatureLevel::EFeatureLevel_01));
        RamsesClient& fl0Client = *fl0Framework.createClient("localTestClient");
        const SceneConfig sceneConfig{ sceneId_t{456u}, EScenePublicationMode::LocalOnly, ERenderBackendCompatibility::VulkanAndOpenGL };
        auto* scene = fl0Client.createScene(sceneConfig);
        EXPECT_EQ(nullptr, scene);
    }

    TEST_F(AScene, failsValidationIfContainsInvalidSceneObject)
    {
        ramses::RenderPass* passWithoutCamera = m_scene.createRenderPass();
        ValidationReport report;
        m_scene.validate(report);
        EXPECT_TRUE(report.hasIssue());

        m_scene.destroy(*passWithoutCamera);
        report.clear();
        m_scene.validate(report);
        EXPECT_FALSE(report.hasIssue());

        ramses::Camera* cameraWithoutValidValues = m_scene.createPerspectiveCamera();
        report.clear();
        m_scene.validate(report);
        EXPECT_TRUE(report.hasError());

        m_scene.destroy(*cameraWithoutValidValues);
    }

    TEST_F(AScene, doesNotDestroyCameraWhileItIsStillUsedByARenderPass)
    {
        ramses::RenderPass* pass = m_scene.createRenderPass();
        PerspectiveCamera* perspCam = m_scene.createPerspectiveCamera();
        perspCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        perspCam->setViewport(0, 0, 100, 200);
        pass->setCamera(*perspCam);

        EXPECT_FALSE(m_scene.destroy(*perspCam));
    }

    TEST_F(AScene, removesMeshNodeFromRenderGroupsOnDestruction)
    {
        ramses::RenderGroup* group = m_scene.createRenderGroup();
        ramses::RenderGroup* group2 = m_scene.createRenderGroup();
        MeshNode* mesh = m_scene.createMeshNode();
        group->addMeshNode(*mesh);
        group2->addMeshNode(*mesh);

        m_scene.destroy(*mesh);

        EXPECT_TRUE(group->impl().getAllMeshes().empty());
        EXPECT_TRUE(group2->impl().getAllMeshes().empty());
    }

    TEST_F(AScene, failsToCreateAppearanceWhenEffectIsFromAnotherScene)
    {
        ramses::Scene& anotherScene(*client.createScene(SceneConfig(sceneId_t{ 0xf00 })));
        Effect* effect = anotherScene.createEffect(effectDescriptionEmpty, "emptyEffect");
        ASSERT_TRUE(nullptr != effect);

        Appearance* appearance = m_scene.createAppearance(*effect, "appearance");
        EXPECT_TRUE(nullptr == appearance);
    }

    TEST_F(AScene, failsToCreateGeometryWhenEffectIsFromAnotherClient)
    {
        ramses::Scene& anotherScene(*client.createScene(SceneConfig(sceneId_t{ 0xf00 })));
        Effect* effect = anotherScene.createEffect(effectDescriptionEmpty, "emptyEffect");
        ASSERT_TRUE(nullptr != effect);

        Geometry* geometryBinding = m_scene.createGeometry(*effect, "geometryBinding");
        EXPECT_TRUE(nullptr == geometryBinding);
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenTextureIsFromAnotherScene)
    {
        ramses::Scene& anotherScene(*client.createScene(SceneConfig(sceneId_t{ 0xf00 })));
        {
            const std::vector<MipLevelData> mipData{ { std::byte{1}, std::byte{2}, std::byte{3} } };
            Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::RGB8, 1u, 1u, mipData, false);
            ASSERT_TRUE(texture != nullptr);

            ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(nullptr == textureSampler);
        }
        {
            const MipLevelData data = {
                std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8},
                std::byte{9}, std::byte{10}, std::byte{11}, std::byte{12}, std::byte{13}, std::byte{14}, std::byte{15}, std::byte{16} };
            ASSERT_TRUE(data.size() == 1 * 2 * 2 * 4);
            std::vector<MipLevelData> mipLevelData{ data };
            Texture3D* texture = anotherScene.createTexture3D(ETextureFormat::RGBA8, 2, 1, 2, mipLevelData, false, {});
            ASSERT_TRUE(nullptr != texture);

            ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(nullptr == textureSampler);
        }
        {
            const std::vector<std::byte> data(4 * 10 * 10);
            std::vector<CubeMipLevelData> mipLevelData{ { data, data, data, data, data, data } };
            TextureCube* texture = anotherScene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false);
            ASSERT_TRUE(nullptr != texture);

            ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(nullptr == textureSampler);
        }
        {
            ramses::RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(4u, 4u, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
            ASSERT_TRUE(nullptr != renderBuffer);

            TextureSamplerMS* textureSampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
            EXPECT_TRUE(nullptr == textureSampler);
        }
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenRenderTargetIsFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));
        ramses::RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != renderBuffer);

        ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *renderBuffer);
        EXPECT_TRUE(nullptr == textureSampler);

        client.destroy(anotherScene);
    }

    TEST_F(AScene, failsToCreateTextureSamplerMSWhenRenderTargetIsFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));
        ramses::RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != renderBuffer);

        TextureSamplerMS* textureSampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        EXPECT_TRUE(nullptr == textureSampler);

        client.destroy(anotherScene);
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenRenderBufferIsNotReadable)
    {
        ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::WriteOnly);
        ASSERT_TRUE(nullptr != renderBuffer);

        ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *renderBuffer);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, failsToCreateTextureSamplerMSWhenRenderBufferIsNotReadable)
    {
        ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::WriteOnly);
        ASSERT_TRUE(nullptr != renderBuffer);

        TextureSamplerMS* textureSampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, reportsErrorWhenCreateTransformationDataProviderWithNodeFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));

        Node* node = anotherScene.createNode("node");
        ASSERT_TRUE(nullptr != node);

        EXPECT_FALSE(m_scene.createTransformationDataProvider(*node, dataProviderId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateTransformationDataConsumerWithNodeFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));

        Node* node = anotherScene.createNode("groupNode");
        ASSERT_TRUE(nullptr != node);

        EXPECT_FALSE(m_scene.createTransformationDataConsumer(*node, dataConsumerId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityChangedByParentVisibilityNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);

        m_vis2.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_vis1.setVisibility(EVisibilityMode::Off);
        m_vis2.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityAddedToInvisibleParent)
    {
        MeshNode* addMeshNode = m_scene.createMeshNode("another mesh node");

        m_vis2.setVisibility(EVisibilityMode::Invisible);
        addMeshNode->setParent(m_vis2);

        m_scene.flush();
        EXPECT_EQ(addMeshNode->impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        //cleanup
        addMeshNode->removeParent();
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityAddedToOffParent)
    {
        MeshNode* addMeshNode = m_scene.createMeshNode("another mesh node");

        m_vis2.setVisibility(EVisibilityMode::Off);
        addMeshNode->setParent(m_vis2);

        m_scene.flush();
        EXPECT_EQ(addMeshNode->impl().getFlattenedVisibility(), EVisibilityMode::Off);

        //cleanup
        addMeshNode->removeParent();
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityChangedByGrandParentVisibilityNode)
    {
        //Add another visibility hierarchy level below mesh 4
        Node& m_node8 = *m_scene.createNode("node8");
        MeshNode& m_node9 = *m_scene.createMeshNode("node9");
        EXPECT_TRUE(m_node8.setParent(m_mesh1a));
        EXPECT_TRUE(m_node9.setParent(m_node8));

        //Newly added mesh should be visible by default
        EXPECT_EQ(m_node9.impl().getFlattenedVisibility(), EVisibilityMode::Visible);

        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_node8.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_vis1.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_node8.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, subTreeStaysInvisibleIfSettingVisibilityNodeVisibleParentedByInvisibleNode)
    {
        // 'root' invisible
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        // add another visibility node under invisible visibility node
        Node& vis1v = *m_scene.createNode();
        m_vis1.addChild(vis1v);
        // reparent meshes under it
        vis1v.addChild(m_mesh1a);
        vis1v.addChild(m_mesh1b);
        // explicitly set visible
        m_scene.flush();
        vis1v.setVisibility(EVisibilityMode::Invisible);
        vis1v.setVisibility(EVisibilityMode::Visible);

        // should stay invisible
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, nodesStayVisibleWhenRemovedVisibleParent)
    {
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesBecomeVisibleWhenRemovedInvisibleParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesBecomeVisibleWhenRemovedOffParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, visibleMeshBecomesInvisibleWhenMovedUnderInvisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        m_mesh2a.setParent(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, visibleMeshBecomesOffWhenMovedUnderOffNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        m_mesh2a.setParent(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh2a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesInvisibleWhenSetInvisibleNodeAsParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        meshOrphan.setParent(m_vis1);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesOffWhenSetOffNodeAsParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        meshOrphan.setParent(m_vis1);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesInvisibleWhenMovedUnderInvisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        m_vis1.addChild(meshOrphan);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesOffWhenMovedUnderOffNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        m_vis1.addChild(meshOrphan);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, meshInInvisibleBranchBecomesVisibleWhenMovedUnderVisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        m_mesh1a.setParent(m_vis2);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, meshInOffBranchBecomesVisibleWhenMovedUnderVisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        m_mesh1a.setParent(m_vis2);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesStayVisibleIfParentBecomesInvisibleAndIsDeletedInSameCommit)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesStayVisibleIfParentBecomesOffAndIsDeletedInSameCommit)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodeBecomesVisibleIfInvisibleParentIsRemovedFromIt)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        m_mesh1a.removeParent();
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodeBecomesVisibleIfOffParentIsRemovedFromIt)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        m_mesh1a.removeParent();
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodeBecomesInvisibleIfAnyOfItsAncestorsIsInvisible)
    {
        // add another visibility node under 'root' visibility node
        Node& vis1a = *m_scene.createNode();
        m_vis1.addChild(vis1a);
        // reparent meshes under it
        vis1a.addChild(m_mesh1a);
        vis1a.addChild(m_mesh1b);

        // set parent invisible
        vis1a.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        // revert
        vis1a.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, nodeBecomesOffIfAnyOfItsAncestorsIsOff)
    {
        // add another visibility node under 'root' visibility node
        Node& vis1a = *m_scene.createNode();
        m_vis1.addChild(vis1a);
        // reparent meshes under it
        vis1a.addChild(m_mesh1a);
        vis1a.addChild(m_mesh1b);

        // set parent invisible
        vis1a.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Off);

        // revert
        vis1a.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, nodeBecomesOffIfAnyOfItsAncestorsIsOffAndInvisible)
    {
        // add another visibility node under 'root' visibility node
        Node& vis1a = *m_scene.createNode();
        m_vis1.addChild(vis1a);
        // reparent meshes under it
        vis1a.addChild(m_mesh1a);
        vis1a.addChild(m_mesh1b);

        // set parent invisible
        vis1a.setVisibility(EVisibilityMode::Off);
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Off);

        // revert
        vis1a.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, offNodeStaysOffIndependentOfParentVisibility)
    {
        m_vis1.addChild(m_mesh1a);
        m_vis1.addChild(m_mesh1b);

        // set parent invisible
        m_mesh1a.setVisibility(EVisibilityMode::Off);
        m_mesh1b.setVisibility(EVisibilityMode::Invisible);
        m_vis1.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        // revert
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(AScene, canCreateATransformDataSlot)
    {
        Node* node = m_scene.createNode();
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        EXPECT_TRUE(m_scene.createTransformationDataConsumer(*node, dataConsumerId_t(666u)));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(node->impl().getNodeHandle(), m_scene.impl().getIScene().getDataSlot(slotHandle).attachedNode);
        EXPECT_EQ(ramses::internal::DataSlotId(666u), m_scene.impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::TransformationConsumer, m_scene.impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, removesTransformDataSlotsOfNodeOnDestruction)
    {
        Node* node = m_scene.createNode();
        m_scene.createTransformationDataProvider(*node, dataProviderId_t(1412u));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle linkHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(linkHandle));

        m_scene.destroy(*node);

        EXPECT_FALSE(m_scene.impl().getIScene().isDataSlotAllocated(linkHandle));
    }

    TEST_F(AScene, canNotCreateMoreThanOneTransformationDataSlotForANode)
    {
        Node* node = m_scene.createNode();
        EXPECT_TRUE(m_scene.createTransformationDataProvider(*node, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createTransformationDataProvider(*node, dataProviderId_t(2u)));
        EXPECT_FALSE(m_scene.createTransformationDataConsumer(*node, dataConsumerId_t(3u)));
    }

    TEST_F(AScene, canNotCreateMoreThanOneTransformationDataSlotWithTheSameId)
    {
        Node* node1 = m_scene.createNode();
        Node* node2 = m_scene.createNode();
        EXPECT_TRUE(m_scene.createTransformationDataProvider(*node1, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createTransformationDataProvider(*node2, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createTransformationDataConsumer(*node2, dataConsumerId_t(1u)));

        EXPECT_TRUE(m_scene.createTransformationDataConsumer(*node2, dataConsumerId_t(2u)));
        EXPECT_FALSE(m_scene.createTransformationDataConsumer(*node1, dataConsumerId_t(2u)));
        EXPECT_FALSE(m_scene.createTransformationDataProvider(*node1, dataProviderId_t(2u)));
    }

    TEST_F(AScene, reportsErrorWhenCreateDataProviderWithDataObjectFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));

        auto dataObject = anotherScene.createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(nullptr != dataObject);

        EXPECT_FALSE(m_scene.createDataProvider(*dataObject, dataProviderId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateDataConsumerWithDataObjectFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));

        auto dataObject = anotherScene.createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(nullptr != dataObject);

        EXPECT_FALSE(m_scene.createDataConsumer(*dataObject, dataConsumerId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, canCreateADataObjectDataSlot)
    {
        auto dataObject = m_scene.createDataObject(ramses::EDataType::Float);
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        EXPECT_TRUE(m_scene.createDataConsumer(*dataObject, dataConsumerId_t(666u)));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(dataObject->impl().getDataReference(), m_scene.impl().getIScene().getDataSlot(slotHandle).attachedDataReference);
        EXPECT_EQ(ramses::internal::DataSlotId(666u), m_scene.impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::DataConsumer, m_scene.impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, removesDataSlotsOfDataObjectOnDestruction)
    {
        auto dataObject = m_scene.createDataObject(ramses::EDataType::Float);
        m_scene.createDataProvider(*dataObject, dataProviderId_t(1412u));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle linkHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(linkHandle));

        m_scene.destroy(*dataObject);

        EXPECT_FALSE(m_scene.impl().getIScene().isDataSlotAllocated(linkHandle));
    }

    TEST_F(AScene, canNotCreateMoreThanOneDataSlotForADataObject)
    {
        auto dataObject = m_scene.createDataObject(ramses::EDataType::Float);
        EXPECT_TRUE(m_scene.createDataProvider(*dataObject, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createDataProvider(*dataObject, dataProviderId_t(2u)));
        EXPECT_FALSE(m_scene.createDataConsumer(*dataObject, dataConsumerId_t(3u)));
    }

    TEST_F(AScene, canNotCreateMoreThanOneDataSlotWithTheSameId)
    {
        auto dataObject1 = m_scene.createDataObject(ramses::EDataType::Float);
        auto dataObject2 = m_scene.createDataObject(ramses::EDataType::Float);
        EXPECT_TRUE(m_scene.createDataProvider(*dataObject1, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createDataProvider(*dataObject2, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createDataConsumer(*dataObject2, dataConsumerId_t(1u)));

        EXPECT_TRUE(m_scene.createDataConsumer(*dataObject2, dataConsumerId_t(2u)));
        EXPECT_FALSE(m_scene.createDataConsumer(*dataObject1, dataConsumerId_t(2u)));
        EXPECT_FALSE(m_scene.createDataProvider(*dataObject1, dataProviderId_t(2u)));
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureProviderWithTextureFromAnotherScene)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework anotherFramework{config};
        ramses::Scene& anotherScene(*client.createScene(SceneConfig(sceneId_t{ 0xf00 })));
        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        const Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_FALSE(m_scene.createTextureProvider(*texture, dataProviderId_t{1u}));
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureConsumerWithSamplerFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));
        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        const ramses::TextureSampler* sampler = anotherScene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureConsumerWithSamplerMSFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));
        ramses::RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(4u, 4u, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
        ASSERT_TRUE(nullptr != renderBuffer);

        const TextureSamplerMS* sampler = anotherScene.createTextureSamplerMS(*renderBuffer, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 1u }));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureConsumerWithSamplerExternalFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));
        const TextureSamplerExternal* sampler = anotherScene.createTextureSamplerExternal(ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 1u }));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, canCreateATextureProviderDataSlot)
    {
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_TRUE(m_scene.createTextureProvider(*texture, dataProviderId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(texture->impl().getLowlevelResourceHash(), m_scene.impl().getIScene().getDataSlot(slotHandle).attachedTexture);
        EXPECT_EQ(ramses::internal::DataSlotId(666u), m_scene.impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::TextureProvider, m_scene.impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canUpdateATextureProviderDataSlot)
    {
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        const std::vector<MipLevelData> mipData1{ { std::byte{ 0u } } };
        Texture2D* texture1 = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData1, false);
        ASSERT_TRUE(nullptr != texture1);

        const std::vector<MipLevelData> mipData2{ { std::byte{ 1u } } };
        Texture2D* texture2 = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData2, false);
        ASSERT_TRUE(nullptr != texture2);

        EXPECT_TRUE(m_scene.createTextureProvider(*texture1, dataProviderId_t{666u}));
        EXPECT_TRUE(m_scene.updateTextureProvider(*texture2, dataProviderId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(texture2->impl().getLowlevelResourceHash(), m_scene.impl().getIScene().getDataSlot(slotHandle).attachedTexture);
        EXPECT_EQ(ramses::internal::DataSlotId(666u), m_scene.impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::TextureProvider, m_scene.impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canCreateATextureConsumerDataSlot)
    {
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        const std::vector<MipLevelData> mipData{ { std::byte{0u} } };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        const ramses::TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(sampler->impl().getTextureSamplerHandle(), m_scene.impl().getIScene().getDataSlot(slotHandle).attachedTextureSampler);
        EXPECT_EQ(ramses::internal::DataSlotId(666u), m_scene.impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::TextureConsumer, m_scene.impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canCreateATextureConsumerDataSlotWithTextureSamplerMS)
    {
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(4u, 4u, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
        ASSERT_TRUE(nullptr != renderBuffer);

        const TextureSamplerMS* sampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");

        ASSERT_TRUE(nullptr != sampler);

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(sampler->impl().getTextureSamplerHandle(), m_scene.impl().getIScene().getDataSlot(slotHandle).attachedTextureSampler);
        EXPECT_EQ(ramses::internal::DataSlotId(666u), m_scene.impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::TextureConsumer, m_scene.impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canCreateATextureConsumerDataSlotWithTextureSamplerExternal)
    {
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        const TextureSamplerExternal* sampler = m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear);

        ASSERT_TRUE(nullptr != sampler);

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(sampler->impl().getTextureSamplerHandle(), m_scene.impl().getIScene().getDataSlot(slotHandle).attachedTextureSampler);
        EXPECT_EQ(ramses::internal::DataSlotId(666u), m_scene.impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::TextureConsumer, m_scene.impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canNotCreateATextureConsumerDataSlotIfOtherThan2DTextureAssigned)
    {
        EXPECT_EQ(0u, m_scene.impl().getIScene().getDataSlotCount());

        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::R8, 1u, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        const ramses::TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));
    }

    TEST_F(AScene, removesDataSlotsOfTextureSamplerOnDestruction)
    {
        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl().getIScene().getDataSlotCount());
        ramses::internal::DataSlotHandle linkHandle(0u);
        ASSERT_TRUE(m_scene.impl().getIScene().isDataSlotAllocated(linkHandle));

        m_scene.destroy(*sampler);

        EXPECT_FALSE(m_scene.impl().getIScene().isDataSlotAllocated(linkHandle));
    }

    TEST_F(AScene, canNotCreateMoreThanOneConsumerForATextureSampler)
    {
        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));
        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{667u}));
    }

    TEST_F(AScene, canNotCreateMoreThanOneConsumerForATextureSamplerMS)
    {
        ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(4u, 4u, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
        ASSERT_TRUE(nullptr != renderBuffer);

        const TextureSamplerMS* sampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));
        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 667u }));
    }

    TEST_F(AScene, canNotCreateMoreThanOneConsumerForATextureSamplerExternal)
    {
        const TextureSamplerExternal* sampler = m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));
        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 667u }));
    }

    TEST_F(AScene, canNotCreateMoreThanOneProviderForATexture)
    {
        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_TRUE(m_scene.createTextureProvider(*texture, dataProviderId_t{666u}));
        EXPECT_FALSE(m_scene.createTextureProvider(*texture, dataProviderId_t{667u}));
    }

    TEST_F(AScene, canNotCreateMoreThanOneTextureConsumerOrProviderWithTheSameId)
    {
        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);
        Texture2D* texture2 = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture2);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);
        ramses::TextureSampler* sampler2 = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture2);
        ASSERT_TRUE(nullptr != sampler2);

        EXPECT_TRUE(m_scene.createTextureProvider(*texture, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createTextureProvider(*texture2, dataProviderId_t(1u)));
        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t(1u)));

        EXPECT_TRUE(m_scene.createTextureConsumer(*sampler, dataConsumerId_t(2u)));
        EXPECT_FALSE(m_scene.createTextureConsumer(*sampler2, dataConsumerId_t(2u)));
        EXPECT_FALSE(m_scene.createTextureProvider(*texture, dataProviderId_t(2u)));
    }

    TEST_F(AScene, canNotUpdateTextureProviderWhichWasNotCreatedBefore)
    {
        const std::vector<MipLevelData> mipData{ { std::byte{ 0u } } };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_FALSE(m_scene.updateTextureProvider(*texture, dataProviderId_t(1u)));
    }

    TEST_F(AScene, canCreateTextureSamplerForTexture2DWithDefaultAnisotropyLevel)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, texture2D);

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Nearest, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::Texture2D, sampler->getTextureType());
    }

    TEST_F(AScene, canCreateTextureSamplerForRenderBufferWithDefaultAnisotropyLevel)
    {
        const ramses::RenderBuffer& renderBuffer = createObject<ramses::RenderBuffer>("renderBuffer");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, renderBuffer);

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Nearest, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::RenderBuffer, sampler->getTextureType());
    }

    TEST_F(AScene, canCreateTextureSamplerForTextureCubeWithDefaultAnisotropyLevel)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const ramses::TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, textureCube);

        ASSERT_NE(static_cast<ramses::TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode::Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod::Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType::TextureCube, sampler->getTextureType());
    }

    TEST_F(AScene, cantCreateTextureSamplerForTexture2DWithWrongAnisotropyValue)
    {
        const Texture2D& texture = createObject<Texture2D>("testTexture2D");
        ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, texture, 0);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerForTextureCubeWithWrongAnisotropyValue)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, textureCube, 0);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerForRenderBufferWithWrongAnisotropyValue)
    {
        const ramses::RenderBuffer& renderBuffer = createObject<ramses::RenderBuffer>("renderBuffer");
        const ramses::TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, renderBuffer, 0);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerWithWrongMagSamplingMethod)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        EXPECT_EQ(nullptr, this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear_MipMapLinear, textureCube));
        EXPECT_EQ(nullptr, this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear_MipMapNearest, textureCube));
        EXPECT_EQ(nullptr, this->m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Nearest_MipMapNearest, textureCube));
    }

    TEST_F(AScene, canCreateBlitPass)
    {
        const ramses::RenderBuffer& sourceRenderBuffer = createObject<ramses::RenderBuffer>("src renderBuffer");
        const ramses::RenderBuffer& destinationRenderBuffer = createObject<ramses::RenderBuffer>("dst renderBuffer");
        const ramses::BlitPass* blitPass = m_scene.createBlitPass(sourceRenderBuffer, destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr != blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceRenderBufferFromDifferentScene)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));
        const ramses::RenderBuffer* sourceRenderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);

        const ramses::RenderBuffer& destinationRenderBuffer = createObject<ramses::RenderBuffer>("dst renderBuffer");
        const ramses::BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBufferFromDifferentScenes)
    {
        ramses::Scene& anotherScene = *client.createScene(SceneConfig(sceneId_t(12u)));
        const ramses::RenderBuffer* destinationRenderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const ramses::RenderBuffer& sourceRenderBuffer = createObject<ramses::RenderBuffer>("src renderBuffer");
        const ramses::BlitPass* blitPass = m_scene.createBlitPass(sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentType)
    {
        const ramses::RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::R8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::Depth24, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const ramses::BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentFormat)
    {
        const ramses::RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::R8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const ramses::BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavinghDifferentWidth)
    {
        const ramses::RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(1u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const ramses::BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentHeight)
    {
        const ramses::RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const ramses::BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSameSourceAndDestinationRenderBuffers)
    {
        const ramses::RenderBuffer* rb = m_scene.createRenderBuffer(100u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(nullptr != rb);
        const ramses::BlitPass* blitPass = m_scene.createBlitPass(*rb, *rb, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, canCreatePickableObject)
    {
        const ArrayBuffer* geometryBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 3);
        ASSERT_TRUE(nullptr != geometryBuffer);

        const ramses::PickableObject* pickableObject = m_scene.createPickableObject(*geometryBuffer, pickableObjectId_t(1u));
        ASSERT_NE(nullptr, pickableObject);
    }

    TEST_F(AScene, cannotCreatePickableObjectWithGeometryBufferOfAnotherScene)
    {
        ramses::Scene* anotherScene = client.createScene(SceneConfig(sceneId_t(111u)));
        const ArrayBuffer* geometryBufferFromOtherScene = anotherScene->createArrayBuffer(ramses::EDataType::Vector3F, 3);

        const ramses::PickableObject* pickableObject = m_scene.createPickableObject(*geometryBufferFromOtherScene, pickableObjectId_t(1u));
        EXPECT_EQ(nullptr, pickableObject);
    }

    TEST_F(AScene, cannotCreatePickableObjectWithWrongGeometryBufferSize)
    {
        const ArrayBuffer* geometryBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 2u);
        ASSERT_TRUE(nullptr != geometryBuffer);

        const ramses::PickableObject* pickableObject = m_scene.createPickableObject(*geometryBuffer, pickableObjectId_t(1u));
        EXPECT_EQ(nullptr, pickableObject);
    }

    TEST_F(AScene, cannotCreatePickableObjectWithWrongGeometryBuffer)
    {
        const ArrayBuffer* geometryBuffer1 = m_scene.createArrayBuffer(ramses::EDataType::Float, 3u);
        const ArrayBuffer* geometryBuffer2 = m_scene.createArrayBuffer(ramses::EDataType::Vector2F, 3u);
        const ArrayBuffer* geometryBuffer3 = m_scene.createArrayBuffer(ramses::EDataType::Vector4F, 3u);

        EXPECT_EQ(nullptr, m_scene.createPickableObject(*geometryBuffer1, pickableObjectId_t(1u)));
        EXPECT_EQ(nullptr, m_scene.createPickableObject(*geometryBuffer2, pickableObjectId_t(2u)));
        EXPECT_EQ(nullptr, m_scene.createPickableObject(*geometryBuffer3, pickableObjectId_t(3u)));
    }

    TEST_F(AScene, canCreateDataBufferWithIntegralTypesForUsageAsIndexBuffer)
    {
        ArrayBuffer* const indexDataBuffer16 = m_scene.createArrayBuffer(ramses::EDataType::UInt16, 4u);
        ASSERT_NE(nullptr, indexDataBuffer16);
        ArrayBuffer* const indexDataBuffer32 = m_scene.createArrayBuffer(ramses::EDataType::UInt32, 4u);
        ASSERT_NE(nullptr, indexDataBuffer32);

        m_scene.destroy(*indexDataBuffer16);
        m_scene.destroy(*indexDataBuffer32);
    }

    TEST_F(AScene, canCreateDataBufferWithFloatTypesForUseAsVertexBuffer)
    {
        ArrayBuffer* const vertexDataBufferFloat = m_scene.createArrayBuffer(ramses::EDataType::Float, 4u);
        ASSERT_NE(nullptr, vertexDataBufferFloat);
        ArrayBuffer* const vertexDataBufferVec2 = m_scene.createArrayBuffer(ramses::EDataType::Vector2F, 4u);
        ASSERT_NE(nullptr, vertexDataBufferVec2);
        ArrayBuffer* const vertexDataBufferVec3 = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 4u);
        ASSERT_NE(nullptr, vertexDataBufferVec3);
        ArrayBuffer* const vertexDataBufferVec4 = m_scene.createArrayBuffer(ramses::EDataType::Vector4F, 4u);
        ASSERT_NE(nullptr, vertexDataBufferVec4);

        m_scene.destroy(*vertexDataBufferFloat);
        m_scene.destroy(*vertexDataBufferVec2);
        m_scene.destroy(*vertexDataBufferVec3);
        m_scene.destroy(*vertexDataBufferVec4);
    }

    TEST_F(AScene, canCreateInterleavedVertexDataBuffer)
    {
        ArrayBuffer* const interleavedVertexBuffer = m_scene.createArrayBuffer(ramses::EDataType::ByteBlob, 14u);
        ASSERT_NE(nullptr, interleavedVertexBuffer);

        EXPECT_EQ(ramses::EDataType::ByteBlob, interleavedVertexBuffer->getDataType());
        EXPECT_EQ(1u, GetSizeOfDataType(interleavedVertexBuffer->getDataType()));
        EXPECT_EQ(14u, interleavedVertexBuffer->getMaximumNumberOfElements());

        m_scene.destroy(*interleavedVertexBuffer);
    }

    TEST_F(AScene, flushIncreasesStatisticCounter)
    {
        EXPECT_EQ(0u, m_scene.impl().getStatisticCollection().statFlushesTriggered.getCounterValue());
        m_scene.flush();
        EXPECT_EQ(1u, m_scene.impl().getStatisticCollection().statFlushesTriggered.getCounterValue());
    }

    TEST_F(AScene, canGetResourceByID)
    {
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr != effectFixture);

        const resourceId_t resourceID = effectFixture->getResourceId();
        ramses::Resource* resource = m_scene.getResource(resourceID);
        ASSERT_TRUE(nullptr != resource);

        const auto* effectFound = resource->as<ramses::Effect>();
        ASSERT_TRUE(nullptr != effectFound);

        ASSERT_TRUE(effectFound == effectFixture);

        const resourceId_t nonExistEffectId = { 0, 0 };
        ASSERT_TRUE(nullptr == m_scene.getResource(nonExistEffectId));
    }

    TEST_F(AScene, returnsNULLWhenResourceWithIDCannotBeFound)
    {
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr != effectFixture);

        const resourceId_t nonExistEffectId = { 0, 0 };
        ASSERT_TRUE(nullptr == m_scene.getResource(nonExistEffectId));
    }

    TEST_F(AScene, returnsNULLWhenTryingToFindDeletedResource)
    {
        auto effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr != effectFixture);

        const resourceId_t resourceID = effectFixture->getResourceId();
        ramses::Resource* resource = m_scene.getResource(resourceID);
        ASSERT_TRUE(nullptr != resource);

        m_scene.destroy(*effectFixture);

        ramses::Resource* resourceFound = m_scene.getResource(resourceID);
        ASSERT_TRUE(nullptr == resourceFound);
    }

    // effect from string: valid uses
    TEST_F(AScene, createEffectFromGLSLString_withName)
    {
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr != effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_withDefines)
    {
        effectDescriptionEmpty.addCompilerDefine("float dummy;");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr != effectFixture);
    }

    // effect from string: invalid uses
    TEST_F(AScene, createEffectFromGLSLString_invalidVertexShader)
    {
        effectDescriptionEmpty.setVertexShader("void main(void) {dsadsadasd}");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_emptyVertexShader)
    {
        effectDescriptionEmpty.setVertexShader("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_invalidFragmentShader)
    {
        effectDescriptionEmpty.setFragmentShader("void main(void) {dsadsadasd}");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_emptyFragmentShader)
    {
        effectDescriptionEmpty.setFragmentShader("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_invalidDefines)
    {
        effectDescriptionEmpty.addCompilerDefine("thisisinvalidstuff\n8fd7f9ds");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_withInputSemantics)
    {
        effectDescriptionEmpty.setVertexShader(
            "uniform mat4 someMatrix;"
            "void main(void)"
            "{"
            "gl_Position = someMatrix * vec4(1.0);"
            "}");
        effectDescriptionEmpty.setUniformSemantic("someMatrix", ramses::EEffectUniformSemantic::ProjectionMatrix);
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_TRUE(nullptr != effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_withInputSemanticsOfWrongType)
    {
        effectDescriptionEmpty.setVertexShader(
            "uniform mat2 someMatrix;"
            "void main(void)"
            "{"
            "gl_Position = someMatrix * vec4(1.0);"
            "}");
        effectDescriptionEmpty.setUniformSemantic("someMatrix", ramses::EEffectUniformSemantic::ProjectionMatrix);
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, "name");
        EXPECT_FALSE(nullptr != effectFixture);
    }

    // effect from file: valid uses
    TEST_F(AScene, createEffectFromGLSL_withName)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-client-test_minimalShader.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, "name");
        EXPECT_TRUE(nullptr != effectFixture);
    }


    // effect from file: invalid uses
    TEST_F(AScene, createEffectFromGLSL_nonExistantVertexShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSL_nonExistantFragmentShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-client-test_minimalShader.frag");
        effectDesc.setFragmentShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSL_NULLVertexShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("");
        effectDesc.setFragmentShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSL_NULLFragmentShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, verifyHLAPILogCanHandleNullPtrReturnWhenEnabled)
    {
        ELogLevel oldLogLevel = ramses::internal::CONTEXT_HLAPI_CLIENT.getLogLevel();
        ramses::internal::CONTEXT_HLAPI_CLIENT.setLogLevel(ELogLevel::Trace);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, "name");
        EXPECT_TRUE(nullptr == effectFixture);

        ramses::internal::CONTEXT_HLAPI_CLIENT.setLogLevel(oldLogLevel);
    }

    TEST_F(AScene, returnsFalseOnFlushWhenResourcesAreMissing)
    {
        m_creationHelper.createObjectOfType<Appearance>("meh");
        EXPECT_TRUE(m_scene.flush()); // test legal scene state => flush success
        m_scene.destroy(*m_scene.findObject<SceneObject>("appearance effect"));
        m_scene.destroy(*m_scene.findObject<SceneObject>("meh"));
        EXPECT_TRUE(m_scene.flush()); // resource is deleted, but nobody needs it => flush success

        auto* appearnace = m_creationHelper.createObjectOfType<Appearance>("meh2");
        auto* mesh = m_creationHelper.createObjectOfType<MeshNode>("meh3");
        mesh->setAppearance(*appearnace);
        m_scene.destroy(*m_scene.findObject<SceneObject>("appearance effect"));
        EXPECT_FALSE(m_scene.flush()); // test scene with resource missing => flush failed
    }

    TEST_F(AScene, uniformTimeMatchesSyncClockInitiallyAndCanBeReset)
    {
        const int32_t tolerance = 10000; // 10 seconds to tolerate stucks during test execution
        const auto now = ramses::internal::PlatformTime::GetMillisecondsSynchronized();
        const auto s32now = static_cast<int32_t>(now % std::numeric_limits<int32_t>::max());
        EXPECT_GE(tolerance, GetElapsed(s32now, m_scene.getUniformTimeMs()));
        EXPECT_TRUE(m_scene.resetUniformTimeMs());
        EXPECT_GE(tolerance, GetElapsed(0, m_scene.getUniformTimeMs()));
    }

    TEST_F(AScene, resetUniformTimeMs)
    {
        EXPECT_EQ(ramses::internal::FlushTime::InvalidTimestamp, m_scene.impl().getIScene().getEffectTimeSync());
        EXPECT_TRUE(m_scene.resetUniformTimeMs());
        const auto timeSync = m_scene.impl().getIScene().getEffectTimeSync();
        EXPECT_NE(0, timeSync.time_since_epoch().count());

        EXPECT_TRUE(m_scene.flush());

        EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(_, _));
        EXPECT_TRUE(m_scene.publish(ramses::EScenePublicationMode::LocalOnly));

        // first flush sends scene init
        EXPECT_CALL(sceneActionsCollector, handleInitializeScene(_, _));

        // internal timestamp contains sync time
        ramses::internal::PlatformThread::Sleep(10);
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses::internal::SceneId(123u), _, _)).WillOnce([&](auto /*unused*/, const auto& update, auto /*unused*/) {
            const ramses::internal::FlushTimeInformation& timeInfo = update.flushInfos.flushTimeInfo;
            EXPECT_TRUE(timeInfo.isEffectTimeSync);
            EXPECT_EQ(timeSync, timeInfo.internalTimestamp);
            });
        EXPECT_TRUE(m_scene.flush(42u));

        // internal timestamp contains flush time
        ramses::internal::PlatformThread::Sleep(10);
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses::internal::SceneId(123u), _, _)).WillOnce([&](auto /*unused*/, const auto& update, auto /*unused*/) {
            const ramses::internal::FlushTimeInformation& timeInfo = update.flushInfos.flushTimeInfo;
            EXPECT_FALSE(timeInfo.isEffectTimeSync);
            EXPECT_NE(timeSync, timeInfo.internalTimestamp);
            });
        EXPECT_TRUE(m_scene.flush(43u));

        EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(_, _));
    }

    TEST_F(AScene, canFindByNameWithSameNameUsedForDifferentTypes)
    {
        auto node = m_scene.createNode("test");
        auto camNode1 = m_scene.createOrthographicCamera("test");
        auto camNode2 = m_scene.createPerspectiveCamera("test");
        auto arrBuffer = m_scene.createArrayBuffer(ramses::EDataType::Float, 1u, "test");
        auto logicEngine = m_scene.createLogicEngine("test");
        const auto logicObject = logicEngine->createTimerNode("test");
        ASSERT_TRUE(node);
        ASSERT_TRUE(camNode1);
        ASSERT_TRUE(camNode2);
        ASSERT_TRUE(arrBuffer);
        ASSERT_TRUE(logicEngine);
        ASSERT_TRUE(logicObject);

        // concrete types
        EXPECT_EQ(camNode1, m_scene.findObject<ramses::OrthographicCamera>("test"));
        EXPECT_EQ(camNode2, m_scene.findObject<ramses::PerspectiveCamera>("test"));
        EXPECT_EQ(arrBuffer, m_scene.findObject<ramses::ArrayBuffer>("test"));
        EXPECT_EQ(logicEngine, m_scene.findObject<ramses::LogicEngine>("test"));
        EXPECT_EQ(logicObject, m_scene.findObject<ramses::LogicObject>("test"));

        EXPECT_THAT(m_scene.findObject<ramses::SceneObject>("test"), AnyOf(node, camNode1, camNode2, logicEngine, logicObject));

        // this must be one of the 2 cameras
        EXPECT_THAT(m_scene.findObject<ramses::Camera>("test"), AnyOf(camNode1, camNode2));

        // this must be one of the 3 nodes
        EXPECT_THAT(m_scene.findObject<ramses::Node>("test"), AnyOf(node, camNode1, camNode2));

        EXPECT_TRUE(node->setName("other"));
        // now this must be one of the 2 cameras
        EXPECT_THAT(m_scene.findObject<ramses::Node>("test"), AnyOf(camNode1, camNode2));

        EXPECT_TRUE(m_scene.destroy(*node));
        // also now this must be one of the 2 cameras
        EXPECT_THAT(m_scene.findObject<ramses::Node>("test"), AnyOf(camNode1, camNode2));

        EXPECT_TRUE(camNode1->setName("other"));
        EXPECT_TRUE(camNode2->setName("other"));
        EXPECT_TRUE(arrBuffer->setName("other"));
        EXPECT_TRUE(logicEngine->setName("other"));
        // only one remaining scene object with test name
        EXPECT_EQ(logicObject, m_scene.findObject<ramses::SceneObject>("test"));
    }

    TEST_F(AScene, willFailFlushIfRenderTargetInvalid)
    {
        const auto rb1 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        const auto rb2 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        EXPECT_TRUE(m_scene.flush());

        RenderTargetDescription desc;
        EXPECT_TRUE(desc.addRenderBuffer(*rb1));
        EXPECT_TRUE(desc.addRenderBuffer(*rb2));
        const auto rt = m_scene.createRenderTarget(desc);
        EXPECT_TRUE(m_scene.flush());

        // simulate RB properties changed, there is currently no direct public API (only RenderBufferBinding via LogicEngine)
        EXPECT_TRUE(rb2->impl().setProperties(1u, 1u, 1u));

        EXPECT_FALSE(m_scene.flush());
        const auto err = framework.getLastError();
        ASSERT_TRUE(err);
        EXPECT_EQ(rt, err->object);
        EXPECT_EQ("Scene::flush: Flushing scene failed: RenderTarget uses render buffers with mismatching resolution and/or sample count."
            " This RenderTarget cannot be used and attempt to flush scene will fail.", err->message);

        // back to matching values
        EXPECT_TRUE(rb2->impl().setProperties(1u, 2u, 3u));
        EXPECT_TRUE(m_scene.flush());
    }

    TEST_F(AScene, willFailFlushIfBlitPassInvalid)
    {
        const auto rb1 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        const auto rb2 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        EXPECT_TRUE(m_scene.flush());

        const auto bp = m_scene.createBlitPass(*rb1, *rb2);
        EXPECT_TRUE(m_scene.flush());

        // simulate RB properties changed, there is currently no direct public API (only RenderBufferBinding via LogicEngine)
        EXPECT_TRUE(rb2->impl().setProperties(1u, 1u, 1u));

        EXPECT_FALSE(m_scene.flush());
        const auto err = framework.getLastError();
        ASSERT_TRUE(err);
        EXPECT_EQ(bp, err->object);
        EXPECT_EQ("Scene::flush: Flushing scene failed: blitpass blit region out of source or destination buffer size", err->message);

        // back to matching values
        EXPECT_TRUE(rb2->impl().setProperties(1u, 2u, 3u));
        EXPECT_TRUE(m_scene.flush());
    }

    TEST_F(AScene, failedFlushDoesNotSendChanges)
    {
        // create valid RT and flush
        const auto rb1 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        const auto rb2 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        RenderTargetDescription desc;
        EXPECT_TRUE(desc.addRenderBuffer(*rb1));
        EXPECT_TRUE(desc.addRenderBuffer(*rb2));
        m_scene.createRenderTarget(desc);
        EXPECT_TRUE(m_scene.flush());

        // publis scene to initiate sending of actions
        {
            EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(_, _));
            EXPECT_TRUE(m_scene.publish(ramses::EScenePublicationMode::LocalOnly));
            // first flush sends scene init
            EXPECT_CALL(sceneActionsCollector, handleInitializeScene(_, _));
            // send update
            EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(_, _, _)).Times(1u);
            EXPECT_TRUE(m_scene.flush());
        }

        // simulate invalid RT so that next flush fails
        EXPECT_TRUE(rb2->impl().setProperties(1u, 1u, 1u));

        // no action sent due to failed flush
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(_, _, _)).Times(0u);
        EXPECT_FALSE(m_scene.flush());

        // fix invalid RT so flush succeeds
        EXPECT_TRUE(rb2->impl().setProperties(1u, 2u, 3u));
        // now changes are sent
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(_, _, _)).Times(1u);
        EXPECT_TRUE(m_scene.flush());

        EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(_, _));
    }

    TEST_F(AScene, failsToSaveToFileIfFlushFails)
    {
        // simulate invalid RT so that flush fails
        const auto rb1 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        const auto rb2 = m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u);
        RenderTargetDescription desc;
        EXPECT_TRUE(desc.addRenderBuffer(*rb1));
        EXPECT_TRUE(desc.addRenderBuffer(*rb2));
        m_scene.createRenderTarget(desc);
        EXPECT_TRUE(m_scene.flush());
        EXPECT_TRUE(rb2->impl().setProperties(1u, 1u, 1u)); // mismatching RBs
        EXPECT_FALSE(m_scene.flush());

        EXPECT_FALSE(m_scene.saveToFile("will_not_write"));
        const auto err = framework.getLastError();
        ASSERT_TRUE(err);
        EXPECT_EQ(&m_scene, err->object);
        EXPECT_EQ("Scene::saveToFile failed due to failed scene flush: Scene::flush: Flushing scene failed: RenderTarget uses render buffers with mismatching resolution and/or sample count."
            " This RenderTarget cannot be used and attempt to flush scene will fail.", err->message);

        // fix invalid RT so flush succeeds
        EXPECT_TRUE(rb2->impl().setProperties(1u, 2u, 3u));
        EXPECT_TRUE(m_scene.saveToFile("tmp.ramses"));
    }

    TEST_F(AScene, failsToSaveToFileIfLogicEngineUpdateFails)
    {
        const std::string_view srcCode = R"(
            function interface(IN,OUT)
                IN.target = Type:Bool()
                OUT.source = Type:Bool()
            end
            function run(IN,OUT)
                error("This will die")
            end
        )";

        // simulate error in script so that update fails
        auto logic = m_scene.createLogicEngine();
        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);
        auto script = logic->createLuaScript(srcCode, config);
        EXPECT_FALSE(logic->update());

        EXPECT_FALSE(m_scene.saveToFile("will_not_write"));
        const auto err = framework.getLastError();
        ASSERT_TRUE(err);
        EXPECT_EQ(logic, err->object);
        EXPECT_EQ("Scene::saveToFile failed due to failed logic engine update: [string \"RL_lua_script\"]:7:"
            " This will die\nstack traceback:\n\t[C]: in function 'error'\n\t[string \"RL_lua_script\"]:7: in function <[string \"RL_lua_script\"]:6>", err->message);

        // destroy invalid script so update succeeds
        EXPECT_TRUE(logic->destroy(*script));
        EXPECT_TRUE(m_scene.saveToFile("tmp.ramses"));
    }

    TEST_F(ASceneWithContent, sceneIsFlushedBeforeSavingAndHierarchicalVisibilityIsAppliedToSavedFile)
    {
        // set root invisible and flush so all children are invisible
        EXPECT_TRUE(m_vis1.setVisibility(EVisibilityMode::Invisible));
        EXPECT_TRUE(m_scene.flush());
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        // destroying root will make children visible again, dont flush, only save to file
        EXPECT_TRUE(m_scene.destroy(m_vis1));
        EXPECT_TRUE(m_scene.saveToFile("tmp.ramses"));
        // hierchical visibility was applied, children are now visible
        EXPECT_EQ(m_mesh1a.impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl().getFlattenedVisibility(), EVisibilityMode::Visible);

        // same state (visible children) was saved to file
        EXPECT_TRUE(client.destroy(m_scene));
        auto loadedScene = client.loadSceneFromFile("tmp.ramses");
        ASSERT_TRUE(loadedScene);
        EXPECT_EQ(loadedScene->findObject<MeshNode>("mesh1a")->impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(loadedScene->findObject<MeshNode>("mesh1b")->impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(AScene, logicEngineIsUpdatedBeforeSaving)
    {
        auto node = m_scene.createNode("boundNode");
        auto logic = m_scene.createLogicEngine();
        auto binding = logic->createNodeBinding(*node);
        EXPECT_TRUE(binding->getInputs()->getChild("translation")->set(glm::vec3{ 1, 2, 3 }));
        EXPECT_TRUE(logic->update());
        glm::vec3 nodeTr;
        EXPECT_TRUE(node->getTranslation(nodeTr));
        EXPECT_EQ(glm::vec3(1, 2, 3), nodeTr);

        // now set translation via binding to new value but with no update
        EXPECT_TRUE(binding->getInputs()->getChild("translation")->set(glm::vec3{ 111, 222, 333 }));
        // no change in node
        EXPECT_TRUE(node->getTranslation(nodeTr));
        EXPECT_EQ(glm::vec3(1, 2, 3), nodeTr);

        // saving should update logic
        EXPECT_TRUE(m_scene.saveToFile("tmp.ramses"));
        // now node changed
        EXPECT_TRUE(node->getTranslation(nodeTr));
        EXPECT_EQ(glm::vec3(111, 222, 333), nodeTr);

        // same state (node with new values) was saved to file
        EXPECT_TRUE(client.destroy(m_scene));
        auto loadedScene = client.loadSceneFromFile("tmp.ramses");
        ASSERT_TRUE(loadedScene);
        EXPECT_TRUE(loadedScene->findObject<Node>("boundNode")->getTranslation(nodeTr));
        EXPECT_EQ(glm::vec3(111, 222, 333), nodeTr);
    }

    TEST(SceneAPILog, log_export)
    {
        static ramses::RamsesFrameworkConfig config(ramses::EFeatureLevel::EFeatureLevel_01);
        config.setLogLevel(ramses::ELogLevel::Trace);
        ramses::RamsesFramework ramsesFramework{config};
        ramses::RamsesClient&   client = *ramsesFramework.createClient("example client");
        ramses::Scene*          scene  = client.createScene(ramses::sceneId_t(123u), "example scene");

        ramses::SaveFileConfig saveConfig;
        saveConfig.setMetadataString(R"({
"generator" : "test"
})");
        ASSERT_TRUE(scene->saveToFile("saveToFile_empty.ramses", saveConfig));
        ASSERT_TRUE(std::filesystem::remove("saveToFile_empty.ramses"));
    }

    TEST(SceneAPILog, log_create_node)
    {
        ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel::EFeatureLevel_01};
        config.setLogLevel(ramses::ELogLevel::Trace);
        ramses::RamsesFramework ramsesFramework{config};
        ramses::RamsesClient&   client = *ramsesFramework.createClient("example client");
        ramses::Scene*          scene  = client.createScene(ramses::sceneId_t(123u), "example scene");

        scene->createNode("{}");
    }

    TEST_F(ASceneWithContent, canMergeSceneFromFile)
    {
        const std::string_view fileName{"tmp.ramses"};

        ASSERT_TRUE(m_scene.saveToFile(fileName));
        EXPECT_TRUE(client.destroy(m_scene));

        const sceneId_t sceneId{123u};
        auto* newScene = client.createScene(sceneId);
        ASSERT_NE(nullptr, newScene);
        auto* node = newScene->createNode();
        ASSERT_NE(nullptr, node);

        ASSERT_TRUE(client.mergeSceneFromFile(*newScene, fileName));
        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1a"));
        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1b"));
    }

    TEST_F(ASceneWithContent, canMergeSceneFromMemory)
    {
        const std::string_view fileName{"tmp.ramses"};

        ASSERT_TRUE(m_scene.saveToFile(fileName));
        EXPECT_TRUE(client.destroy(m_scene));

        const sceneId_t sceneId{123u};
        auto* newScene = client.createScene(sceneId);
        ASSERT_NE(nullptr, newScene);
        auto* node = newScene->createNode();
        ASSERT_NE(nullptr, node);

        ramses::internal::File file(fileName);
        size_t fileSize = 0;
        EXPECT_TRUE(file.getSizeInBytes(fileSize));

        std::unique_ptr<std::byte[], void(*)(const std::byte*)> data(new std::byte[fileSize], [](const auto* ptr) { delete[] ptr; });
        size_t numBytesRead = 0;
        EXPECT_TRUE(file.open(ramses::internal::File::Mode::ReadOnlyBinary));
        EXPECT_EQ(ramses::internal::EStatus::Ok, file.read(data.get(), fileSize, numBytesRead));
        EXPECT_TRUE(client.mergeSceneFromMemory(*newScene, std::move(data), fileSize));

        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1a"));
        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1b"));
    }

    TEST_F(ASceneWithContent, canMergeSceneFromFileDescriptor)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));

        size_t fileSize = 0;
        {
            // write to a file with some offset
            ramses::internal::File inFile("someTemporaryFile.ram");
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
            std::vector<unsigned char> data(fileSize);
            size_t numBytesRead = 0;
            EXPECT_TRUE(inFile.open(ramses::internal::File::Mode::ReadOnlyBinary));
            EXPECT_EQ(ramses::internal::EStatus::Ok, inFile.read(data.data(), fileSize, numBytesRead));

            ramses::internal::File outFile("someTemporaryFileWithOffset.ram");
            EXPECT_TRUE(outFile.open(ramses::internal::File::Mode::WriteOverWriteOldBinary));

            uint32_t zeroData = 0;
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
            EXPECT_TRUE(outFile.write(data.data(), data.size()));
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
        }

        EXPECT_TRUE(client.destroy(m_scene));

        const sceneId_t sceneId{123u};
        auto* newScene = client.createScene(sceneId);
        ASSERT_NE(nullptr, newScene);
        auto* node = newScene->createNode();
        ASSERT_NE(nullptr, node);

        const int fd = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary ("someTemporaryFileWithOffset.ram");
        EXPECT_TRUE(client.mergeSceneFromFileDescriptor(*newScene, fd, 4u, fileSize));

        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1a"));
        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1b"));
    }

    TEST_F(ASceneWithContent, canMergeSceneFromSceneFileWithDeletedNodeAndRes)
    {
        const std::string_view fileName{"sceneWithDeletedNodeAndRes.ramses"};

        // A scene with deleted nodes and resources
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        SceneObjectVector objs;
        for (int i = 0; i < 10; ++i) {
            objs.push_back(m_scene.createNode("NodeToDelete"));
            objs.push_back(m_scene.createArrayResource(2, data, "ResToDelete"));
        }
        m_scene.createNode("dummyNode1");
        m_scene.createArrayResource(2, data, "dummyRes1");
        for (auto obj : objs) {
            ASSERT_NE(nullptr, obj);
            m_scene.destroy(*obj);
        }
        ASSERT_TRUE(m_scene.saveToFile(fileName));
        EXPECT_TRUE(client.destroy(m_scene));

        const sceneId_t sceneId{123u};
        auto* newScene = client.createScene(sceneId);
        ASSERT_NE(nullptr, newScene);
        auto* node = newScene->createNode("dummyNode2");
        ASSERT_NE(nullptr, node);
        auto* res = newScene->createArrayResource(2, data, "dummyRes2");
        ASSERT_NE(nullptr, res);

        ASSERT_TRUE(client.mergeSceneFromFile(*newScene, fileName));
        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1a"));
        EXPECT_NE(nullptr, newScene->findObject<MeshNode>("mesh1b"));
        EXPECT_EQ(nullptr, newScene->findObject<Node>("NodeToDelete"));
        EXPECT_EQ(nullptr, newScene->findObject<ArrayResource>("ResToDelete"));
        EXPECT_NE(nullptr, newScene->findObject<Node>("dummyNode1"));
        EXPECT_NE(nullptr, newScene->findObject<ArrayResource>("dummyRes1"));
        EXPECT_NE(nullptr, newScene->findObject<Node>("dummyNode2"));
        EXPECT_NE(nullptr, newScene->findObject<ArrayResource>("dummyRes2"));
    }

    class SceneMergeTest : public ::testing::Test
    {
    protected:
        static void RunMergeSceneRenderBackendCompatibilityTest(ERenderBackendCompatibility compatibilityMergeScene, ERenderBackendCompatibility compatibilityMainScene, bool expectSuccess)
        {
            RamsesFramework framework(LocalTestClient::GetDefaultFrameworkConfig(EFeatureLevel::EFeatureLevel_Latest));
            RamsesClient& client = *framework.createClient("localTestClient");

            const std::string_view fileName{ "tmp.ramses" };
            {
                const sceneId_t mergeSceneId{ 3121u };
                auto* mergeScene = client.createScene(SceneConfig{ mergeSceneId, EScenePublicationMode::LocalOnly, compatibilityMergeScene });
                ASSERT_NE(nullptr, mergeScene);
                auto* node = mergeScene->createNode();
                ASSERT_NE(nullptr, node);
                ASSERT_TRUE(mergeScene->saveToFile(fileName));
            }

            const sceneId_t sceneId{ 123u };
            auto* newScene = client.createScene(SceneConfig{ sceneId, EScenePublicationMode::LocalOnly, compatibilityMainScene });
            ASSERT_NE(nullptr, newScene);
            auto* node = newScene->createNode();
            ASSERT_NE(nullptr, node);

            EXPECT_EQ(expectSuccess, client.mergeSceneFromFile(*newScene, fileName));
        }
    };

    TEST_F(SceneMergeTest, mergeSceneFromFileFailsWithFL01)
    {
        const std::string_view fileName{"tmp.ramses"};
        {
            RamsesFramework framework(LocalTestClient::GetDefaultFrameworkConfig(EFeatureLevel::EFeatureLevel_01));
            RamsesClient& client = *framework.createClient("localTestClient");
            const sceneId_t mergeSceneId{3121u};
            auto* mergeScene = client.createScene(mergeSceneId);
            ASSERT_NE(nullptr, mergeScene);
            auto* node = mergeScene->createNode();
            ASSERT_NE(nullptr, node);
            ASSERT_TRUE(mergeScene->saveToFile(fileName));
        }

        RamsesFramework framework(LocalTestClient::GetDefaultFrameworkConfig(EFeatureLevel::EFeatureLevel_Latest));
        RamsesClient& client = *framework.createClient("localTestClient");
        const sceneId_t sceneId{123u};
        auto* newScene = client.createScene(sceneId);
        ASSERT_NE(nullptr, newScene);
        auto* node = newScene->createNode();
        ASSERT_NE(nullptr, node);

        EXPECT_FALSE(client.mergeSceneFromFile(*newScene, fileName));
    }

    TEST_F(SceneMergeTest, mergeSceneFromFileFailsWithIncompatibleRenderBackends)
    {
        RunMergeSceneRenderBackendCompatibilityTest(ERenderBackendCompatibility::OpenGL, ERenderBackendCompatibility::VulkanAndOpenGL, false);
    }

    TEST_F(SceneMergeTest, canMergeScenesFromFileWithOpenGLCompatibility)
    {
        RunMergeSceneRenderBackendCompatibilityTest(ERenderBackendCompatibility::OpenGL, ERenderBackendCompatibility::OpenGL, true);
    }

    TEST_F(SceneMergeTest, canMergeScenesFromFileWithVulkanAndOpenGLCompatibility)
    {
        RunMergeSceneRenderBackendCompatibilityTest(ERenderBackendCompatibility::VulkanAndOpenGL, ERenderBackendCompatibility::VulkanAndOpenGL, true);
    }
}
