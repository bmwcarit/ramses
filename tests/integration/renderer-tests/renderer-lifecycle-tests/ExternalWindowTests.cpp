//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLifecycleTests.h"
#include "TestScenes/MultipleTrianglesScene.h"

#include "ramses/renderer/DisplayConfig.h"
#include "impl/DisplayConfigImpl.h"
#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"

#ifdef ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS
#include "internal/Platform/Windows/Window_Windows.h"
#elif ramses_sdk_ENABLE_WINDOW_TYPE_X11
#include "internal/Platform/X11/Window_X11.h"
#endif

namespace ramses::internal
{
    TEST_F(ARendererLifecycleTest, CanRenderSceneToExternallyOwnedWindow)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();

        class DummyEventHandler : public IWindowEventHandler
        {
        public:
            ~DummyEventHandler() override = default;

            void onKeyEvent(EKeyEvent event, KeyModifiers mod, EKeyCode code) override
            {
                (void)event;
                (void)mod;
                (void)code;
            }

            void onMouseEvent(EMouseEvent event, int32_t param1, int32_t param2) override
            {
                (void)event;
                (void)param1;
                (void)param2;
            }
            void onClose() override
            {
            }
            void onResize(uint32_t x, uint32_t y) override
            {
                (void)x;
                (void)y;
            }
            void onWindowMove(int32_t x, int32_t y) override
            {
                (void)x;
                (void)y;
            }
        };

        ramses::DisplayConfig dispConfigExternalWindow = RendererTestUtils::CreateTestDisplayConfig(0u, false);
        if(dispConfigExternalWindow.getWindowType() != EWindowType::X11
            && dispConfigExternalWindow.getWindowType() != EWindowType::Windows)
        {
            GTEST_SKIP();
        }

        dispConfigExternalWindow.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
        DummyEventHandler dummyEventHandler;

        auto dispConfig = RendererTestUtils::CreateTestDisplayConfig(0u, false);
        dispConfig.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);

#ifdef ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS
        Window_Windows window(dispConfigExternalWindow.impl().getInternalDisplayConfig(), dummyEventHandler, 1);
        ASSERT_TRUE(window.init());
        dispConfig.setWindowsWindowHandle(window.getNativeWindowHandle());
#elif ramses_sdk_ENABLE_WINDOW_TYPE_X11
        Window_X11 window(dispConfigExternalWindow.impl().getInternalDisplayConfig(), dummyEventHandler, 1);
        ASSERT_TRUE(window.init());
        dispConfig.setX11WindowHandle(X11WindowHandle{window.getNativeWindowHandle()});
#else
        FAIL() << "Test running on unsupported platform";
#endif

        const displayId_t display = testRenderer.createDisplay(dispConfig);
        ASSERT_TRUE(display != displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);

        auto& scene = testScenesAndRenderer.getScenesRegistry().getScene(sceneId);
        ASSERT_TRUE(testRenderer.getSceneToState(scene, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }
}
