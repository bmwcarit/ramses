#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB
    RAMSES_CLIENT_FILES_SOURCE
    ramses-client/impl/*.cpp
    ramses-client/impl/glslEffectBlock/*.cpp
    ramses-client/impl/ClientCommands/*.cpp
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
        ramses-client/impl/ramses-text/*.h
        ramses-text-api/include/*.h
        ramses-text-api/include/ramses-text-api/*.h
    )

    file(GLOB
        RAMSES_CLIENT_TEXT_FILES_SOURCE
        ramses-client/impl/ramses-text/*.cpp
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


# Logic hacky integration TODO fix and improve

if(ramses-sdk_DISABLE_LOGIC)
    set(ramses_logic_api_include_base "")
    set(ramses_logic_internal_includes "")
    set(ramses_logic_src_files "")
    set(ramses_logic_deps "")
else()
    if(ramses-sdk_ENABLE_FLATBUFFERS_GENERATION)
        include(${CMAKE_CURRENT_SOURCE_DIR}/logic/cmake/flatbuffersGeneration.cmake)
    endif()

    file(GLOB
        ramses_logic_api_include_base
        logic/include)

    set(ramses_logic_internal_includes
        logic/include
        logic/lib
        logic/lib/flatbuffers
        )

    file(GLOB logic_public_headers        logic/include/ramses-logic/*.h)
    file(GLOB logic_impl_headers          logic/lib/impl/*.h)
    file(GLOB logic_internal_headers      logic/lib/internals/*.h)
    file(GLOB logic_flatbuf_gen_headers   logic/lib/flatbuffers/generated/*.h)
    file(GLOB logic_flatbuf_schemas       logic/lib/flatbuffers/schemas/*.fbs)

    file(GLOB logic_impl_src              logic/lib/impl/*.cpp)
    file(GLOB logic_internals_src         logic/lib/internals/*.cpp)

    # Attach sol headers to ramses-logic-obj to be able to navigate/debug with them
    # Workaround for MSVC (ignores interface targets otherwise if not attached to a non-interface target)
    file(GLOB logic_sol_headers
        ${PROJECT_SOURCE_DIR}/external/sol/include/sol/*.hpp
        ${PROJECT_SOURCE_DIR}/external/sol/include/sol/compatibility/*
        )

    set(ramses_logic_src_files
        ${logic_public_headers}
        ${logic_impl_headers}
        ${logic_internal_headers}
        ${logic_flatbuf_gen_headers}
        ${logic_flatbuf_schemas}
        ${logic_impl_src}
        ${logic_internals_src}
    )

    # This is the only robust way to add files to a VS project with CMake
    # without having them treated as source files
    set_source_files_properties(${logic_flatbuf_gen_headers} PROPERTIES HEADER_FILE_ONLY TRUE)
    set_source_files_properties(${logic_flatbuf_schemas} PROPERTIES HEADER_FILE_ONLY TRUE)
    set_source_files_properties(${logic_sol_headers} PROPERTIES HEADER_FILE_ONLY TRUE)

    set(ramses_logic_deps
        sol2::sol2
        lua::lua
        ramses::flatbuffers
        fmt::fmt
    )


    file(GLOB logic_tests_src
        logic/unittests/api/*.cpp
        logic/unittests/api/*.h
        logic/unittests/internal/*.cpp
        logic/unittests/internal/*.h
        logic/unittests/shared/*.cpp
        logic/unittests/shared/*.h
    )

    set(logic_tests_include_dirs
        logic/lib/flatbuffers
        logic/lib
        logic/unittests/shared
    )

    set(logic_tests_res
        logic/unittests/res
    )

    # TODO fix this, doesn't work with the ramses build system...
    #if(TARGET FlatbufGen)
    #    set(ramses_logic_deps ${ramses_logic_deps} FlatbufGen)
    #endif()
endif()


target_include_directories(ramses-client-api INTERFACE ${RAMSES_CLIENT_API_INCLUDE_BASE} ${RAMSES_CLIENT_TEXT_API_INCLUDE_BASE} ${ramses_logic_api_include_base})

createModule(
    NAME                    ramses-client
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           ramses-client/impl
                            ${ramses_logic_internal_includes}
    SRC_FILES               ramses-client/impl/*.h
                            ramses-client/impl/glslEffectBlock/*.h
                            ramses-client/impl/ClientCommands/*.h
                            ramses-client-api/include/*.h
                            ramses-client-api/include/ramses-client-api/*.h
                            ${RAMSES_CLIENT_FILES_SOURCE}
                            ${RAMSES_CLIENT_TEXT_FILES_SOURCE}
                            ${ramses_logic_src_files}
                            ${client_text_includes}
    DEPENDENCIES            ramses-client-api
                            ramses-framework
                            ramses-glslang
                            ${client_text_dependencies}
                            ${ramses_logic_deps}
)

if(ramses-sdk_ENABLE_INSTALL)
    install(DIRECTORY ramses-client-api/include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
    if(ramses-sdk_TEXT_SUPPORT)
        install(DIRECTORY ramses-text-api/include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
    endif()
    if(NOT ramses-sdk_DISABLE_LOGIC)
        install(DIRECTORY logic/include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-client PRIVATE "-Wsuggest-override")
endif()


if (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    ramses-client-test
        TYPE                    BINARY
        INCLUDE_PATHS           test
                                ${logic_tests_include_dirs}
        SRC_FILES               test/*.h
                                test/ClientCommands/*.h
                                ${RAMSES_CLIENT_TEXT_TEST_FILES_INCLUDES}
                                test/*.cpp
                                ${RAMSES_CLIENT_TEXT_TEST_FILES_SOURCE}
                                test/ClientCommands/*.cpp
                                ${logic_tests_src}
        RESOURCE_FOLDERS        test/res
                                ${logic_tests_res}
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
    INCLUDE_PATHS ${RAMSES_CLIENT_API_INCLUDE_BASE} ${RAMSES_CLIENT_TEXT_API_INCLUDE_BASE} ${ramses_logic_api_include_base}
    SRC_FILES ${RAMSES_CLIENT_FILES_SOURCE} ${RAMSES_CLIENT_TEXT_FILES_SOURCE} ${ramses_logic_src_files}
    DEPENDENCIES  ramses-client
    CACHE INTERNAL "")
