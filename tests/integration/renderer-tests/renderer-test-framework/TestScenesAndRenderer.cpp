//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenesAndRenderer.h"
#include "RendererTestUtils.h"
#include "impl/DisplayConfigImpl.h"
#include "impl/RamsesRendererImpl.h"

namespace ramses::internal
{
    TestScenesAndRenderer::TestScenesAndRenderer(const ramses::RamsesFrameworkConfig& config)
        : m_ramsesFramework(config)
        , m_client(*m_ramsesFramework.createClient("test renderer client"))
        , m_scenes(m_client)
    {
    }

    void TestScenesAndRenderer::initializeRenderer(const ramses::RendererConfig& rendererConfig)
    {
        m_renderer.initializeRendererWithFramework(m_ramsesFramework, rendererConfig);
    }

    void TestScenesAndRenderer::destroyRenderer()
    {
        m_renderer.destroyRendererWithFramework(m_ramsesFramework);
    }

    void TestScenesAndRenderer::publish(ramses::sceneId_t sceneId)
    {
        m_scenes.getScene(sceneId).publish(ramses::EScenePublicationMode::LocalOnly);
    }

    void TestScenesAndRenderer::flush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag)
    {
        ramses::Scene& clientScene = m_scenes.getScene(sceneId);
        clientScene.flush(sceneVersionTag);
    }

    void TestScenesAndRenderer::unpublish(ramses::sceneId_t sceneId)
    {
        m_scenes.getScene(sceneId).unpublish();
    }

    void TestScenesAndRenderer::setExpirationTimestamp(ramses::sceneId_t sceneId, FlushTime::Clock::time_point expirationTS)
    {
        m_scenes.getScene(sceneId).setExpirationTimestamp(std::chrono::duration_cast<std::chrono::milliseconds>(expirationTS.time_since_epoch()).count());
    }

    bool TestScenesAndRenderer::loopTillClientEvent(TestClientEventHandler& handlerWithCondition)
    {
        const auto startTime = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds{ 20 })
        {
            handlerWithCondition.onUpdate();
            getClient().dispatchEvents(handlerWithCondition);

            if (!getTestRenderer().isRendererThreadEnabled())
                getTestRenderer().doOneLoop();

            if (handlerWithCondition.waitCondition())
                return true;

            std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
        }

        return false;
    }

    bool TestScenesAndRenderer::getSceneToState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        return m_renderer.getSceneToState(m_scenes.getScene(sceneId), state);
    }

    bool TestScenesAndRenderer::waitForSceneStateChange(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        return m_renderer.waitForSceneStateChange(m_scenes.getScene(sceneId), state);
    }

    ramses::ValidationReport TestScenesAndRenderer::validateScene(ramses::sceneId_t sceneId)
    {
        ramses::Scene& clientScene = m_scenes.getScene(sceneId);
        ramses::ValidationReport report;
        clientScene.validate(report);
        return report;
    }

    TestScenes& TestScenesAndRenderer::getScenesRegistry()
    {
        return m_scenes;
    }

    const ramses::RamsesClient& TestScenesAndRenderer::getClient() const
    {
        return m_client;
    }

    ramses::RamsesClient& TestScenesAndRenderer::getClient()
    {
        return m_client;
    }

    const ramses::internal::TestRenderer& TestScenesAndRenderer::getTestRenderer() const
    {
        return m_renderer;
    }

    ramses::internal::TestRenderer& TestScenesAndRenderer::getTestRenderer()
    {
        return m_renderer;
    }
}
