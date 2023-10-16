#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB_RECURSE
    RAMSES_RENDERER_LIB_FILES
    *.h
    *.cpp)

createModule(
    NAME                    ramses-renderer-lib
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    INCLUDE_PATHS           ${PROJECT_SOURCE_DIR}/src/renderer
    SRC_FILES               ${RAMSES_RENDERER_LIB_FILES}
    DEPENDENCIES            ramses-api
                            ramses-framework
)

if(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS)
    target_compile_definitions(ramses-renderer-lib PUBLIC ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
    target_compile_definitions(ramses-renderer-lib PUBLIC ramses_sdk_ENABLE_WINDOW_TYPE_X11)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID)
    target_compile_definitions(ramses-renderer-lib PUBLIC ramses_sdk_ENABLE_WINDOW_TYPE_ANDROID)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_IOS)
    target_compile_definitions(ramses-renderer-lib PUBLIC ramses_sdk_ENABLE_WINDOW_TYPE_IOS)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
    target_compile_definitions(ramses-renderer-lib PUBLIC ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
endif()

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
    target_compile_definitions(ramses-renderer-lib PUBLIC ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
endif()
