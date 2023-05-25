//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTTESTUTILS_H
#define RAMSES_CLIENTTESTUTILS_H

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"

#include "CreationHelper.h"
#include "MockActionCollector.h"
#include "RamsesFrameworkImpl.h"
#include "RamsesFrameworkConfigImpl.h"
#include "MeshNodeImpl.h"
#include "SceneImpl.h"
#include "Scene/ClientScene.h"
#include "RamsesClientImpl.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/EffectDescription.h"
#include "TestEffects.h"

#include <string_view>

namespace ramses_internal
{
    class ClientScene;
}

namespace ramses
{
    class LocalTestClient
    {
    public:
        LocalTestClient()
            : framework{ GetDefaultFrameworkConfig() }
            , client(*framework.createClient("localTestClient"))
            , m_creationHelper(nullptr, &client)
            , sceneActionsCollector()
        {
            sceneActionsCollector.init(framework.m_impl.getScenegraphComponent());
            framework.m_impl.getScenegraphComponent().setSceneRendererHandler(&sceneActionsCollector);
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
            static RamsesFrameworkConfig config;
            config.setLogLevel(ramses::ELogLevel::Off);
            config.setConnectionSystem(ramses::EConnectionSystem::Off);
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
        explicit LocalTestClientWithScene(const SceneConfig& sceneConfig = SceneConfig())
            : LocalTestClient()
            , m_scene(*client.createScene(sceneId_t(123u), sceneConfig))
            , m_constRefToScene(m_scene)
            , m_internalScene(m_scene.m_impl.getIScene())
        {
            m_creationHelper.setScene(&m_scene);
        }

        virtual ~LocalTestClientWithScene()
        {
            m_creationHelper.destroyAdditionalAllocatedSceneObjects();
            client.destroy(m_scene);
        }

        Scene& getScene()
        {
            return m_scene;
        }

        ramses_internal::ClientScene& getInternalScene()
        {
            return m_internalScene;
        }

        ramses::ArrayResource& createValidIndexArray()
        {
            return createObject<ArrayResource>("indices");
        }

        GeometryBinding& createValidGeometry(Effect* effect = nullptr, bool useIndices = true)
        {
            if (nullptr == effect)
            {
                effect = TestEffects::CreateTestEffect(m_scene);
            }
            GeometryBinding* geometry = m_scene.createGeometryBinding(*effect, "geometry");
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
            GeometryBinding& geometry = createValidGeometry();
            mesh->setGeometryBinding(geometry);
            return *mesh;
        }

        void setValidPerspectiveCameraParameters(PerspectiveCamera& camera)
        {
            camera.setFrustum(-0.01f, 0.01f, -0.01f, 0.01f, 0.1f, 1000.f);
            camera.setViewport(0u, 0u, 16u, 32u);
        }

    protected:
        Scene& m_scene;
        const Scene& m_constRefToScene;
        ramses_internal::ClientScene& m_internalScene;
    };

    class ClientTestUtils
    {
    public:
        static bool CompareBinaryFiles(char const* fileName1, char const* fileName2)
        {
            using namespace ramses_internal;
            File file1(fileName1);
            EXPECT_TRUE(file1.open(File::Mode::ReadOnlyBinary));
            File file2(fileName2);
            EXPECT_TRUE(file2.open(File::Mode::ReadOnlyBinary));

            if (!file1.isOpen() || !file2.isOpen())
            {
                return false;
            }

            UInt length1 = 0u;
            UInt length2 = 0u;
            EXPECT_TRUE(file1.getSizeInBytes(length1));
            EXPECT_TRUE(file2.getSizeInBytes(length2));
            if (length1 != length2)
            {
                return false;
            }

            const UInt bufferLength = 1024 * 1024;
            std::vector<char> buf1(bufferLength);
            std::vector<char> buf2(bufferLength);

            UInt offset = 0;
            UInt dummy = 0;
            bool equal = true;
            while (offset < length1 && equal)
            {
                UInt left = std::min(length1 - offset, bufferLength);
                EXPECT_EQ(EStatus::Ok, file1.read(buf1.data(), left, dummy));
                EXPECT_EQ(EStatus::Ok, file2.read(buf2.data(), left, dummy));
                equal = (PlatformMemory::Compare(buf1.data(), buf2.data(), left) == 0);
                offset += left;
            }

            return equal;
        }
    };
}

#endif
