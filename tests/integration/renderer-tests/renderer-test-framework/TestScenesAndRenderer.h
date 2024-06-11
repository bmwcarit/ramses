//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "TestRenderer.h"
#include "TestScenes.h"
#include "ramses/framework/ValidationReport.h"
#include "ramses/client/IClientEventHandler.h"
#include "internal/Components/FlushTimeInformation.h"

#include <chrono>
#include <string_view>

namespace ramses::internal
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

        bool getSceneToState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
        bool waitForSceneStateChange(ramses::sceneId_t sceneId, ramses::RendererSceneState state);

        ramses::ValidationReport validateScene(ramses::sceneId_t sceneId);

        [[nodiscard]] const TestScenes& getScenesRegistry() const;
        [[nodiscard]] TestScenes& getScenesRegistry();

        [[nodiscard]] const ramses::RamsesClient& getClient() const;
        [[nodiscard]] ramses::RamsesClient& getClient();

        [[nodiscard]] const TestRenderer& getTestRenderer() const;
        [[nodiscard]] TestRenderer& getTestRenderer();

        [[nodiscard]] EFeatureLevel getFeatureLevel() const;

    private:
        ramses::RamsesFramework m_ramsesFramework;
        ramses::RamsesClient& m_client;
        TestScenes m_scenes;
        TestRenderer m_renderer;
    };

    class TestClientEventHandler : public ramses::IClientEventHandler
    {
    public:
        void sceneFileLoadFailed(std::string_view /*filename*/) override {}
        void sceneFileLoadSucceeded(std::string_view /*filename*/, ramses::Scene* /*loadedScene*/) override {}
        void sceneReferenceFlushed(ramses::SceneReference& /*sceneRef*/, ramses::sceneVersionTag_t /*versionTag*/) override {}
        void dataLinked(ramses::sceneId_t /*providerScene*/, ramses::dataProviderId_t /*providerId*/, ramses::sceneId_t /*consumerScene*/, ramses::dataConsumerId_t /*consumerId*/, bool /*success*/) override {}
        void dataUnlinked(ramses::sceneId_t /*consumerScene*/, ramses::dataConsumerId_t /*consumerId*/, bool /*success*/) override {}
        void sceneReferenceStateChanged(ramses::SceneReference& /*sceneRef*/, ramses::RendererSceneState /*state*/) override {}

        [[nodiscard]] virtual bool waitCondition() const = 0;
        virtual void onUpdate() {}
    };
}
