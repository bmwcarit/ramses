//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-utils.h"

#include "PlatformAbstraction/PlatformThread.h"
#include "TestEffects.h"

namespace ramses
{
    using namespace testing;
    using namespace ramses_internal;

    namespace
    {
        class MockClientEventHandler : public IClientEventHandler
        {
        public:
            MOCK_METHOD1(resourceFileLoadFailed, void(const char* filename));
            MOCK_METHOD1(resourceFileLoadSucceeded, void(const char* filename));
            MOCK_METHOD1(sceneFileLoadFailed, void(const char* filename));
            MOCK_METHOD2(sceneFileLoadSucceeded, void(const char* filename, Scene* loadedScene));
        };

        class RamsesFileCreator
        {
        public:
            static void SaveSceneToFile(sceneId_t sceneId, const ramses_internal::String& sceneFilename, const ramses_internal::String& resourceFilename)
            {
                RamsesFramework framework;
                RamsesClient client("RamsesFileCreator", framework);

                ResourceFileDescription resources(resourceFilename.c_str());
                Scene* scene = CreateScene(client, resources, sceneId);
                ASSERT_TRUE(scene != NULL);

                ResourceFileDescriptionSet    resourceVector;
                resourceVector.add(resources);
                ASSERT_EQ(StatusOK, client.saveSceneToFile(*scene, sceneFilename.c_str(), resourceVector, false));
            }

        private:
            static Scene* CreateScene(RamsesClient& client, ResourceFileDescription& resourcesDescription, sceneId_t sceneId)
            {
                Scene* scene = client.createScene(sceneId, SceneConfig());

                PerspectiveCamera* camera = scene->createPerspectiveCamera("my cam");
                camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
                camera->setViewport(1, 2, 3, 4);

                RenderGroup* renderGroup   = scene->createRenderGroup("a rendergroup");
                renderGroup->addMeshNode(*scene->createMeshNode(), 3);

                // stream texture
                uint8_t data[4] = { 0u };
                MipLevelData mipLevelData(sizeof(data), data);
                Texture2D* fallbackTexture   = client.createTexture2D(1u, 1u, ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "fallbackTexture");
                scene->createStreamTexture(*fallbackTexture, streamSource_t(3), "resourceName");
                resourcesDescription.add(fallbackTexture);

                //appearance
                Effect* effect = TestEffects::CreateTestEffect(client);
                resourcesDescription.add(effect);
                scene->createAppearance(*effect, "appearance");

                // geometry binding
                static const uint16_t inds[3] = { 0, 1, 2 };
                const UInt16Array* const indices = client.createConstUInt16Array(3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
                resourcesDescription.add(indices);

                GeometryBinding* geometry = scene->createGeometryBinding(*effect, "geometry");
                assert(geometry != NULL);
                EXPECT_EQ(StatusOK, geometry->setIndices(*indices));

                //mesh node
                scene->createMeshNode("a meshnode");

                return scene;
            }
        };
    }

    static const char* sceneFile = "ARamsesFileLoadedInSeveralThread_1.ramscene";
    static const char* otherSceneFile = "ARamsesFileLoadedInSeveralThread_2.ramscene";
    static const char* nonexistingSceneFile = "this_file_should_really_not_exist.ramscene";

    static const char* resFile = "ARamsesFileLoadedInSeveralThread_1.ramres";
    static const char* otherResFile = "ARamsesFileLoadedInSeveralThread_2.ramres";
    static const char* nonexistingResFile = "this_file_should_really_not_exist.ramres";

    class ARamsesFileLoadedInSeveralThread : public ::testing::Test
    {
    public:
        ARamsesFileLoadedInSeveralThread()
            : framework()
            , client("ARamsesFileLoadedInSeveralThread", framework)
            , numClientEvents(0)
            , loadedScene(nullptr)
        {
            ON_CALL(eventHandler, resourceFileLoadSucceeded(_)).WillByDefault(InvokeWithoutArgs(this, &ARamsesFileLoadedInSeveralThread::incNumClientEvents));
            ON_CALL(eventHandler, resourceFileLoadFailed(_)).WillByDefault(InvokeWithoutArgs(this, &ARamsesFileLoadedInSeveralThread::incNumClientEvents));
            ON_CALL(eventHandler, sceneFileLoadSucceeded(_, _)).WillByDefault(DoAll(SaveArg<1>(&loadedScene), InvokeWithoutArgs(this, &ARamsesFileLoadedInSeveralThread::incNumClientEvents)));
            ON_CALL(eventHandler, sceneFileLoadFailed(_)).WillByDefault(InvokeWithoutArgs(this, &ARamsesFileLoadedInSeveralThread::incNumClientEvents));
        }

        void incNumClientEvents()
        {
            ++numClientEvents;
        }

        bool waitForNumClientEvents(int num, UInt32 timeoutMs = 60000)
        {
            const UInt32 sleepMs = 50;
            for (UInt32 cnt = 0; cnt < (timeoutMs / sleepMs); ++cnt)
            {
                client.dispatchEvents(eventHandler);
                if (numClientEvents >= num)
                {
                    break;
                }
                PlatformThread::Sleep(sleepMs);
            }
            return numClientEvents == num;
        }

        RamsesFramework framework;
        RamsesClient client;
        StrictMock<MockClientEventHandler> eventHandler;
        int numClientEvents;
        Scene* loadedScene;

