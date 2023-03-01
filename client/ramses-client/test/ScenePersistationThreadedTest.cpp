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
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Effect.h"
#include "ramses-utils.h"

#include "ClientEventHandlerMock.h"
#include "TestEffects.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/File.h"

namespace ramses
{
    using namespace testing;
    using namespace ramses_internal;

    namespace
    {
        class RamsesFileCreator
        {
        public:
            static void SaveSceneToFile(sceneId_t sceneId, const ramses_internal::String& sceneFilename)
            {
                RamsesFramework framework;
                RamsesClient& client(*framework.createClient("RamsesFileCreator"));

                Scene* scene = CreateScene(client, sceneId);
                ASSERT_TRUE(scene != nullptr);

                ASSERT_EQ(StatusOK, scene->saveToFile(sceneFilename.c_str(), false));
            }

        private:
            static Scene* CreateScene(RamsesClient& client, sceneId_t sceneId)
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
                Texture2D* fallbackTexture = scene->createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "fallbackTexture");
                scene->createStreamTexture(*fallbackTexture, waylandIviSurfaceId_t(3), "resourceName");

                //appearance
                Effect* effect = TestEffects::CreateTestEffect(*scene);
                scene->createAppearance(*effect, "appearance");

                // geometry binding
                static const uint16_t inds[3] = { 0, 1, 2 };
                ArrayResource* const indices = scene->createArrayResource(EDataType::UInt16, 3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");

                GeometryBinding* geometry = scene->createGeometryBinding(*effect, "geometry");
                assert(geometry != nullptr);
                EXPECT_EQ(StatusOK, geometry->setIndices(*indices));

                //mesh node
                scene->createMeshNode("a meshnode");

                return scene;
            }
        };
    }

    static const char* sceneFile = "ARamsesFileLoadedInSeveralThread_1.ramscene";
    static const char* otherSceneFile = "ARamsesFileLoadedInSeveralThread_2.ramscene";

    class ARamsesFileLoadedInSeveralThread : public ::testing::Test
    {
    public:
        ARamsesFileLoadedInSeveralThread()
            : framework()
            , client(*framework.createClient("ARamsesFileLoadedInSeveralThread"))
            , numClientEvents(0)
            , loadedScene(nullptr)
        {
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
        RamsesClient& client;
        StrictMock<ClientEventHandlerMock> eventHandler;
        int numClientEvents;
        Scene* loadedScene;

        static void SetUpTestCase()
        {
            RamsesFileCreator::SaveSceneToFile(sceneId_t(123u), sceneFile);
            RamsesFileCreator::SaveSceneToFile(sceneId_t(124u), otherSceneFile);
        }

        static void TearDownTestCase()
        {
            File(sceneFile).remove();
            File(otherSceneFile).remove();
        }
    };

    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadSceneFile)
    {
        EXPECT_CALL(eventHandler, sceneFileLoadSucceeded(StrEq(sceneFile), _));
        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(sceneFile));
        ASSERT_TRUE(waitForNumClientEvents(1));

        ASSERT_TRUE(loadedScene != nullptr);
        EXPECT_EQ(sceneId_t(123u), loadedScene->getSceneId());
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, asyncLoadSceneFileWithoutEverCallingDispatchDoesNotLeakMemory)
    {
        EXPECT_EQ(StatusOK, client.loadSceneFromFileAsync(sceneFile));
        // do nothing, scene load will finish before RamsesClient is destructed
    }
}
