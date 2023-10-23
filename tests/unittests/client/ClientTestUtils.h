//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"

#include "CreationHelper.h"
#include "MockActionCollector.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/RamsesFrameworkConfigImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/SceneImpl.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "impl/RamsesClientImpl.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/EffectDescription.h"
#include "TestEffects.h"

#include <string_view>

namespace ramses::internal
{
    class ClientScene;

    template <typename T>
    std::vector<T> SomeDataVector();

    class LocalTestClient
    {
    public:
        LocalTestClient()
            : framework{ GetDefaultFrameworkConfig() }
            , client(*framework.createClient("localTestClient"))
            , m_creationHelper(nullptr, &client)
        {
            EXPECT_EQ(EFeatureLevel_Latest, framework.getFeatureLevel());
            sceneActionsCollector.init(framework.impl().getScenegraphComponent());
            framework.impl().getScenegraphComponent().setSceneRendererHandler(&sceneActionsCollector);
        }

        template <typename ObjectType>
        ObjectType& createObject(std::string_view name = {})
        {
            return *m_creationHelper.createObjectOfType<ObjectType>(name);
        }

        RamsesClient& getClient()
        {
            return client;
        }

        RamsesFramework& getFramework()
        {
            return framework;
        }

        static const RamsesFrameworkConfig& GetDefaultFrameworkConfig()
        {
            //disabling the periodic logs is important as otherwise race conditions can occur in the tests that check
            //that the statistic counters are updated (periodic logger resets the counters)
            static RamsesFrameworkConfig config{EFeatureLevel_Latest};
            config.setLogLevel(ELogLevel::Off);
            config.setConnectionSystem(EConnectionSystem::Off);
            config.setPeriodicLogInterval(std::chrono::seconds(0));
            return config;
        }

    protected:
        RamsesFramework framework;
        RamsesClient& client;
        CreationHelper m_creationHelper;
        testing::StrictMock<MockActionCollector> sceneActionsCollector;
    };

    class LocalTestClientWithScene : public LocalTestClient
    {
    public:
        explicit LocalTestClientWithScene(EScenePublicationMode publicationMode = EScenePublicationMode::LocalOnly)
            : m_scene(CreateScene(client, publicationMode))
            , m_constRefToScene(m_scene)
            , m_internalScene(m_scene.impl().getIScene())
        {
            m_creationHelper.setScene(&m_scene);
        }

        virtual ~LocalTestClientWithScene()
        {
            m_creationHelper.destroyAdditionalAllocatedSceneObjects();
            client.destroy(m_scene);
        }

        static ramses::Scene& CreateScene(RamsesClient& client, EScenePublicationMode publicationMode)
        {
            SceneConfig config{sceneId_t(123u), publicationMode};
            return *client.createScene(config);
        }

        ramses::Scene& getScene()
        {
            return m_scene;
        }

        ClientScene& getInternalScene()
        {
            return m_internalScene;
        }

        ramses::ArrayResource& createValidIndexArray()
        {
            return createObject<ArrayResource>("indices");
        }

        Geometry& createValidGeometry(Effect* effect = nullptr, bool useIndices = true)
        {
            if (nullptr == effect)
            {
                effect = TestEffects::CreateTestEffect(m_scene);
            }
            Geometry* geometry = m_scene.createGeometry(*effect, "geometry");
            EXPECT_TRUE(geometry != nullptr);
            if (useIndices)
            {
                const ArrayResource& indexArray = createValidIndexArray();
                geometry->setIndices(indexArray);
            }
            return *geometry;
        }

        MeshNode& createValidMeshNode()
        {
            MeshNode* mesh = m_scene.createMeshNode();
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(m_scene), "appearance");
            mesh->setAppearance(*appearance);
            Geometry& geometry = createValidGeometry();
            mesh->setGeometry(geometry);
            return *mesh;
        }

        static void SetValidPerspectiveCameraParameters(PerspectiveCamera& camera)
        {
            camera.setFrustum(-0.01f, 0.01f, -0.01f, 0.01f, 0.1f, 1000.f);
            camera.setViewport(0u, 0u, 16u, 32u);
        }

    protected:
        ramses::Scene& m_scene;
        const ramses::Scene& m_constRefToScene;
        ClientScene& m_internalScene;
    };

    class ClientTestUtils
    {
    public:
        static bool CompareBinaryFiles(char const* fileName1, char const* fileName2)
        {
            using namespace ramses::internal;
            File file1(fileName1);
            EXPECT_TRUE(file1.open(File::Mode::ReadOnlyBinary));
            File file2(fileName2);
            EXPECT_TRUE(file2.open(File::Mode::ReadOnlyBinary));

            if (!file1.isOpen() || !file2.isOpen())
            {
                return false;
            }

            size_t length1 = 0u;
            size_t length2 = 0u;
            EXPECT_TRUE(file1.getSizeInBytes(length1));
            EXPECT_TRUE(file2.getSizeInBytes(length2));
            if (length1 != length2)
            {
                return false;
            }

            const size_t bufferLength = 1024 * 1024;
            std::vector<char> buf1(bufferLength);
            std::vector<char> buf2(bufferLength);

            size_t offset = 0;
            size_t dummy = 0;
            bool equal = true;
            while (offset < length1 && equal)
            {
                size_t left = std::min(length1 - offset, bufferLength);
                EXPECT_EQ(EStatus::Ok, file1.read(buf1.data(), left, dummy));
                EXPECT_EQ(EStatus::Ok, file2.read(buf2.data(), left, dummy));
                equal = (PlatformMemory::Compare(buf1.data(), buf2.data(), left) == 0);
                offset += left;
            }

            return equal;
        }
    };
}
