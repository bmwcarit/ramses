//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesRendererUtils.h"

namespace ramses
{
    ramses::EMouseEvent RamsesRendererUtils::GetMouseEvent(ramses_internal::EMouseEventType type)
    {
        switch (type)
        {
        case ramses_internal::EMouseEventType_LeftButtonDown:
            return ramses::EMouseEvent_LeftButtonDown;
        case ramses_internal::EMouseEventType_LeftButtonUp:
            return ramses::EMouseEvent_LeftButtonUp;
        case ramses_internal::EMouseEventType_MiddleButtonDown:
            return ramses::EMouseEvent_MiddleButtonDown;
        case ramses_internal::EMouseEventType_MiddleButtonUp:
            return ramses::EMouseEvent_MiddleButtonUp;
        case ramses_internal::EMouseEventType_RightButtonDown:
            return ramses::EMouseEvent_RightButtonDown;
        case ramses_internal::EMouseEventType_RightButtonUp:
            return ramses::EMouseEvent_RightButtonUp;
        case ramses_internal::EMouseEventType_WheelDown:
            return ramses::EMouseEvent_WheelDown;
        case ramses_internal::EMouseEventType_WheelUp:
            return ramses::EMouseEvent_WheelUp;
        case ramses_internal::EMouseEventType_Move:
            return ramses::EMouseEvent_Move;
        case ramses_internal::EMouseEventType_WindowEnter:
            return ramses::EMouseEvent_WindowEnter;
        case ramses_internal::EMouseEventType_WindowLeave:
            return ramses::EMouseEvent_WindowLeave;
        default:
            assert(!"Not a mouse button event type");
            return ramses::EMouseEvent_Invalid;
        }
    }

    ramses::EKeyEvent RamsesRendererUtils::GetKeyEvent(ramses_internal::EKeyEventType type)
    {
        switch (type)
        {
        case ramses_internal::EKeyEventType_Pressed:
            return ramses::EKeyEvent_Pressed;
        case ramses_internal::EKeyEventType_Released:
            return ramses::EKeyEvent_Released;
        default:
            assert(!"Not a key event type");
            return ramses::EKeyEvent_Invalid;
        }
    }

