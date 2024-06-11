//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/X11/Window_X11.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/Warnings.h"
#include <array>

namespace ramses::internal
{
    EKeyCode Window_X11::convertKeySymbolIntoRamsesKeyCode(uint32_t virtualKeyCode)
    {
        // number keys (0-9)
        if (virtualKeyCode >= XK_0 && virtualKeyCode <= XK_9)
            return static_cast<EKeyCode>(virtualKeyCode - XK_0 + EKeyCode_0);

        // numpad number keys (0-9)
        if (virtualKeyCode >= XK_KP_0 && virtualKeyCode <= XK_KP_9)
            return static_cast<EKeyCode>(virtualKeyCode - XK_KP_0 + EKeyCode_Numpad_0);

        // character keys (A-Z)
        if (virtualKeyCode >= XK_A && virtualKeyCode <= XK_Z)
            return static_cast<EKeyCode>(virtualKeyCode - XK_A + EKeyCode_A);

        // character keys (a-z)
        if (virtualKeyCode >= XK_a && virtualKeyCode <= XK_z)
            return static_cast<EKeyCode>(virtualKeyCode - XK_a + EKeyCode_A);

        // function keys (F1-F12)
        if (virtualKeyCode >= XK_F1 && virtualKeyCode <= XK_F12)
            return static_cast<EKeyCode>(virtualKeyCode - XK_F1 + EKeyCode_F1);

        // rest of keys
        switch (virtualKeyCode)
        {
        case XK_Super_L:
            return EKeyCode_WindowsLeft;
        case XK_Super_R:
            return EKeyCode_WindowsRight;
        case XK_Caps_Lock:
            return EKeyCode_CapsLock;
        case XK_Num_Lock:
            return EKeyCode_NumLock;
        case XK_Control_L:
            return EKeyCode_ControlLeft;
        case XK_Control_R:
            return EKeyCode_ControlRight;
        case XK_Shift_L:
            return EKeyCode_ShiftLeft;
        case XK_Shift_R:
            return EKeyCode_ShiftRight;
        case XK_space:
            return EKeyCode_Space;
        case XK_Return:
            return EKeyCode_Return;
        case XK_Escape:
            return EKeyCode_Escape;
        case XK_Tab:
            return EKeyCode_Tab;
        case XK_BackSpace:
            return EKeyCode_Backspace;
        case XK_Alt_L:
            return EKeyCode_AltLeft;
        case XK_Alt_R:
        case XK_ISO_Level3_Shift:
            return EKeyCode_AltRight;
        case XK_Scroll_Lock:
            return EKeyCode_ScrollLock;
        case XK_Print:
            return EKeyCode_PrintScreen;
        case XK_Pause:
            return EKeyCode_Pause;
        case XK_Insert:
            return EKeyCode_Insert;
        case XK_Home:
            return EKeyCode_Home;
        case XK_Next:
            return EKeyCode_PageDown;
        case XK_Delete:
            return EKeyCode_Delete;
        case XK_End:
            return EKeyCode_End;
        case XK_Prior:
            return EKeyCode_PageUp;
        case XK_Left:
            return EKeyCode_Left;
        case XK_Right:
            return EKeyCode_Right;
        case XK_Up:
            return EKeyCode_Up;
        case XK_Down:
            return EKeyCode_Down;
        case XK_udiaeresis:
            return EKeyCode_LeftBracket;
        case XK_odiaeresis:
            return EKeyCode_Semicolon;
        case XK_ssharp:
            return EKeyCode_Minus;
        case XK_dead_circumflex:
            return EKeyCode_Grave;
        case XK_dead_acute:
            return EKeyCode_Equals;
        case XK_apostrophe:
        case XK_adiaeresis:
            return EKeyCode_Apostrophe;
        case XK_less:
        case XK_backslash:
            return EKeyCode_Backslash;
        case XK_bracketright:
        case XK_plus:
            return EKeyCode_RightBracket;
        case XK_comma:
            return EKeyCode_Comma;
        case XK_slash:
        case XK_minus:
            return EKeyCode_Slash;
        case XK_period:
            return EKeyCode_Period;
        case XK_Menu:
            return EKeyCode_Menu;
        case XK_KP_Add:
            return EKeyCode_Numpad_Add;
        case XK_KP_Subtract:
            return EKeyCode_Numpad_Subtract;
        case XK_KP_Multiply:
            return EKeyCode_Numpad_Multiply;
        case XK_KP_Divide:
            return EKeyCode_Numpad_Divide;
        case XK_KP_Decimal:
        case XK_KP_Separator:
            return EKeyCode_Numpad_Decimal;
        case XK_numbersign:
            return EKeyCode_NumberSign;
        case XK_KP_Enter:
            return EKeyCode_Numpad_Enter;
        case XK_KP_Home:
            return EKeyCode_Numpad_7;
        case XK_KP_Insert:
            return EKeyCode_Numpad_0;
        case XK_KP_Next:
            return EKeyCode_Numpad_3;
        case XK_KP_Delete:
            return EKeyCode_Numpad_Decimal;
        case XK_KP_End:
            return EKeyCode_Numpad_1;
        case XK_KP_Prior:
            return EKeyCode_Numpad_9;
        case XK_KP_Begin:
            return EKeyCode_Numpad_5;
        case XK_KP_Left:
            return EKeyCode_Numpad_4;
        case XK_KP_Up:
            return EKeyCode_Numpad_8;
        case XK_KP_Down:
            return EKeyCode_Numpad_2;
        case XK_KP_Right:
            return EKeyCode_Numpad_6;
        default:
            return EKeyCode_Unknown;
        }
    }

