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

function(folderizeTarget tgt)
    # skip interface libs because VS generator ignores INTERFACE_FOLDER property
    get_target_property(tgt_type ${tgt} TYPE)
    if (tgt_type STREQUAL INTERFACE_LIBRARY)
        return()
    endif()

    cmakePathToFolderName(folderName)
    set_property(TARGET ${tgt} PROPERTY FOLDER "${folderName}")

    # sort sources in groups
    get_target_property(tgt_content ${tgt} SOURCES)
    if (tgt_content)
        foreach(file_iter ${tgt_content})
            string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" tmp1 "${file_iter}")
            string(REGEX REPLACE "/[^/]*$" "" tmp2 "${tmp1}")
            string(REPLACE "/" "\\" module_internal_path "${tmp2}")
            source_group(${module_internal_path} FILES ${file_iter})
        endforeach()
    endif()
endfunction()
