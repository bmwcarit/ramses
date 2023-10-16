//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses/renderer/BinaryShaderCache.h"
#include "ramses/renderer/RendererConfig.h"
#include "impl/RendererConfigImpl.h"

namespace ramses::internal
{
    TEST(ARendererConfig, hasDefaultValuesUponConstruction)
    {
        const ramses::internal::RendererConfig defaultConfig;
        ramses::RendererConfig config;
        EXPECT_EQ(nullptr, config.impl().getBinaryShaderCache());

        const ramses::internal::RendererConfig& internalConfig = config.impl().getInternalRendererConfig();

        EXPECT_EQ(defaultConfig.getFrameCallbackMaxPollTime(), internalConfig.getFrameCallbackMaxPollTime());
        EXPECT_EQ(defaultConfig.getRenderThreadLoopTimingReportingPeriod(), internalConfig.getRenderThreadLoopTimingReportingPeriod());
        EXPECT_EQ(defaultConfig.getSystemCompositorControlEnabled(), internalConfig.getSystemCompositorControlEnabled());
    }

    TEST(ARendererConfig, canEnableSystemCompositor)
    {
        ramses::RendererConfig config;
        EXPECT_TRUE(config.enableSystemCompositorControl());
        EXPECT_TRUE(config.impl().getInternalRendererConfig().getSystemCompositorControlEnabled());
    }

    TEST(ARendererConfig, CanBeCopyAndMoveConstructed)
    {
        ramses::RendererConfig config;
        config.setRenderThreadLoopTimingReportingPeriod(std::chrono::seconds{ 1 });

        ramses::RendererConfig configCopy{ config };
        EXPECT_EQ(std::chrono::seconds{ 1 }, configCopy.getRenderThreadLoopTimingReportingPeriod());

        ramses::RendererConfig configMove{ std::move(config) };
        EXPECT_EQ(std::chrono::seconds{ 1 }, configCopy.getRenderThreadLoopTimingReportingPeriod());
    }

    TEST(ARendererConfig, CanBeCopyAndMoveAssigned)
    {
        ramses::RendererConfig config;
        config.setRenderThreadLoopTimingReportingPeriod(std::chrono::seconds{ 1 });

        ramses::RendererConfig configCopy;
        configCopy = config;
        EXPECT_EQ(std::chrono::seconds{ 1 }, configCopy.getRenderThreadLoopTimingReportingPeriod());

        ramses::RendererConfig configMove;
        configMove = std::move(config);
        EXPECT_EQ(std::chrono::seconds{ 1 }, configCopy.getRenderThreadLoopTimingReportingPeriod());
    }

    TEST(ARendererConfig, CanBeSelfAssigned)
    {
        ramses::RendererConfig config;
        config.setRenderThreadLoopTimingReportingPeriod(std::chrono::seconds{ 1 });

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        config = config;
        EXPECT_EQ(std::chrono::seconds{ 1 }, config.getRenderThreadLoopTimingReportingPeriod());
        config = std::move(config);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        EXPECT_EQ(std::chrono::seconds{ 1 }, config.getRenderThreadLoopTimingReportingPeriod());
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }

    TEST(ARendererConfig, canSetBinaryShaderCache)
    {
        ramses::RendererConfig config;

        ramses::BinaryShaderCache cache;
        config.setBinaryShaderCache(cache);
        EXPECT_EQ(&cache, config.impl().getBinaryShaderCache());
    }

    TEST(ARendererConfig, setsAndGetsWaylandDisplay)
    {
        ramses::RendererConfig config;
        EXPECT_TRUE(config.setSystemCompositorWaylandDisplay("xxx"));
        EXPECT_EQ("xxx", config.getSystemCompositorWaylandDisplay());
        EXPECT_EQ("xxx", config.impl().getInternalRendererConfig().getWaylandDisplayForSystemCompositorController());
    }

    TEST(ARendererConfig, setsAndGetsLoopCountPeriod)
    {
        ramses::RendererConfig config;
        EXPECT_TRUE(config.setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds(1234)));
        EXPECT_EQ(std::chrono::milliseconds(1234), config.getRenderThreadLoopTimingReportingPeriod());
    }
}