    ramses::EKeyCode RamsesRendererUtils::GetKeyCode(ramses_internal::EKeyCode keyCode)
    {
        switch (keyCode)
        {
        case ramses_internal::EKeyCode_Unknown:
            return ramses::EKeyCode_Unknown;
        case ramses_internal::EKeyCode_A:
            return ramses::EKeyCode_A;
        case ramses_internal::EKeyCode_B:
            return ramses::EKeyCode_B;
        case ramses_internal::EKeyCode_C:
            return ramses::EKeyCode_C;
        case ramses_internal::EKeyCode_D:
            return ramses::EKeyCode_D;
        case ramses_internal::EKeyCode_E:
            return ramses::EKeyCode_E;
        case ramses_internal::EKeyCode_F:
            return ramses::EKeyCode_F;
        case ramses_internal::EKeyCode_G:
            return ramses::EKeyCode_G;
        case ramses_internal::EKeyCode_H:
            return ramses::EKeyCode_H;
        case ramses_internal::EKeyCode_I:
            return ramses::EKeyCode_I;
        case ramses_internal::EKeyCode_J:
            return ramses::EKeyCode_J;
        case ramses_internal::EKeyCode_K:
            return ramses::EKeyCode_K;
        case ramses_internal::EKeyCode_L:
            return ramses::EKeyCode_L;
        case ramses_internal::EKeyCode_M:
            return ramses::EKeyCode_M;
        case ramses_internal::EKeyCode_N:
            return ramses::EKeyCode_N;
        case ramses_internal::EKeyCode_O:
            return ramses::EKeyCode_O;
        case ramses_internal::EKeyCode_P:
            return ramses::EKeyCode_P;
        case ramses_internal::EKeyCode_Q:
            return ramses::EKeyCode_Q;
        case ramses_internal::EKeyCode_R:
            return ramses::EKeyCode_R;
        case ramses_internal::EKeyCode_S:
            return ramses::EKeyCode_S;
        case ramses_internal::EKeyCode_T:
            return ramses::EKeyCode_T;
        case ramses_internal::EKeyCode_U:
            return ramses::EKeyCode_U;
        case ramses_internal::EKeyCode_V:
            return ramses::EKeyCode_V;
        case ramses_internal::EKeyCode_W:
            return ramses::EKeyCode_W;
        case ramses_internal::EKeyCode_X:
            return ramses::EKeyCode_X;
        case ramses_internal::EKeyCode_Y:
            return ramses::EKeyCode_Y;
        case ramses_internal::EKeyCode_Z:
            return ramses::EKeyCode_Z;

        case ramses_internal::EKeyCode_0:
            return ramses::EKeyCode_0;
        case ramses_internal::EKeyCode_1:
            return ramses::EKeyCode_1;
        case ramses_internal::EKeyCode_2:
            return ramses::EKeyCode_2;
        case ramses_internal::EKeyCode_3:
            return ramses::EKeyCode_3;
        case ramses_internal::EKeyCode_4:
            return ramses::EKeyCode_4;
        case ramses_internal::EKeyCode_5:
            return ramses::EKeyCode_5;
        case ramses_internal::EKeyCode_6:
            return ramses::EKeyCode_6;
        case ramses_internal::EKeyCode_7:
            return ramses::EKeyCode_7;
        case ramses_internal::EKeyCode_8:
            return ramses::EKeyCode_8;
        case ramses_internal::EKeyCode_9:
            return ramses::EKeyCode_9;

        case ramses_internal::EKeyCode_NumLock:
            return ramses::EKeyCode_NumLock;
        case ramses_internal::EKeyCode_Numpad_Add:
            return ramses::EKeyCode_Numpad_Add;
        case ramses_internal::EKeyCode_Numpad_Subtract:
            return ramses::EKeyCode_Numpad_Subtract;
        case ramses_internal::EKeyCode_Numpad_Multiply:
            return ramses::EKeyCode_Numpad_Multiply;
        case ramses_internal::EKeyCode_Numpad_Divide:
            return ramses::EKeyCode_Numpad_Divide;
        case ramses_internal::EKeyCode_Numpad_Enter:
            return ramses::EKeyCode_Numpad_Enter;
        case ramses_internal::EKeyCode_Numpad_Decimal:
            return ramses::EKeyCode_Numpad_Decimal;
        case ramses_internal::EKeyCode_Numpad_0:
            return ramses::EKeyCode_Numpad_0;
        case ramses_internal::EKeyCode_Numpad_1:
            return ramses::EKeyCode_Numpad_1;
        case ramses_internal::EKeyCode_Numpad_2:
            return ramses::EKeyCode_Numpad_2;
        case ramses_internal::EKeyCode_Numpad_3:
            return ramses::EKeyCode_Numpad_3;
        case ramses_internal::EKeyCode_Numpad_4:
            return ramses::EKeyCode_Numpad_4;
        case ramses_internal::EKeyCode_Numpad_5:
            return ramses::EKeyCode_Numpad_5;
        case ramses_internal::EKeyCode_Numpad_6:
            return ramses::EKeyCode_Numpad_6;
        case ramses_internal::EKeyCode_Numpad_7:
            return ramses::EKeyCode_Numpad_7;
        case ramses_internal::EKeyCode_Numpad_8:
            return ramses::EKeyCode_Numpad_8;
        case ramses_internal::EKeyCode_Numpad_9:
            return ramses::EKeyCode_Numpad_9;

        case ramses_internal::EKeyCode_Return:
            return ramses::EKeyCode_Return;
        case ramses_internal::EKeyCode_Escape:
            return ramses::EKeyCode_Escape;
        case ramses_internal::EKeyCode_Backspace:
            return ramses::EKeyCode_Backspace;
        case ramses_internal::EKeyCode_Tab:
            return ramses::EKeyCode_Tab;
        case ramses_internal::EKeyCode_Space:
            return ramses::EKeyCode_Space;
        case ramses_internal::EKeyCode_Menu:
            return ramses::EKeyCode_Menu;
        case ramses_internal::EKeyCode_CapsLock:
            return ramses::EKeyCode_CapsLock;
        case ramses_internal::EKeyCode_ShiftLeft:
            return ramses::EKeyCode_ShiftLeft;
        case ramses_internal::EKeyCode_ShiftRight:
            return ramses::EKeyCode_ShiftRight;
        case ramses_internal::EKeyCode_AltLeft:
            return ramses::EKeyCode_AltLeft;
        case ramses_internal::EKeyCode_AltRight:
            return ramses::EKeyCode_AltRight;
        case ramses_internal::EKeyCode_ControlLeft:
            return ramses::EKeyCode_ControlLeft;
        case ramses_internal::EKeyCode_ControlRight:
            return ramses::EKeyCode_ControlRight;
        case ramses_internal::EKeyCode_WindowsLeft:
            return ramses::EKeyCode_WindowsLeft;
        case ramses_internal::EKeyCode_WindowsRight:
            return ramses::EKeyCode_WindowsRight;

        case ramses_internal::EKeyCode_F1:
            return ramses::EKeyCode_F1;
        case ramses_internal::EKeyCode_F2:
            return ramses::EKeyCode_F2;
        case ramses_internal::EKeyCode_F3:
            return ramses::EKeyCode_F3;
        case ramses_internal::EKeyCode_F4:
            return ramses::EKeyCode_F4;
        case ramses_internal::EKeyCode_F5:
            return ramses::EKeyCode_F5;
        case ramses_internal::EKeyCode_F6:
            return ramses::EKeyCode_F6;
        case ramses_internal::EKeyCode_F7:
            return ramses::EKeyCode_F7;
        case ramses_internal::EKeyCode_F8:
            return ramses::EKeyCode_F8;
        case ramses_internal::EKeyCode_F9:
            return ramses::EKeyCode_F9;
        case ramses_internal::EKeyCode_F10:
            return ramses::EKeyCode_F10;
        case ramses_internal::EKeyCode_F11:
            return ramses::EKeyCode_F11;
        case ramses_internal::EKeyCode_F12:
            return ramses::EKeyCode_F12;

        case ramses_internal::EKeyCode_PrintScreen:
            return ramses::EKeyCode_PrintScreen;
        case ramses_internal::EKeyCode_ScrollLock:
            return ramses::EKeyCode_ScrollLock;
        case ramses_internal::EKeyCode_Pause:
            return ramses::EKeyCode_Pause;

        case ramses_internal::EKeyCode_Insert:
            return ramses::EKeyCode_Insert;
        case ramses_internal::EKeyCode_Home:
            return ramses::EKeyCode_Home;
        case ramses_internal::EKeyCode_PageUp:
            return ramses::EKeyCode_PageUp;
        case ramses_internal::EKeyCode_Delete:
            return ramses::EKeyCode_Delete;
        case ramses_internal::EKeyCode_End:
            return ramses::EKeyCode_End;
        case ramses_internal::EKeyCode_PageDown:
            return ramses::EKeyCode_PageDown;

        case ramses_internal::EKeyCode_Right:
            return ramses::EKeyCode_Right;
        case ramses_internal::EKeyCode_Left:
            return ramses::EKeyCode_Left;
        case ramses_internal::EKeyCode_Up:
            return ramses::EKeyCode_Up;
        case ramses_internal::EKeyCode_Down:
            return ramses::EKeyCode_Down;

        case ramses_internal::EKeyCode_Minus:
            return ramses::EKeyCode_Minus;
        case ramses_internal::EKeyCode_Equals:
            return ramses::EKeyCode_Equals;
        case ramses_internal::EKeyCode_LeftBracket:
            return ramses::EKeyCode_LeftBracket;
        case ramses_internal::EKeyCode_RightBracket:
            return ramses::EKeyCode_RightBracket;
        case ramses_internal::EKeyCode_Backslash:
            return ramses::EKeyCode_Backslash;
        case ramses_internal::EKeyCode_Semicolon:
            return ramses::EKeyCode_Semicolon;
        case ramses_internal::EKeyCode_Comma:
            return ramses::EKeyCode_Comma;
        case ramses_internal::EKeyCode_Period:
            return ramses::EKeyCode_Period;
        case ramses_internal::EKeyCode_Slash:
            return ramses::EKeyCode_Slash;
        case ramses_internal::EKeyCode_Apostrophe:
            return ramses::EKeyCode_Apostrophe;
        case ramses_internal::EKeyCode_Grave:
            return ramses::EKeyCode_Grave;
        case ramses_internal::EKeyCode_NumberSign:
            return ramses::EKeyCode_NumberSign;

        default:
            assert(!"Not a valid key code ");
            return ramses::EKeyCode_Unknown;
        }
    }
}
