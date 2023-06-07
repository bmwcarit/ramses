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

    if(${SUBDIR_MODE} STREQUAL "AUTO")
        set(GLOBAL_MODULE_DEPENDENCY_CHECK ON)
    elseif(${SUBDIR_MODE} STREQUAL "ON")
        set(GLOBAL_MODULE_DEPENDENCY_CHECK OFF)
    else()
        message(FATAL_ERROR "Unsupported mode passed to addSubdirectories: `${SUBDIR_MODE}`")
    endif()

    add_subdirectory(${SUBDIR_PATH})
endfunction()
