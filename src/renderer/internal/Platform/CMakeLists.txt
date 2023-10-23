#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS)
    list(APPEND PLATFORM_SOURCES    Windows/*.h
                                    Windows/*.cpp)
    message("+ Windows Window")
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI OR ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL OR ramses-sdk_ENABLE_WINDOW_TYPE_X11 OR ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID OR ramses-sdk_ENABLE_WINDOW_TYPE_IOS)
    list(APPEND PLATFORM_SOURCES    EGL/*.h
                                    EGL/*.cpp)
    list(APPEND PLATFORM_LIBS       EGL)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
    list(APPEND PLATFORM_SOURCES    X11/*.h
                                    X11/*.cpp)
    list(APPEND PLATFORM_LIBS       X11)
    message("+ X11 Window")
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID)
    list(APPEND PLATFORM_SOURCES    Android/*.h
                                    Android/*.cpp)
    list(APPEND PLATFORM_LIBS       AndroidSDK)
    message("+ Android Window")
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_IOS)
    list(APPEND PLATFORM_SOURCES    iOS/*.h
                                    iOS/*.cpp
                                    iOS/*.mm)
    list(APPEND PLATFORM_LIBS       QuartzCore)
    message("+ iOS Window")
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI OR ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    list(APPEND PLATFORM_SOURCES    Wayland/*.h
                                    Wayland/*.cpp
                                    Wayland/EmbeddedCompositor/*.h
                                    Wayland/EmbeddedCompositor/*.cpp)
    list(APPEND PLATFORM_LIBS       wayland-zwp-linux-dmabuf-v1-extension
                                    wayland-client
                                    wayland-server
                                    wayland-egl
                                    LinuxInput)

    # Optional extension that's dependant on gbm and libdrm for support of DMA offscreen buffers
    find_package(gbm)
    find_package(libdrm)
    if(gbm_FOUND AND libdrm_FOUND)
        list(APPEND PLATFORM_SOURCES    Device_EGL_Extension/*.h
                                        Device_EGL_Extension/*.cpp)
        list(APPEND PLATFORM_LIBS       gbm libdrm)
        set(DEVICE_EGL_EXTENSION_SUPPORTED true)
    endif()
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    list(APPEND PLATFORM_SOURCES    Wayland/WlShell/*.h
                                    Wayland/WlShell/*.cpp)
    message("+ Wayland wl_shell Window")
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
    list(APPEND PLATFORM_SOURCES    Wayland/IVI/*.h
                                    Wayland/IVI/*.cpp
                                    Wayland/IVI/SystemCompositorController/*.h
                                    Wayland/IVI/SystemCompositorController/*.cpp)
    list(APPEND PLATFORM_LIBS       libdrm
                                    wayland-ivi-extension)
    message("+ Wayland ivi Window")
endif()

createModule(
    NAME                    Platform
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    SRC_FILES               *.h
                            *.cpp
                            OpenGL/*.h
                            OpenGL/*.cpp
                            ${PLATFORM_SOURCES}
    DEPENDENCIES            ramses-renderer-lib
                            OpenGL
                            ${PLATFORM_LIBS}

    # TODO move this to the OpenGL target where it belongs
    # This tech debt is inherited from Device_GL
    PUBLIC_DEFINES          ${OpenGL_DEFINITIONS}
)

# check for eglmesaext header and ensure it's included when available
find_file(ramses-sdk_HAS_EGLMESAEXT "EGL/eglmesaext.h")
if (ramses-sdk_HAS_EGLMESAEXT)
    target_compile_definitions(Platform PUBLIC "RAMSES_HAS_EGLMESAEXT=1")
endif()

if(${DEVICE_EGL_EXTENSION_SUPPORTED})
    target_compile_definitions(Platform PUBLIC DEVICE_EGL_EXTENSION_SUPPORTED)
endif()
