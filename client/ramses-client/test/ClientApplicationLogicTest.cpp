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
    ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream(fileName));

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

TEST_F(AClientApplicationLogic, triesToGetResourcesFromResourceComponent)
{
    ON_CALL(resourceComponent, getResources()).WillByDefault(Return(ManagedResourceVector()));
    EXPECT_CALL(resourceComponent, getResources());
    logic.getResources();
}

TEST_F(AClientApplicationLogic, addsAndRemovesResourceFilesFromComponent)
{
    ResourceFileInputStreamSPtr resourceFileInputStream;
    ResourceTableOfContents toc;

    EXPECT_CALL(resourceComponent, addResourceFile(_,_));
    logic.addResourceFile(resourceFileInputStream, toc);

    EXPECT_CALL(resourceComponent, removeResourceFile(String("testfilename")));
    logic.removeResourceFile("testfilename");
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
