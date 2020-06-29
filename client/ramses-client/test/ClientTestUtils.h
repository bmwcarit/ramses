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
#include "ramses-client-api/AnimationSystem.h"

#include "CreationHelper.h"
#include "MockActionCollector.h"
#include "RamsesFrameworkImpl.h"
#include "SceneImpl.h"
#include "Scene/ClientScene.h"
#include "RamsesClientImpl.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/EffectDescription.h"
#include "TestEffects.h"

namespace ramses_internal
{
    class ClientScene;
}

namespace ramses
{
    //disabling the periodic logs is important as otherwise race conditions can occur in the tests that check
    //that the statistic counters are updated (periodic logger resets the counters)
    static const char* clientArgs[] = { "LocalTestClient", "-l", "0", "-fakeConnection", "-disablePeriodicLogs" };

    class LocalTestClient
    {
    public:
        LocalTestClient()
            : framework(sizeof(clientArgs) / sizeof(char*), clientArgs)
            , client(*framework.createClient("localTestClient"))
            , m_creationHelper(nullptr, nullptr, &client)
            , sceneActionsCollector()
        {
            sceneActionsCollector.init(framework.impl.getScenegraphComponent());
            framework.impl.getScenegraphComponent().setSceneRendererServiceHandler(&sceneActionsCollector);
        }

        template <typename ObjectType>
        ObjectType& createObject(const char* name = nullptr)
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
            , m_internalScene(m_scene.impl.getIScene())
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

        const ramses::UInt16Array& createValidIndexArray()
        {
            return createObject<UInt16Array>("indices");
        }

        GeometryBinding& createValidGeometry(Effect* effect = nullptr, bool useIndices = true)
        {
            if (nullptr == effect)
            {
                effect = TestEffects::CreateTestEffect(client);
            }
            GeometryBinding* geometry = m_scene.createGeometryBinding(*effect, "geometry");
            EXPECT_TRUE(geometry != nullptr);
            if (useIndices)
            {
                const UInt16Array& indexArray = createValidIndexArray();
                geometry->setIndices(indexArray);
            }
            return *geometry;
        }

        MeshNode& createValidMeshNode()
        {
            MeshNode* mesh = m_scene.createMeshNode();
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");
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

    class LocalTestClientWithSceneAndAnimationSystem : public LocalTestClientWithScene
    {
    protected:
        explicit LocalTestClientWithSceneAndAnimationSystem(uint32_t animationSystemCreationFlags = EAnimationSystemFlags_Default)
            : LocalTestClientWithScene()
            , animationSystem(*m_scene.createAnimationSystem(animationSystemCreationFlags, "animation system"))
        {
            m_creationHelper.setAnimationSystem(&animationSystem);
        }

        ~LocalTestClientWithSceneAndAnimationSystem()
        {
            m_creationHelper.destroyAdditionalAllocatedAnimationSystemObjects();
            m_scene.destroy(animationSystem);
        }

        AnimationSystem& animationSystem;
    };
}

#endif
