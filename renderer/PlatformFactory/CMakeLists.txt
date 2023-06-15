#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    PlatformFactory
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/PlatformFactory/*.h
                            src/*.cpp
    DEPENDENCIES            ramses-renderer-lib
)

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
    target_link_libraries(PlatformFactory PUBLIC Platform_Wayland_IVI_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    target_link_libraries(PlatformFactory PUBLIC Platform_Wayland_Shell_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
    target_link_libraries(PlatformFactory PUBLIC Platform_X11_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID)
    target_link_libraries(PlatformFactory PUBLIC Platform_Android_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS)
    target_link_libraries(PlatformFactory PUBLIC Platform_Windows_WGL)
endif()
