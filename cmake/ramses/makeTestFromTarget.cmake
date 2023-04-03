#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

function(makeTestFromTarget)
    cmake_parse_arguments(
        TEST                # Prefix of parsed args
        "SKIPPABLE"         # Options
        "TARGET;SUFFIX"     # Single-value args
        "EXTRA_ARGS"        # Multi-value-args
        ${ARGN}
    )

    # If not specified, the test is assumed to be a unit test
    if (TEST_SUFFIX STREQUAL "")
        message(FATAL_ERROR "makeTestFromTarget: SUFFIX not specified for '${TEST_TARGET}'")
    endif()

    set(TEST_NAME "${TEST_TARGET}_${TEST_SUFFIX}")

    if (NOT TARGET ${TEST_TARGET})
        if(TEST_SKIPPABLE)
            message(STATUS "Skipping test '${TEST_NAME}' because it's executable ${TEST_TARGET} is missing")
            return()
        else()
            message(FATAL_ERROR "makeTestFromTarget: Test target '${TEST_TARGET}' not found")
        endif()
    endif()

    # TODO: tests should not have to be installed, this line could be deleted after tests have been refactored to
    # not have the requirement to be installed
    if(ramses-sdk_ENABLE_INSTALL)
        install(TARGETS ${TEST_TARGET} DESTINATION ${RAMSES_INSTALL_RUNTIME_PATH} COMPONENT ramses-tests)
    endif()

    add_test(
        NAME ${TEST_NAME}
        COMMAND ${TEST_TARGET} --gtest_output=xml:${TEST_NAME}.xml ${TEST_EXTRA_ARGS}
        WORKING_DIRECTORY ${RAMSES_INSTALL_RUNTIME_PATH}
        )

    # attach environment variable for clang coverage
    set_tests_properties(${TEST_NAME} PROPERTIES
        ENVIRONMENT LLVM_PROFILE_FILE=${TEST_NAME}_%p.profraw)
endfunction()
