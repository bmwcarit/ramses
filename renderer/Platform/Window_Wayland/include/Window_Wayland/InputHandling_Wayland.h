//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INPUTHANDLING_WAYLAND_H
#define RAMSES_INPUTHANDLING_WAYLAND_H

#include "RendererAPI/IWindowEventHandler.h"

#include "RendererLib/EKeyCode.h"
#include "RendererLib/EKeyModifier.h"

#include "wayland-client-protocol.h"
#include "map"
#include "Math3d/Vector2i.h"

namespace ramses_internal
{
    class InputHandling_Wayland
    {
    public:
        InputHandling_Wayland(IWindowEventHandler& windowEventHandler);
        void deinit();
        void registerSeat(wl_registry* wl_registry, uint32_t name);

    private:
        static EKeyCode RamsesKeyCodeFromWaylandKey(uint32_t waylandKey);

        static void SeatHandleCapabilities  (void *data, wl_seat *seat, unsigned int caps);

        static void PointerHandleEnter      (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
        static void PointerHandleLeave      (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
        static void PointerHandleMotion     (void *data, wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
        static void PointerHandleButton     (void *data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        static void PointerHandleAxis       (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

        static void KeyboardHandleKeymap    (void *data, wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
        static void KeyboardHandleEnter     (void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys);
        static void KeyboardHandleLeave     (void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface);
        static void KeyboardHandleKey       (void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state_w);
        static void KeyboardHandleModifiers (void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

        static void TouchHandleDown(void*       data,
                                    wl_touch*   touch,
                                    uint32_t    serial,
                                    uint32_t    time,
                                    wl_surface* surface,
                                    int32_t     id,
                                    wl_fixed_t  x,
                                    wl_fixed_t  y);

        static void TouchHandleUp(void *data,
                struct wl_touch *wl_touch,
                uint32_t serial,
                uint32_t time,
                int32_t id);

        static void TouchHandleMotion(void *data,
               struct wl_touch *wl_touch,
               uint32_t time,
               int32_t id,
               wl_fixed_t x,
               wl_fixed_t y);

        static void TouchHandleFrame(void* data, struct wl_touch* wl_touch);

        static void TouchHandleCancel(void* data, struct wl_touch* wl_touch);

        IWindowEventHandler& m_windowEventHandler;
        wl_seat*             m_seat     = nullptr;
        wl_keyboard*         m_keyboard = nullptr;
        wl_pointer*          m_pointer  = nullptr;
        wl_touch*            m_touch    = nullptr;

        uint32_t m_keyModifiers          = EKeyModifier_NoModifier;
        double   m_cursorPosX            = 0.0;
        double   m_cursorPosY            = 0.0;
        std::map<int32_t, Vector2i> m_touchPos;
        Bool     m_leftMouseButtonDown   = false;
        Bool     m_rightMouseButtonDown  = false;
        Bool     m_middleMouseButtonDown = false;

        const struct Seat_Listener : public wl_seat_listener
        {
            Seat_Listener()
            {
                capabilities = SeatHandleCapabilities;
            }
        } m_seatListener;

        const struct Pointer_Listener : public wl_pointer_listener
        {
            Pointer_Listener()
            {
                enter  = PointerHandleEnter;
                leave  = PointerHandleLeave;
                motion = PointerHandleMotion;
                button = PointerHandleButton;
                axis   = PointerHandleAxis;
            }
        } m_pointerListener;

        const struct Keyboard_Listener : public wl_keyboard_listener
        {
            Keyboard_Listener()
            {
                keymap      = KeyboardHandleKeymap;
                enter       = KeyboardHandleEnter;
                leave       = KeyboardHandleLeave;
                key         = KeyboardHandleKey;
                modifiers   = KeyboardHandleModifiers;
            }
        } m_keyboardListener;

        const struct Touch_Listener : public wl_touch_listener
        {
            Touch_Listener()
            {
                down        = TouchHandleDown;
                up          = TouchHandleUp;
                motion      = TouchHandleMotion;
                frame       = TouchHandleFrame;
                cancel      = TouchHandleCancel;
            }
        } m_touchListener;
    };
}

#endif
