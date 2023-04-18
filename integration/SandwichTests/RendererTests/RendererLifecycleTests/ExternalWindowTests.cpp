//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLifecycleTests.h"
#include "TestScenes/MultipleTrianglesScene.h"

#include "ramses-renderer-api/DisplayConfig.h"
#include "DisplayConfigImpl.h"
#include "RendererAPI/IWindowEventHandler.h"
#include "Utils/ThreadLocalLog.h"

#ifdef __WIN32
#include "Window_Windows/Window_Windows.h"
#elif __linux__
#include "Platform_X11/Window_X11.h"
#endif

namespace ramses_internal
{
    TEST_F(ARendererLifecycleTest, CanRenderSceneToExternallyOwnedWindow)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();

        class DummyEventHandler : public IWindowEventHandler
        {
        public:
            ~DummyEventHandler() override {};

            void onKeyEvent(EKeyEventType , UInt32 , EKeyCode ) override {}
            void onMouseEvent(EMouseEventType , Int32 , Int32 ) override {}
            void onClose() override {}
            void onResize(UInt32 , UInt32 ) override {}
            void onWindowMove(Int32 , Int32) override {}
        };

        ramses::DisplayConfig dispConfigExternalWindow = RendererTestUtils::CreateTestDisplayConfig(0u, false);
        dispConfigExternalWindow.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
        DummyEventHandler dummyEventHandler;

        auto dispConfig = RendererTestUtils::CreateTestDisplayConfig(0u, false);
        dispConfig.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);

#ifdef __WIN32
        Window_Windows window(dispConfigExternalWindow.impl.getInternalDisplayConfig(), dummyEventHandler, 1);
        ASSERT_TRUE(window.init());
        dispConfig.setWindowsWindowHandle(window.getNativeWindowHandle());
#elif __linux__
        ThreadLocalLog::SetPrefix(1);
        Window_X11 window(dispConfigExternalWindow.impl.getInternalDisplayConfig(), dummyEventHandler, 1);
        ASSERT_TRUE(window.init());
        dispConfig.setX11WindowHandle(window.getNativeWindowHandle());
#endif

        const ramses::displayId_t display = testRenderer.createDisplay(dispConfig);
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }
}
