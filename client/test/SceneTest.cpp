//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-framework-api/EDataType.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-utils.h"

#include "DataObjectImpl.h"
#include "RenderGroupImpl.h"
#include "RenderPassImpl.h"
#include "ArrayBufferImpl.h"
#include "BlitPassImpl.h"
#include "TextureSamplerImpl.h"
#include "Texture2DImpl.h"
#include "ClientTestUtils.h"
#include "SimpleSceneTopology.h"
#include "Components/FlushTimeInformation.h"
#include "PlatformAbstraction/PlatformTime.h"

using namespace testing;

namespace ramses
{
    class AScene : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        AScene()
            : LocalTestClientWithScene()
        {
            effectDescriptionEmpty.setVertexShader("void main(void) {gl_Position=vec4(0);}");
            effectDescriptionEmpty.setFragmentShader("void main(void) {gl_FragColor=vec4(0);}");
        }

    protected:
        ramses::EffectDescription effectDescriptionEmpty;
    };

    TEST(SceneOjects, canGiveSceneID)
    {
        RamsesFramework framework;
        RamsesClient& client(*framework.createClient(nullptr));
        Scene* sceneA = client.createScene(sceneId_t(1u));
        Scene* sceneB = client.createScene(sceneId_t(2u));

        ramses::Node* fromSceneA = sceneA->createNode();
        ramses::SceneObject* objectFromB = sceneB->createNode();

        EXPECT_EQ(sceneId_t(1u), fromSceneA->getSceneId());
        EXPECT_EQ(sceneId_t(2u), objectFromB->getSceneId());

        // use case to retrieve scene object itself
        EXPECT_EQ(sceneA, client.getScene(fromSceneA->getSceneId()));
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
        else
        {
            constexpr auto limit = std::numeric_limits<int32_t>::max();
            return (limit - start) + end + 1;
        }
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
        RamsesClient& remoteClient(*framework.createClient(nullptr));
        framework.connect();
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode_LocalOnly);
        Scene* distributedScene = remoteClient.createScene(sceneId_t(1u), config);
        EXPECT_NE(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishRemotelyIfSetInConfig)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient(nullptr));
        framework.connect();
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode_LocalAndRemote);
        Scene* distributedScene = remoteClient.createScene(sceneId_t(1u), config);
        EXPECT_EQ(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishRemotelyIfNoSettingInConfig)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient(nullptr));
        framework.connect();
        SceneConfig config;
        Scene* distributedScene = remoteClient.createScene(sceneId_t(1u), config);
        EXPECT_EQ(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishLocallyWhenEnablingLocalOnlyOptimisations)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient(nullptr));
        framework.connect();
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode_LocalOnly);
        Scene* distributedScene = remoteClient.createScene(sceneId_t(1u), config);
        EXPECT_EQ(StatusOK, distributedScene->publish(EScenePublicationMode_LocalOnly));
    }

    TEST(DistributedSceneTest, isPublishedAfterDisconnect)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient(nullptr));
        framework.connect();
        SceneConfig config;
        Scene* distributedScene = remoteClient.createScene(sceneId_t(1u), config);
        EXPECT_EQ(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
        framework.disconnect();
        EXPECT_TRUE(distributedScene->isPublished());
        framework.connect();
        EXPECT_TRUE(distributedScene->isPublished());
        EXPECT_NE(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishSceneIfNotConnected)
    {
        RamsesFramework framework{ LocalTestClient::GetDefaultFrameworkConfig() };
        RamsesClient& remoteClient(*framework.createClient(nullptr));
        Scene* distributedScene = remoteClient.createScene(sceneId_t(1u));
        EXPECT_EQ(StatusOK, distributedScene->publish());
        EXPECT_TRUE(distributedScene->isPublished());
        framework.connect();
        EXPECT_TRUE(distributedScene->isPublished());
    }

    TEST_F(AScene, canValidate)
    {
        EXPECT_EQ(StatusOK, m_scene.validate());
    }

    TEST_F(AScene, failsValidationIfContainsInvalidSceneObject)
    {
        RenderPass* passWithoutCamera = m_scene.createRenderPass();
        EXPECT_NE(StatusOK, m_scene.validate());

        m_scene.destroy(*passWithoutCamera);
        EXPECT_EQ(StatusOK, m_scene.validate());

        Camera* cameraWithoutValidValues = m_scene.createPerspectiveCamera();
        EXPECT_NE(StatusOK, m_scene.validate());

        m_scene.destroy(*cameraWithoutValidValues);
    }

    TEST_F(AScene, doesNotDestroyCameraWhileItIsStillUsedByARenderPass)
    {
        RenderPass* pass = m_scene.createRenderPass();
        PerspectiveCamera* perspCam = m_scene.createPerspectiveCamera();
        perspCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        perspCam->setViewport(0, 0, 100, 200);
        pass->setCamera(*perspCam);

        EXPECT_NE(StatusOK, m_scene.destroy(*perspCam));
    }

    TEST_F(AScene, removesMeshNodeFromRenderGroupsOnDestruction)
    {
        RenderGroup* group = m_scene.createRenderGroup();
        RenderGroup* group2 = m_scene.createRenderGroup();
        MeshNode* mesh = m_scene.createMeshNode();
        group->addMeshNode(*mesh);
        group2->addMeshNode(*mesh);

        m_scene.destroy(*mesh);

        EXPECT_TRUE(group->impl.getAllMeshes().empty());
        EXPECT_TRUE(group2->impl.getAllMeshes().empty());
    }

    TEST_F(AScene, failsToCreateAppearanceWhenEffectIsFromAnotherScene)
    {
        Scene& anotherScene(*client.createScene(sceneId_t{ 0xf00 }));
        Effect* effect = anotherScene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "emptyEffect");
        ASSERT_TRUE(nullptr != effect);

        Appearance* appearance = m_scene.createAppearance(*effect, "appearance");
        EXPECT_TRUE(nullptr == appearance);
    }

    TEST_F(AScene, failsToCreateGeometryBindingWhenEffectIsFromAnotherClient)
    {
        Scene& anotherScene(*client.createScene(sceneId_t{ 0xf00 }));
        Effect* effect = anotherScene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "emptyEffect");
        ASSERT_TRUE(nullptr != effect);

        GeometryBinding* geometryBinding = m_scene.createGeometryBinding(*effect, "geometryBinding");
        EXPECT_TRUE(nullptr == geometryBinding);
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenTextureIsFromAnotherScene)
    {
        Scene& anotherScene(*client.createScene(sceneId_t{ 0xf00 }));
        {
            const uint8_t data[] = { 1, 2, 3 };
            const MipLevelData mipData(3u, data);
            Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
            ASSERT_TRUE(texture != nullptr);

            TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(nullptr == textureSampler);
        }
        {
            const uint8_t data[1 * 2 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
            MipLevelData mipLevelData(sizeof(data), data);
            Texture3D* texture = anotherScene.createTexture3D(ETextureFormat::RGBA8, 2, 1, 2, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, nullptr);
            ASSERT_TRUE(nullptr != texture);

            TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(nullptr == textureSampler);
        }
        {
            const uint8_t data[4 * 10 * 10] = {};
            CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
            TextureCube* texture = anotherScene.createTextureCube(ETextureFormat::RGBA8, 10, 1, &mipLevelData, false);
            ASSERT_TRUE(nullptr != texture);

            TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(nullptr == textureSampler);
        }
        {
            RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(4u, 4u, ERenderBufferType_Color, ERenderBufferFormat_RGB8, ERenderBufferAccessMode_ReadWrite, 4u);
            ASSERT_TRUE(nullptr != renderBuffer);

            TextureSamplerMS* textureSampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
            EXPECT_TRUE(nullptr == textureSampler);
        }
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenRenderTargetIsFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != renderBuffer);

        TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *renderBuffer);
        EXPECT_TRUE(nullptr == textureSampler);

        client.destroy(anotherScene);
    }

    TEST_F(AScene, failsToCreateTextureSamplerMSWhenRenderTargetIsFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != renderBuffer);

        TextureSamplerMS* textureSampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        EXPECT_TRUE(nullptr == textureSampler);

        client.destroy(anotherScene);
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenRenderBufferIsNotReadable)
    {
        RenderBuffer* renderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_WriteOnly);
        ASSERT_TRUE(nullptr != renderBuffer);

        TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *renderBuffer);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, failsToCreateTextureSamplerMSWhenRenderBufferIsNotReadable)
    {
        RenderBuffer* renderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_WriteOnly);
        ASSERT_TRUE(nullptr != renderBuffer);

        TextureSamplerMS* textureSampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, reportsErrorWhenCreateTransformationDataProviderWithNodeFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));

        Node* node = anotherScene.createNode("node");
        ASSERT_TRUE(nullptr != node);

        EXPECT_NE(StatusOK, m_scene.createTransformationDataProvider(*node, dataProviderId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateTransformationDataConsumerWithNodeFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));

        Node* node = anotherScene.createNode("groupNode");
        ASSERT_TRUE(nullptr != node);

        EXPECT_NE(StatusOK, m_scene.createTransformationDataConsumer(*node, dataConsumerId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityChangedByParentVisibilityNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);

        m_vis2.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_vis1.setVisibility(EVisibilityMode::Off);
        m_vis2.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityAddedToInvisibleParent)
    {
        MeshNode* addMeshNode = m_scene.createMeshNode("another mesh node");

        m_vis2.setVisibility(EVisibilityMode::Invisible);
        addMeshNode->setParent(m_vis2);

        m_scene.flush();
        EXPECT_EQ(addMeshNode->impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        //cleanup
        addMeshNode->removeParent();
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityAddedToOffParent)
    {
        MeshNode* addMeshNode = m_scene.createMeshNode("another mesh node");

        m_vis2.setVisibility(EVisibilityMode::Off);
        addMeshNode->setParent(m_vis2);

        m_scene.flush();
        EXPECT_EQ(addMeshNode->impl.getFlattenedVisibility(), EVisibilityMode::Off);

        //cleanup
        addMeshNode->removeParent();
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityChangedByGrandParentVisibilityNode)
    {
        //Add another visibility hierarchy level below mesh 4
        Node& m_node8 = *m_scene.createNode("node8");
        MeshNode& m_node9 = *m_scene.createMeshNode("node9");
        EXPECT_EQ(StatusOK, m_node8.setParent(m_mesh1a));
        EXPECT_EQ(StatusOK, m_node9.setParent(m_node8));

        //Newly added mesh should be visible by default
        EXPECT_EQ(m_node9.impl.getFlattenedVisibility(), EVisibilityMode::Visible);

        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_node8.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_vis1.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        m_node8.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();

        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh2b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_node9.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
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
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, nodesStayVisibleWhenRemovedVisibleParent)
    {
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesBecomeVisibleWhenRemovedInvisibleParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesBecomeVisibleWhenRemovedOffParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, visibleMeshBecomesInvisibleWhenMovedUnderInvisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        m_mesh2a.setParent(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, visibleMeshBecomesOffWhenMovedUnderOffNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        m_mesh2a.setParent(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh2a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesInvisibleWhenSetInvisibleNodeAsParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        meshOrphan.setParent(m_vis1);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesOffWhenSetOffNodeAsParent)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        meshOrphan.setParent(m_vis1);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesInvisibleWhenMovedUnderInvisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        m_vis1.addChild(meshOrphan);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesOffWhenMovedUnderOffNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        m_vis1.addChild(meshOrphan);
        m_scene.flush();

        EXPECT_EQ(meshOrphan.impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneWithContent, meshInInvisibleBranchBecomesVisibleWhenMovedUnderVisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();

        m_mesh1a.setParent(m_vis2);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, meshInOffBranchBecomesVisibleWhenMovedUnderVisibleNode)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();

        m_mesh1a.setParent(m_vis2);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesStayVisibleIfParentBecomesInvisibleAndIsDeletedInSameCommit)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodesStayVisibleIfParentBecomesOffAndIsDeletedInSameCommit)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodeBecomesVisibleIfInvisibleParentIsRemovedFromIt)
    {
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        m_mesh1a.removeParent();
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(ASceneWithContent, nodeBecomesVisibleIfOffParentIsRemovedFromIt)
    {
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        m_mesh1a.removeParent();
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
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
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        // revert
        vis1a.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
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
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Off);

        // revert
        vis1a.setVisibility(EVisibilityMode::Visible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Visible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Off);
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
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Off);

        // revert
        vis1a.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Off);
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
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        // revert
        m_vis1.setVisibility(EVisibilityMode::Invisible);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        // set grandparent invisible
        m_vis1.setVisibility(EVisibilityMode::Off);
        m_scene.flush();
        EXPECT_EQ(m_mesh1a.impl.getFlattenedVisibility(), EVisibilityMode::Off);
        EXPECT_EQ(m_mesh1b.impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(AScene, canCreateATransformDataSlot)
    {
        Node* node = m_scene.createNode();
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        EXPECT_EQ(StatusOK, m_scene.createTransformationDataConsumer(*node, dataConsumerId_t(666u)));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(node->impl.getNodeHandle(), m_scene.impl.getIScene().getDataSlot(slotHandle).attachedNode);
        EXPECT_EQ(ramses_internal::DataSlotId(666u), m_scene.impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TransformationConsumer, m_scene.impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, removesTransformDataSlotsOfNodeOnDestruction)
    {
        Node* node = m_scene.createNode();
        m_scene.createTransformationDataProvider(*node, dataProviderId_t(1412u));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle linkHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));

        m_scene.destroy(*node);

        EXPECT_FALSE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));
    }

    TEST_F(AScene, canNotCreateMoreThanOneTransformationDataSlotForANode)
    {
        Node* node = m_scene.createNode();
        EXPECT_EQ(StatusOK, m_scene.createTransformationDataProvider(*node, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createTransformationDataProvider(*node, dataProviderId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createTransformationDataConsumer(*node, dataConsumerId_t(3u)));
    }

    TEST_F(AScene, canNotCreateMoreThanOneTransformationDataSlotWithTheSameId)
    {
        Node* node1 = m_scene.createNode();
        Node* node2 = m_scene.createNode();
        EXPECT_EQ(StatusOK, m_scene.createTransformationDataProvider(*node1, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createTransformationDataProvider(*node2, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createTransformationDataConsumer(*node2, dataConsumerId_t(1u)));

        EXPECT_EQ(StatusOK, m_scene.createTransformationDataConsumer(*node2, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createTransformationDataConsumer(*node1, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createTransformationDataProvider(*node1, dataProviderId_t(2u)));
    }

    TEST_F(AScene, reportsErrorWhenCreateDataProviderWithDataObjectFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));

        auto dataObject = anotherScene.createDataObject(EDataType::Float);
        ASSERT_TRUE(nullptr != dataObject);

        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject, dataProviderId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateDataConsumerWithDataObjectFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));

        auto dataObject = anotherScene.createDataObject(EDataType::Float);
        ASSERT_TRUE(nullptr != dataObject);

        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject, dataConsumerId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, canCreateADataObjectDataSlot)
    {
        auto dataObject = m_scene.createDataObject(EDataType::Float);
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        EXPECT_EQ(StatusOK, m_scene.createDataConsumer(*dataObject, dataConsumerId_t(666u)));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(dataObject->impl.getDataReference(), m_scene.impl.getIScene().getDataSlot(slotHandle).attachedDataReference);
        EXPECT_EQ(ramses_internal::DataSlotId(666u), m_scene.impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_DataConsumer, m_scene.impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, removesDataSlotsOfDataObjectOnDestruction)
    {
        auto dataObject = m_scene.createDataObject(EDataType::Float);
        m_scene.createDataProvider(*dataObject, dataProviderId_t(1412u));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle linkHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));

        m_scene.destroy(*dataObject);

        EXPECT_FALSE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));
    }

    TEST_F(AScene, canNotCreateMoreThanOneDataSlotForADataObject)
    {
        auto dataObject = m_scene.createDataObject(EDataType::Float);
        EXPECT_EQ(StatusOK, m_scene.createDataProvider(*dataObject, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject, dataProviderId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject, dataConsumerId_t(3u)));
    }

    TEST_F(AScene, canNotCreateMoreThanOneDataSlotWithTheSameId)
    {
        auto dataObject1 = m_scene.createDataObject(EDataType::Float);
        auto dataObject2 = m_scene.createDataObject(EDataType::Float);
        EXPECT_EQ(StatusOK, m_scene.createDataProvider(*dataObject1, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject2, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject2, dataConsumerId_t(1u)));

        EXPECT_EQ(StatusOK, m_scene.createDataConsumer(*dataObject2, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject1, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject1, dataProviderId_t(2u)));
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureProviderWithTextureFromAnotherScene)
    {
        RamsesFramework anotherFramework;
        Scene& anotherScene(*client.createScene(sceneId_t{ 0xf00 }));
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        const Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_NE(StatusOK, m_scene.createTextureProvider(*texture, dataProviderId_t{1u}));
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureConsumerWithSamplerFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        const TextureSampler* sampler = anotherScene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{1u}));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureConsumerWithSamplerMSFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(4u, 4u, ERenderBufferType_Color, ERenderBufferFormat_RGB8, ERenderBufferAccessMode_ReadWrite, 4u);
        ASSERT_TRUE(nullptr != renderBuffer);

        const TextureSamplerMS* sampler = anotherScene.createTextureSamplerMS(*renderBuffer, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 1u }));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureConsumerWithSamplerExternalFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        const TextureSamplerExternal* sampler = anotherScene.createTextureSamplerExternal(ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 1u }));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, canCreateATextureProviderDataSlot)
    {
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_EQ(StatusOK, m_scene.createTextureProvider(*texture, dataProviderId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(texture->impl.getLowlevelResourceHash(), m_scene.impl.getIScene().getDataSlot(slotHandle).attachedTexture);
        EXPECT_EQ(ramses_internal::DataSlotId(666u), m_scene.impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TextureProvider, m_scene.impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canUpdateATextureProviderDataSlot)
    {
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        uint8_t data1 = 0u;
        MipLevelData mipData1(1u, &data1);
        Texture2D* texture1 = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData1, false);
        ASSERT_TRUE(nullptr != texture1);

        uint8_t data2 = 1u;
        MipLevelData mipData2(1u, &data2);
        Texture2D* texture2 = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData2, false);
        ASSERT_TRUE(nullptr != texture2);

        EXPECT_EQ(StatusOK, m_scene.createTextureProvider(*texture1, dataProviderId_t{666u}));
        EXPECT_EQ(StatusOK, m_scene.updateTextureProvider(*texture2, dataProviderId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(texture2->impl.getLowlevelResourceHash(), m_scene.impl.getIScene().getDataSlot(slotHandle).attachedTexture);
        EXPECT_EQ(ramses_internal::DataSlotId(666u), m_scene.impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TextureProvider, m_scene.impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canCreateATextureConsumerDataSlot)
    {
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        const TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(sampler->impl.getTextureSamplerHandle(), m_scene.impl.getIScene().getDataSlot(slotHandle).attachedTextureSampler);
        EXPECT_EQ(ramses_internal::DataSlotId(666u), m_scene.impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TextureConsumer, m_scene.impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canCreateATextureConsumerDataSlotWithTextureSamplerMS)
    {
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        RenderBuffer* renderBuffer = m_scene.createRenderBuffer(4u, 4u, ERenderBufferType_Color, ERenderBufferFormat_RGB8, ERenderBufferAccessMode_ReadWrite, 4u);
        ASSERT_TRUE(nullptr != renderBuffer);

        const TextureSamplerMS* sampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");

        ASSERT_TRUE(nullptr != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(sampler->impl.getTextureSamplerHandle(), m_scene.impl.getIScene().getDataSlot(slotHandle).attachedTextureSampler);
        EXPECT_EQ(ramses_internal::DataSlotId(666u), m_scene.impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TextureConsumer, m_scene.impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canCreateATextureConsumerDataSlotWithTextureSamplerExternal)
    {
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        const TextureSamplerExternal* sampler = m_scene.createTextureSamplerExternal(ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear);

        ASSERT_TRUE(nullptr != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle slotHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(sampler->impl.getTextureSamplerHandle(), m_scene.impl.getIScene().getDataSlot(slotHandle).attachedTextureSampler);
        EXPECT_EQ(ramses_internal::DataSlotId(666u), m_scene.impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TextureConsumer, m_scene.impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(AScene, canNotCreateATextureConsumerDataSlotIfOtherThan2DTextureAssigned)
    {
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::R8, 1u, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        const TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));
    }

    TEST_F(AScene, removesDataSlotsOfTextureSamplerOnDestruction)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle linkHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));

        m_scene.destroy(*sampler);

        EXPECT_FALSE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));
    }

    TEST_F(AScene, canNotCreateMoreThanOneConsumerForATextureSampler)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{666u}));
        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{667u}));
    }

    TEST_F(AScene, canNotCreateMoreThanOneConsumerForATextureSamplerMS)
    {
        RenderBuffer* renderBuffer = m_scene.createRenderBuffer(4u, 4u, ERenderBufferType_Color, ERenderBufferFormat_RGB8, ERenderBufferAccessMode_ReadWrite, 4u);
        ASSERT_TRUE(nullptr != renderBuffer);

        const TextureSamplerMS* sampler = m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));
        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 667u }));
    }

    TEST_F(AScene, canNotCreateMoreThanOneConsumerForATextureSamplerExternal)
    {
        const TextureSamplerExternal* sampler = m_scene.createTextureSamplerExternal(ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear);
        ASSERT_TRUE(nullptr != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 666u }));
        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t{ 667u }));
    }

    TEST_F(AScene, canNotCreateMoreThanOneProviderForATexture)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_EQ(StatusOK, m_scene.createTextureProvider(*texture, dataProviderId_t{666u}));
        EXPECT_NE(StatusOK, m_scene.createTextureProvider(*texture, dataProviderId_t{667u}));
    }

    TEST_F(AScene, canNotCreateMoreThanOneTextureConsumerOrProviderWithTheSameId)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);
        Texture2D* texture2 = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture2);

        TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(nullptr != sampler);
        TextureSampler* sampler2 = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture2);
        ASSERT_TRUE(nullptr != sampler2);

        EXPECT_EQ(StatusOK, m_scene.createTextureProvider(*texture, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createTextureProvider(*texture2, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t(1u)));

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler2, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createTextureProvider(*texture, dataProviderId_t(2u)));
    }

    TEST_F(AScene, canNotUpdateTextureProviderWhichWasNotCreatedBefore)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::R8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(nullptr != texture);

        EXPECT_NE(StatusOK, m_scene.updateTextureProvider(*texture, dataProviderId_t(1u)));
    }

    TEST_F(AScene, canCreateTextureSamplerForTexture2DWithDefaultAnisotropyLevel)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, texture2D);

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Nearest, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_Texture2D, sampler->getTextureType());
    }

    TEST_F(AScene, canCreateTextureSamplerForRenderBufferWithDefaultAnisotropyLevel)
    {
        const RenderBuffer& renderBuffer = createObject<RenderBuffer>("renderBuffer");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, renderBuffer);

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Nearest, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_RenderBuffer, sampler->getTextureType());
    }

    TEST_F(AScene, canCreateTextureSamplerForTextureCubeWithDefaultAnisotropyLevel)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, textureCube);

        ASSERT_NE(static_cast<TextureSampler*>(nullptr), sampler);
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapUMode());
        EXPECT_EQ(ETextureAddressMode_Clamp, sampler->getWrapVMode());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMinSamplingMethod());
        EXPECT_EQ(ETextureSamplingMethod_Linear, sampler->getMagSamplingMethod());
        EXPECT_EQ(1u, sampler->getAnisotropyLevel());
        EXPECT_EQ(ERamsesObjectType_TextureCube, sampler->getTextureType());
    }

    TEST_F(AScene, cantCreateTextureSamplerForTexture2DWithWrongAnisotropyValue)
    {
        const Texture2D& texture = createObject<Texture2D>("testTexture2D");
        TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, texture, 0);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerForTextureCubeWithWrongAnisotropyValue)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, textureCube, 0);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerForRenderBufferWithWrongAnisotropyValue)
    {
        const RenderBuffer& renderBuffer = createObject<RenderBuffer>("renderBuffer");
        const TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, renderBuffer, 0);
        EXPECT_TRUE(nullptr == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerWithWrongMagSamplingMethod)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        EXPECT_EQ(nullptr, this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear_MipMapLinear, textureCube));
        EXPECT_EQ(nullptr, this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear_MipMapNearest, textureCube));
        EXPECT_EQ(nullptr, this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Nearest_MipMapNearest, textureCube));
    }

    TEST_F(AScene, canCreateBlitPass)
    {
        const RenderBuffer& sourceRenderBuffer = createObject<RenderBuffer>("src renderBuffer");
        const RenderBuffer& destinationRenderBuffer = createObject<RenderBuffer>("dst renderBuffer");
        const BlitPass* blitPass = m_scene.createBlitPass(sourceRenderBuffer, destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr != blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceRenderBufferFromDifferentScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        const RenderBuffer* sourceRenderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);

        const RenderBuffer& destinationRenderBuffer = createObject<RenderBuffer>("dst renderBuffer");
        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBufferFromDifferentScenes)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        const RenderBuffer* destinationRenderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const RenderBuffer& sourceRenderBuffer = createObject<RenderBuffer>("src renderBuffer");
        const BlitPass* blitPass = m_scene.createBlitPass(sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentType)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_R8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Depth, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentFormat)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_R8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavinghDifferentWidth)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(1u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentHeight)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 1u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != sourceRenderBuffer);
        ASSERT_TRUE(nullptr != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSameSourceAndDestinationRenderBuffers)
    {
        const RenderBuffer* rb = m_scene.createRenderBuffer(100u, 1u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != rb);
        const BlitPass* blitPass = m_scene.createBlitPass(*rb, *rb, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, canCreatePickableObject)
    {
        const ArrayBuffer* geometryBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 3);
        ASSERT_TRUE(nullptr != geometryBuffer);

        const PickableObject* pickableObject = m_scene.createPickableObject(*geometryBuffer, pickableObjectId_t(1u));
        ASSERT_NE(nullptr, pickableObject);
    }

    TEST_F(AScene, cannotCreatePickableObjectWithGeometryBufferOfAnotherScene)
    {
        Scene* anotherScene = client.createScene(sceneId_t(111u));
        const ArrayBuffer* geometryBufferFromOtherScene = anotherScene->createArrayBuffer(ramses::EDataType::Vector3F, 3);

        const PickableObject* pickableObject = m_scene.createPickableObject(*geometryBufferFromOtherScene, pickableObjectId_t(1u));
        EXPECT_EQ(nullptr, pickableObject);
    }

    TEST_F(AScene, cannotCreatePickableObjectWithWrongGeometryBufferSize)
    {
        const ArrayBuffer* geometryBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 2u);
        ASSERT_TRUE(nullptr != geometryBuffer);

        const PickableObject* pickableObject = m_scene.createPickableObject(*geometryBuffer, pickableObjectId_t(1u));
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
        ArrayBuffer* const interleavedVertexBuffer = m_scene.createArrayBuffer(EDataType::ByteBlob, 14u);
        ASSERT_NE(nullptr, interleavedVertexBuffer);

        EXPECT_EQ(ramses::EDataType::ByteBlob, interleavedVertexBuffer->getDataType());
        EXPECT_EQ(1u, GetSizeOfDataType(interleavedVertexBuffer->getDataType()));
        EXPECT_EQ(14u, interleavedVertexBuffer->getMaximumNumberOfElements());

        m_scene.destroy(*interleavedVertexBuffer);
    }

    TEST_F(AScene, flushIncreasesStatisticCounter)
    {
        EXPECT_EQ(0u, m_scene.impl.getStatisticCollection().statFlushesTriggered.getCounterValue());
        m_scene.flush();
        EXPECT_EQ(1u, m_scene.impl.getStatisticCollection().statFlushesTriggered.getCounterValue());
    }

    TEST_F(AScene, canGetResourceByID)
    {
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr != effectFixture);

        const resourceId_t resourceID = effectFixture->getResourceId();
        ramses::Resource* resource = m_scene.getResource(resourceID);
        ASSERT_TRUE(nullptr != resource);

        const ramses::Effect* effectFound = RamsesUtils::TryConvert<ramses::Effect>(*resource);
        ASSERT_TRUE(nullptr != effectFound);

        ASSERT_TRUE(effectFound == effectFixture);

        const resourceId_t nonExistEffectId = { 0, 0 };
        ASSERT_TRUE(nullptr == m_scene.getResource(nonExistEffectId));
    }

    TEST_F(AScene, returnsNULLWhenResourceWithIDCannotBeFound)
    {
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr != effectFixture);

        const resourceId_t nonExistEffectId = { 0, 0 };
        ASSERT_TRUE(nullptr == m_scene.getResource(nonExistEffectId));
    }

    TEST_F(AScene, returnsNULLWhenTryingToFindDeletedResource)
    {
        auto effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
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
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr != effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_withDefines)
    {
        effectDescriptionEmpty.addCompilerDefine("float dummy;");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr != effectFixture);
    }

    // effect from string: invalid uses
    TEST_F(AScene, createEffectFromGLSLString_invalidVertexShader)
    {
        effectDescriptionEmpty.setVertexShader("void main(void) {dsadsadasd}");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_emptyVertexShader)
    {
        effectDescriptionEmpty.setVertexShader("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_invalidFragmentShader)
    {
        effectDescriptionEmpty.setFragmentShader("void main(void) {dsadsadasd}");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_emptyFragmentShader)
    {
        effectDescriptionEmpty.setFragmentShader("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSLString_invalidDefines)
    {
        effectDescriptionEmpty.addCompilerDefine("thisisinvalidstuff\n8fd7f9ds");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
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
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
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
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_FALSE(nullptr != effectFixture);
    }

    // effect from file: valid uses
    TEST_F(AScene, createEffectFromGLSL_withName)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-client-test_minimalShader.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr != effectFixture);
    }


    // effect from file: invalid uses
    TEST_F(AScene, createEffectFromGLSL_nonExistantVertexShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSL_nonExistantFragmentShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-client-test_minimalShader.frag");
        effectDesc.setFragmentShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSL_NULLVertexShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("");
        effectDesc.setFragmentShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, createEffectFromGLSL_NULLFragmentShader)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);
    }

    TEST_F(AScene, verifyHLAPILogCanHandleNullPtrReturnWhenEnabled)
    {
        ramses_internal::ELogLevel oldLogLevel = ramses_internal::CONTEXT_HLAPI_CLIENT.getLogLevel();
        ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(ramses_internal::ELogLevel::Trace);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/this_file_should_not_exist_fdsfdsjf84w9wufw.vert");
        effectDesc.setFragmentShaderFromFile("");
        const ramses::Effect* effectFixture = m_scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "name");
        EXPECT_TRUE(nullptr == effectFixture);

        ramses_internal::CONTEXT_HLAPI_CLIENT.setLogLevel(oldLogLevel);
    }

    TEST_F(AScene, returnsFalseOnFlushWhenResourcesAreMissing)
    {
        m_creationHelper.createObjectOfType<Appearance>("meh");
        EXPECT_EQ(StatusOK, m_scene.flush()); // test legal scene state => flush success
        m_scene.destroy(*RamsesUtils::TryConvert<SceneObject>(*m_scene.findObjectByName("appearance effect")));
        m_scene.destroy(*RamsesUtils::TryConvert<SceneObject>(*m_scene.findObjectByName("meh")));
        EXPECT_EQ(StatusOK, m_scene.flush()); // resource is deleted, but nobody needs it => flush success

        auto* appearnace = m_creationHelper.createObjectOfType<Appearance>("meh2");
        auto* mesh = m_creationHelper.createObjectOfType<MeshNode>("meh3");
        mesh->setAppearance(*appearnace);
        m_scene.destroy(*RamsesUtils::TryConvert<SceneObject>(*m_scene.findObjectByName("appearance effect")));
        EXPECT_NE(StatusOK, m_scene.flush()); // test scene with resource missing => flush failed
    }

    TEST_F(AScene, uniformTimeMatchesSyncClockInitiallyAndCanBeReset)
    {
        const int32_t tolerance = 10000; // 10 seconds to tolerate stucks during test execution
        const auto now = ramses_internal::PlatformTime::GetMillisecondsSynchronized();
        const auto s32now = static_cast<int32_t>(now % std::numeric_limits<int32_t>::max());
        EXPECT_GE(tolerance, GetElapsed(s32now, m_scene.getUniformTimeMs()));
        EXPECT_EQ(StatusOK, m_scene.resetUniformTimeMs());
        EXPECT_GE(tolerance, GetElapsed(0, m_scene.getUniformTimeMs()));
    }

    TEST_F(AScene, resetUniformTimeMs)
    {
        EXPECT_EQ(ramses_internal::FlushTime::InvalidTimestamp, m_scene.impl.getIScene().getEffectTimeSync());
        EXPECT_EQ(StatusOK, m_scene.resetUniformTimeMs());
        const auto timeSync = m_scene.impl.getIScene().getEffectTimeSync();
        EXPECT_NE(0, timeSync.time_since_epoch().count());

        EXPECT_EQ(StatusOK, m_scene.flush());

        EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(_, _));
        EXPECT_CALL(sceneActionsCollector, handleInitializeScene(_, _));

        // internal timestamp contains sync time
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses_internal::SceneId(123u), _, _)).WillOnce([&](auto, const auto& update, auto) {
            const ramses_internal::FlushTimeInformation& timeInfo = update.flushInfos.flushTimeInfo;
            EXPECT_TRUE(timeInfo.isEffectTimeSync);
            EXPECT_EQ(timeSync, timeInfo.internalTimestamp);
            });

        EXPECT_EQ(StatusOK, m_scene.publish(ramses::EScenePublicationMode_LocalOnly));
        Mock::VerifyAndClearExpectations(&sceneActionsCollector);

        // internal timestamp contains flush time
        ramses_internal::PlatformThread::Sleep(10);
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses_internal::SceneId(123u), _, _)).WillOnce([&](auto, const auto& update, auto) {
            const ramses_internal::FlushTimeInformation& timeInfo = update.flushInfos.flushTimeInfo;
            EXPECT_FALSE(timeInfo.isEffectTimeSync);
            EXPECT_NE(timeSync, timeInfo.internalTimestamp);
            });
        EXPECT_EQ(StatusOK, m_scene.flush(42u));

        EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(_, _));
    }
}