        static void SetUpTestCase()
        {
            RamsesFileCreator::SaveSceneToFile(123u, sceneFile, resFile);
            RamsesFileCreator::SaveSceneToFile(124u, otherSceneFile, otherResFile);
        }

        static void TearDownTestCase()
        {
            File(sceneFile).remove();
            File(otherSceneFile).remove();
            File(resFile).remove();
            File(otherResFile).remove();
        }
    };

    TEST_F(ARamsesFileLoadedInSeveralThread, loadingANonExistingResourceFileAsyncTriggersLoadFailedEvent)
    {
        EXPECT_CALL(eventHandler, resourceFileLoadFailed(StrEq(nonexistingResFile)));
        EXPECT_EQ(StatusOK, client.loadResourcesAsync(ResourceFileDescription(nonexistingResFile)));
        EXPECT_TRUE(waitForNumClientEvents(1));
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadResourceFile)
    {
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_EQ(StatusOK, client.loadResourcesAsync(ResourceFileDescription(resFile)));
        EXPECT_TRUE(waitForNumClientEvents(1));
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadMultipleResourceFiles)
    {
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(otherResFile)));

        ResourceFileDescriptionSet resources;
        resources.add(ResourceFileDescription(resFile));
        resources.add(ResourceFileDescription(otherResFile));

        EXPECT_EQ(StatusOK, client.loadResourcesAsync(resources));

        EXPECT_TRUE(waitForNumClientEvents(2));
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadResourcesParallelToSynchronousResourceLoad)
    {
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_EQ(StatusOK, client.loadResourcesAsync(ResourceFileDescription(resFile)));
        EXPECT_EQ(StatusOK, client.loadResources(ResourceFileDescription(otherResFile)));

        EXPECT_TRUE(waitForNumClientEvents(1));
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, loadingANonExistingSceneFileAsyncTriggersLoadFailedEvent)
    {
        ResourceFileDescriptionSet resources;
        resources.add(ResourceFileDescription(resFile));

        InSequence seq;
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_CALL(eventHandler, sceneFileLoadFailed(StrEq(nonexistingSceneFile)));
        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(nonexistingSceneFile, resources));
        EXPECT_TRUE(waitForNumClientEvents(2));
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, loadingASceneFileWithNonExistingResourceFileAsyncTriggersLoadFailedEvent)
    {
        ResourceFileDescriptionSet resources;
        resources.add(ResourceFileDescription(resFile));
        resources.add(ResourceFileDescription(nonexistingResFile));

        InSequence seq;
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_CALL(eventHandler, resourceFileLoadFailed(StrEq(nonexistingResFile)));
        EXPECT_CALL(eventHandler, sceneFileLoadFailed(StrEq(sceneFile)));
        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(sceneFile, resources));
        EXPECT_TRUE(waitForNumClientEvents(3));
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadSceneFile)
    {
        ResourceFileDescriptionSet resources;
        resources.add(ResourceFileDescription(resFile));

        InSequence seq;
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_CALL(eventHandler, sceneFileLoadSucceeded(StrEq(sceneFile), _));
        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(sceneFile, resources));
        ASSERT_TRUE(waitForNumClientEvents(2));

        ASSERT_TRUE(loadedScene != nullptr);
        EXPECT_EQ(123u, loadedScene->getSceneId());
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadSceneParallelToSynchronousResourceLoad)
    {
        ResourceFileDescriptionSet resources;
        resources.add(ResourceFileDescription(resFile));

        InSequence seq;
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_CALL(eventHandler, sceneFileLoadSucceeded(StrEq(sceneFile), _));
        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(sceneFile, resources));
        EXPECT_EQ(StatusOK, client.loadResources(ResourceFileDescription(otherResFile)));

        ASSERT_TRUE(waitForNumClientEvents(2));

        ASSERT_TRUE(loadedScene != nullptr);
        EXPECT_EQ(123u, loadedScene->getSceneId());
    }


    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadSceneParallelToSynchronousSceneLoad)
    {
        ResourceFileDescriptionSet resourcesForAsync;
        resourcesForAsync.add(ResourceFileDescription(resFile));

        ResourceFileDescriptionSet resourcesForSync;
        resourcesForSync.add(ResourceFileDescription(otherResFile));

        InSequence seq;
        EXPECT_CALL(eventHandler, resourceFileLoadSucceeded(StrEq(resFile)));
        EXPECT_CALL(eventHandler, sceneFileLoadSucceeded(StrEq(sceneFile), _));
        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(sceneFile, resourcesForAsync));

        Scene* syncScene = client.loadSceneFromFile(otherSceneFile, resourcesForSync);
        ASSERT_TRUE(syncScene != nullptr);
        EXPECT_EQ(124u, syncScene->getSceneId());

        ASSERT_TRUE(waitForNumClientEvents(2));

        ASSERT_TRUE(loadedScene != nullptr);
        EXPECT_EQ(123u, loadedScene->getSceneId());
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, asyncLoadSceneFileWithoutEverCallingDispatchDoesNotLeakMemory)
    {
        ResourceFileDescriptionSet resources;
        resources.add(ResourceFileDescription(resFile));

        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(sceneFile, resources));
        // do nothing, scene load will finish before RamsesClient is destructed
    }
}
