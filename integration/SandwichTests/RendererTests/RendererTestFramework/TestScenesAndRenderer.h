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
#include <chrono>
#include "Components/FlushTimeInformation.h"

namespace ramses_internal
{
    class TestScenesAndRenderer
    {
    public:
        explicit TestScenesAndRenderer(const ramses::RamsesFrameworkConfig& config);

        void initializeRenderer(const ramses::RendererConfig& rendererConfig = RendererTestUtils::CreateTestRendererConfig());
        void destroyRenderer();

        void             publish(ramses::sceneId_t sceneId);
        void             flush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag = ramses::InvalidSceneVersionTag);
        void             unpublish(ramses::sceneId_t sceneId);
        void             setExpirationTimestamp(ramses::sceneId_t sceneId, FlushTime::Clock::time_point expirationTS);

        ramses::status_t validateScene(ramses::sceneId_t sceneId);
        const char*      getValidationReport(ramses::sceneId_t sceneId);

        const TestScenes& getScenesRegistry() const;
        TestScenes& getScenesRegistry();

        const ramses::RamsesClient& getClient() const;
        ramses::RamsesClient& getClient();

        const TestRenderer& getTestRenderer() const;
        TestRenderer& getTestRenderer();

    private:
        ramses::RamsesFramework m_ramsesFramework;
        ramses::RamsesClient& m_client;
        TestScenes m_scenes;
        TestRenderer m_renderer;
    };
}

#endif
