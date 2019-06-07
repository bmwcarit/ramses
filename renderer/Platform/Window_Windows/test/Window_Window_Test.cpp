//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "Window_Windows/Window_Windows.h"
#include "WindowEventHandlerMock.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/EKeyModifier.h"

using namespace testing;

namespace ramses_internal
{
    class AWindowWindows : public testing::Test
    {
    public:
        AWindowWindows()
            : window(config, eventHandlerMock, 0)
        {
        }

        virtual void SetUp()
        {
            ASSERT_TRUE(window.init());
            processAllEvents();
        }

        void processAllEvents()
        {
            for (UInt32 i = 0; i < 10; i++) // enforce handling of all enqueued window events which will trigger our event handler
                window.handleEvents();
        }

        void sendKeyEvent(UInt8 virtualKeyCode)
        {
            ASSERT_TRUE(PostMessage(window.getNativeWindowHandle(), WM_KEYDOWN, virtualKeyCode, 0));
            ASSERT_TRUE(PostMessage(window.getNativeWindowHandle(), WM_KEYUP, virtualKeyCode, 0));
        }

        void sendMouseEvent(short posX, short posY, UINT eventType, UINT wParam = 0)
        {
            const int packedMousePosition = (static_cast<int>(posY) << 16) | posX;
            ASSERT_TRUE(PostMessage(window.getNativeWindowHandle(), eventType, wParam, packedMousePosition));
        }

        void sendWindowLeaveEvent()
        {
            ASSERT_TRUE(PostMessage(window.getNativeWindowHandle(), WM_MOUSELEAVE, 0, 0));
        }

        void testKeyCode(UInt8 virtualKeyCode, EKeyCode expectedRamesKeyCode, UInt32 expectedModifier = EKeyModifier_NoModifier)
        {
            sendKeyEvent(virtualKeyCode);

            Sequence seq;
            EXPECT_CALL(eventHandlerMock, onKeyEvent(EKeyEventType_Pressed, expectedModifier, expectedRamesKeyCode)).Times(1).InSequence(seq);
            EXPECT_CALL(eventHandlerMock, onKeyEvent(EKeyEventType_Released, EKeyModifier_NoModifier, expectedRamesKeyCode)).Times(1).InSequence(seq);
            processAllEvents();
        }

        DisplayConfig config;
        NiceMock<WindowEventHandlerMock> eventHandlerMock;
        Window_Windows window;
    };

    TEST(Window_Windows, convertVirtualKeyCodeToRamsesKeyCode)
    {
        // only test special calculation code for consecutive key codes
        EXPECT_EQ(Window_Windows::convertVirtualKeyCodeIntoRamsesKeyCode(0x41, 0), EKeyCode_A);
        EXPECT_EQ(Window_Windows::convertVirtualKeyCodeIntoRamsesKeyCode(0x31, 0), EKeyCode_1);
        EXPECT_EQ(Window_Windows::convertVirtualKeyCodeIntoRamsesKeyCode(0x62, 0), EKeyCode_Numpad_2);
        EXPECT_EQ(Window_Windows::convertVirtualKeyCodeIntoRamsesKeyCode(0x73, 0), EKeyCode_F4);
    }

    TEST(Window_Windows, propagatesResizeEvents)
    {
        DisplayConfig config;
        config.setResizable(true);
        NiceMock<WindowEventHandlerMock> eventHandlerMock;
        Window_Windows window(config, eventHandlerMock, 0);

        ASSERT_TRUE(window.init());
        for (UInt32 i = 0; i < 10; i++) // enforce handling of all enqueued window events which will trigger our event handler
            window.handleEvents();

        const uint32_t width = 15;
        const uint32_t height = 20;
        const int packedWindowSize = (static_cast<int>(height) << 16) | width;
        ASSERT_TRUE(PostMessage(window.getNativeWindowHandle(), WM_SIZE, 0, packedWindowSize));

        EXPECT_CALL(eventHandlerMock, onResize(15, 20)).Times(1);
        window.handleEvents();
    }

