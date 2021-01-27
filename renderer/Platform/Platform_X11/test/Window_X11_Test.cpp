//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "Platform_X11/Window_X11.h"
#include "WindowEventHandlerMock.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/EKeyModifier.h"

using namespace testing;

namespace ramses_internal
{
    class WindowX11 : public testing::Test
    {
    public:
        WindowX11()
            : window(config, eventHandlerMock, 0)
        {
        }

        virtual void SetUp() override
        {
            ASSERT_TRUE(window.init());
            proceseAllEvents();
        }

        void proceseAllEvents()
        {
            for (UInt32 i = 0; i < 10; i++) // enforce handling of all enqueued window events which will trigger our event handler
                window.handleEvents();
        }

        void sendKeyEvent(UInt32 keycode)
        {
            Display* display = window.getNativeDisplayHandle();
            assert(display);

            Window winFocus = window.getNativeWindowHandle();

            XEvent event;
            event.xkey.display = display;
            event.xkey.window = winFocus;
            event.xkey.root = winFocus;
            event.xkey.subwindow = 0;
            event.xkey.time = 0;
            event.xkey.x = 1;
            event.xkey.y = 1;
            event.xkey.x_root = 1;
            event.xkey.y_root = 1;
            event.xkey.same_screen = True;
            event.xkey.keycode = XKeysymToKeycode(display, keycode);
            event.xkey.state = 0; // modifier
            event.xkey.type = KeyPress;
            XSendEvent(display, winFocus, True, KeyPressMask, &event);

            event.xkey.type = KeyRelease;
            XSendEvent(display, winFocus, True, KeyPressMask, &event);

            XSync(display, static_cast<int>(false));
        }

        void sendMouseMoveEvent(int posX, int posY)
        {
            Display* display = window.getNativeDisplayHandle();
            assert(display);

            Window winFocus = window.getNativeWindowHandle();

            XEvent event;
            event.xmotion.display = display;
            event.xmotion.window = winFocus;
            event.xmotion.root = winFocus;
            event.xmotion.subwindow = 0;
            event.xmotion.time = 0;
            event.xmotion.x = posX;
            event.xmotion.y = posY;
            event.xmotion.x_root = 1;
            event.xmotion.y_root = 1;
            event.xmotion.same_screen = True;
            event.xmotion.is_hint = 0;
            event.xmotion.state = 0;
            event.xmotion.type = MotionNotify;
            XSendEvent(display, winFocus, True, PointerMotionMask, &event);

            XSync(display, static_cast<int>(false));
        }

        void sendCrossingEvent(Bool enterEvent, int posX, int posY)
        {
            Display* display = window.getNativeDisplayHandle();
            assert(display);

            Window winFocus = window.getNativeWindowHandle();

            XEvent event;
            event.xcrossing.display = display;
            event.xcrossing.window = winFocus;
            event.xcrossing.root = winFocus;
            event.xcrossing.subwindow = 0;
            event.xcrossing.time = 0;
            event.xcrossing.x = posX;
            event.xcrossing.y = posY;
            event.xcrossing.x_root = 1;
            event.xcrossing.y_root = 1;
            event.xcrossing.same_screen = True;
            event.xcrossing.mode = NotifyNormal;
            event.xcrossing.detail = 0;
            event.xcrossing.state = 0;
            event.xcrossing.focus = static_cast<int>(false);
            event.xcrossing.type = enterEvent ? EnterNotify : LeaveNotify;
            XSendEvent(display, winFocus, True, enterEvent ? EnterWindowMask : LeaveWindowMask, &event);

            XSync(display, static_cast<int>(false));
        }

        void sendMouseButtonEvent(int posX, int posY, int buttonType, Bool pressed)
        {
            Display* display = window.getNativeDisplayHandle();
            assert(display);

            Window winFocus = window.getNativeWindowHandle();

            XEvent event;
            event.xbutton.display = display;
            event.xbutton.window = winFocus;
            event.xbutton.root = winFocus;
            event.xbutton.subwindow = 0;
            event.xbutton.time = 0;
            event.xbutton.x = posX;
            event.xbutton.y = posY;
            event.xbutton.x_root = 1;
            event.xbutton.y_root = 1;
            event.xbutton.same_screen = True;
            event.xbutton.state = 0;
            event.xbutton.button = buttonType;
            event.xbutton.type = pressed ? ButtonPress : ButtonRelease;
            XSendEvent(display, winFocus, True, pressed ? ButtonPressMask : ButtonReleaseMask, &event);

            XSync(display, static_cast<int>(false));
        }

