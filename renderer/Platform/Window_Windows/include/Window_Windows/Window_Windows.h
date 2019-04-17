//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_WINDOWS_H
#define RAMSES_WINDOW_WINDOWS_H

#include <windows.h>
#undef min  // revert windows breaking stuff
#undef max

#include "Platform_Base/Window_Base.h"
#include "RendererAPI/IWindowEventHandler.h"

namespace ramses_internal
{
    class Window_Windows : public Window_Base
    {
    public:
        Window_Windows(const DisplayConfig& displayConfig, IWindowEventHandler& eventHandler, UInt32 id);
        ~Window_Windows() override;

        // Can be overridden for special windows (used by RAMSES Studio with window provided by Qt)
        virtual Bool init();

        Bool setFullscreen(Bool fullscreen) override;
        void handleEvents() override;
        void setTitle(const String& title) override;

        bool hasTitle() const override final
        {
            return !m_fullscreen;
        }

        // Platform specific stuff, used by other platform specific classes
        HDC getNativeDisplayHandle();
        HWND getNativeWindowHandle();

        // public as it is used by tests
        static EKeyCode convertVirtualKeyCodeIntoRamsesKeyCode(WPARAM virtualKeyCode, LPARAM lParam);
    protected:
        HDC         m_displayHandle;
        HWND        m_windowHandle;
    private:
        WNDCLASSA   m_windowClass;
        DWORD       m_windowStyle;
        DWORD       m_windowEXStyle;
        UInt32      m_keyModifiers;

        Bool        m_bLButtonDown;
        Bool        m_bRButtonDown;
        Bool        m_bMButtonDown;

        Int32       m_mousePosX;
        Int32       m_mousePosY;

        Bool        m_isMouseTracked;

        String      m_classname;

        Bool        m_userProvidedWindowHandle;

        void generateUniqueClassname();
        static LRESULT WINAPI WindowProcedure(HWND hWnd, ::UINT uMsg, WPARAM wParam, LPARAM lParam);

        void handleSysCommand(WPARAM wParam);
        void handleKeyEvent(UInt32 windowsMsg, WPARAM wParam, LPARAM lParam);
        void handleMouseEvent(EMouseEventType type, Int32 posX, Int32 posY);
        void handleWindowCloseEvent();

        Bool setVisibility(Bool visible);

        static HWND WindowsWindowHandleToHWND(WindowsWindowHandle handle);
        static void PrintError();
    };
}

#endif
