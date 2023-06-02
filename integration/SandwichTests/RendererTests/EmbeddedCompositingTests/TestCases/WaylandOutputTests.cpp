//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandOutputTests.h"
#include "TestScenes/EmbeddedCompositorScene.h"
#include "WaylandOutputTestParams.h"
#include "Utils/LogMacros.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "wayland-server-protocol.h"

namespace ramses_internal
{
    WaylandOutputTests::WaylandOutputTests()
    {
    }

    void WaylandOutputTests::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCase(WaylandOutput_Version1, *this, "WaylandOutput_Version1");
        testFramework.createTestCase(WaylandOutput_Version2, *this, "WaylandOutput_Version2");
        testFramework.createTestCase(WaylandOutput_Version3, *this, "WaylandOutput_Version3");
    }

    bool WaylandOutputTests::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        bool testResultValue = true;

        switch(testCase.m_id)
        {

        case WaylandOutput_Version1:
        {
            const uint32_t displayWidth = 123u;
            const uint32_t displayHeight = 156u;

            auto displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
            displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());
            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplication();
            testFramework.sendSetRequiredWaylandOutputVersion(1);
            testFramework.initializeTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(1u);

            WaylandOutputTestParams resultWaylandOutputParams = {};
            const bool errorsFoundInWaylandOutput = testFramework.getWaylandOutputParamsFromTestApplication(resultWaylandOutputParams);
            testResultValue = !errorsFoundInWaylandOutput;
            testResultValue &= checkWaylandOutputParams(resultWaylandOutputParams, displayWidth, displayHeight, true, false, false);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case WaylandOutput_Version2:
        {
            const uint32_t displayWidth = 123u;
            const uint32_t displayHeight = 156u;

            auto displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
            displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());
            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplication();
            testFramework.sendSetRequiredWaylandOutputVersion(2);
            testFramework.initializeTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(1u);

            WaylandOutputTestParams resultWaylandOutputParams = {};
            const bool errorsFoundInWaylandOutput = testFramework.getWaylandOutputParamsFromTestApplication(resultWaylandOutputParams);
            testResultValue = !errorsFoundInWaylandOutput;
            testResultValue &= checkWaylandOutputParams(resultWaylandOutputParams, displayWidth, displayHeight, true, true, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        case WaylandOutput_Version3:
        {
            const uint32_t displayWidth = 123u;
            const uint32_t displayHeight = 156u;

            auto displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
            displayConfig.setWaylandEmbeddedCompositingSocketName(EmbeddedCompositingTestsFramework::TestEmbeddedCompositingDisplayName.c_str());
            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplication();
            testFramework.sendSetRequiredWaylandOutputVersion(3);
            testFramework.initializeTestApplication();
            testFramework.waitUntilNumberOfCompositorConnections(1u);

            WaylandOutputTestParams resultWaylandOutputParams = {};
            const bool errorsFoundInWaylandOutput = testFramework.getWaylandOutputParamsFromTestApplication(resultWaylandOutputParams);
            testResultValue = !errorsFoundInWaylandOutput;
            testResultValue &= checkWaylandOutputParams(resultWaylandOutputParams, displayWidth, displayHeight, true, true, true);

            testFramework.stopTestApplicationAndWaitUntilDisconnected();
            break;
        }

        default:
            assert(false);
        }

        return testResultValue;
    }

    bool WaylandOutputTests::checkWaylandOutputParams(const WaylandOutputTestParams& waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight, bool expectMode, bool expectScale, bool expectDone)
    {
        bool success = true;

        const bool geometryActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_GeometryReceived) != 0;
        const bool modeActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ModeReceived) != 0;
        const bool scaleActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_ScaleReceived) != 0;
        const bool doneActuallyReceived = (waylandOutputParams.m_waylandOutputReceivedFlags & WaylandOutputTestParams::WaylandOutput_DoneReceived) != 0;

        if(!geometryActuallyReceived)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandOutputTests::checkWaylandOutputParams: geometry was not received!");
            success = false;
        }

        if(modeActuallyReceived != expectMode)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandOutputTests::checkWaylandOutputParams: expected mode received :" << expectMode
                      << ", actual mode received :" << modeActuallyReceived);
            success = false;
        }

        if(scaleActuallyReceived != expectScale)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandOutputTests::checkWaylandOutputParams: expected scale received :" << expectScale
                      << ", actual scale received :" << scaleActuallyReceived);
            success = false;
        }

        if(doneActuallyReceived != expectDone)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandOutputTests::checkWaylandOutputParams: expected done received :" << expectDone
                      << ", actual done received :" << doneActuallyReceived);
            success = false;
        }

        if(!checkWaylandOutputGeometry(waylandOutputParams))
            success = false;

        if(expectMode && !checkWaylandOutputMode(waylandOutputParams, expectedWidth, expectedHeight))
            success = false;

        if(expectScale && !checkWaylandOutputScale(waylandOutputParams))
            success = false;

        return success;
    }

    bool WaylandOutputTests::checkWaylandOutputGeometry(const WaylandOutputTestParams& waylandOutputParams)
    {

        const int32_t xExpected = 0;
        const int32_t yExpected = 0;
        const int32_t physicalWidthExpected = 0;
        const int32_t physicalHeightExpected= 0;
        const int32_t subpixelExpected = WL_OUTPUT_SUBPIXEL_UNKNOWN;
        const int32_t transformExpected = WL_OUTPUT_TRANSFORM_NORMAL;

        if(waylandOutputParams.x != xExpected || waylandOutputParams.y != yExpected
            || waylandOutputParams.physicalWidth != physicalWidthExpected || waylandOutputParams.physicalHeight != physicalHeightExpected
            || waylandOutputParams.subpixel != subpixelExpected || waylandOutputParams.transform != transformExpected)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandOutputTests::checkWaylandOutputParams: wrong values received"
                      << ": x = "               << waylandOutputParams.x                << " (expected = " << xExpected << ")"
                      << ", y = "               << waylandOutputParams.y                << " (expected = " << yExpected << ")"
                      << ", physical_width = "  << waylandOutputParams.physicalWidth    << " (expected = " << physicalWidthExpected << ")"
                      << ", physical_height = " << waylandOutputParams.physicalHeight   << " (expected = " << physicalHeightExpected<< ")"
                      << ", subpixel format = " << waylandOutputParams.subpixel         << " (expected = " << subpixelExpected<< ")"
                      << ", transform = "       << waylandOutputParams.transform        << " (expected = " << transformExpected<< ")"
                      );

            return false;
        }

        return true;
    }

    bool WaylandOutputTests::checkWaylandOutputMode(const WaylandOutputTestParams &waylandOutputParams, uint32_t expectedWidth, uint32_t expectedHeight)
    {
        const uint32_t flagsExpected = WL_OUTPUT_MODE_CURRENT;
        const int32_t refreshExpected= 0;

        if(waylandOutputParams.modeFlags != flagsExpected
            || waylandOutputParams.width != static_cast<int32_t>(expectedWidth)
            || waylandOutputParams.height != static_cast<int32_t>(expectedHeight)
            || waylandOutputParams.refreshRate != refreshExpected)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandOutputTests::checkWaylandOutputParams: wrong values received"
                      << ": flags = "           << waylandOutputParams.modeFlags            << " (expected = " << flagsExpected << ")"
                      << ": width = "           << waylandOutputParams.width                << " (expected = " << expectedWidth << ")"
                      << ": height = "          << waylandOutputParams.height               << " (expected = " << expectedHeight << ")"
                      << ": refresh = "         << waylandOutputParams.refreshRate          << " (expected = " << refreshExpected << ")"
                      );

            return false;
        }

        return true;
    }

    bool WaylandOutputTests::checkWaylandOutputScale(const WaylandOutputTestParams &waylandOutputParams)
    {
        const int32_t factorExpected = 1;

        if(waylandOutputParams.factor != factorExpected)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandOutputTests::checkWaylandOutputParams: wrong values received"
                      << ": factor = " << waylandOutputParams.factor << " (expected = " << factorExpected << ")"
                      );

            return false;
        }

        return true;
    }
}
