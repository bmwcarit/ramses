//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_X11_H
#define RAMSES_WINDOW_X11_H

#include "Platform_Base/Window_Base.h"
#include "RendererAPI/IWindowEventHandler.h"

// This dependency is needed due to the broken window abstraction of EGL
// EGL acts as if it does not care about the window type (X11, wayland etc),
// but it does - internally it typedefs its native handles to native handles
// of the underlying window system
//#include "EGL/egl.h"

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#undef Bool // Xlib.h defines Bool as int - this collides with ramses_internal::Bool
#undef Status
#undef None

namespace ramses_internal
{
    struct X11WindowData
    {
        X11WindowData()
            : window(0)
            , display(nullptr)
            , screen(0)
        {
        }

        ::Window window;
        ::Display* display;
        long screen;
        XVisualInfo visual;
        Colormap colormap;
    };

    class Window_X11 final : public Window_Base
    {
    public:
        Window_X11(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);
        ~Window_X11() override;

        virtual bool init() override;

        void handleEvents() override final;

        ::Display* getNativeDisplayHandle() const;
        ::Window getNativeWindowHandle() const;

        bool hasTitle() const override final
        {
            return !m_fullscreen;
        }

        // public as it is used by tests
        static EKeyCode convertKeySymbolIntoRamsesKeyCode(UInt32 virtualKeyCode);
    private:
        Bool setFullscreen(Bool fullscreen) override final;
        void setTitle(const String& title) override final;

        X11WindowData m_X11WindowData;

        UInt32 m_keyModifiers;

        Bool m_bLButtonDown;
        Bool m_bRButtonDown;
        Bool m_bMButtonDown;
        X11WindowHandle m_userProvidedWindowHandle;
    };
}

#endif
