//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/RendererSceneState.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "RendererEventTestHandler.h"
#include "SceneAPI/SceneId.h"

#include "PlatformMock.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/PickableObject.h"

namespace ramses_internal
{
    class RendererSceneControlEventHandler final : public ramses::RendererSceneControlEventHandlerEmpty
    {
    public:
        void objectsPicked(ramses::sceneId_t, const ramses::pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override
        {
            for (size_t i = 0; i < pickedObjectsCount; ++i)
                m_pickedObjects.push_back(std::move(pickedObjects[i]));
        }

        void expectObjectsPicked(const std::vector<ramses::pickableObjectId_t>& expectedPickedObjs)
        {
            EXPECT_EQ(expectedPickedObjs, m_pickedObjects);
            m_pickedObjects.clear();
        }

        std::vector<ramses::pickableObjectId_t> m_pickedObjects;
    };

    using namespace testing;

    //RamsesRendererDispatchTest is already applying a hack to setup mock factory which is automatically inherited by the following class
    extern NiceMock<PlatformNiceMock>* gPlatformMock;

    class ARamsesRendererPicking : public ::testing::Test
    {
    public:
        void createDisplay()
        {
            m_displayId = m_renderer.createDisplay({});
            updateAndDispatch();
        }

        void updateAndDispatch()
        {
            EXPECT_EQ(ramses::StatusOK, m_renderer.flush());
            EXPECT_EQ(ramses::StatusOK, m_sceneControl.flush());
            EXPECT_EQ(ramses::StatusOK, m_renderer.doOneLoop());
            EXPECT_EQ(ramses::StatusOK, m_sceneControl.dispatchEvents(m_sceneControlEventHandler));
        }

    protected:
        ARamsesRendererPicking()
            : m_framework()
            , m_renderer(*m_framework.createRenderer(ramses::RendererConfig()))
            , m_client(*m_framework.createClient("client"))
            , m_sceneControl(*m_renderer.getSceneControlAPI())
            , m_scene(*m_client.createScene(m_sceneId))
        {
            createDisplay();

            m_scene.publish(ramses::EScenePublicationMode_LocalOnly);
            m_scene.flush();

            m_sceneControl.setSceneMapping(m_sceneId, m_displayId);
            m_sceneControl.setSceneState(m_sceneId, ramses::RendererSceneState::Rendered);
            //Update a few times to make sure scene is really in state Rendered
            for (size_t updateLoops = 0; updateLoops < 5; ++updateLoops)
                updateAndDispatch();
        }

        ramses::RamsesFramework m_framework;
        ramses::RamsesRenderer& m_renderer;
        ramses::RamsesClient& m_client;
        ramses::RendererSceneControl& m_sceneControl;

        ramses::displayId_t m_displayId;
        const ramses::sceneId_t m_sceneId{ 33u };
        ramses::Scene& m_scene;
        RendererSceneControlEventHandler m_sceneControlEventHandler;
    };

    TEST_F(ARamsesRendererPicking, generatesEventForObjectsPicked)
    {
        const ramses::pickableObjectId_t pickableId(4u);
        const std::array<ramses::vec3f, 3u> geometryData{ ramses::vec3f{-1.f, 0.f, -0.5f}, ramses::vec3f{0.f, 1.f, -0.5f}, ramses::vec3f{0.f, 0.f, -0.5f} };

        ramses::OrthographicCamera* orthographicCamera = m_scene.createOrthographicCamera("my orthographicCamera");
        orthographicCamera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
        orthographicCamera->setViewport(0, 0, 1280, 480);

        ramses::ArrayBuffer* pickableGeometryBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 3u, "geometryBuffer");
        pickableGeometryBuffer->updateData(0, 3, geometryData.data());

        ramses::PickableObject* pickableObject = m_scene.createPickableObject(*pickableGeometryBuffer, pickableId, "pickableObject");
        pickableObject->setCamera(*orthographicCamera);

        m_scene.flush();
        updateAndDispatch();

        m_sceneControl.handlePickEvent(m_sceneId, -0.375000f, 0.250000f);
        updateAndDispatch();
        m_sceneControlEventHandler.expectObjectsPicked({ pickableId });
    }

    TEST_F(ARamsesRendererPicking, generatesEventForObjectsPicked_regressionRoundingIssues)
    {
        const ramses::pickableObjectId_t    pickableId1(2u);
        const ramses::pickableObjectId_t    pickableId2(3u);
        const std::array<ramses::vec3f, 3u> geometryData{ramses::vec3f{-1.f, 0.f, 0.f}, ramses::vec3f{1.f, 0.f, 0.f}, ramses::vec3f{0.f, 1.f, 0.f}};

        ramses::PerspectiveCamera* perspectiveCamera = m_scene.createPerspectiveCamera("my perspectiveCamera");
        perspectiveCamera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 100.f);
        perspectiveCamera->setViewport(0, 0, 1280, 480);
        perspectiveCamera->translate({-4.f, 0.f, 11.f});
        perspectiveCamera->setRotation({0.f, -40.f, 0.f}, ramses::ERotationType::Euler_XYZ);

        ramses::ArrayBuffer* pickableGeometryBuffer = m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 3u, "geometryBuffer");
        pickableGeometryBuffer->updateData(0, 3, geometryData.data());

        ramses::PickableObject* pickableObject1 = m_scene.createPickableObject(*pickableGeometryBuffer, pickableId1, "pickableObject1");
        pickableObject1->setCamera(*perspectiveCamera);
        pickableObject1->translate({0.1f, 1.0f, -1.0f});
        pickableObject1->setRotation({-70.0f, 0.0f, 0.0f}, ramses::ERotationType::Euler_XYZ);
        pickableObject1->scale({3.0f, 3.0f, 3.0f});

        ramses::PickableObject* pickableObject2 = m_scene.createPickableObject(*pickableGeometryBuffer, pickableId2, "pickableObject1");
        pickableObject2->setCamera(*perspectiveCamera);
        pickableObject2->translate({6.0f, 0.9f, -1.0f});
        pickableObject2->setRotation({-70.0f, 0.0f, 0.0f}, ramses::ERotationType::Euler_XYZ);
        pickableObject2->scale({3.0f, 3.0f, 3.0f});

        m_scene.flush();
        updateAndDispatch();

        m_sceneControl.handlePickEvent(m_sceneId, -0.382812f, 0.441667f);
        updateAndDispatch();
        m_sceneControlEventHandler.expectObjectsPicked({ pickableId1 });

        m_sceneControl.handlePickEvent(m_sceneId, -0.379687f, 0.395833f);
        updateAndDispatch();
        m_sceneControlEventHandler.expectObjectsPicked({ pickableId2 });
    }
}
