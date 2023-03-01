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

if(ramses-sdk_TEXT_SUPPORT)
    set(client_text_dependencies
        freetype
        harfbuzz)
    set(client_text_includes
        impl/ramses-text/*.h
        ramses-text-api/include/*.h
        ramses-text-api/include/ramses-text-api/*.h
    )

    file(GLOB
        RAMSES_CLIENT_TEXT_FILES_SOURCE
        impl/ramses-text/*.cpp
        ramses-text-api/*.cpp)
    file(GLOB RAMSES_CLIENT_TEXT_API_INCLUDE_BASE
        ramses-text-api/include)
    file(GLOB
        RAMSES_CLIENT_TEXT_TEST_FILES_SOURCE
        test/text/*.cpp)
    file(GLOB
        RAMSES_CLIENT_TEXT_TEST_FILES_INCLUDES
        test/text/*.h)

    target_compile_definitions(ramses-client-api INTERFACE RAMSES_TEXT_ENABLED)
else()
    set(client_text_dependencies "")
    set(client_text_includes "")

    set(RAMSES_CLIENT_TEXT_FILES_SOURCE "")
    set(RAMSES_CLIENT_TEXT_API_INCLUDE_BASE "")
    set(RAMSES_CLIENT_TEXT_TEST_FILES_SOURCE "")
    set(RAMSES_CLIENT_TEXT_TEST_FILES_INCLUDES "")

endif()

target_include_directories(ramses-client-api INTERFACE ${RAMSES_CLIENT_API_INCLUDE_BASE} ${RAMSES_CLIENT_TEXT_API_INCLUDE_BASE})

createModule(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-client
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_PATHS           impl
    SRC_FILES               impl/*.h
                            impl/glslEffectBlock/*.h
                            impl/ClientCommands/*.h
                            ramses-client-api/include/*.h
                            ramses-client-api/include/ramses-client-api/*.h
                            ${RAMSES_CLIENT_FILES_SOURCE}
                            ${RAMSES_CLIENT_TEXT_FILES_SOURCE}
                            ${client_text_includes}
    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-client-api
                            ramses-framework
                            ramses-glslang
                            ${client_text_dependencies}
)

install(DIRECTORY ramses-client-api/include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
if(ramses-sdk_TEXT_SUPPORT)
    install(DIRECTORY ramses-text-api/include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-client PRIVATE "-Wsuggest-override")
endif()


if (${ramses-sdk_BUILD_TESTS})
    createModule(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    ramses-client-test
        TYPE                    BINARY

        #==========================================================================
        # files of this module
        #==========================================================================
        INCLUDE_PATHS           test
        SRC_FILES               test/*.h
                                test/ClientCommands/*.h
                                ${RAMSES_CLIENT_TEXT_TEST_FILES_INCLUDES}
                                test/*.cpp
                                ${RAMSES_CLIENT_TEXT_TEST_FILES_SOURCE}
                                test/ClientCommands/*.cpp
        RESOURCE_FOLDERS        test/res

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            ramses-client
                                FrameworkTestUtils
                                ramses-gmock-main
    )

    makeTestFromTarget(
        TARGET ramses-client-test
        SUFFIX UNITTEST)
endif()

set(ramses-shared-lib-MIXIN
    ${ramses-shared-lib-MIXIN}
    INCLUDE_PATHS ${RAMSES_CLIENT_API_INCLUDE_BASE} ${RAMSES_CLIENT_TEXT_API_INCLUDE_BASE}
    SRC_FILES ${RAMSES_CLIENT_FILES_SOURCE} ${RAMSES_CLIENT_TEXT_FILES_SOURCE}
    DEPENDENCIES  ramses-client
    CACHE INTERNAL "")
