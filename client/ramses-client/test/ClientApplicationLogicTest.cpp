//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "framework_common_gmock_header.h"
#include "ClientApplicationLogic.h"
#include "ComponentMocks.h"
#include "ResourceMock.h"
#include "DummyResource.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Scene/ClientScene.h"
#include "Components/ResourceComponent.h"
#include "Components/SceneGraphComponent.h"
#include "Resource/TextureResource.h"
#include "TransportCommon/FakeConnectionSystem.h"
#include "TransportCommon/FakeConnectionStatusUpdateNotifier.h"
#include "Components/FileInputStreamContainer.h"

using namespace ramses_internal;

class AClientApplicationLogic : public ::testing::Test
{
public:
    AClientApplicationLogic()
        : dummyGuid(555)
        , logic(dummyGuid, frameworkLock)
        , sceneId(44u)
        , dummyScene(SceneInfo(sceneId))
    {
        logic.init(resourceComponent, scenegraphProviderComponent);
    }

protected:
    Guid dummyGuid;
    StrictMock<ResourceProviderComponentMock> resourceComponent;
    StrictMock<SceneGraphProviderComponentMock> scenegraphProviderComponent;
    PlatformLock frameworkLock;
    ClientApplicationLogic logic;

    SceneId sceneId;
    ClientScene dummyScene;

    void createDummyScene()
    {
        EXPECT_CALL(scenegraphProviderComponent, handleCreateScene(Ref(dummyScene), false, Ref(logic)));
        logic.createScene(dummyScene, false);
    }
};

TEST_F(AClientApplicationLogic, sceneDistributionIsDisabledInitially)
{
    createDummyScene();
    EXPECT_FALSE(logic.isScenePublished(sceneId));
}

TEST_F(AClientApplicationLogic, canEnableLocalOnlySceneDistribution)
{
    createDummyScene();

    EXPECT_CALL(scenegraphProviderComponent, handlePublishScene(sceneId, EScenePublicationMode_LocalOnly));
    logic.publishScene(sceneId, EScenePublicationMode_LocalOnly);

    EXPECT_TRUE(logic.isScenePublished(sceneId));
}

TEST_F(AClientApplicationLogic, canEnableSceneDistribution)
{
    createDummyScene();
    EXPECT_CALL(scenegraphProviderComponent, handlePublishScene(sceneId, EScenePublicationMode_LocalAndRemote));
    logic.publishScene(sceneId, EScenePublicationMode_LocalAndRemote);

    EXPECT_TRUE(logic.isScenePublished(sceneId));
}

TEST_F(AClientApplicationLogic, canDisableLocalOnlySceneDistribution)
{
    createDummyScene();
    EXPECT_CALL(scenegraphProviderComponent, handlePublishScene(sceneId, EScenePublicationMode_LocalOnly));
    logic.publishScene(sceneId, EScenePublicationMode_LocalOnly);

    EXPECT_CALL(scenegraphProviderComponent, handleUnpublishScene(sceneId));
    logic.unpublishScene(sceneId);

    EXPECT_FALSE(logic.isScenePublished(sceneId));
}

