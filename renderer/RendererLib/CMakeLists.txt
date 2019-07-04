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
    ramses-renderer-impl/src/*.cpp
    ramses-renderer-impl/src/DisplayManager/*.cpp)


add_library(ramses-renderer-api INTERFACE)
target_include_directories(ramses-renderer-api INTERFACE ${RAMSES_RENDERER_API_INCLUDE_BASE})

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-renderer-lib
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================

    FILES_PRIVATE_HEADER    ramses-renderer-api/include/*.h
                            ramses-renderer-api/include/ramses-renderer-api/*.h

    # This needs to be fixed in upcoming story regarding renderer lib
    # Not possible to solve now, without splitting in two libs (which is the correct solution)
    INCLUDE_BASE            ramses-renderer-impl/include
    FILES_PRIVATE_HEADER    ramses-renderer-impl/include/*.h
                            ramses-renderer-impl/include/DisplayManager/*.h
    FILES_SOURCE            ${RAMSES_RENDERER_FILES_SOURCE}

    INCLUDE_BASE            RendererAPI/include
    FILES_PRIVATE_HEADER    RendererAPI/include/RendererAPI/*.h
    FILES_SOURCE            RendererAPI/src/*.cpp

    INCLUDE_BASE            Platform_Base/include
    FILES_PRIVATE_HEADER    Platform_Base/include/Platform_Base/*.h
    FILES_SOURCE            Platform_Base/src/*.cpp

    INCLUDE_BASE            RendererCommands/include
    FILES_PRIVATE_HEADER    RendererCommands/include/RendererCommands/*.h
    FILES_SOURCE            RendererCommands/src/*.cpp

    INCLUDE_BASE            RendererFramework/include
    FILES_PRIVATE_HEADER    RendererFramework/include/RendererFramework/*.h
                            RendererFramework/include/*.h
    FILES_SOURCE            RendererFramework/src/*.cpp

    INCLUDE_BASE            RendererLib/include
    FILES_PRIVATE_HEADER    RendererLib/include/RendererLib/*.h
                            RendererLib/include/*.h
    FILES_SOURCE            RendererLib/src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-api
                            ramses-framework
)

INSTALL(DIRECTORY ramses-renderer-api/include/ DESTINATION "${PROJECT_INSTALL_HEADER}" COMPONENT ramses-sdk-devel)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-renderer-lib PRIVATE "-Wsuggest-override")
endif()

IF (NOT ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB)
    set(ramses-shared-lib-MIXIN
        ${ramses-shared-lib-MIXIN}
        INCLUDE_BASE ${RAMSES_RENDERER_API_INCLUDE_BASE}
        FILES_SOURCE ${RAMSES_RENDERER_FILES_SOURCE}
        DEPENDENCIES  ramses-renderer-lib
        CACHE INTERNAL "")
endif()


IF (${ramses-sdk_BUILD_TESTS})
    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    RendererTestUtils
        TYPE                    STATIC_LIBRARY

        #==========================================================================
        # files of this module
        #==========================================================================

        INCLUDE_BASE            RendererTestUtils/include
        FILES_PRIVATE_HEADER    RendererTestUtils/include/*.h
        FILES_SOURCE            RendererTestUtils/src/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            ramses-renderer-lib
                                FrameworkTestUtils
    )
ENDIF()

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-renderer-lib-test
    TYPE                    TEST

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_SOURCE            RendererFramework/test/*.cpp
                            RendererFramework/test/*.h
                            RendererAPI/test/*.cpp
                            RendererAPI/test/*.h
                            RendererLib/test/*.cpp
                            RendererLib/test/*.h
                            ShaderLanguage_Base/test/*.cpp
                            ShaderLanguage_Base/test/*.h
                            RendererCommands/test/*.cpp
                            RendererCommands/test/*.h

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-lib
                            ramses-gmock-main
                            RendererTestUtils
                            FrameworkTestUtils
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-renderer-test
    TYPE                    TEST

    #==========================================================================
    # files of this module
    #==========================================================================

    FILES_SOURCE            ramses-renderer-impl/test/*.h
                            ramses-renderer-impl/test/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-lib
                            ramses-client
                            ramses-gmock-main
                            RendererTestUtils
                            FrameworkTestUtils
)