        void testKeyCode(UInt32 virtualKeyCode, EKeyCode expectedRamesKeyCode, UInt32 expectedModifier = EKeyModifier_NoModifier)
        {
            sendKeyEvent(virtualKeyCode);

            Sequence seq;
            EXPECT_CALL(eventHandlerMock, onKeyEvent(EKeyEventType_Pressed, expectedModifier, expectedRamesKeyCode)).Times(1).InSequence(seq);
            EXPECT_CALL(eventHandlerMock, onKeyEvent(EKeyEventType_Released, EKeyModifier_NoModifier, expectedRamesKeyCode)).Times(1).InSequence(seq);
            proceseAllEvents();
        }

        DisplayConfig config;
        NiceMock<WindowEventHandlerMock> eventHandlerMock;
        Window_X11 window;
    };

    TEST(Window_X11, convertVirtualKeyCodeToRamsesKeyCode)
    {
        // only test special calculation code for consecutive key codes
        EXPECT_EQ(Window_X11::convertKeySymbolIntoRamsesKeyCode(XK_a),      EKeyCode_A);
        EXPECT_EQ(Window_X11::convertKeySymbolIntoRamsesKeyCode(XK_A),      EKeyCode_A);
        EXPECT_EQ(Window_X11::convertKeySymbolIntoRamsesKeyCode(XK_1),      EKeyCode_1);
        EXPECT_EQ(Window_X11::convertKeySymbolIntoRamsesKeyCode(XK_KP_2),   EKeyCode_Numpad_2);
        EXPECT_EQ(Window_X11::convertKeySymbolIntoRamsesKeyCode(XK_F4),     EKeyCode_F4);
    }

    TEST_F(WindowX11, singleKeyPressEventTriggersKeyPressedEventWithCorrectKeyCode)
    {
        testKeyCode(XK_A,        EKeyCode_A);
        testKeyCode(XK_a,        EKeyCode_A);
        testKeyCode(XK_0,        EKeyCode_0);
        testKeyCode(XK_Escape,   EKeyCode_Escape);
        testKeyCode(XK_Return,   EKeyCode_Return);
        testKeyCode(XK_Tab,      EKeyCode_Tab);
        testKeyCode(XK_F1,       EKeyCode_F1);
        testKeyCode(XK_space,    EKeyCode_Space);
        testKeyCode(XK_Shift_L,  EKeyCode_ShiftLeft,     EKeyModifier_Shift);
    }

    TEST_F(WindowX11, leftMouseButtonDownEventTriggersLeftButtonDownEvent)
    {
        sendMouseButtonEvent(5, 10, Button1, true);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonDown, 5, 10)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, leftMouseButtonUpEventTriggersLeftButtonUpEvent)
    {
        sendMouseButtonEvent(5, 10, Button1, false);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonUp, 5, 10)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, rightMouseButtonDownEventTriggersRightButtonDownEvent)
    {
        sendMouseButtonEvent(6, 11, Button3, true);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_RightButtonDown, 6, 11)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, rightMouseButtonUpEventTriggersRightButtonUpEvent)
    {
        sendMouseButtonEvent(6, 11, Button3, false);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_RightButtonUp, 6, 11)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, middleMouseButtonDownEventTriggersMiddleButtonDownEvent)
    {
        sendMouseButtonEvent(6, 11, Button2, true);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_MiddleButtonDown, 6, 11)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, middleMouseButtonUpEventTriggersMiddleButtonUpEvent)
    {
        sendMouseButtonEvent(6, 11, Button2, false);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_MiddleButtonUp, 6, 11)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, positiveMouseWheelEventTriggersMouseWheelUpEvent)
    {
        sendMouseButtonEvent(7, 12, Button4, true);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WheelUp, 7, 12)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, negativeMouseWheelEventTriggersMouseWheelDownEvent)
    {
        sendMouseButtonEvent(7, 12, Button5, true);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WheelDown, 7, 12)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, MouseMoveEventTriggersMouseMoveEvent)
    {
        sendMouseMoveEvent(5, 10);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_Move, 5, 10)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, leaveEventTriggersWindowLeaveEvent)
    {
        sendCrossingEvent(false, 30, 50);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowLeave, 30, 50)).Times(1);
        proceseAllEvents();
    }

    TEST_F(WindowX11, enterEventTriggersWindowEnterEvent)
    {
        sendCrossingEvent(true, 10, 20);

        EXPECT_CALL(eventHandlerMock, onMouseEvent(EMouseEventType_WindowEnter, 10, 20)).Times(1);
        proceseAllEvents();
    }
}