TEST_F(AClientApplicationLogic, canDisableSceneDistribution)
{
    createDummyScene();
    EXPECT_CALL(scenegraphProviderComponent, handlePublishScene(sceneId, EScenePublicationMode_LocalAndRemote));
    logic.publishScene(sceneId, EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(scenegraphProviderComponent, handleUnpublishScene(sceneId));
    logic.unpublishScene(sceneId);

    EXPECT_FALSE(logic.isScenePublished(sceneId));
}

TEST_F(AClientApplicationLogic, forwardsAddingOfResourceFileToResourceComponent)
{
    const ResourceInfo resourceInfo(EResourceType_VertexArray, ResourceContentHash(44u, 0), 2u, 1u);
    ResourceTableOfContents resourceToc;
    resourceToc.registerContents(resourceInfo, 0u, 1u);
    const String fileName("resourceFile");
    InputStreamContainerSPtr resourceFileStream(std::make_shared<FileInputStreamContainer>(fileName));

    EXPECT_CALL(resourceComponent, addResourceFile(resourceFileStream, Ref(resourceToc)));
    logic.addResourceFile(resourceFileStream, resourceToc);
}

TEST_F(AClientApplicationLogic, triesToGetRequestedResourceFromResourceComponent)
{
    const ResourceContentHash dummyResourceHash(44u, 0);
    ON_CALL(resourceComponent, getResource(_)).WillByDefault(Return(ManagedResource()));
    EXPECT_CALL(resourceComponent, getResource(dummyResourceHash));
    logic.getResource(dummyResourceHash);
}

TEST_F(AClientApplicationLogic, triesToGetHashUsageFromResourceComponent)
{
    const ResourceContentHash dummyResourceHash(44u, 0);
    ON_CALL(resourceComponent, getResourceHashUsage(_)).WillByDefault(Return(ResourceHashUsage()));
    EXPECT_CALL(resourceComponent, getResourceHashUsage(dummyResourceHash));
    logic.getHashUsage(dummyResourceHash);
}

TEST_F(AClientApplicationLogic, addsAndRemovesResourceFilesFromComponent)
{
    InputStreamContainerSPtr resourceFileInputStream;
    ResourceTableOfContents toc;

    EXPECT_CALL(resourceComponent, addResourceFile(_,_));
    const auto handle = logic.addResourceFile(resourceFileInputStream, toc);

    EXPECT_CALL(resourceComponent, removeResourceFile(handle));
    logic.removeResourceFile(handle);
}

TEST_F(AClientApplicationLogic, gathersSceneReferenceEventsInAContainer)
{
    SceneReferenceEvent event(SceneId { 123 });
    event.referencedScene = SceneId{ 123456789 };

    EXPECT_TRUE(logic.popSceneReferenceEvents().empty());
    logic.handleSceneReferenceEvent(event, Guid{});
    const auto result = logic.popSceneReferenceEvents();
    EXPECT_EQ(result.size(), 1u);
    const auto sre = result.front();
    EXPECT_EQ(sre.referencedScene, event.referencedScene);

    logic.handleSceneReferenceEvent(event, Guid{});
    logic.handleSceneReferenceEvent(event, Guid{});
    logic.handleSceneReferenceEvent(event, Guid{});
    logic.handleSceneReferenceEvent(event, Guid{});
    EXPECT_EQ(logic.popSceneReferenceEvents().size(), 4u);
}


TEST_F(AClientApplicationLogic, incomingResourceAvailabilityEventDoesNoHarm)
{
    ResourceAvailabilityEvent event;
    event.sceneid = SceneId { 123 };

    logic.handleResourceAvailabilityEvent(event, Guid {});
}

TEST_F(AClientApplicationLogic, returnsReturnValueFromComponentOnFlush)
{
    EXPECT_CALL(scenegraphProviderComponent, handleFlush(_, _, _)).WillOnce(Return(true));
    EXPECT_TRUE(logic.flush(sceneId, {}, {}));

    EXPECT_CALL(scenegraphProviderComponent, handleFlush(_, _, _)).WillOnce(Return(false));
    EXPECT_FALSE(logic.flush(sceneId, {}, {}));
}

class AClientApplicationLogicWithRealComponents : public ::testing::Test
{
public:
    AClientApplicationLogicWithRealComponents()
        : resComp(stats, fwlock)
        , sceneComp(clientId, commSystem, connStatusUpdateNotifier, resComp, fwlock, ramses::EFeatureLevel_Latest)
        , logic(clientId, fwlock)
    {
        logic.init(resComp, sceneComp);
    }

protected:
    const SceneId sceneId = SceneId(1u);
    const Guid clientId = Guid(1);
    const Guid renderer1 = Guid(2);
    const Guid renderer2 = Guid(3);

    PlatformLock fwlock;
    StatisticCollectionFramework stats;
    ResourceComponent resComp;
    FakeConnectionSystem commSystem;
    FakeConnectionStatusUpdateNotifier connStatusUpdateNotifier;
    SceneGraphComponent sceneComp;

    ClientApplicationLogic logic;
};

TEST_F(AClientApplicationLogicWithRealComponents, keepsResourcesAliveForNewSubscriberForShadowCopyScene)
{
    ClientScene clientScene{ SceneInfo(sceneId) };
    logic.createScene(clientScene, false);
    logic.publishScene(sceneId, EScenePublicationMode_LocalAndRemote);
    auto res = new TextureResource(EResourceType_Texture2D, TextureMetaInfo(1u, 1u, 1u, ETextureFormat::R8, false, {}, { 1u }), ResourceCacheFlag_DoNotCache, String());
    res->setResourceData(ResourceBlob{ 1 }, { 1u, 1u });
    auto hash = res->getHash();
    {
        // simulate creation of texture2d
        auto manRes = logic.addResource(res);

        auto hashUsage = logic.getHashUsage(hash);

        // use texture2d
        auto stHandle = clientScene.allocateStreamTexture(WaylandIviSurfaceId{ 1u }, hash);

        sceneComp.handleSubscribeScene(sceneId, renderer1);
        EXPECT_TRUE(logic.flush(sceneId, {}, {}));

        // release stream texture along with its texture2d (by leaving scope)
        clientScene.releaseStreamTexture(stHandle);
    }

    EXPECT_EQ(resComp.getResource(hash).get(), res);
    sceneComp.handleSubscribeScene(sceneId, renderer2);
}

TEST_F(AClientApplicationLogicWithRealComponents, keepsAlsoOldResourcesAliveForNewSubscriberForShadowCopyScene)
{
    ClientScene clientScene{ SceneInfo(sceneId) };
    logic.createScene(clientScene, false);
    logic.publishScene(sceneId, EScenePublicationMode_LocalAndRemote);
    auto res = new TextureResource(EResourceType_Texture2D, TextureMetaInfo(1u, 1u, 1u, ETextureFormat::R8, false, {}, { 1u }), ResourceCacheFlag_DoNotCache, String());
    res->setResourceData(ResourceBlob{ 1 }, { 1u, 1u });
    auto res2 = new TextureResource(EResourceType_Texture2D, TextureMetaInfo(2u, 2u, 1u, ETextureFormat::R8, true, {}, { 4u }), ResourceCacheFlag_DoNotCache, String());
    res2->setResourceData(ResourceBlob{ 2 }, { 2u, 2u });
    auto hash = res->getHash();
    auto hash2 = res2->getHash();
    {
        // simulate creation of texture2d
        auto manRes = logic.addResource(res);
        auto hashUsage = logic.getHashUsage(hash);

        // use texture2d
        auto stHandle = clientScene.allocateStreamTexture(WaylandIviSurfaceId{ 1u }, hash);

        sceneComp.handleSubscribeScene(sceneId, renderer1);
        EXPECT_TRUE(logic.flush(sceneId, {}, {}));

        // another stream texture
        auto manRes2 = logic.addResource(res2);
        auto hashUsage2 = logic.getHashUsage(hash2);

        // use texture2d
        clientScene.allocateStreamTexture(WaylandIviSurfaceId{ 2u }, hash2);

        EXPECT_TRUE(logic.flush(sceneId, {}, {}));

        // release first stream texture along with its texture2d (by leaving scope)
        clientScene.releaseStreamTexture(stHandle);
    }

    EXPECT_EQ(resComp.getResource(hash).get(), res);
    EXPECT_EQ(resComp.getResource(hash2).get(), res2);
    sceneComp.handleSubscribeScene(sceneId, renderer2);
}
