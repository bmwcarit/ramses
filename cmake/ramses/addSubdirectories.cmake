#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


function(addSubdirectories)
    cmake_parse_arguments(PROJECT "" "" "CONTENT" ${ARGV})

    if(${PROJECT_UNPARSED_ARGUMENTS})
        message(FATAL_ERROR "Unparsed addSubdirectories properties: '${PROJECT_UNPARSED_ARGUMENTS}'")
    endif()

    createBuildConfig()

    # TODO this global variable is not needed, FIND_PACKAGE has its own caching
    set(GLOBAL_WILL_NOT_FIND_DEPENDENCY "" CACHE INTERNAL "")

    set(LAST_STATE "ON")
    foreach(CONTENT ${PROJECT_CONTENT})
        if(("${CONTENT}" STREQUAL "ON") OR ("${CONTENT}" STREQUAL "OFF") OR ("${CONTENT}" STREQUAL "AUTO"))
            set(LAST_STATE "${CONTENT}")
        else()
            string(REPLACE "/" "_" CONTENT_STRING "${CONTENT}")
            set(ramses-sdk_${CONTENT_STRING} "${LAST_STATE}" CACHE STRING "")
            set_property(CACHE ramses-sdk_${CONTENT_STRING} PROPERTY STRINGS ON OFF AUTO)

            if("${ramses-sdk_${CONTENT_STRING}}" STREQUAL "OFF")
                message(STATUS "- ${CONTENT}")
            endif()

            if("${ramses-sdk_${CONTENT_STRING}}" STREQUAL "AUTO")
                if (EXISTS "${PROJECT_SOURCE_DIR}/${CONTENT}")
                    set(GLOBAL_MODULE_DEPENDENCY_CHECK ON)
                    add_subdirectory(${CONTENT})
                endif()
            endif()

            if("${ramses-sdk_${CONTENT_STRING}}" STREQUAL "ON")
                set(GLOBAL_MODULE_DEPENDENCY_CHECK OFF)
                add_subdirectory(${CONTENT})
            endif()
        endif()
    endforeach()
endfunction()