    TEST_F(AWindowWindows, singleKeyPressEventTriggersKeyPressedEventWithCorrectKeyCode)
    {
        testKeyCode(0x41,        EKeyCode_A);
        testKeyCode(0x30,        EKeyCode_0);

        testKeyCode(VK_ESCAPE,   EKeyCode_Escape);
        testKeyCode(VK_RETURN,   EKeyCode_Return);
        testKeyCode(VK_TAB,      EKeyCode_Tab);
        testKeyCode(VK_F1,       EKeyCode_F1);
        testKeyCode(VK_SPACE,    EKeyCode_Space);
        testKeyCode(VK_SHIFT,    EKeyCode_ShiftLeft,     EKeyModifier_Shift);
    }

    TEST_F(AWindowWindows, leftMouseButtonDownEventTriggersLeftButtonDownEvent)
    {
        sendMouseEvent(5, 10, WM_LBUTTONDOWN);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonDown, 5, 10)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, leftMouseButtonUpEventTriggersLeftButtonUpEvent)
    {
        sendMouseEvent(5, 10, WM_LBUTTONUP);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonUp, 5, 10)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, rightMouseButtonDownEventTriggersRightButtonDownEvent)
    {
        sendMouseEvent(6, 11, WM_RBUTTONDOWN);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_RightButtonDown, 6, 11)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, rightMouseButtonUpEventTriggersRightButtonUpEvent)
    {
        sendMouseEvent(6, 11, WM_RBUTTONUP);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_RightButtonUp, 6, 11)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, middleMouseButtonDownEventTriggersMiddleButtonDownEvent)
    {
        sendMouseEvent(6, 11, WM_MBUTTONDOWN);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_MiddleButtonDown, 6, 11)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, middleMouseButtonUpEventTriggersMiddleButtonUpEvent)
    {
        sendMouseEvent(6, 11, WM_MBUTTONUP);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_MiddleButtonUp, 6, 11)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, positiveMouseWheelEventTriggersMouseWheelUpEvent)
    {
        const int packedWheelDelta = WHEEL_DELTA << 16;
        sendMouseEvent(7, 12, WM_MOUSEWHEEL, packedWheelDelta);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WheelUp, 7, 12)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, negativeMouseWheelEventTriggersMouseWheelDownEvent)
    {
        const auto packedWheelDelta = static_cast<unsigned int>(-(WHEEL_DELTA << 16));
        sendMouseEvent(7, 12, WM_MOUSEWHEEL, packedWheelDelta);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WheelDown, 7, 12)).Times(1);
        processAllEvents();
    }

    TEST_F(AWindowWindows, largeMouseWheelEventTriggersMultipleMouseWheelEvents)
    {
        const int packedWheelDelta = (3*WHEEL_DELTA) << 16;
        sendMouseEvent(7, 12, WM_MOUSEWHEEL, packedWheelDelta);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WheelUp, 7, 12)).Times(3);
        processAllEvents();
    }

    TEST_F(AWindowWindows, mouseMoveEventTriggersMouseMoveEvent)
    {
        sendMouseEvent(5, 10, WM_MOUSEMOVE);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowEnter, 5, 10)).Times(AnyNumber());
        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_Move, 5, 10)).Times(1);
        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowLeave, 5, 10)).Times(AnyNumber());
        processAllEvents();
    }

    TEST_F(AWindowWindows, mouseLeaveEventTriggersWindowLeaveEvent)
    {
        sendMouseEvent(5, 10, WM_MOUSEMOVE); // to simulate a window leave event we have to enter the window first
        sendWindowLeaveEvent();

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowEnter, 5, 10)).Times(AnyNumber());
        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_Move, 5, 10)).Times(1);
        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowLeave, 5, 10)).Times(AtLeast(1));
        processAllEvents();
    }

    TEST_F(AWindowWindows, mouseMoveEventTriggersWindowEnterEvent)
    {
        sendWindowLeaveEvent(); // to simulate a window enter event we have to leave the window first
        sendMouseEvent(5, 10, WM_MOUSEMOVE);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowEnter, 5, 10)).Times(1);
        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_Move, 5, 10)).Times(1);
        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowLeave, _, _)).Times(AnyNumber());
        processAllEvents();
    }
}
