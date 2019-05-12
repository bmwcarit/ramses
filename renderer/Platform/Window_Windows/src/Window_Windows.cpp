//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Windows/Window_Windows.h"
#include "RendererAPI/IWindowEventHandler.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/EKeyModifier.h"
#include "Utils/LogMacros.h"

#include <WindowsX.h>


namespace ramses_internal
{
    EKeyCode Window_Windows::convertVirtualKeyCodeIntoRamsesKeyCode(WPARAM virtualKeyCode, LPARAM lParam)
    {
        // number keys (0-9)
        if (virtualKeyCode >= 0x30 && virtualKeyCode <= 0x39)
            return static_cast<EKeyCode>(virtualKeyCode - 0x30 + EKeyCode_0);

        // numpad number keys (0-9)
        if (virtualKeyCode >= 0x60 && virtualKeyCode <= 0x69)
            return static_cast<EKeyCode>(virtualKeyCode - 0x60 + EKeyCode_Numpad_0);

        // character keys (A-Z)
        if (virtualKeyCode >= 0x41 && virtualKeyCode <= 0x5A)
            return static_cast<EKeyCode>(virtualKeyCode - 0x41 + EKeyCode_A);

        // function keys (F1-F24)
        if (virtualKeyCode >= 0x70 && virtualKeyCode <= 0x87)
            return static_cast<EKeyCode>(virtualKeyCode - 0x70 + EKeyCode_F1);

        const Bool isExtendedKey = (lParam & (1 << 24)) != 0;
        const UInt8 nScanCode = (lParam >> 16) & 0xFF;

        // rest of keys
        switch (virtualKeyCode)
        {
        case VK_LWIN:
            return EKeyCode_WindowsLeft;
        case VK_RWIN:
            return EKeyCode_WindowsRight;
        case VK_CAPITAL:
            return EKeyCode_CapsLock;
        case VK_NUMLOCK:
            return EKeyCode_NumLock;
        case VK_CONTROL:
            return isExtendedKey ? EKeyCode_ControlRight : EKeyCode_ControlLeft;
        case VK_SHIFT:
            return nScanCode == 54 ? EKeyCode_ShiftRight : EKeyCode_ShiftLeft;
        case VK_SPACE:
            return EKeyCode_Space;
        case VK_ESCAPE:
            return EKeyCode_Escape;
        case VK_TAB:
            return EKeyCode_Tab;
        case VK_BACK:
            return EKeyCode_Backspace;
        case VK_MENU:
            return isExtendedKey ? EKeyCode_AltRight : EKeyCode_AltLeft;
        case VK_SCROLL:
            return EKeyCode_ScrollLock;
        case VK_SNAPSHOT:
            return EKeyCode_PrintScreen;
        case VK_PAUSE:
            return EKeyCode_Pause;
        case VK_RETURN:
            return isExtendedKey ? EKeyCode_Numpad_Enter : EKeyCode_Return;
        case VK_INSERT:
            return isExtendedKey ? EKeyCode_Insert : EKeyCode_Numpad_0;
        case VK_HOME:
            return isExtendedKey ? EKeyCode_Home : EKeyCode_Numpad_7;
        case VK_NEXT:
            return isExtendedKey ? EKeyCode_PageDown : EKeyCode_Numpad_3;
        case VK_DELETE:
            return isExtendedKey ? EKeyCode_Delete : EKeyCode_Numpad_Decimal;
        case VK_END:
            return isExtendedKey ? EKeyCode_End : EKeyCode_Numpad_1;
        case VK_PRIOR:
            return isExtendedKey ? EKeyCode_PageUp : EKeyCode_Numpad_9;
        case VK_LEFT:
            return isExtendedKey ? EKeyCode_Left : EKeyCode_Numpad_4;
        case VK_RIGHT:
            return isExtendedKey ? EKeyCode_Right : EKeyCode_Numpad_6;
        case VK_UP:
            return isExtendedKey ? EKeyCode_Up : EKeyCode_Numpad_8;
        case VK_DOWN:
            return isExtendedKey ? EKeyCode_Down : EKeyCode_Numpad_2;
        case VK_CLEAR:
            return EKeyCode_Numpad_5;
        case VK_ADD:
            return EKeyCode_Numpad_Add;
        case VK_SUBTRACT:
            return EKeyCode_Numpad_Subtract;
        case VK_MULTIPLY:
            return EKeyCode_Numpad_Multiply;
        case VK_DIVIDE:
            return EKeyCode_Numpad_Divide;
        case VK_DECIMAL:
            return EKeyCode_Numpad_Decimal;
        case VK_OEM_1:
            return EKeyCode_LeftBracket;
        case VK_OEM_2:
            return EKeyCode_NumberSign;
        case VK_OEM_3:
            return EKeyCode_Semicolon;
        case VK_OEM_4:
            return EKeyCode_Minus;
        case VK_OEM_5:
            return EKeyCode_Grave;
        case VK_OEM_6:
            return EKeyCode_Equals;
        case VK_OEM_7:
            return EKeyCode_Apostrophe;
        case VK_OEM_102:
            return EKeyCode_Backslash;
        case VK_OEM_PLUS:
            return EKeyCode_RightBracket;
        case VK_OEM_COMMA:
            return EKeyCode_Comma;
        case VK_OEM_MINUS:
            return EKeyCode_Slash;
        case VK_OEM_PERIOD:
            return EKeyCode_Period;
        case VK_APPS:
            return EKeyCode_Menu;
        default:
            return EKeyCode_Unknown;
        }
    }

