#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# Converts e.g. "/some/path/" to "some-path"
function(cmakePathToFolderName OUT)
    # first get path relative to ramses root dir
    string(REGEX REPLACE "${PROJECT_SOURCE_DIR}/" "" relative_path "${CMAKE_CURRENT_SOURCE_DIR}")
    string(REGEX REPLACE "/[^/]*$" "" folder_path "${relative_path}")
    if (ramses-sdk_FOLDER_PREFIX)
        # use user provided prefix if given
        set(folder_path "${ramses-sdk_FOLDER_PREFIX}/${folder_path}")
    elseif (NOT CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        # otherwise adjust path for ramses used as subdirectory
        string(REGEX REPLACE "${CMAKE_SOURCE_DIR}/" "" folder_prefix_path "${PROJECT_SOURCE_DIR}")
        set(folder_path "${folder_prefix_path}/${folder_path}")
    endif()
    set(${OUT} ${folder_path} PARENT_SCOPE)
endfunction()

function(folderizeTarget TARGET_NAME)
    cmakePathToFolderName(folderName)
    set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "${folderName}")

    # sort sources in groups
    get_target_property(TARGET_SOURCE_DIR ${TARGET_NAME} SOURCE_DIR)

    get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)

    foreach(SOURCE_PATH ${TARGET_SOURCES})
        get_filename_component(SOURCE_FULL_PATH "${SOURCE_PATH}" ABSOLUTE)
        get_filename_component(SOURCE_FULL_DIR "${SOURCE_FULL_PATH}" DIRECTORY)
        file(RELATIVE_PATH SOURCE_RELATIVE_DIR "${TARGET_SOURCE_DIR}" "${SOURCE_FULL_DIR}")
        if(NOT "${SOURCE_RELATIVE_DIR}" STREQUAL "")
            source_group(${SOURCE_RELATIVE_DIR} FILES ${SOURCE_FULL_PATH})
        endif()
    endforeach()
endfunction()
