//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/DisplayConfig.h"
#include "internal/Core/Utils/Image.h"

#include <optional>
#include <chrono>
#include <string>

namespace ramses
{
    class RamsesRenderer;
    class RamsesDisplay;
}

namespace ramses::internal
{
    class RendererTestUtils
    {
    public:
        static void SaveScreenshotForDisplay(
            RamsesRenderer& renderer,
            displayId_t displayId,
            displayBufferId_t displayBuffer,
            uint32_t x,
            uint32_t y,
            uint32_t width,
            uint32_t height,
            const std::string& screenshotFileName);

        static bool PerformScreenshotTestForDisplay(
            RamsesRenderer& renderer,
            displayId_t displayId,
            displayBufferId_t displayBuffer,
            uint32_t x,
            uint32_t y,
            uint32_t width,
            uint32_t height,
            const std::string& screenshotFileName,
            float maxAveragePercentErrorPerPixel = DefaultMaxAveragePercentPerPixel,
            bool saveDiffOnError = true
        );

        static ramses::internal::Image ReadPixelData(
            RamsesRenderer& renderer,
            displayId_t displayId,
            displayBufferId_t displayBuffer,
            uint32_t x,
            uint32_t y,
            uint32_t width,
            uint32_t height);

        static bool CompareBitmapToImageInFile(
            const ramses::internal::Image& actualBitmap,
            const std::string& expectedScreenshotFileName,
            float maxAveragePercentErrorPerPixel,
            bool saveDiffOnError);

        static displayId_t  CreateDisplayImmediate(RamsesRenderer& renderer, const ramses::DisplayConfig& displayConfig);
        static void         DestroyDisplayImmediate(RamsesRenderer& renderer, displayId_t displayId);

        // All renderer tests should use this renderer config which adds dummy embedded compositor
        static ramses::RendererConfig CreateTestRendererConfig();
        // All renderer tests should use this display config which makes window visible by default for Wayland
        static ramses::DisplayConfig CreateTestDisplayConfig(uint32_t iviSurfaceIdOffset, bool iviWindowStartVisible = true);
        static void SetMaxFrameCallbackPollingTimeForAllTests(std::chrono::microseconds time);
        static void SetDefaultConfigForAllTests(const ramses::RendererConfig& rendererConfig, const ramses::DisplayConfig& displayConfig);

        static void SetWaylandDisplayForSystemCompositorControllerForAllTests(const std::string& wd);

        static const float DefaultMaxAveragePercentPerPixel;

    private:
        static std::optional<std::chrono::microseconds> MaxFrameCallbackPollingTime;
        static std::optional<std::string> WaylandDisplayForSystemCompositorController;

        // config objects are not copyable..
        static std::unique_ptr<ramses::RendererConfig> defaultRendererConfig;
        static std::unique_ptr<ramses::DisplayConfig> defaultDisplayConfig;
    };
}
