//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <windows.h>
#undef min  // revert windows breaking stuff
#undef max

#include "internal/RendererLib/PlatformBase/Window_Base.h"
#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"

#include <string>

namespace ramses::internal
{
    class Window_Windows : public Window_Base
    {
    public:
        Window_Windows(const DisplayConfig& displayConfig, IWindowEventHandler& eventHandler, uint32_t id);
        ~Window_Windows() override;

        virtual bool init() override;

        bool setFullscreen(bool fullscreen) override;
        void handleEvents() override;
        void setTitle(std::string_view title) override;
        bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

        bool hasTitle() const override
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
        KeyModifiers m_keyModifiers;

        bool        m_bLButtonDown;
        bool        m_bRButtonDown;
        bool        m_bMButtonDown;

        int32_t       m_mousePosX;
        int32_t       m_mousePosY;

        bool        m_isMouseTracked;

        std::string m_classname;

        bool        m_userProvidedWindowHandle;

        void generateUniqueClassname();
        static LRESULT WINAPI WindowProcedure(HWND hWnd, ::UINT uMsg, WPARAM wParam, LPARAM lParam);

        void handleSysCommand(WPARAM wParam);
        void handleKeyEvent(uint32_t windowsMsg, WPARAM wParam, LPARAM lParam);
        void handleMouseEvent(EMouseEvent type, int32_t posX, int32_t posY);
        void handleWindowCloseEvent();
        void handleWindowMoveEvent(int32_t posX, int32_t posY);

        bool setVisibility(bool visible);

        static HWND WindowsWindowHandleToHWND(WindowsWindowHandle handle);
        static void PrintError();
    };
}
