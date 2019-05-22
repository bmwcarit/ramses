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
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/EDataType.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "DataObjectImpl.h"
#include "RenderGroupImpl.h"
#include "RenderPassImpl.h"
#include "BlitPassImpl.h"
#include "TextureSamplerImpl.h"
#include "Texture2DImpl.h"
#include "StreamTextureImpl.h"
#include "ClientTestUtils.h"
#include "SimpleSceneTopology.h"

using namespace testing;

namespace ramses
{
    class AScene : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        AScene()
            : LocalTestClientWithScene()
        {
        }
    };

    class ASceneWithContent : public SimpleSceneTopology
    {
    };

    TEST(DistributedSceneTest, givesErrorWhenRemotePublishingAfterEnablingLocalOnlyOptmisations)
    {
        RamsesFramework framework(sizeof(clientArgs) / sizeof(char*), clientArgs);
        RamsesClient remoteClient(NULL, framework);
        framework.connect();
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode_LocalOnly);
        Scene* distributedScene = remoteClient.createScene(1u, config);
        EXPECT_NE(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishRemotelyIfSetInConfig)
    {
        RamsesFramework framework(sizeof(clientArgs) / sizeof(char*), clientArgs);
        RamsesClient remoteClient(NULL, framework);
        framework.connect();
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode_LocalAndRemote);
        Scene* distributedScene = remoteClient.createScene(1u, config);
        EXPECT_EQ(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishRemotelyIfNoSettingInConfig)
    {
        RamsesFramework framework(sizeof(clientArgs) / sizeof(char*), clientArgs);
        RamsesClient remoteClient(NULL, framework);
        framework.connect();
        SceneConfig config;
        Scene* distributedScene = remoteClient.createScene(1u, config);
        EXPECT_EQ(StatusOK, distributedScene->publish(EScenePublicationMode_LocalAndRemote));
    }

    TEST(DistributedSceneTest, canPublishLocallyWhenEnablingLocalOnlyOptimisations)
    {
        RamsesFramework framework(sizeof(clientArgs) / sizeof(char*), clientArgs);
        RamsesClient remoteClient(NULL, framework);
        framework.connect();
        SceneConfig config;
        config.setPublicationMode(EScenePublicationMode_LocalOnly);
        Scene* distributedScene = remoteClient.createScene(1u, config);
        EXPECT_EQ(StatusOK, distributedScene->publish(EScenePublicationMode_LocalOnly));
    }

    TEST(DistributedSceneTest, reportsErrorWhenPublishSceneIfNotConnected)
    {
        RamsesFramework framework;
        RamsesClient remoteClient(NULL, framework);
        Scene* distributedScene = remoteClient.createScene(1u);
        EXPECT_NE(StatusOK, distributedScene->publish());
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
        Camera* camera = m_scene.createRemoteCamera();
        pass->setCamera(*camera);

        EXPECT_NE(StatusOK, m_scene.destroy(*camera));
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

    TEST_F(AScene, failsToCreateAppearanceWhenEffectIsFromAnotherClient)
    {
        EffectDescription effectDescriptionEmpty;
        effectDescriptionEmpty.setVertexShader("void main(void) {gl_Position=vec4(0);}");
        effectDescriptionEmpty.setFragmentShader("void main(void) {gl_FragColor=vec4(0);}");

        RamsesClient anotherClient("anotherClient", framework);
        Effect* effect = anotherClient.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "emptyEffect");
        ASSERT_TRUE(NULL != effect);

        Appearance* appearance = m_scene.createAppearance(*effect, "appearance");
        EXPECT_TRUE(NULL == appearance);
    }

    TEST_F(AScene, failsToCreateGeometryBindingWhenEffectIsFromAnotherClient)
    {
        EffectDescription effectDescriptionEmpty;
        effectDescriptionEmpty.setVertexShader("void main(void) {gl_Position=vec4(0);}");
        effectDescriptionEmpty.setFragmentShader("void main(void) {gl_FragColor=vec4(0);}");

        RamsesClient anotherClient("anotherClient", framework);
        Effect* effect = anotherClient.createEffect(effectDescriptionEmpty, ramses::ResourceCacheFlag_DoNotCache, "emptyEffect");
        ASSERT_TRUE(NULL != effect);

        GeometryBinding* geometryBinding = m_scene.createGeometryBinding(*effect, "geometryBinding");
        EXPECT_TRUE(NULL == geometryBinding);
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenTextureIsFromAnotherClient)
    {
        RamsesClient anotherClient("anotherClient", framework);
        {
            const uint8_t data[] = { 1, 2, 3 };
            const MipLevelData mipData(3u, data);
            Texture2D* texture = anotherClient.createTexture2D(1u, 1u, ETextureFormat_RGB8, 1u, &mipData, false);
            ASSERT_TRUE(texture != NULL);

            TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(NULL == textureSampler);
        }
        {
            const uint8_t data[1 * 2 * 2 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
            MipLevelData mipLevelData(sizeof(data), data);
            Texture3D* texture = anotherClient.createTexture3D(2, 1, 2, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, NULL);
            ASSERT_TRUE(NULL != texture);

            TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(NULL == textureSampler);
        }
        {
            const uint8_t data[4 * 10 * 10] = {};
            CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
            TextureCube* texture = anotherClient.createTextureCube(10, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false);
            ASSERT_TRUE(NULL != texture);

            TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(NULL == textureSampler);
        }
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenRenderTargetIsFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);
        RenderBuffer* renderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(NULL != renderBuffer);

        TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *renderBuffer);
        EXPECT_TRUE(NULL == textureSampler);

        client.destroy(anotherScene);
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenStreamTextureIsFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);
        const Texture2D& fallbackTexture = createObject<Texture2D>("fallbackTexture");
        StreamTexture* streamTexture = anotherScene.createStreamTexture(fallbackTexture, streamSource_t(1), "testStreamTexture");
        ASSERT_TRUE(NULL != streamTexture);

        TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *streamTexture);
        EXPECT_TRUE(NULL == textureSampler);

        client.destroy(anotherScene);
    }

    TEST_F(AScene, failsToCreateTextureSamplerWhenRenderBufferIsNotReadable)
    {
        RenderBuffer* renderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_WriteOnly);
        ASSERT_TRUE(NULL != renderBuffer);

        TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *renderBuffer);
        EXPECT_TRUE(NULL == textureSampler);
    }

    TEST_F(AScene, reportsErrorWhenCreateTransformationDataProviderWithNodeFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);

        Node* node = anotherScene.createNode("node");
        ASSERT_TRUE(NULL != node);

        EXPECT_NE(StatusOK, m_scene.createTransformationDataProvider(*node, 1u));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateTransformationDataConsumerWithNodeFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);

        Node* node = anotherScene.createNode("groupNode");
        ASSERT_TRUE(NULL != node);

        EXPECT_NE(StatusOK, m_scene.createTransformationDataConsumer(*node, 1u));

        client.destroy(anotherScene);
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityChangedByParentVisibilityNode)
    {
        m_vis1.setVisibility(false);
        m_scene.flush();

        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh1b.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2b.impl.getFlattenedVisibility());

        m_vis2.setVisibility(false);
        m_scene.flush();

        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh1b.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh2a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh2b.impl.getFlattenedVisibility());

        m_vis1.setVisibility(true);
        m_vis2.setVisibility(true);
        m_scene.flush();

        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2b.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, checksMeshVisibilityAddedToInvisibleParent)
    {
        MeshNode* addMeshNode = m_scene.createMeshNode("another mesh node");

        m_vis2.setVisibility(false);
        addMeshNode->setParent(m_vis2);

        m_scene.flush();
        EXPECT_FALSE(addMeshNode->impl.getFlattenedVisibility());

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
        EXPECT_TRUE(m_node9.impl.getFlattenedVisibility());

        m_vis1.setVisibility(false);
        m_scene.flush();

        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh1b.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2b.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_node9.impl.getFlattenedVisibility());

        m_node8.setVisibility(false);
        m_scene.flush();

        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh1b.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2b.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_node9.impl.getFlattenedVisibility());

        m_vis1.setVisibility(true);
        m_scene.flush();

        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2b.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_node9.impl.getFlattenedVisibility());

        m_node8.setVisibility(true);
        m_scene.flush();

        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh2b.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_node9.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, subTreeStaysInvisibleIfSettingVisibilityNodeVisibleParentedByInvisibleNode)
    {
        // 'root' invisible
        m_vis1.setVisibility(false);
        // add another visibility node under invisible visibility node
        Node& vis1v = *m_scene.createNode();
        m_vis1.addChild(vis1v);
        // reparent meshes under it
        vis1v.addChild(m_mesh1a);
        vis1v.addChild(m_mesh1b);
        // explicitly set visible
        m_scene.flush();
        vis1v.setVisibility(false);
        vis1v.setVisibility(true);

        // should stay invisible
        m_scene.flush();
        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh1b.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, nodesStayVisibleWhenRemovedVisibleParent)
    {
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, nodesBecomeVisibleWhenRemovedInvisibleParent)
    {
        m_vis1.setVisibility(false);
        m_scene.flush();

        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, visibleMeshBecomesInvisibleWhenMovedUnderInvisibleNode)
    {
        m_vis1.setVisibility(false);
        m_scene.flush();

        m_mesh2a.setParent(m_vis1);
        m_scene.flush();
        EXPECT_FALSE(m_mesh2a.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesInvisibleWhenSetInvisibleNodeAsParent)
    {
        m_vis1.setVisibility(false);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        meshOrphan.setParent(m_vis1);
        m_scene.flush();

        EXPECT_FALSE(meshOrphan.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, visibleMeshWithoutParentBecomesInvisibleWhenMovedUnderInvisibleNode)
    {
        m_vis1.setVisibility(false);
        m_scene.flush();

        MeshNode& meshOrphan = *m_scene.createMeshNode("meshOrphan");
        m_vis1.addChild(meshOrphan);
        m_scene.flush();

        EXPECT_FALSE(meshOrphan.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, invisibleMeshBecomesVisibleWhenMovedUnderVisibleNode)
    {
        m_vis1.setVisibility(false);
        m_scene.flush();

        m_mesh1a.setParent(m_vis2);
        m_scene.flush();
        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, nodesStayVisibleIfParentBecomesInvisibleAndIsDeletedInSameCommit)
    {
        m_vis1.setVisibility(false);
        m_scene.destroy(m_vis1);
        m_scene.flush();
        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());
    }

    TEST_F(ASceneWithContent, nodeBecomesVisibleIfInvisibleParentIsRemovedFromIt)
    {
        m_vis1.setVisibility(false);
        m_scene.flush();
        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        m_mesh1a.removeParent();
        m_scene.flush();
        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
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
        vis1a.setVisibility(false);
        m_scene.flush();
        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh1b.impl.getFlattenedVisibility());

        // revert
        vis1a.setVisibility(true);
        m_scene.flush();
        EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());

        // set grandparent invisible
        m_vis1.setVisibility(false);
        m_scene.flush();
        EXPECT_FALSE(m_mesh1a.impl.getFlattenedVisibility());
        EXPECT_FALSE(m_mesh1b.impl.getFlattenedVisibility());
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
        Scene& anotherScene = *client.createScene(12u);

        DataFloat* dataObject = anotherScene.createDataFloat();
        ASSERT_TRUE(NULL != dataObject);

        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject, 1u));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, reportsErrorWhenCreateDataConsumerWithDataObjectFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);

        DataFloat* dataObject = anotherScene.createDataFloat();
        ASSERT_TRUE(NULL != dataObject);

        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject, 1u));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, canCreateADataObjectDataSlot)
    {
        DataFloat* dataObject = m_scene.createDataFloat();
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
        DataFloat* dataObject = m_scene.createDataFloat();
        m_scene.createDataProvider(*dataObject, dataProviderId_t(1412u));

        EXPECT_EQ(1u, m_scene.impl.getIScene().getDataSlotCount());
        ramses_internal::DataSlotHandle linkHandle(0u);
        ASSERT_TRUE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));

        m_scene.destroy(*dataObject);

        EXPECT_FALSE(m_scene.impl.getIScene().isDataSlotAllocated(linkHandle));
    }

    TEST_F(AScene, canNotCreateMoreThanOneDataSlotForADataObject)
    {
        DataFloat* dataObject = m_scene.createDataFloat();
        EXPECT_EQ(StatusOK, m_scene.createDataProvider(*dataObject, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject, dataProviderId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject, dataConsumerId_t(3u)));
    }

    TEST_F(AScene, canNotCreateMoreThanOneDataSlotWithTheSameId)
    {
        DataFloat* dataObject1 = m_scene.createDataFloat();
        DataFloat* dataObject2 = m_scene.createDataFloat();
        EXPECT_EQ(StatusOK, m_scene.createDataProvider(*dataObject1, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject2, dataProviderId_t(1u)));
        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject2, dataConsumerId_t(1u)));

        EXPECT_EQ(StatusOK, m_scene.createDataConsumer(*dataObject2, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createDataConsumer(*dataObject1, dataConsumerId_t(2u)));
        EXPECT_NE(StatusOK, m_scene.createDataProvider(*dataObject1, dataProviderId_t(2u)));
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureProviderWithTextureFromAnotherClient)
    {
        RamsesFramework anotherFramework;
        RamsesClient anotherClient("", anotherFramework);
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        const Texture2D* texture = anotherClient.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        EXPECT_NE(StatusOK, m_scene.createTextureProvider(*texture, 1u));
    }

    TEST_F(AScene, reportsErrorWhenCreateTextureConsumerWithSamplerFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(12u);
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        const TextureSampler* sampler = anotherScene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(NULL != sampler);

        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, 1u));

        client.destroy(anotherScene);
    }

    TEST_F(AScene, canCreateATextureProviderDataSlot)
    {
        EXPECT_EQ(0u, m_scene.impl.getIScene().getDataSlotCount());

        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        EXPECT_EQ(StatusOK, m_scene.createTextureProvider(*texture, 666u));

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
        Texture2D* texture1 = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData1, false);
        ASSERT_TRUE(NULL != texture1);

        uint8_t data2 = 1u;
        MipLevelData mipData2(1u, &data2);
        Texture2D* texture2 = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData2, false);
        ASSERT_TRUE(NULL != texture2);

        EXPECT_EQ(StatusOK, m_scene.createTextureProvider(*texture1, 666u));
        EXPECT_EQ(StatusOK, m_scene.updateTextureProvider(*texture2, 666u));

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
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        const TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(NULL != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, 666u));

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
        Texture3D* texture = client.createTexture3D(1u, 1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        const TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(NULL != sampler);

        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, 666u));
    }

    TEST_F(AScene, removesDataSlotsOfTextureSamplerOnDestruction)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(NULL != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, 666u));

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
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(NULL != sampler);

        EXPECT_EQ(StatusOK, m_scene.createTextureConsumer(*sampler, 666u));
        EXPECT_NE(StatusOK, m_scene.createTextureConsumer(*sampler, 667u));
    }

    TEST_F(AScene, canNotCreateMoreThanOneProviderForATexture)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        EXPECT_EQ(StatusOK, m_scene.createTextureProvider(*texture, 666u));
        EXPECT_NE(StatusOK, m_scene.createTextureProvider(*texture, 667u));
    }

    TEST_F(AScene, canNotCreateMoreThanOneTextureConsumerOrProviderWithTheSameId)
    {
        uint8_t data = 0u;
        MipLevelData mipData(1u, &data);
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);
        Texture2D* texture2 = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture2);

        TextureSampler* sampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(NULL != sampler);
        TextureSampler* sampler2 = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture2);
        ASSERT_TRUE(NULL != sampler2);

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
        Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_R8, 1u, &mipData, false);
        ASSERT_TRUE(NULL != texture);

        EXPECT_NE(StatusOK, m_scene.updateTextureProvider(*texture, dataProviderId_t(1u)));
    }

    TEST_F(AScene, canCreateStreamTextureWithFallbackTextureAndStreamSource)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const streamSource_t source(1);
        StreamTexture* streamTexture = this->m_scene.createStreamTexture(texture2D, source, "testStreamTexture");

        ASSERT_NE(static_cast<StreamTexture*>(0), streamTexture);
        EXPECT_EQ(source, streamTexture->impl.getStreamSource());
        EXPECT_EQ(texture2D.impl.getLowlevelResourceHash(), streamTexture->impl.getFallbackTextureHash());
    }


    TEST_F(AScene, cannotCreateStreamTextureWithFallbackTextureFromDifferentClient)
    {
        const uint8_t data[4 * 10 * 12] = {};
        MipLevelData mipLevelData(sizeof(data), data);

        RamsesClient anotherClient("anotherLocalTestClient", framework);
        Texture2D* anotherTexture = anotherClient.createTexture2D(10, 12, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "name");
        ASSERT_TRUE(NULL != anotherTexture);

        StreamTexture* streamTexture = this->m_scene.createStreamTexture(*anotherTexture, streamSource_t(0), "StreamTexture");
        EXPECT_TRUE(NULL == streamTexture);
    }

    TEST_F(AScene, canCreateTextureSamplerForTexture2DWithDefaultAnisotropyLevel)
    {
        const Texture2D& texture2D = createObject<Texture2D>("testTexture2D");
        const TextureSampler* sampler = this->m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, texture2D);

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
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

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
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

        ASSERT_NE(static_cast<TextureSampler*>(0), sampler);
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
        EXPECT_TRUE(NULL == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerForTextureCubeWithWrongAnisotropyValue)
    {
        const TextureCube& textureCube = createObject<TextureCube>("testTextureCube");
        const TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, textureCube, 0);
        EXPECT_TRUE(NULL == textureSampler);
    }

    TEST_F(AScene, cantCreateTextureSamplerForRenderBufferWithWrongAnisotropyValue)
    {
        const RenderBuffer& renderBuffer = createObject<RenderBuffer>("renderBuffer");
        const TextureSampler* textureSampler = m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, renderBuffer, 0);
        EXPECT_TRUE(NULL == textureSampler);
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
        EXPECT_TRUE(NULL != blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceRenderBufferFromDifferentScene)
    {
        Scene& anotherScene = *client.createScene(12u);
        const RenderBuffer* sourceRenderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(NULL != sourceRenderBuffer);

        const RenderBuffer& destinationRenderBuffer = createObject<RenderBuffer>("dst renderBuffer");
        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(NULL == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBufferFromDifferentScenes)
    {
        Scene& anotherScene = *client.createScene(12u);
        const RenderBuffer* destinationRenderBuffer = anotherScene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(NULL != destinationRenderBuffer);

        const RenderBuffer& sourceRenderBuffer = createObject<RenderBuffer>("src renderBuffer");
        const BlitPass* blitPass = m_scene.createBlitPass(sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(NULL == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentType)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_R8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Depth, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(NULL != sourceRenderBuffer);
        ASSERT_TRUE(NULL != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(NULL == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentFormat)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_R8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(NULL != sourceRenderBuffer);
        ASSERT_TRUE(NULL != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(NULL == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavinghDifferentWidth)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(1u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(NULL != sourceRenderBuffer);
        ASSERT_TRUE(NULL != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(NULL == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSourceAndDestinationRenderBuffersHavingDifferentHeight)
    {
        const RenderBuffer* sourceRenderBuffer = m_scene.createRenderBuffer(100u, 1u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        const RenderBuffer* destinationRenderBuffer = m_scene.createRenderBuffer(100u, 100u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(NULL != sourceRenderBuffer);
        ASSERT_TRUE(NULL != destinationRenderBuffer);

        const BlitPass* blitPass = m_scene.createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, "blitpass");
        EXPECT_TRUE(NULL == blitPass);
    }

    TEST_F(AScene, cannotCreateBlitPass_WithSameSourceAndDestinationRenderBuffers)
    {
        const RenderBuffer* rb = m_scene.createRenderBuffer(100u, 1u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(nullptr != rb);
        const BlitPass* blitPass = m_scene.createBlitPass(*rb, *rb, "blitpass");
        EXPECT_TRUE(nullptr == blitPass);
    }

    TEST_F(AScene, canCreateIndexDataBufferWithIntegralTypes)
    {
        IndexDataBuffer* const indexDataBuffer16 = m_scene.createIndexDataBuffer(4, ramses::EDataType_UInt16);
        ASSERT_NE(nullptr, indexDataBuffer16);
        IndexDataBuffer* const indexDataBuffer32 = m_scene.createIndexDataBuffer(4, ramses::EDataType_UInt32);
        ASSERT_NE(nullptr, indexDataBuffer32);

        m_scene.destroy(*indexDataBuffer16);
        m_scene.destroy(*indexDataBuffer32);
    }

    TEST_F(AScene, cannotCreateIndexDataBufferWithNonIntegralType)
    {
        EXPECT_EQ(nullptr, m_scene.createIndexDataBuffer(4, ramses::EDataType_Float));
        EXPECT_EQ(nullptr, m_scene.createIndexDataBuffer(4, ramses::EDataType_Vector2F));
        EXPECT_EQ(nullptr, m_scene.createIndexDataBuffer(4, ramses::EDataType_Vector3F));
        EXPECT_EQ(nullptr, m_scene.createIndexDataBuffer(4, ramses::EDataType_Vector4F));
    }

    TEST_F(AScene, canCreateVertexDataBufferWithFloatTypes)
    {
        VertexDataBuffer* const vertexDataBufferFloat = m_scene.createVertexDataBuffer(4, ramses::EDataType_Float);
        ASSERT_NE(nullptr, vertexDataBufferFloat);
        VertexDataBuffer* const vertexDataBufferVec2 = m_scene.createVertexDataBuffer(4, ramses::EDataType_Vector2F);
        ASSERT_NE(nullptr, vertexDataBufferVec2);
        VertexDataBuffer* const vertexDataBufferVec3 = m_scene.createVertexDataBuffer(4, ramses::EDataType_Vector3F);
        ASSERT_NE(nullptr, vertexDataBufferVec3);
        VertexDataBuffer* const vertexDataBufferVec4 = m_scene.createVertexDataBuffer(4, ramses::EDataType_Vector4F);
        ASSERT_NE(nullptr, vertexDataBufferVec4);

        m_scene.destroy(*vertexDataBufferFloat);
        m_scene.destroy(*vertexDataBufferVec2);
        m_scene.destroy(*vertexDataBufferVec3);
        m_scene.destroy(*vertexDataBufferVec4);
    }

    TEST_F(AScene, cannotCreateVertexDataBufferWithNonFloatType)
    {
        EXPECT_EQ(nullptr, m_scene.createVertexDataBuffer(4, ramses::EDataType_UInt16));
        EXPECT_EQ(nullptr, m_scene.createVertexDataBuffer(4, ramses::EDataType_UInt32));
    }

    TEST_F(AScene, flushIncreasesStatisticCounter)
    {
        EXPECT_EQ(0u, m_scene.impl.getStatisticCollection().statFlushesTriggered.getCounterValue());
        m_scene.flush();
        EXPECT_EQ(1u, m_scene.impl.getStatisticCollection().statFlushesTriggered.getCounterValue());
    }
}
