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
    ramses-client-api/*.cpp)
file(GLOB
    RAMSES_CLIENT_API_INCLUDE_BASE
    ramses-client-api/include)


add_library(ramses-client-api INTERFACE)
target_include_directories(ramses-client-api INTERFACE ${RAMSES_CLIENT_API_INCLUDE_BASE})

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-client
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

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

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-client-api
                            ramses-framework
                            ramses-glslang
)

INSTALL(DIRECTORY ramses-client-api/include/ DESTINATION "${PROJECT_INSTALL_HEADER}" COMPONENT ramses-sdk-devel)


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
    FILES_RESOURCE          test/res/*

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-client
                            FrameworkTestUtils
                            gmock_main
)

set(ramses-shared-lib-MIXIN
    ${ramses-shared-lib-MIXIN}
    INCLUDE_BASE ${RAMSES_CLIENT_API_INCLUDE_BASE}
    FILES_SOURCE ${RAMSES_CLIENT_FILES_SOURCE}
    DEPENDENCIES  ramses-client
    CACHE INTERNAL "")
