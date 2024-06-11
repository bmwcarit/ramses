//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositingTestsWithFD.h"
#include "EmbeddedCompositingTestsFramework.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "impl/DisplayConfigImpl.h"
#include "internal/Platform/Wayland/UnixDomainSocket.h"
#include "internal/Platform/Wayland/WaylandEnvironmentUtils.h"
#include "RendererTestUtils.h"

namespace ramses::internal
{
    void EmbeddedCompositingTestsWithFD::setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithoutRenderer(FDTest_CanConnectUsingWaylandSocket, *this, "FDTest_CanConnectUsingWaylandSocket");
        testFramework.createTestCaseWithoutRenderer(FDTest_CanConnectUsingWaylandSocket_TwoApplicationsInSequence, *this, "FDTest_CanConnectUsingWaylandSocket_TwoApplicationsInSameTime");
        testFramework.createTestCaseWithoutRenderer(FDTest_WaylandClientCanConnectToSocketBeforeEmbeddedCompositorIsInitialized, *this, "FDTest_WaylandClientCanConnectToSocketBeforeEmbeddedCompositorIsInitialized");
        testFramework.createTestCaseWithoutRenderer(FDTest_WaylandClientCanConnectToSocketBeforeAndAfterEmbeddedCompositorIsInitialized, *this, "FDTest_WaylandClientCanConnectToSocketBeforeAndAfterEmbeddedCompositorIsInitialized");
        testFramework.createTestCaseWithoutRenderer(FDTest_CanConnectUsingWaylandSocketAndWaylandDisplay, *this, "FDTest_CanConnectUsingWaylandSocketAndWaylandDisplay");
    }

    bool EmbeddedCompositingTestsWithFD::runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        UnixDomainSocket socket(m_waylandSocket, WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));

        ramses::RendererConfig rendererConfig = RendererTestUtils::CreateTestRendererConfig();

        const uint32_t displayWidth = IntegrationScene::DefaultViewportWidth;
        const uint32_t displayHeight = IntegrationScene::DefaultViewportHeight;

        ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, true);
        displayConfig.setWindowRectangle(0, 0, displayWidth, displayHeight);
        displayConfig.setWaylandEmbeddedCompositingSocketName("");
        displayConfig.setWaylandEmbeddedCompositingSocketGroup("");
        displayConfig.setWaylandEmbeddedCompositingSocketFD(socket.createBoundFileDescriptor());

        switch(testCase.m_id)
        {
        case FDTest_CanConnectUsingWaylandSocket:
        {
            testFramework.initializeRenderer(rendererConfig);

            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayFD);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            testFramework.destroyDisplays();
            testFramework.destroyRenderer();
            return true;
        }
        case FDTest_CanConnectUsingWaylandSocket_TwoApplicationsInSequence:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfig);

            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayFD);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            //reconnect
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayFD);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            testFramework.destroyDisplays();
            testFramework.destroyRenderer();
            return true;
        }
        case FDTest_WaylandClientCanConnectToSocketBeforeEmbeddedCompositorIsInitialized:
        {
            testFramework.startTestApplication();
            testFramework.initializeTestApplication(EConnectionMode::DisplayFD);

            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfig);

            testFramework.waitUntilNumberOfCompositorConnections(1);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            testFramework.destroyDisplays();
            testFramework.destroyRenderer();
            return true;
        }
        case FDTest_WaylandClientCanConnectToSocketBeforeAndAfterEmbeddedCompositorIsInitialized:
        {
            //before EC init
            testFramework.startTestApplication();
            testFramework.initializeTestApplication(EConnectionMode::DisplayFD);

            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfig);

            testFramework.waitUntilNumberOfCompositorConnections(1);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            //reconnect
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayFD);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            testFramework.destroyDisplays();
            testFramework.destroyRenderer();
            return true;
        }
        case FDTest_CanConnectUsingWaylandSocketAndWaylandDisplay:
        {
            testFramework.initializeRenderer(rendererConfig);
            testFramework.createDisplay(displayConfig);

            //connect using wayland socket
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayFD);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            //connect using wayland display
            testFramework.startTestApplicationAndWaitUntilConnected(EConnectionMode::DisplayName);
            testFramework.stopTestApplicationAndWaitUntilDisconnected();

            testFramework.destroyDisplays();
            testFramework.destroyRenderer();
            return true;
        }
        default:
            assert(false);
            return false;
        }
    }
}
