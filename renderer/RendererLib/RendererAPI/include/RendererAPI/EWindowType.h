//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EWINDOWTYPE_H
#define RAMSES_EWINDOWTYPE_H

#include <array>

namespace ramses_internal
{
    enum class EWindowType
    {
        Windows,
        X11,
        Wayland_IVI,
        Wayland_Shell,
        Android
    };

    //This is needed to decide default platform based on build configuration
    constexpr static std::array SupportedWindowTypes{
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS)
        EWindowType::Windows,
#endif

#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_X11)
        EWindowType::X11,
#endif
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
        EWindowType::Wayland_IVI,
#endif
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
        EWindowType::Wayland_Shell,
#endif
#if defined(ramses_sdk_ENABLE_WINDOW_TYPE_ANDROID)
        EWindowType::Android
#endif
    };
}

#endif
