//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERLIFECYCLETESTS_H
#define RAMSES_RENDERERLIFECYCLETESTS_H

#include "TestScenesAndRenderer.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    class ARendererLifecycleTest : public ::testing::Test
    {
    public:
        ARendererLifecycleTest()
            : frameworkConfig()
            , testScenesAndRenderer(frameworkConfig)
            , testRenderer(testScenesAndRenderer.getTestRenderer())
        {}

    protected:
        ramses::displayId_t createDisplayForWindow(uint32_t iviSurfaceIdOffset = 0u, bool iviWindowStartVisible = true)
        {
            ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(iviSurfaceIdOffset, iviWindowStartVisible);
            displayConfig.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
            return testRenderer.createDisplay(displayConfig);
        }

        testing::AssertionResult checkScreenshot(ramses::displayId_t display, const char* screenshotFile)
        {
            if (testRenderer.performScreenshotCheck(display, {}, 0u, 0u, WindowWidth, WindowHeight, screenshotFile))
                return testing::AssertionSuccess();

            return testing::AssertionFailure() << "Screenshot failed " << screenshotFile;
        }

        template <typename INTEGRATION_SCENE>
        ramses::sceneId_t createScene(uint32_t state, const glm::vec3& cameraPosition = { 0.f, 0.f, 0.f }, uint32_t vpWidth = WindowWidth, uint32_t vpHeight = WindowHeight)
        {
            return testScenesAndRenderer.getScenesRegistry().createScene<INTEGRATION_SCENE>(state, cameraPosition, vpWidth, vpHeight);
        }
        template <typename INTEGRATION_SCENE>
        void createScene(uint32_t state, ramses::sceneId_t sceneId, const glm::vec3& cameraPosition = { 0.f, 0.f, 0.f })
        {
            testScenesAndRenderer.getScenesRegistry().createScene<INTEGRATION_SCENE>(state, sceneId, cameraPosition, WindowWidth, WindowHeight);
        }

        static const uint32_t WindowX = 0u;
        static const uint32_t WindowY = 0u;
        static const uint32_t WindowWidth = 128u;
        static const uint32_t WindowHeight = 64u;

        ramses::RamsesFrameworkConfig frameworkConfig;
        TestScenesAndRenderer testScenesAndRenderer;
        TestRenderer& testRenderer;
    };
}

#endif
