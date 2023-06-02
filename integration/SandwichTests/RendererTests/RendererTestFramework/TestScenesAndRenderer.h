//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTSCENESANDRENDERER_H
#define RAMSES_TESTSCENESANDRENDERER_H

#include "TestRenderer.h"
#include "TestScenes.h"
#include "ramses-client-api/IClientEventHandler.h"
#include "Components/FlushTimeInformation.h"

#include <chrono>
#include <string_view>

namespace ramses_internal
{
    class TestClientEventHandler;

    class TestScenesAndRenderer
    {
    public:
        explicit TestScenesAndRenderer(const ramses::RamsesFrameworkConfig& config);

        void initializeRenderer(const ramses::RendererConfig& rendererConfig = RendererTestUtils::CreateTestRendererConfig());
        void destroyRenderer();

        void publish(ramses::sceneId_t sceneId);
        void flush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag = ramses::InvalidSceneVersionTag);
        void unpublish(ramses::sceneId_t sceneId);
        void setExpirationTimestamp(ramses::sceneId_t sceneId, FlushTime::Clock::time_point expirationTS);
        bool loopTillClientEvent(TestClientEventHandler& handlerWithCondition);

        ramses::status_t validateScene(ramses::sceneId_t sceneId);
        const char*      getValidationReport(ramses::sceneId_t sceneId);

        [[nodiscard]] const TestScenes& getScenesRegistry() const;
        [[nodiscard]] TestScenes& getScenesRegistry();

        [[nodiscard]] const ramses::RamsesClient& getClient() const;
        [[nodiscard]] ramses::RamsesClient& getClient();

        [[nodiscard]] const TestRenderer& getTestRenderer() const;
        [[nodiscard]] TestRenderer& getTestRenderer();

    private:
        ramses::RamsesFramework m_ramsesFramework;
        ramses::RamsesClient& m_client;
        TestScenes m_scenes;
        TestRenderer m_renderer;
    };

    class TestClientEventHandler : public ramses::IClientEventHandler
    {
    public:
        void sceneFileLoadFailed(std::string_view) override {}
        void sceneFileLoadSucceeded(std::string_view, ramses::Scene*) override {}
        void sceneReferenceFlushed(ramses::SceneReference&, ramses::sceneVersionTag_t) override {}
        void dataLinked(ramses::sceneId_t, ramses::dataProviderId_t, ramses::sceneId_t, ramses::dataConsumerId_t, bool) override {}
        void dataUnlinked(ramses::sceneId_t, ramses::dataConsumerId_t, bool) override {}
        void sceneReferenceStateChanged(ramses::SceneReference&, ramses::RendererSceneState) override {}

        [[nodiscard]] virtual bool waitCondition() const = 0;
    };
}

#endif
