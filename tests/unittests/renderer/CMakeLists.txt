#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(renderer-test-common)
add_subdirectory(renderer-lib)
add_subdirectory(PlatformFactoryFake)
add_subdirectory(ramses-renderer)

if(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS)
    add_subdirectory(window-windows)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
    add_subdirectory(window-x11)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI OR ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    add_subdirectory(wayland-test-utils)
    add_subdirectory(window-wayland-common)
    add_subdirectory(embedded-compositor-wayland)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    add_subdirectory(window-wayland-wl-shell)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
    add_subdirectory(system-compositor-controller-wayland)
    add_subdirectory(window-wayland-ivi)
endif()
