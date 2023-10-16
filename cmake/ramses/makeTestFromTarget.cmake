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
        ""                  # Options
        "TARGET;SUFFIX"     # Single-value args
        "EXTRA_ARGS"        # Multi-value-args
        ${ARGN}
    )

    if (TEST_SUFFIX STREQUAL "")
        message(FATAL_ERROR "makeTestFromTarget: SUFFIX not specified for '${TEST_TARGET}'")
    endif()

    set(TEST_NAME "${TEST_TARGET}_${TEST_SUFFIX}")

    if (NOT TARGET ${TEST_TARGET})
        message(FATAL_ERROR "makeTestFromTarget: Test target '${TEST_TARGET}' not found")
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


function(makeTestPerWindowTypeFromTarget)
    cmake_parse_arguments(
        PTEST                                   # Prefix of parsed args
        ""                                      # Options (Unused)
        "TARGET;SUFFIX"                         # Single-value args
        "WINDOW_TYPE_FILTER;EXTRA_ARGS"         # Multi-value-args
        ${ARGN}
    )

    if(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS)
        list(APPEND TEST_PLATFORMS "windows gles30")
        list(APPEND TEST_PLATFORMS "windows gl42")
        list(APPEND TEST_PLATFORMS "windows gl45")
    endif()

    if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
        list(APPEND TEST_PLATFORMS "x11 gles30")
    endif()

    if(ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID)
        list(APPEND TEST_PLATFORMS "android gles30")
    endif()

    if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
        list(APPEND TEST_PLATFORMS "wayland-ivi gles30")
    endif()

    if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
        list(APPEND TEST_PLATFORMS "wayland-wl-shell gles30")
    endif()

    foreach(TEST_PLATFORM IN LISTS TEST_PLATFORMS)
        string(REPLACE " " ";" TEST_PLATFORM_ ${TEST_PLATFORM})
        list(GET TEST_PLATFORM_ 0 TEST_PLATFORM_WINDOW)
        list(GET TEST_PLATFORM_ 1 TEST_PLATFORM_DEVICE)

        #if filter exists and window types is not in filter -> continue
        if(PTEST_WINDOW_TYPE_FILTER)
            if(NOT TEST_PLATFORM_WINDOW IN_LIST PTEST_WINDOW_TYPE_FILTER)
                continue()
            endif()
        endif()

        makeTestFromTarget(
            TARGET ${PTEST_TARGET}
            SUFFIX ${TEST_PLATFORM_WINDOW}-${TEST_PLATFORM_DEVICE}_${PTEST_SUFFIX}
            EXTRA_ARGS ${PTEST_EXTRA_ARGS} --window-type ${TEST_PLATFORM_WINDOW} --device-type ${TEST_PLATFORM_DEVICE}
            ${PTEST_UNPARSED_ARGUMENTS}
        )

    endforeach()
endfunction()
