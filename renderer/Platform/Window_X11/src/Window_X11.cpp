//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_X11/Window_X11.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/EKeyModifier.h"
#include "Utils/LogMacros.h"
#include "Utils/Warnings.h"

#include "RendererAPI/IWindowEventHandler.h"

namespace ramses_internal
{
    EKeyCode Window_X11::convertKeySymbolIntoRamsesKeyCode(UInt32 virtualKeyCode)
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

    Window_X11::Window_X11(const DisplayConfig& displayConfig, IWindowEventHandler &windowEventHandler, UInt32 id)
        : Window_Base(displayConfig, windowEventHandler, id)
        , m_keyModifiers(0)
        , m_bLButtonDown(false)
        , m_bRButtonDown(false)
        , m_bMButtonDown(false)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::Window_X11");
    }

    Window_X11::~Window_X11()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::~Window_X11");

        if (m_X11WindowData.window && m_X11WindowData.display)
        {
            XDestroyWindow(m_X11WindowData.display, m_X11WindowData.window);
            XFlush(m_X11WindowData.display);

            LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::destroyLastInstance XCloseDisplay");
            XCloseDisplay(m_X11WindowData.display);
            m_X11WindowData.display = 0;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Error closing X11 window");
            return;
        }
    }

    Bool Window_X11::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_X11::initX11WindowAndGetNativeHandles XOpenDisplay");
        m_X11WindowData.display = XOpenDisplay(0);
        if (!m_X11WindowData.display)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Error: Unable to open X display\n");
            return false;
        }

        LOG_DEBUG(CONTEXT_RENDERER, "Opening X11 window");

        ::Window rootWindow(0);
        UInt32 windowMask(0u);
        Int32 colorDepth(0);

        XSetWindowAttributes windowAttributes;
        PlatformMemory::Set(&windowAttributes, 0, sizeof(windowAttributes));

        m_X11WindowData.screen = XDefaultScreen(m_X11WindowData.display);

        // Get the root window parameters
        PUSH_DISABLE_C_STYLE_CAST_WARNING
        rootWindow = RootWindow(m_X11WindowData.display, m_X11WindowData.screen);
        colorDepth = DefaultDepth(m_X11WindowData.display, m_X11WindowData.screen);
        POP_DISABLE_C_STYLE_CAST_WARNING

        // update window size in global config, if required
        const Int32 displayWidth = XDisplayWidth(m_X11WindowData.display, m_X11WindowData.screen);
        const Int32 displayHeight = XDisplayHeight(m_X11WindowData.display, m_X11WindowData.screen);
        LOG_DEBUG(CONTEXT_RENDERER, "Screen size is " << displayWidth << " by " << displayHeight << " pixels");
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
            x11_event.xclient.data.l[1] = x11_fs_atom;
            x11_event.xclient.data.l[2] = 0;

            XSendEvent(m_X11WindowData.display,
                rootWindow,
                False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                &x11_event);
        }

        // Try to find a visual which is matching the needed parameters
        if (!XMatchVisualInfo(m_X11WindowData.display,
            m_X11WindowData.screen,
            colorDepth,
            TrueColor,
            &m_X11WindowData.visual))
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

        if(!m_resizable)
        {
            XSizeHints* sh = XAllocSizeHints();
            sh->flags = PMinSize | PMaxSize;
            sh->min_width = m_width;
            sh->max_width = m_width;
            sh->min_height = m_height;
            sh->max_height = m_height;
            XSetWMNormalHints(m_X11WindowData.display, m_X11WindowData.window, sh);
            XFree(sh);
        }

        // disable window decorations
        if( m_borderless )
        {
            struct MotifHints
            {
                unsigned long flags;
                unsigned long functions;
                unsigned long decorations;
                long inputMode;
                unsigned long status;
            };

            Atom motifHintsProperty = XInternAtom(m_X11WindowData.display, "_MOTIF_WM_HINTS", 0);
            struct MotifHints hints;
            hints.flags = (1L << 1); // 2 = change window decorations
            hints.decorations = 0; // false, ask the window manager to not decorate this window (NB. "ask" not "tell" - since the WM might not handle this atom)
            XChangeProperty(m_X11WindowData.display,
                            m_X11WindowData.window,
                            motifHintsProperty, motifHintsProperty, 32,
                            PropModeReplace, reinterpret_cast<unsigned char *>(&hints), 5);
        }

        // map the window
        XMapWindow(m_X11WindowData.display, m_X11WindowData.window);
        XFlush(m_X11WindowData.display);

        // set window title
        XStoreName(m_X11WindowData.display, m_X11WindowData.window, m_windowName.c_str());

        LOG_INFO(CONTEXT_RENDERER, "Created X11 Window, size " << m_width << " by " << m_height << " pixels");

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

    Bool Window_X11::setFullscreen(Bool fullscreen)
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
        while (XPending(m_X11WindowData.display))
        {
            XEvent event;
            char text[100];

            // take first event for current window out of event queue and process it,
            // early-out if no event for current window is present
            if (!XCheckWindowEvent(m_X11WindowData.display, m_X11WindowData.window, ~0, &event))
            {
                return;
            }

            switch (event.type)
            {
            case EnterNotify:
                m_eventHandler.onMouseEvent(EMouseEventType_WindowEnter, event.xcrossing.x, event.xcrossing.y);
                break;
            case LeaveNotify:
                m_eventHandler.onMouseEvent(EMouseEventType_WindowLeave, event.xcrossing.x, event.xcrossing.y);
                break;
            case ConfigureNotify:
            {
                if (event.xconfigure.x != m_posX || event.xconfigure.y != m_posY)
                {
                    m_posX = event.xconfigure.x;
                    m_posY = event.xconfigure.y;
                }
                if (static_cast<UInt32>(event.xconfigure.width) != m_width || static_cast<UInt32>(event.xconfigure.height) != m_height)
                {
                    m_width = static_cast<UInt32>(event.xconfigure.width);
                    m_height = static_cast<UInt32>(event.xconfigure.height);
                    m_eventHandler.onResize(m_width, m_height);
                }
            }
                break;
            case MotionNotify:
                m_eventHandler.onMouseEvent(EMouseEventType_Move, event.xmotion.x, event.xmotion.y);
                break;
            case ButtonPress:
            {
                EMouseEventType eventType = EMouseEventType_Invalid;
                switch (event.xbutton.button)
                {
                case Button1:
                    eventType = EMouseEventType_LeftButtonDown;
                    m_bLButtonDown = true;
                    break;
                case Button3:
                    eventType = EMouseEventType_RightButtonDown;
                    m_bRButtonDown = true;
                    break;
                case Button2:
                    eventType = EMouseEventType_MiddleButtonDown;
                    m_bMButtonDown = true;
                    break;
                case Button4: // scroll up
                    eventType = EMouseEventType_WheelUp;
                    break;
                case Button5: // scroll down
                    eventType = EMouseEventType_WheelDown;
                    break;
                default:
                    break;
                }
                if (EMouseEventType_Invalid != eventType)
                {
                    m_eventHandler.onMouseEvent(eventType, event.xbutton.x, event.xbutton.y);
                }
            }
                break;
            case ButtonRelease:
            {
                EMouseEventType eventType = EMouseEventType_Invalid;
                switch (event.xbutton.button)
                {
                case Button1:
                    eventType = EMouseEventType_LeftButtonUp;
                    m_bLButtonDown = false;
                    break;
                case Button3:
                    eventType = EMouseEventType_RightButtonUp;
                    m_bRButtonDown = false;
                    break;
                case Button2:
                    eventType = EMouseEventType_MiddleButtonUp;
                    m_bMButtonDown = false;
                    break;
                default:
                    break;
                }
                if (EMouseEventType_Invalid != eventType)
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
                        m_keyModifiers |= EKeyModifier_Shift;
                        break;
                    case 37:        //XK_Control_L:
                    case 105:       //XK_Control_R:
                        m_keyModifiers |= EKeyModifier_Ctrl;
                        break;
                    case 64:        //XK_Alt_L:
                    case 108:       //XK_Alt_R:
                        m_keyModifiers |= EKeyModifier_Alt;
                        break;
                }

                KeySym keySym = NoSymbol;
                if (XLookupString(&event.xkey, text, sizeof(text), &keySym, NULL) != -1)
                {
                    const EKeyCode keyCode = convertKeySymbolIntoRamsesKeyCode(keySym);
                    m_eventHandler.onKeyEvent(EKeyEventType_Pressed, m_keyModifiers, keyCode);
                }
            }
                break;
            case KeyRelease:
            {
                // Even when a key is hold the x server sends KeyPress and KeyRelease events
                // We skip these obsolete KeyRelease events by checking the following KeyPress event
                if (XPending(m_X11WindowData.display))
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
                    m_keyModifiers &= ~EKeyModifier_Shift;
                    break;
                case 37:        //XK_Control_L:
                case 105:       //XK_Control_R:
                    m_keyModifiers &= ~EKeyModifier_Ctrl;
                    break;
                case 64:        //XK_Alt_L:
                case 108:       //XK_Alt_R:
                    m_keyModifiers &= ~EKeyModifier_Alt;
                    break;
                default:
                    break;
                }

                KeySym keySym = NoSymbol;
                if (XLookupString(&event.xkey, text, sizeof(text), &keySym, NULL) != -1)
                {
                    const EKeyCode keyCode = convertKeySymbolIntoRamsesKeyCode(keySym);
                    m_eventHandler.onKeyEvent(EKeyEventType_Released, m_keyModifiers, keyCode);
                }
                break;
            }
            default:
                LOG_DEBUG(CONTEXT_RENDERER, "Other X11 event received, type " << event.type);
                break;
            }
        }
    }

    void Window_X11::setTitle(const String& title)
    {
        Window_Base::setTitle(title);
        XStoreName(m_X11WindowData.display, m_X11WindowData.window, title.c_str());
    }
}
