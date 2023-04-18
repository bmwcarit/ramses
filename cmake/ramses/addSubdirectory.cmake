#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


function(addSubdirectory)
    cmake_parse_arguments(SUBDIR "" "PATH;MODE" "" ${ARGV})

    if(SUBDIR_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed addSubdirectories properties: '${SUBDIR_UNPARSED_ARGUMENTS}'")
    endif()

    # TODO this global variable is not needed, FIND_PACKAGE has its own caching
    set(GLOBAL_WILL_NOT_FIND_DEPENDENCY "" CACHE INTERNAL "")

    string(REPLACE "/" "_" CONTENT_STRING "${SUBDIR_PATH}")
    set(ramses-sdk_${CONTENT_STRING} "${SUBDIR_MODE}" CACHE STRING "")
    set_property(CACHE ramses-sdk_${CONTENT_STRING} PROPERTY STRINGS ON OFF AUTO)

    if("${ramses-sdk_${CONTENT_STRING}}" STREQUAL "OFF")
        message(STATUS "- ${SUBDIR_PATH} ${CONTENT_STRING}")
    endif()

    if("${ramses-sdk_${CONTENT_STRING}}" STREQUAL "AUTO")
        set(GLOBAL_MODULE_DEPENDENCY_CHECK ON)
    else()
        set(GLOBAL_MODULE_DEPENDENCY_CHECK OFF)
    endif()

    if(NOT "${ramses-sdk_${CONTENT_STRING}}" STREQUAL "OFF")
        add_subdirectory(${SUBDIR_PATH})
    endif()
endfunction()
