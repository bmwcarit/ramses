#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(ramses-renderer-api)
add_subdirectory(RendererLib)

add_subdirectory(Platform/Device_GL)

if(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS)
    add_subdirectory(Platform/Window_Windows)
    add_subdirectory(Platform/Context_WGL)
    add_subdirectory(Platform/Platform_Windows_WGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI OR ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL OR ramses-sdk_ENABLE_WINDOW_TYPE_X11 OR ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID OR ramses-sdk_ENABLE_WINDOW_TYPE_IOS)
    add_subdirectory(Platform/Context_EGL)
    add_subdirectory(Platform/Platform_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
    add_subdirectory(Platform/Platform_X11_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID)
    add_subdirectory(Platform/Platform_Android_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_IOS)
    add_subdirectory(Platform/Platform_iOS_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI OR ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    add_subdirectory(Platform/WaylandEGLExtensionProcs)
    add_subdirectory(Platform/WaylandUtilities)
    add_subdirectory(Platform/Device_EGL_Extension)
    add_subdirectory(Platform/Window_Wayland)
    add_subdirectory(Platform/Window_Wayland_Test)

    add_subdirectory(Platform/EmbeddedCompositor_Wayland)

    add_subdirectory(Platform/Platform_Wayland_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    add_subdirectory(Platform/Window_Wayland_Shell)
    add_subdirectory(Platform/Platform_Wayland_Shell_EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
    add_subdirectory(Platform/SystemCompositorController_Wayland_IVI)
    add_subdirectory(Platform/Window_Wayland_IVI)
    add_subdirectory(Platform/Platform_Wayland_IVI_EGL)
endif()

add_subdirectory(PlatformFactory)
add_subdirectory(ramses-renderer-impl)
add_subdirectory(RendererTestUtils)
