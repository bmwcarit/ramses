#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB
    RAMSES_RENDERER_API_INCLUDE_BASE
    ramses-renderer-api/include)

file(GLOB
    RAMSES_RENDERER_FILES_SOURCE
    ramses-renderer-impl/src/*.cpp)


add_library(ramses-renderer-api INTERFACE)
target_include_directories(ramses-renderer-api INTERFACE ${RAMSES_RENDERER_API_INCLUDE_BASE})

createModule(
    NAME                    ramses-renderer-lib
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    SRC_FILES               ramses-renderer-api/include/*.h
                            ramses-renderer-api/include/ramses-renderer-api/*.h

    # This needs to be fixed in upcoming story regarding renderer lib
    # Not possible to solve now, without splitting in two libs (which is the correct solution)
    INCLUDE_PATHS           ramses-renderer-impl/include
    SRC_FILES               ramses-renderer-impl/include/*.h
                            ${RAMSES_RENDERER_FILES_SOURCE}

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

if(ramses-sdk_ENABLE_INSTALL)
    install(DIRECTORY ramses-renderer-api/include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-renderer-lib PRIVATE "-Wsuggest-override")
endif()

if (DEFINED ramses-sdk_RENDER_THREAD_PRIORITY AND NOT ramses-sdk_RENDER_THREAD_PRIORITY STREQUAL "")
    target_compile_definitions(ramses-renderer-lib PRIVATE "-DRAMSES_RENDER_THREAD_PRIORITY=${ramses-sdk_RENDER_THREAD_PRIORITY}")
endif()

if (DEFINED ramses-sdk_RENDER_THREAD_CORE_BINDING AND NOT ramses-sdk_RENDER_THREAD_CORE_BINDING STREQUAL "")
    target_compile_definitions(ramses-renderer-lib PRIVATE "-DRAMSES_RENDER_THREAD_CORE_BINDING=${ramses-sdk_RENDER_THREAD_CORE_BINDING}")
endif()

if (DEFINED ramses-sdk_ASYNC_SHADERCOMPILE_THREAD_PRIORITY AND NOT ramses-sdk_ASYNC_SHADERCOMPILE_THREAD_PRIORITY STREQUAL "")
    target_compile_definitions(ramses-renderer-lib PRIVATE "-DRAMSES_ASYNC_SHADERCOMPILE_THREAD_PRIORITY=${ramses-sdk_ASYNC_SHADERCOMPILE_THREAD_PRIORITY}")
endif()

if (DEFINED ramses-sdk_ASYNC_SHADERCOMPILE_THREAD_CORE_BINDING AND NOT ramses-sdk_ASYNC_SHADERCOMPILE_THREAD_CORE_BINDING STREQUAL "")
    target_compile_definitions(ramses-renderer-lib PRIVATE "-DRAMSES_ASYNC_SHADERCOMPILE_THREAD_CORE_BINDING=${ramses-sdk_ASYNC_SHADERCOMPILE_THREAD_CORE_BINDING}")
endif()

set(ramses-shared-lib-renderer-MIXIN
    ${ramses-shared-lib-renderer-MIXIN}
    INCLUDE_PATHS ${RAMSES_RENDERER_API_INCLUDE_BASE}
    SRC_FILES ${RAMSES_RENDERER_FILES_SOURCE}
    DEPENDENCIES  ramses-renderer-lib
    CACHE INTERNAL "")


IF (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    RendererTestUtils
        TYPE                    STATIC_LIBRARY

        INCLUDE_PATHS           RendererTestUtils/include
        SRC_FILES               RendererTestUtils/include/*.h
                                RendererTestUtils/src/*.cpp
        DEPENDENCIES            ramses-renderer-lib
                                FrameworkTestUtils
    )
ENDIF()

if (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    ramses-renderer-lib-test
        TYPE                    BINARY
        SRC_FILES            RendererFramework/test/*.cpp
                                RendererFramework/test/*.h
                                RendererAPI/test/*.cpp
                                RendererAPI/test/*.h
                                RendererLib/test/*.cpp
                                RendererLib/test/*.h
                                ShaderLanguage_Base/test/*.cpp
                                ShaderLanguage_Base/test/*.h
                                RendererCommands/test/*.cpp
                                RendererCommands/test/*.h
                                ramses-renderer-impl/test/*.h
                                ramses-renderer-impl/test/*.cpp
        DEPENDENCIES            ramses-renderer-lib
                                ramses-client
                                ramses-gmock-main
                                RendererTestUtils
                                FrameworkTestUtils
    )

    makeTestFromTarget(
        TARGET ramses-renderer-lib-test
        SUFFIX UNITTEST)
endif()
