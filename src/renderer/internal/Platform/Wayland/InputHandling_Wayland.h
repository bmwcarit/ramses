//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"

#include "internal/RendererLib/Enums/EKeyCode.h"
#include "internal/RendererLib/Enums/EKeyModifier.h"

#include "wayland-client-protocol.h"

namespace ramses::internal
{
    class InputHandling_Wayland
    {
    public:
        explicit InputHandling_Wayland(IWindowEventHandler& windowEventHandler);
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

        IWindowEventHandler& m_windowEventHandler;
        wl_seat*             m_seat     = nullptr;
        wl_keyboard*         m_keyboard = nullptr;
        wl_pointer*          m_pointer  = nullptr;

        KeyModifiers m_keyModifiers;
        int      m_cursorPosX            = 0;
        int      m_cursorPosY            = 0;
        bool     m_leftMouseButtonDown   = false;
        bool     m_rightMouseButtonDown  = false;
        bool     m_middleMouseButtonDown = false;

        const struct Seat_Listener : public wl_seat_listener
        {
            Seat_Listener() : wl_seat_listener()
            {
                capabilities = SeatHandleCapabilities;
            }
        } m_seatListener;

        const struct Pointer_Listener : public wl_pointer_listener
        {
            Pointer_Listener() : wl_pointer_listener()
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
            Keyboard_Listener() : wl_keyboard_listener()
            {
                keymap      = KeyboardHandleKeymap;
                enter       = KeyboardHandleEnter;
                leave       = KeyboardHandleLeave;
                key         = KeyboardHandleKey;
                modifiers   = KeyboardHandleModifiers;
            }
        } m_keyboardListener;
    };
}
