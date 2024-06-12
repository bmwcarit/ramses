//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Effect.h"
#include "ramses/client/ramses-utils.h"

#include "ClientEventHandlerMock.h"
#include "TestEffects.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Core/Utils/File.h"

#include <string_view>

namespace ramses::internal
{
    using namespace testing;

    namespace
    {
        class RamsesFileCreator
        {
        public:
            static void SaveSceneToFile(sceneId_t sceneId, std::string_view sceneFilename)
            {
                RamsesFramework framework{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
                RamsesClient& client(*framework.createClient("RamsesFileCreator"));

                Scene* scene = CreateScene(client, sceneId);
                ASSERT_TRUE(scene != nullptr);

                ASSERT_TRUE(scene->saveToFile(sceneFilename, {}));
            }

        private:
            static Scene* CreateScene(RamsesClient& client, sceneId_t sceneId)
            {
                Scene* scene = client.createScene(sceneId);

                PerspectiveCamera* camera = scene->createPerspectiveCamera("my cam");
                camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
                camera->setViewport(1, 2, 3, 4);

                ramses::RenderGroup* renderGroup   = scene->createRenderGroup("a rendergroup");
                renderGroup->addMeshNode(*scene->createMeshNode(), 3);

                //appearance
                Effect* effect = TestEffects::CreateTestEffect(*scene);
                scene->createAppearance(*effect, "appearance");

                // geometry binding
                static const uint16_t inds[3] = { 0, 1, 2 };
                ArrayResource* const indices = scene->createArrayResource(3u, inds, "indices");

                Geometry* geometry = scene->createGeometry(*effect, "geometry");
                assert(geometry != nullptr);
                EXPECT_TRUE(geometry->setIndices(*indices));

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
            : client(*framework.createClient("ARamsesFileLoadedInSeveralThread"))
        {
            ON_CALL(eventHandler, sceneFileLoadSucceeded(_, _)).WillByDefault(DoAll(SaveArg<1>(&loadedScene), InvokeWithoutArgs(this, &ARamsesFileLoadedInSeveralThread::incNumClientEvents)));
            ON_CALL(eventHandler, sceneFileLoadFailed(_)).WillByDefault(InvokeWithoutArgs(this, &ARamsesFileLoadedInSeveralThread::incNumClientEvents));
        }

        void incNumClientEvents()
        {
            ++numClientEvents;
        }

        bool waitForNumClientEvents(int num, uint32_t timeoutMs = 60000)
        {
            const uint32_t sleepMs = 50;
            for (uint32_t cnt = 0; cnt < (timeoutMs / sleepMs); ++cnt)
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

        RamsesFramework framework{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
        RamsesClient& client;
        StrictMock<ClientEventHandlerMock> eventHandler{};
        int numClientEvents{0};
        Scene* loadedScene{nullptr};

        static void SetUpTestSuite()
        {
            RamsesFileCreator::SaveSceneToFile(sceneId_t(123u), sceneFile);
            RamsesFileCreator::SaveSceneToFile(sceneId_t(124u), otherSceneFile);
        }

        static void TearDownTestSuite()
        {
            File(sceneFile).remove();
            File(otherSceneFile).remove();
        }
    };

    TEST_F(ARamsesFileLoadedInSeveralThread, canAsyncLoadSceneFile)
    {
        EXPECT_CALL(eventHandler, sceneFileLoadSucceeded(StrEq(sceneFile), _));
        EXPECT_TRUE(client.loadSceneFromFileAsync(sceneFile));
        ASSERT_TRUE(waitForNumClientEvents(1));

        ASSERT_TRUE(loadedScene != nullptr);
        EXPECT_EQ(sceneId_t(123u), loadedScene->getSceneId());
    }

    TEST_F(ARamsesFileLoadedInSeveralThread, asyncLoadSceneFileWithoutEverCallingDispatchDoesNotLeakMemory)
    {
        EXPECT_TRUE(client.loadSceneFromFileAsync(sceneFile));
        // do nothing, scene load will finish before RamsesClient is destructed
    }
}
