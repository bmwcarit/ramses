//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERTESTUTILS_H
#define RAMSES_RENDERERTESTUTILS_H

#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "Utils/Image.h"

#include <optional>
#include <chrono>
#include <string>

namespace ramses
{
    class RamsesRenderer;
    class RamsesDisplay;
}

class RendererTestUtils
{
public:
    static void SaveScreenshotForDisplay(
        ramses::RamsesRenderer& renderer,
        ramses::displayId_t displayId,
        ramses::displayBufferId_t displayBuffer,
        ramses_internal::UInt32 x,
        ramses_internal::UInt32 y,
        ramses_internal::UInt32 width,
        ramses_internal::UInt32 height,
        const std::string& screenshotFileName);

    static bool PerformScreenshotTestForDisplay(
        ramses::RamsesRenderer& renderer,
        ramses::displayId_t displayId,
        ramses::displayBufferId_t displayBuffer,
        ramses_internal::UInt32 x,
        ramses_internal::UInt32 y,
        ramses_internal::UInt32 width,
        ramses_internal::UInt32 height,
        const std::string& screenshotFileName,
        float maxAveragePercentErrorPerPixel = DefaultMaxAveragePercentPerPixel,
        bool saveDiffOnError = true
    );

    static ramses_internal::Image ReadPixelData(
        ramses::RamsesRenderer& renderer,
        ramses::displayId_t displayId,
        ramses::displayBufferId_t displayBuffer,
        ramses_internal::UInt32 x,
        ramses_internal::UInt32 y,
        ramses_internal::UInt32 width,
        ramses_internal::UInt32 height);

    static bool CompareBitmapToImageInFile(
        const ramses_internal::Image& actualBitmap,
        const std::string& expectedScreenshotFileName,
        float maxAveragePercentErrorPerPixel,
        bool saveDiffOnError);

    static ramses::displayId_t  CreateDisplayImmediate(ramses::RamsesRenderer& renderer, const ramses::DisplayConfig& displayConfig);
    static void                 DestroyDisplayImmediate(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId);

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

#endif
