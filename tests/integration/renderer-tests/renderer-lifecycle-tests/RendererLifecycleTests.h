//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "TestScenesAndRenderer.h"
#include "ramses/client/SceneConfig.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    class ARendererLifecycleTest : public ::testing::Test
    {
    public:
        ARendererLifecycleTest()
            : testScenesAndRenderer(frameworkConfig)
            , testRenderer(testScenesAndRenderer.getTestRenderer())
        {}

        static const uint32_t WindowX = 0u;
        static const uint32_t WindowY = 0u;
        static const uint32_t WindowWidth = 128u;
        static const uint32_t WindowHeight = 64u;

    protected:
        displayId_t createDisplayForWindow(uint32_t iviSurfaceIdOffset = 0u, bool iviWindowStartVisible = true)
        {
            ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(iviSurfaceIdOffset, iviWindowStartVisible);
            displayConfig.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
            return testRenderer.createDisplay(displayConfig);
        }

        testing::AssertionResult checkScreenshot(displayId_t display, const char* screenshotFile)
        {
            if (testRenderer.performScreenshotCheck(display, {}, 0u, 0u, WindowWidth, WindowHeight, screenshotFile))
                return testing::AssertionSuccess();

            return testing::AssertionFailure() << "Screenshot failed " << screenshotFile;
        }

        template <typename INTEGRATION_SCENE>
        sceneId_t createScene(uint32_t state, const glm::vec3& cameraPosition = { 0.f, 0.f, 0.f }, uint32_t vpWidth = WindowWidth, uint32_t vpHeight = WindowHeight, const ramses::SceneConfig& config = {})
        {
            return testScenesAndRenderer.getScenesRegistry().createScene<INTEGRATION_SCENE>(state, cameraPosition, vpWidth, vpHeight, config);
        }
        template <typename INTEGRATION_SCENE>
        void createScene(uint32_t state, sceneId_t sceneId, const glm::vec3& cameraPosition = { 0.f, 0.f, 0.f }, const ramses::SceneConfig& config = {})
        {
            testScenesAndRenderer.getScenesRegistry().createScene<INTEGRATION_SCENE>(state, sceneId, cameraPosition, WindowWidth, WindowHeight, config);
        }

        RamsesFrameworkConfig frameworkConfig{EFeatureLevel_Latest};
        TestScenesAndRenderer testScenesAndRenderer;
        TestRenderer& testRenderer;
    };
}