    Bool TrackMouse(HWND hwnd)
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = 0;
        tme.hwndTrack = hwnd;
        return (TrackMouseEvent(&tme) == TRUE);
    }

    Window_Windows::Window_Windows(const DisplayConfig& displayConfig, IWindowEventHandler& eventHandler, UInt32 id)
        : Window_Base(displayConfig, eventHandler, id)
        , m_displayHandle(0)
        , m_windowHandle(WindowsWindowHandleToHWND(displayConfig.getWindowsWindowHandle()))
        , m_windowClass(WNDCLASSA())
        , m_keyModifiers(0)
        , m_bLButtonDown(false)
        , m_bRButtonDown(false)
        , m_bMButtonDown(false)
        , m_mousePosX(-1)
        , m_mousePosY(-1)
        , m_isMouseTracked(false)
        , m_classname("")
        , m_userProvidedWindowHandle(WindowsWindowHandleToHWND(InvalidWindowsWindowHandle) != m_windowHandle)
    {
    }

    void Window_Windows::generateUniqueClassname()
    {
        m_classname = getTitle() + Guid(true).toString().c_str();
        assert(m_classname.getLength() < 255);
    }

    Bool Window_Windows::init()
    {
        generateUniqueClassname();

        m_windowClass.lpszClassName = m_classname.c_str();
        m_windowClass.style = CS_OWNDC; // | CS_NOCLOSE;
        m_windowClass.hInstance = GetModuleHandle(NULL);
        m_windowClass.lpfnWndProc = reinterpret_cast<WNDPROC>(Window_Windows::WindowProcedure);
        m_windowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        m_windowClass.hCursor = 0;

        const ATOM atom = RegisterClassA(&m_windowClass);
        if (0 == atom)
        {
            PrintError();
            return false;
        }

        if (m_fullscreen)
        {
            m_windowStyle = WS_POPUP | WS_MAXIMIZE;
            m_windowEXStyle = WS_EX_TOOLWINDOW;
        }
        else
        {
            if( m_borderless )
            {
                m_windowStyle = WS_POPUP ;
                m_windowEXStyle = WS_EX_TOOLWINDOW;
            }
            else
            {
                m_windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

                if (m_resizable)
                {
                    m_windowStyle |= WS_OVERLAPPEDWINDOW;
                }
                else
                {
                    m_windowStyle |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
                }
                m_windowEXStyle = 0;
            }

        }

        RECT windowRect;
        windowRect.left = getPosX();
        windowRect.top = getPosY();
        windowRect.right = getPosX() + getWidth();
        windowRect.bottom = getPosY() + getHeight();

        BOOL result = AdjustWindowRectEx(&windowRect, m_windowStyle, FALSE, m_windowEXStyle);
        if (result != TRUE)
        {
            PrintError();
            return false;
        }

        if (!m_userProvidedWindowHandle)
        {
            m_windowHandle = CreateWindowExA(m_windowEXStyle,
                                             m_windowClass.lpszClassName,
                                             m_classname.c_str(),
                                             m_windowStyle,
                                             windowRect.left,
                                             windowRect.top,
                                             windowRect.right - windowRect.left,
                                             windowRect.bottom - windowRect.top,
                                             NULL,
                                             NULL,
                                             m_windowClass.hInstance,
                                             NULL);
        }

        if (m_windowHandle != 0)
        {
            //WARNING! This works only in 64 bit systems. For 32 bit, GWLP_USERDATA should be exchanged with GWL_USERDATA
            SetWindowLongPtr(m_windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            setVisibility(true);
        }
        else
        {
            PrintError();
            return false;
        }

        m_displayHandle = GetDC(m_windowHandle);

        if (0 == m_displayHandle)
        {
            PrintError();
            return false;
        }

        setFullscreen(m_fullscreen);

        LOG_INFO(CONTEXT_RENDERER, "Surface_Windows::Surface_Windows:  created window '" << getTitle() << "', size = " << getWidth() << "x" << getHeight() << ", posx = " << getPosX() << ", posy = " << getPosY());

        return true;
    }

    Window_Windows::~Window_Windows()
    {
        if (0 != m_windowHandle && !m_userProvidedWindowHandle)
        {
            DestroyWindow(m_windowHandle);
            UnregisterClassA(m_windowClass.lpszClassName, m_windowClass.hInstance);
        }
    }

    void Window_Windows::PrintError()
    {
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID != 0)
        {
            LPSTR messageBuffer = nullptr;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);

            String errorMessage(messageBuffer);
            LOG_ERROR(CONTEXT_RENDERER, "Windows API error: " << errorMessage);

            //Free the buffer.
            LocalFree(messageBuffer);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Windows API reported error, but refuses to deliver error code... Please call Bill Gates and ask for refund.");
        }
    }

    HDC Window_Windows::getNativeDisplayHandle()
    {
        return m_displayHandle;
    }

    HWND Window_Windows::getNativeWindowHandle()
    {
        return m_windowHandle;
    }

    Bool Window_Windows::setFullscreen(Bool fullscreen)
    {
        assert(0 != m_windowHandle);

        int fullscreenMode = SW_SHOWNORMAL;
        if (fullscreen)
        {
            fullscreenMode = SW_SHOWMAXIMIZED;
        }

        ShowWindow(m_windowHandle, fullscreenMode);

        return true;
    }

    Bool Window_Windows::setVisibility(Bool visible)
    {
        assert(0 != m_windowHandle);

        if (visible)
        {
            EnableWindow(m_windowHandle, TRUE);
            ShowWindow(m_windowHandle, SW_SHOWNORMAL);
        }
        else
        {
            EnableWindow(m_windowHandle, FALSE);
            ShowWindow(m_windowHandle, SW_HIDE);
        }

        return true;
    }

    void Window_Windows::handleEvents()
    {
        MSG msg;
        const INT gotMsg = (PeekMessage(&msg, m_windowHandle, 0, 0, PM_REMOVE) != 0);
        if (gotMsg != 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void Window_Windows::handleKeyEvent(UInt32 windowsMsg, WPARAM wParam, LPARAM lParam)
    {
        const Bool keyPressed = WM_KEYDOWN == windowsMsg || WM_SYSKEYDOWN == windowsMsg;
        const Bool keyReleased = WM_KEYUP == windowsMsg || WM_SYSKEYUP == windowsMsg;
        if (!keyPressed && !keyReleased)
        {
            LOG_WARN(CONTEXT_RENDERER, "invalid handle key event: " << windowsMsg);
            return;
        }

        EKeyModifier keyModifier = EKeyModifier_NoModifier;
        switch (wParam)
        {
        case VK_SHIFT:
            keyModifier = EKeyModifier_Shift;
            break;
        case VK_CONTROL:
            keyModifier = EKeyModifier_Ctrl;
            break;
        case VK_MENU:
            keyModifier = EKeyModifier_Alt;
            break;
        }

        if (keyPressed)
        {
            m_keyModifiers |= keyModifier;
        }
        else if (keyReleased)
        {
            m_keyModifiers &= ~keyModifier;
        }

        const EKeyCode keyCode = convertVirtualKeyCodeIntoRamsesKeyCode(wParam, lParam);
        m_eventHandler.onKeyEvent(keyPressed ? EKeyEventType_Pressed : EKeyEventType_Released, m_keyModifiers, keyCode);
    }

    LRESULT WINAPI Window_Windows::WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT  lRet = 1;
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        //WARNING! This works only in 64 bit systems. For 32 bit, GWLP_USERDATA should be exchanged with GWL_USERDATA
        Window_Windows* window = reinterpret_cast<Window_Windows*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (uMsg)
        {
        case WM_MOUSELEAVE:
            if (NULL == window)
                break;

            window->m_isMouseTracked = false;
            window->handleMouseEvent(EMouseEventType_WindowLeave, window->m_mousePosX, window->m_mousePosY);
            break;
        case WM_MOUSEMOVE:
            if (NULL == window)
                break;

            if (!window->m_isMouseTracked)
            {
                window->m_isMouseTracked = TrackMouse(hWnd);
                window->handleMouseEvent(EMouseEventType_WindowEnter, x, y);
            }

            window->m_mousePosX = x;
            window->m_mousePosY = y;
            window->handleMouseEvent(EMouseEventType_Move, x, y);

            break;
        case WM_LBUTTONDOWN:
        {
            if (NULL != window)
            {
                window->m_bLButtonDown = true;
                window->handleMouseEvent(EMouseEventType_LeftButtonDown, x, y);
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            if (NULL != window)
            {
                window->m_bLButtonDown = false;
                window->handleMouseEvent(EMouseEventType_LeftButtonUp, x, y);
            }
        }
        break;
        case WM_RBUTTONDOWN:
        {
            if (NULL != window)
            {
                window->m_bRButtonDown = true;
                window->handleMouseEvent(EMouseEventType_RightButtonDown, x, y);
            }
        }
        break;
        case WM_RBUTTONUP:
        {
            if (NULL != window)
            {
                window->m_bRButtonDown = false;
                window->handleMouseEvent(EMouseEventType_RightButtonUp, x, y);
            }
        }
        break;
        case WM_MBUTTONDOWN:
        {
            if (NULL != window)
            {
                window->m_bMButtonDown = true;
                window->handleMouseEvent(EMouseEventType_MiddleButtonDown, x, y);
            }
        }
        break;
        case WM_MBUTTONUP:
        {
            if (NULL != window)
            {
                window->m_bMButtonDown = false;
                window->handleMouseEvent(EMouseEventType_MiddleButtonUp, x, y);
            }
        }
        break;
        case WM_MOUSEWHEEL:
        {
            if (NULL != window)
            {
                short totalDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                EMouseEventType event = (totalDelta > 0) ? EMouseEventType_WheelUp : EMouseEventType_WheelDown;

                totalDelta = static_cast<short>(abs(totalDelta));
                totalDelta -= totalDelta % WHEEL_DELTA; // just in case..

                // eat-up all distance scrolled
                while (totalDelta)
                {
                    window->handleMouseEvent(event, x, y);
                    totalDelta -= WHEEL_DELTA;
                }
            }
        }
        break;
        case WM_SIZE:
        {
            if (NULL != window && window->m_resizable)
            {
                window->m_height = HIWORD(lParam);
                window->m_width = LOWORD(lParam);
                (window->eventHandler).onResize(m_width, m_height);
            }
        }
        break;
        case WM_MOVE:
        {
            if (NULL != window)
            {
                window->m_posX = GET_X_LPARAM(lParam);
                window->m_posY = GET_Y_LPARAM(lParam);
            }
        }
        break;
        case WM_SETFOCUS:
        {
            // ignore event
        }
        break;
        case WM_KILLFOCUS:
        {
            // ignore event
        }
        break;
        case WM_SYSCOMMAND:

            lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (NULL != window)
            {
                window->handleKeyEvent(uMsg, wParam, lParam);
            }
            break;
        case WM_CLOSE:
            if (NULL != window)
            {
                window->handleWindowCloseEvent();
            }
            break;
        default:
            lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
        }

        return lRet;
    }

    void Window_Windows::handleMouseEvent(EMouseEventType type, Int32 posX, Int32 posY)
    {
        if (m_bLButtonDown || m_bMButtonDown || m_bRButtonDown)
        {
            SetCapture(m_windowHandle);
        }
        else
        {
            ReleaseCapture();
        }

        m_eventHandler.onMouseEvent(type, posX, posY);
    }

    void Window_Windows::handleWindowCloseEvent()
    {
        m_eventHandler.onClose();
    }

    void Window_Windows::setTitle(const String& title)
    {
        Window_Base::setTitle(title);
        SetWindowText(m_windowHandle, m_windowName.c_str());
    }

    HWND Window_Windows::WindowsWindowHandleToHWND(WindowsWindowHandle handle)
    {
        static_assert(sizeof(HWND) == sizeof(WindowsWindowHandle::BaseType), "WindowsWindowHandle size mismatch");

        HWND result;
        WindowsWindowHandle::BaseType handleValue = handle.getValue();
        memcpy_s(&result, sizeof(result), &handleValue, sizeof(handleValue));
        return result;
    }
}