    Window_X11::Window_X11(const DisplayConfigData& displayConfig, IWindowEventHandler &windowEventHandler, uint32_t id)
        : Window_Base(displayConfig, windowEventHandler, id)
        , m_keyModifiers(0)
        , m_bLButtonDown(false)
        , m_bRButtonDown(false)
        , m_bMButtonDown(false)
        , m_userProvidedWindowHandle(displayConfig.getX11WindowHandle())
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::Window_X11");
    }

    Window_X11::~Window_X11()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::~Window_X11");

        if (m_userProvidedWindowHandle.isValid())
        {
            LOG_INFO(CONTEXT_RENDERER, "Window_X11::~Window_X11: Skip destruction because has user provided window handle");
        }
        else
        {
            if (m_X11WindowData.window != 0u)
            {
                XDestroyWindow(m_X11WindowData.display, m_X11WindowData.window);
                XSync(m_X11WindowData.display, static_cast<int>(true));
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Window_X11::~Window_X11: Error closing X11 window");
            }
        }

        if (m_X11WindowData.display)
        {
            LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::~Window_X11: XCloseDisplay");
            XCloseDisplay(m_X11WindowData.display);
            m_X11WindowData.display = nullptr;
        }
    }

    bool Window_X11::init()
    {
        if(m_userProvidedWindowHandle.isValid())
        {
            m_X11WindowData.window = m_userProvidedWindowHandle.getValue();

            LOG_INFO(CONTEXT_RENDERER, "Window_X11::init from existing X11 window: {}", m_X11WindowData.window);
            m_X11WindowData.display = XOpenDisplay(nullptr);

            if (!m_X11WindowData.display)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Error: Unable to open X display\n");
                return false;
            }

            XWindowAttributes windowAttributes;
            XGetWindowAttributes(m_X11WindowData.display, m_X11WindowData.window, &windowAttributes);
            m_posX = windowAttributes.x;
            m_posY = windowAttributes.y;
            m_width = windowAttributes.width;
            m_height = windowAttributes.height;

            return true;
        }

        LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::initX11WindowAndGetNativeHandles XOpenDisplay");
        m_X11WindowData.display = XOpenDisplay(nullptr);
        if (!m_X11WindowData.display)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Error: Unable to open X display\n");
            return false;
        }

        LOG_DEBUG(CONTEXT_RENDERER, "Opening X11 window");

        ::Window rootWindow(0);
        uint32_t windowMask(0u);
        int32_t colorDepth(0);

        XSetWindowAttributes windowAttributes;
        PlatformMemory::Set(&windowAttributes, 0, sizeof(windowAttributes));

        // TODO Violin/Mohamed this is only used during creation, should not store here
        m_X11WindowData.screen = XDefaultScreen(m_X11WindowData.display);

        // Get the root window parameters
        PUSH_DISABLE_C_STYLE_CAST_WARNING
        rootWindow = RootWindow(m_X11WindowData.display, m_X11WindowData.screen); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        colorDepth = DefaultDepth(m_X11WindowData.display, m_X11WindowData.screen); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        POP_DISABLE_C_STYLE_CAST_WARNING

        // update window size in global config, if required
        const int32_t displayWidth = XDisplayWidth(m_X11WindowData.display, m_X11WindowData.screen);
        const int32_t displayHeight = XDisplayHeight(m_X11WindowData.display, m_X11WindowData.screen);
        LOG_DEBUG(CONTEXT_RENDERER, "Screen size is {} by {} pixels", displayWidth, displayHeight);
        if (m_fullscreen)
        {
            m_width = displayWidth;
            m_height = displayHeight;

            Atom x11_state_atom = XInternAtom(m_X11WindowData.display, "_NET_WM_STATE", False);
            Atom x11_fs_atom = XInternAtom(m_X11WindowData.display, "_NET_WM_STATE_FULLSCREEN", False);

            // request fullscreen window
            XEvent  x11_event;

            x11_event.xclient.type = ClientMessage;
            x11_event.xclient.serial = 0;
            x11_event.xclient.send_event = True;
            x11_event.xclient.window = m_X11WindowData.window;
            x11_event.xclient.message_type = x11_state_atom;
            x11_event.xclient.format = 32;
            x11_event.xclient.data.l[0] = 1;
            x11_event.xclient.data.l[1] = x11_fs_atom; // NOLINT(cppcoreguidelines-narrowing-conversions)
            x11_event.xclient.data.l[2] = 0;

            XSendEvent(m_X11WindowData.display,
                rootWindow,
                False,
                SubstructureRedirectMask | SubstructureNotifyMask, // NOLINT(hicpp-signed-bitwise)
                &x11_event);
        }

        // Try to find a visual which is matching the needed parameters
        if (XMatchVisualInfo(m_X11WindowData.display, m_X11WindowData.screen, colorDepth, TrueColor, &m_X11WindowData.visual) == 0)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Error: Unable to acquire visual\n");
            return false;
        }

        // Create the rendercontext color map
        m_X11WindowData.colormap = XCreateColormap(m_X11WindowData.display,
            rootWindow,
            m_X11WindowData.visual.visual,
            AllocNone);
        windowAttributes.colormap = m_X11WindowData.colormap;

        // Add to these for handling other events
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        windowAttributes.event_mask = StructureNotifyMask
            | ExposureMask
            | ButtonPressMask
            | ButtonReleaseMask
            | KeyPressMask
            | KeyReleaseMask
            | PointerMotionMask
            | FocusChangeMask
            | EnterWindowMask
            | LeaveWindowMask;
        windowAttributes.backing_store = Always;

        // Set the window mask attributes
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        windowMask = CWBackPixel
            | CWBorderPixel
            | CWEventMask
            | CWColormap
            | CWBackingStore;

        // Creates the X11 window
        m_X11WindowData.window = XCreateWindow(m_X11WindowData.display,
            rootWindow,
            m_posX,
            m_posY,
            m_width,
            m_height,
            0,
            m_X11WindowData.visual.depth,
            InputOutput,
            m_X11WindowData.visual.visual,
            windowMask,
            &windowAttributes);

        // prevent the window manager from destroying the window and closing the X11 connection
        // (XPending will exit() if the X11 connection is lost)
        m_X11WindowData.delWindow = XInternAtom(m_X11WindowData.display, "WM_DELETE_WINDOW", 0);
        XSetWMProtocols(m_X11WindowData.display, m_X11WindowData.window, &m_X11WindowData.delWindow, 1);

        XSizeHints* sizeHints = XAllocSizeHints();
        sizeHints->flags = USPosition;
        if(!m_resizable)
        {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            sizeHints->flags |= PMinSize | PMaxSize;
            sizeHints->min_width = static_cast<int>(m_width);
            sizeHints->max_width = static_cast<int>(m_width);
            sizeHints->min_height = static_cast<int>(m_height);
            sizeHints->max_height = static_cast<int>(m_height);
        }
        XSetWMNormalHints(m_X11WindowData.display, m_X11WindowData.window, sizeHints);
        XFree(sizeHints);
        sizeHints = nullptr;

        // map the window
        XMapWindow(m_X11WindowData.display, m_X11WindowData.window);
        XFlush(m_X11WindowData.display);

        // set window title
        XStoreName(m_X11WindowData.display, m_X11WindowData.window, m_windowName.c_str());

        LOG_INFO(CONTEXT_RENDERER, "Created X11 Window, size {} by {} pixels", m_width, m_height);
        return true;
    }

    Display *Window_X11::getNativeDisplayHandle() const
    {
        return m_X11WindowData.display;
    }

    ::Window Window_X11::getNativeWindowHandle() const
    {
        return m_X11WindowData.window;
    }

    bool Window_X11::setFullscreen(bool fullscreen)
    {
        if (fullscreen)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Fullscreen mode is not supported on this platform");
            return false;
        }

        return true;
    }

    void Window_X11::handleEvents()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Updating X11 window");
        while (XPending(m_X11WindowData.display) != 0)
        {
            XEvent event;
            std::array<char, 100> text{};

            XNextEvent(m_X11WindowData.display, &event);
            if (m_X11WindowData.window != event.xany.window)
            {
                continue;
            }

            switch (event.type)
            {
            case EnterNotify:
                m_eventHandler.onMouseEvent(EMouseEvent::WindowEnter, event.xcrossing.x, event.xcrossing.y);
                break;
            case LeaveNotify:
                m_eventHandler.onMouseEvent(EMouseEvent::WindowLeave, event.xcrossing.x, event.xcrossing.y);
                break;
            case ConfigureNotify:
            {
                if (event.xconfigure.x != m_posX || event.xconfigure.y != m_posY)
                {
                    m_posX = event.xconfigure.x;
                    m_posY = event.xconfigure.y;
                    m_eventHandler.onWindowMove(m_posX, m_posY);
                }
                if (static_cast<uint32_t>(event.xconfigure.width) != m_width || static_cast<uint32_t>(event.xconfigure.height) != m_height)
                {
                    m_width = static_cast<uint32_t>(event.xconfigure.width);
                    m_height = static_cast<uint32_t>(event.xconfigure.height);
                    m_eventHandler.onResize(m_width, m_height);
                }
            }
                break;
            case MotionNotify:
                m_eventHandler.onMouseEvent(EMouseEvent::Move, event.xmotion.x, event.xmotion.y);
                break;
            case ButtonPress:
            {
                EMouseEvent eventType = EMouseEvent::Invalid;
                switch (event.xbutton.button)
                {
                case Button1:
                    eventType = EMouseEvent::LeftButtonDown;
                    m_bLButtonDown = true;
                    break;
                case Button3:
                    eventType = EMouseEvent::RightButtonDown;
                    m_bRButtonDown = true;
                    break;
                case Button2:
                    eventType = EMouseEvent::MiddleButtonDown;
                    m_bMButtonDown = true;
                    break;
                case Button4: // scroll up
                    eventType = EMouseEvent::WheelUp;
                    break;
                case Button5: // scroll down
                    eventType = EMouseEvent::WheelDown;
                    break;
                default:
                    break;
                }
                if (EMouseEvent::Invalid != eventType)
                {
                    m_eventHandler.onMouseEvent(eventType, event.xbutton.x, event.xbutton.y);
                }
            }
                break;
            case ButtonRelease:
            {
                EMouseEvent eventType = EMouseEvent::Invalid;
                switch (event.xbutton.button)
                {
                case Button1:
                    eventType = EMouseEvent::LeftButtonUp;
                    m_bLButtonDown = false;
                    break;
                case Button3:
                    eventType = EMouseEvent::RightButtonUp;
                    m_bRButtonDown = false;
                    break;
                case Button2:
                    eventType = EMouseEvent::MiddleButtonUp;
                    m_bMButtonDown = false;
                    break;
                default:
                    break;
                }
                if (EMouseEvent::Invalid != eventType)
                {
                    m_eventHandler.onMouseEvent(eventType, event.xbutton.x, event.xbutton.y);
                }
            }
                break;
            case FocusIn:
                // ignore
                break;
            case FocusOut:
                // ignore
                break;
            case KeyPress:
            {
                // handle modifiers
                switch (event.xkey.keycode)
                {
                    case 50:        //XK_Shift_L:
                    case 62:        //XK_Shift_R:
                        m_keyModifiers.setFlag(EKeyModifier::Shift, true);
                        break;
                    case 37:        //XK_Control_L:
                    case 105:       //XK_Control_R:
                        m_keyModifiers.setFlag(EKeyModifier::Ctrl, true);
                        break;
                    case 64:        //XK_Alt_L:
                    case 108:       //XK_Alt_R:
                        m_keyModifiers.setFlag(EKeyModifier::Alt, true);
                        break;
                }

                KeySym keySym = NoSymbol;
                if (XLookupString(&event.xkey, text.data(), text.size(), &keySym, nullptr) != -1)
                {
                    const EKeyCode keyCode = convertKeySymbolIntoRamsesKeyCode(keySym);
                    m_eventHandler.onKeyEvent(EKeyEvent::Pressed, m_keyModifiers, keyCode);
                }
            }
                break;
            case KeyRelease:
            {
                // Even when a key is hold the x server sends KeyPress and KeyRelease events
                // We skip these obsolete KeyRelease events by checking the following KeyPress event
                if (XPending(m_X11WindowData.display) != 0)
                {
                    XEvent nextEvent;
                    XPeekEvent(m_X11WindowData.display, &nextEvent);
                    if (nextEvent.type == KeyPress &&
                        nextEvent.xkey.time == event.xkey.time &&
                        nextEvent.xkey.keycode == event.xkey.keycode)
                    {
                        break;
                    }
                }

                // Handle releasing of key modifiers
                switch (event.xkey.keycode)
                {
                case 50:        //XK_Shift_L:
                case 62:        //XK_Shift_R:
                    m_keyModifiers .setFlag(EKeyModifier::Shift, false);
                    break;
                case 37:        //XK_Control_L:
                case 105:       //XK_Control_R:
                    m_keyModifiers.setFlag(EKeyModifier::Ctrl, false);
                    break;
                case 64:        //XK_Alt_L:
                case 108:       //XK_Alt_R:
                    m_keyModifiers.setFlag(EKeyModifier::Alt, false);
                    break;
                default:
                    break;
                }

                KeySym keySym = NoSymbol;
                if (XLookupString(&event.xkey, text.data(), text.size(), &keySym, nullptr) != -1)
                {
                    const EKeyCode keyCode = convertKeySymbolIntoRamsesKeyCode(keySym);
                    m_eventHandler.onKeyEvent(EKeyEvent::Released, m_keyModifiers, keyCode);
                }
                break;
            }
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) == m_X11WindowData.delWindow)
                {
                    LOG_DEBUG(CONTEXT_RENDERER, "ClientMessage (close window)");
                    m_eventHandler.onClose();
                }
                else
                {
                    LOG_INFO(CONTEXT_RENDERER, "Unknown X11 ClientMessage");
                }
                break;
            default:
                LOG_DEBUG(CONTEXT_RENDERER, "Other X11 event received, type {}", event.type);
                break;
            }
        }
    }

    bool Window_X11::setExternallyOwnedWindowSize(uint32_t width, uint32_t height)
    {
        if(m_userProvidedWindowHandle.isValid())
        {
            m_width = width;
            m_height = height;
            return true;
        }

        LOG_ERROR(CONTEXT_RENDERER, "Window_X11::setExternallyOwnedWindowSize: X11 window is not externally owned!");
        return false;
    }

    void Window_X11::setTitle(std::string_view title)
    {
        Window_Base::setTitle(title);
        XStoreName(m_X11WindowData.display, m_X11WindowData.window, m_windowName.c_str());
    }
}
