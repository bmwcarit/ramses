//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "windows.h"
#include "internal/Platform/Windows/HiddenWindow.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{

    HiddenWindow::HiddenWindow()
        : successfullyCreated(false)
        , windowHandle(0)
        , displayHandle(0)
        , windowClass(WNDCLASSA())
    {
        const char* classname = "TmpClassName";
        windowClass.lpszClassName = classname;
        windowClass.style = CS_OWNDC;
        windowClass.hInstance = GetModuleHandle(NULL);
        windowClass.lpfnWndProc = reinterpret_cast<WNDPROC>(DummyWindowProcedure);
        windowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        windowClass.hCursor = 0;

        const ATOM atom = RegisterClassA(&windowClass);
        if (0 == atom)
        {
            GetLastError();
            return;
        }

        RECT windowRect;
        windowRect.left = 0;
        windowRect.top = 0;
        windowRect.right = 1280;
        windowRect.bottom = 480;
        AdjustWindowRectEx(&windowRect, 0, FALSE, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

        windowHandle = CreateWindowExA(
            0,
            windowClass.lpszClassName,
            "TmpClassName",
            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            windowRect.left,
            windowRect.top,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            NULL,
            NULL,
            windowClass.hInstance,
            NULL);

        if (0 == windowHandle)
        {
            return;
        }

        EnableWindow(windowHandle, TRUE);
        ShowWindow(windowHandle, SW_HIDE);
        displayHandle = GetDC(windowHandle);

        if (0 == displayHandle)
        {
            return;
        }

        successfullyCreated = true;
    }

    HiddenWindow::~HiddenWindow()
    {
        EnableWindow(windowHandle, FALSE);
        DestroyWindow(windowHandle);

        UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
    }

    bool HiddenWindow::InitSimplePixelFormat(HDC tempDisplayHandle)
    {
        static PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW |
            PFD_SUPPORT_OPENGL |
            PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            24,
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            24,
            8,
            0,
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        const auto pixelFormatResult = ChoosePixelFormat(tempDisplayHandle, &pfd);
        if (!pixelFormatResult) // Did Windows Find A Matching Pixel Format?
        {
            LOG_ERROR(CONTEXT_RENDERER, "HiddenWindow::InitSimplePixelFormat:  no suitable pixelformat found");
            return false;
        }

        if (!SetPixelFormat(tempDisplayHandle, pixelFormatResult, &pfd))   // Are We Able To Set The Pixel Format?
        {
            LOG_ERROR(CONTEXT_RENDERER, "HiddenWindow::InitSimplePixelFormat:  can't set the pixelformat");
            return false;
        }

        return true;
    }

    LRESULT WINAPI HiddenWindow::DummyWindowProcedure(HWND /*hWnd*/, UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
    {
        return 1;
    }
}
