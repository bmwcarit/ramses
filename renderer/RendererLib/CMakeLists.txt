#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    ramses-renderer-lib
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    INCLUDE_PATHS           RendererAPI/include
    SRC_FILES               RendererAPI/include/RendererAPI/*.h
                            RendererAPI/src/*.cpp

    INCLUDE_PATHS           Platform_Base/include
    SRC_FILES               Platform_Base/include/Platform_Base/*.h
                            Platform_Base/src/*.cpp

    INCLUDE_PATHS           RendererCommands/include
    SRC_FILES               RendererCommands/include/RendererCommands/*.h
                            RendererCommands/src/*.cpp

    INCLUDE_PATHS           RendererFramework/include
    SRC_FILES               RendererFramework/include/RendererFramework/*.h
                            RendererFramework/include/*.h
                            RendererFramework/src/*.cpp

    INCLUDE_PATHS           RendererLib/include
    SRC_FILES               RendererLib/include/RendererLib/*.h
                            RendererLib/include/*.h
                            RendererLib/src/*.cpp
    DEPENDENCIES            ramses-renderer-api
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

IF (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    RendererTestCommon
        TYPE                    STATIC_LIBRARY

        INCLUDE_PATHS           RendererTestCommon
        SRC_FILES               RendererTestCommon/*.h
                                RendererTestCommon/*.cpp
        DEPENDENCIES            ramses-renderer-lib
                                FrameworkTestUtils
    )

    createModule(
        NAME                    ramses-renderer-lib-test
        TYPE                    BINARY
        SRC_FILES               RendererFramework/test/*.cpp
                                RendererFramework/test/*.h
                                RendererLib/test/*.cpp
                                RendererLib/test/*.h
                                RendererCommands/test/*.cpp
                                RendererCommands/test/*.h
        DEPENDENCIES            RendererTestCommon
                                ramses-gmock-main
    )

    makeTestFromTarget(
        TARGET ramses-renderer-lib-test
        SUFFIX UNITTEST)
endif()
