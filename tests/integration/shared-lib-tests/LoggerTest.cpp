//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "LogHandler.h"
#include "RendererEventGrabber.h"
#include "RamsesTestUtils.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/client/ramses-client.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/DisplayConfig.h"

namespace ramses::internal
{
    class LoggerTestApp
    {
    public:
        LoggerTestApp()
            : LoggerTestApp(RamsesFrameworkConfig(EFeatureLevel_Latest))
        {
        }

        explicit LoggerTestApp(const RamsesFrameworkConfig& config)
        {
            auto handler = [&](ELogLevel level, std::string_view context, std::string_view msg) { m_log.add(level, context, msg); };
            RamsesFramework::SetLogHandler(handler);

            RendererConfig rendererConfig;
            DisplayConfig  displayConfig;
            m_framework = std::make_unique<RamsesFramework>(config);
            m_client    = m_framework->createClient("defaultClient");
            m_scene     = m_client->createScene(ramses::sceneId_t(123));
            m_renderer  = m_framework->createRenderer(rendererConfig);

            m_renderer->startThread();
            m_eventCollector = std::make_unique<RendererEventGrabber>(m_renderer);
            m_display = m_renderer->createDisplay(displayConfig);
            m_renderer->flush();
        }

        ~LoggerTestApp()
        {
            RamsesFramework::SetLogHandler(LogHandlerFunc());
        }

        std::unique_ptr<RamsesFramework> m_framework;
        RamsesClient* m_client;
        Scene* m_scene;
        RamsesRenderer* m_renderer;
        displayId_t m_display;
        std::unique_ptr<RendererEventGrabber> m_eventCollector;
        LogHandler m_log;
    };

    TEST(LoggerTest, DefaultLogging)
    {
        LoggerTestApp app;
        ASSERT_TRUE(app.m_eventCollector->waitForDisplay(app.m_display));
        EXPECT_FALSE(app.m_log.empty());
        EXPECT_EQ(1, app.m_log.find("R.main: RamsesFramework::createRamsesClient"));
        EXPECT_EQ(1, app.m_log.find("R.main: RamsesFramework::createRamsesRenderer"));
        EXPECT_EQ(1, app.m_log.find("R.DispThrd0:  - executing CreateDisplay"));
    }

    TEST(LoggerTest, DisabledRendererLogs)
    {
        RamsesFrameworkConfig cfg(EFeatureLevel_Latest);
        cfg.setLogLevel("RRND", ELogLevel::Off);
        LoggerTestApp app(cfg);
        ASSERT_TRUE(app.m_eventCollector->waitForDisplay(app.m_display));
        EXPECT_EQ(1, app.m_log.find("R.main: RamsesFramework::createRamsesClient"));
        EXPECT_EQ(1, app.m_log.find("R.main: RamsesFramework::createRamsesRenderer"));
        EXPECT_EQ(0, app.m_log.find("R.DispThrd0:  - executing CreateDisplay"));
    }

    TEST(LoggerTest, DisabledLogs)
    {
        RamsesFrameworkConfig cfg(EFeatureLevel_Latest);
        cfg.setLogLevel(ELogLevel::Error);
        LoggerTestApp app(cfg);
        ASSERT_TRUE(app.m_eventCollector->waitForDisplay(app.m_display));
        EXPECT_TRUE(app.m_log.empty());
    }
}
