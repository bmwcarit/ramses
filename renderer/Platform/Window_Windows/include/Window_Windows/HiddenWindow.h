//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_HIDDENWINDOW_H
#define RAMSES_HIDDENWINDOW_H

#include "PlatformAbstraction/MinimalWindowsH.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    class HiddenWindow
    {
    public:
        HiddenWindow();
        ~HiddenWindow();

        bool successfullyCreated;
        HWND windowHandle;
        HDC displayHandle;
        WNDCLASSA windowClass;

        static bool InitSimplePixelFormat(HDC tempDisplayHandle);
        static LRESULT WINAPI DummyWindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    };

}

#endif
