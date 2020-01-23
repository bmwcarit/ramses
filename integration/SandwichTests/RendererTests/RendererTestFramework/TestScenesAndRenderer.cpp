//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenesAndRenderer.h"
#include "RendererTestUtils.h"
#include "DisplayConfigImpl.h"
#include "RamsesRendererImpl.h"

namespace ramses_internal
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
        ramses::Scene& clientScene = m_scenes.getScene(sceneId);
        clientScene.publish(ramses::EScenePublicationMode_LocalOnly);

        m_renderer.waitForPublication(sceneId);
    }

    void TestScenesAndRenderer::flush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag)
    {
        ramses::Scene& clientScene = m_scenes.getScene(sceneId);
        clientScene.flush(sceneVersionTag);
    }

    void TestScenesAndRenderer::unpublish(ramses::sceneId_t sceneId)
    {
        ramses::Scene& clientScene = m_scenes.getScene(sceneId);
        clientScene.unpublish();
        m_renderer.waitForUnpublished(sceneId);
    }

    void TestScenesAndRenderer::setExpirationTimestamp(ramses::sceneId_t sceneId, FlushTime::Clock::time_point expirationTS)
    {
        m_scenes.getScene(sceneId).setExpirationTimestamp(std::chrono::duration_cast<std::chrono::milliseconds>(expirationTS.time_since_epoch()).count());
    }

    ramses::status_t TestScenesAndRenderer::validateScene(ramses::sceneId_t sceneId)
    {
        ramses::Scene& clientScene = m_scenes.getScene(sceneId);
        return clientScene.validate();
    }

    const char* TestScenesAndRenderer::getValidationReport(ramses::sceneId_t sceneId)
    {
        ramses::Scene& clientScene = m_scenes.getScene(sceneId);
        return clientScene.getValidationReport();
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

    const ramses_internal::TestRenderer& TestScenesAndRenderer::getTestRenderer() const
    {
        return m_renderer;
    }

    ramses_internal::TestRenderer& TestScenesAndRenderer::getTestRenderer()
    {
        return m_renderer;
    }
}
