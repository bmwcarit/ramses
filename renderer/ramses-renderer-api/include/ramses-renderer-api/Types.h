//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_TYPES_H
#define RAMSES_RENDERERAPI_TYPES_H

#include <cstdint>
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    /**
     * @addtogroup RendererAPI
     * @{
     */

    /**
    * @brief Resource identifier used to refer to a resource on the renderer
    *
    * @param[in] low The low 64 bits of the value
    * @param[in] high The high 64 bits of the value
    */
    struct rendererResourceId_t
    {
        /**
        * @brief Default constructor constructs invalid value
        */
        constexpr rendererResourceId_t() = default;

        /**
        * @brief The constructor
        * @param[in] low The low bits of the ID
        * @param[in] high The high bits of the ID
        */
        constexpr rendererResourceId_t(uint64_t low, uint64_t high)
            : lowPart(low)
            , highPart(high)
        {
        }

        /**
        * @brief The comparison operator
        * @param[in] rhs The instance to compare to
        * @return True if same, false otherwise
        */
        constexpr bool operator==(const rendererResourceId_t& rhs) const
        {
            return (lowPart == rhs.lowPart && highPart == rhs.highPart);
        }

        /**
        * @copydoc operator==(const rendererResourceId_t& rhs) const
        */
        constexpr bool operator!=(const rendererResourceId_t& rhs) const
        {
            return !(*this == rhs);
        }

        /**
        * @brief The lower bits of the resource id
        */
        uint64_t lowPart{0};
        /**
        * @brief The higher bits of the resource id
        */
        uint64_t highPart{0};
    };

    /**
    * @brief Effect identifier used by the renderer to refer to an effect
    *
    * Note: the value is not the same as on client if calling ramses::Resource::getResourceId()
    */
    using effectId_t = rendererResourceId_t;

    /**
    * @brief Specifies the result of the operation referred to by renderer event
    *
    */
    enum class ERendererEventResult
    {
        Ok,       //!< Event referring to an operation that succeeded
        Indirect, //!< Event referring to an operation that succeeded but was triggered by another event (eg. unmapped/unsubscribed after scene was unpublished by client)
        Failed    //!< Event referring to an operation that failed
    };

    /**
    * @brief Specifies events for mouse input
    *
    */
    enum class EMouseEvent
    {
        Invalid,

        LeftButtonDown,
        LeftButtonUp,
        RightButtonDown,
        RightButtonUp,
        MiddleButtonDown,
        MiddleButtonUp,

        WheelUp,
        WheelDown,

        Move,

        WindowEnter,
        WindowLeave
    };

    /**
    * @brief Specifies keypress events for keyboard input.
    *
    */
    enum class EKeyEvent
    {
        Invalid,

        Pressed,
        Released
    };

    /**
    * @brief Specifies key codes for keyboard input.
    *
    */
    enum EKeyCode
    {
        EKeyCode_Unknown = 0,

        EKeyCode_A,
        EKeyCode_B,
        EKeyCode_C,
        EKeyCode_D,
        EKeyCode_E,
        EKeyCode_F,
        EKeyCode_G,
        EKeyCode_H,
        EKeyCode_I,
        EKeyCode_J,
        EKeyCode_K,
        EKeyCode_L,
        EKeyCode_M,
        EKeyCode_N,
        EKeyCode_O,
        EKeyCode_P,
        EKeyCode_Q,
        EKeyCode_R,
        EKeyCode_S,
        EKeyCode_T,
        EKeyCode_U,
        EKeyCode_V,
        EKeyCode_W,
        EKeyCode_X,
        EKeyCode_Y,
        EKeyCode_Z,

        EKeyCode_0,
        EKeyCode_1,
        EKeyCode_2,
        EKeyCode_3,
        EKeyCode_4,
        EKeyCode_5,
        EKeyCode_6,
        EKeyCode_7,
        EKeyCode_8,
        EKeyCode_9,

        EKeyCode_NumLock,
        EKeyCode_Numpad_Add,
        EKeyCode_Numpad_Subtract,
        EKeyCode_Numpad_Multiply,
        EKeyCode_Numpad_Divide,
        EKeyCode_Numpad_Enter,
        EKeyCode_Numpad_Decimal,
        EKeyCode_Numpad_0,
        EKeyCode_Numpad_1,
        EKeyCode_Numpad_2,
        EKeyCode_Numpad_3,
        EKeyCode_Numpad_4,
        EKeyCode_Numpad_5,
        EKeyCode_Numpad_6,
        EKeyCode_Numpad_7,
        EKeyCode_Numpad_8,
        EKeyCode_Numpad_9,

        EKeyCode_Return,
        EKeyCode_Escape,
        EKeyCode_Backspace,
        EKeyCode_Tab,
        EKeyCode_Space,
        EKeyCode_Menu,
        EKeyCode_CapsLock,
        EKeyCode_ShiftLeft,
        EKeyCode_ShiftRight,
        EKeyCode_AltLeft,
        EKeyCode_AltRight,
        EKeyCode_ControlLeft,
        EKeyCode_ControlRight,
        EKeyCode_WindowsLeft,
        EKeyCode_WindowsRight,

        EKeyCode_F1,
        EKeyCode_F2,
        EKeyCode_F3,
        EKeyCode_F4,
        EKeyCode_F5,
        EKeyCode_F6,
        EKeyCode_F7,
        EKeyCode_F8,
        EKeyCode_F9,
        EKeyCode_F10,
        EKeyCode_F11,
        EKeyCode_F12,

        EKeyCode_PrintScreen,
        EKeyCode_ScrollLock,
        EKeyCode_Pause,

        EKeyCode_Insert,
        EKeyCode_Home,
        EKeyCode_PageUp,
        EKeyCode_Delete,
        EKeyCode_End,
        EKeyCode_PageDown,

        EKeyCode_Right,
        EKeyCode_Left,
        EKeyCode_Up,
        EKeyCode_Down,

        EKeyCode_Minus,
        EKeyCode_Equals,
        EKeyCode_LeftBracket,
        EKeyCode_RightBracket,
        EKeyCode_Backslash,
        EKeyCode_Semicolon,
        EKeyCode_Comma,
        EKeyCode_Period,
        EKeyCode_Slash,
        EKeyCode_Apostrophe,
        EKeyCode_Grave,
        EKeyCode_NumberSign
    };

    /**
    * @brief Specifies key modifiers for keyboard input.
    *
    */
    enum EKeyModifier
    {
        EKeyModifier_NoModifier = 0,
        EKeyModifier_Ctrl = 1 << 0,
        EKeyModifier_Shift = 1 << 1,
        EKeyModifier_Alt = 1 << 2,
        EKeyModifier_Function = 1 << 3,
        EKeyModifier_Numpad = 1 << 4
    };

    /**
    * @brief Specifies behavior of render loop
    *
    */
    enum class ELoopMode
    {
        UpdateAndRender = 0,  //!< Render loop with update content and render
        UpdateOnly            //!< Render loop will update content without rendering
    };

    /**
    * @brief Specifies type of depth buffer created within an offscreen buffer
    *
    */
    enum class EDepthBufferType
    {
        None = 0,
        Depth,
        DepthStencil
    };

    /**
    * @brief Specifies requested device type to create
    *
    */
    enum class EDeviceType
    {
        GLES_3_0,
        GL_4_2,
        GL_4_5
    };

    /**
    * @brief Specifies type of window to create
    *
    */
    enum class EWindowType
    {
        Windows,
        X11,
        Wayland_IVI,
        Wayland_Shell,
        Android
    };

    /**
     * @}
     */
}

#endif
