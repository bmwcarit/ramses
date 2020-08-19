#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB
    RAMSES_CLIENT_FILES_SOURCE
    impl/*.cpp
    impl/glslEffectBlock/*.cpp
    impl/ClientCommands/*.cpp
    ramses-client-api/*.cpp
    impl/ramses-text/*.cpp
    ramses-text-api/*.cpp)
file(GLOB
    RAMSES_CLIENT_API_INCLUDE_BASE
    ramses-client-api/include
    ramses-text-api/include)


add_library(ramses-client-api INTERFACE)
target_include_directories(ramses-client-api INTERFACE ${RAMSES_CLIENT_API_INCLUDE_BASE})

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-client
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            impl
    FILES_PRIVATE_HEADER    impl/*.h
                            impl/glslEffectBlock/*.h
                            impl/ClientCommands/*.h
                            ramses-client-api/include/*.h
                            ramses-client-api/include/ramses-client-api/*.h
    FILES_SOURCE            ${RAMSES_CLIENT_FILES_SOURCE}
                            impl/ramses-text/*.h
                            ramses-text-api/include/*.h
                            ramses-text-api/include/ramses-text-api/*.h
    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-client-api
                            ramses-framework
                            ramses-glslang
                            freetype
                            harfbuzz
)

INSTALL(DIRECTORY ramses-client-api/include/ DESTINATION "${PROJECT_INSTALL_HEADER}" COMPONENT ramses-sdk-devel)
INSTALL(DIRECTORY ramses-text-api/include/ DESTINATION "${PROJECT_INSTALL_HEADER}" COMPONENT ramses-sdk-devel)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-client PRIVATE "-Wsuggest-override")
endif()

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-client-test
    TYPE                    TEST

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            test
    FILES_PRIVATE_HEADER    test/*.h
                            test/ClientCommands/*.h
    FILES_SOURCE            test/*.cpp
                            test/ClientCommands/*.cpp
    RESOURCE_FOLDER         test/res

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-client
                            FrameworkTestUtils
                            ramses-gmock-main
)

set(ramses-shared-lib-MIXIN
    ${ramses-shared-lib-MIXIN}
    INCLUDE_BASE ${RAMSES_CLIENT_API_INCLUDE_BASE}
    FILES_SOURCE ${RAMSES_CLIENT_FILES_SOURCE}
    DEPENDENCIES  ramses-client
    CACHE INTERNAL "")
