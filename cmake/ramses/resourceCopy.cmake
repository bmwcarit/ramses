#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

function(copyResourcesForTarget)
    cmake_parse_arguments(
        RES                                 # Prefix of parsed args
        ""                                  # Options
        "TARGET;INSTALL;INSTALL_COMPONENT"  # Single-value args
        "FOLDERS"                           # Multi-value-args
        ${ARGN}
    )

    if (NOT TARGET ${RES_TARGET})
        message(FATAL_ERROR "Target ${RES_TARGET} does not exist")
    endif()

    # install whole folders if requested
    # TODO: don't install resources (fix tests and remove installation here)
    if (RES_INSTALL AND ramses-sdk_ENABLE_INSTALL)
        foreach(res_folder ${RES_FOLDERS})
            install(DIRECTORY ${res_folder}/ DESTINATION ${RAMSES_INSTALL_RESOURCES_PATH} COMPONENT "${RES_INSTALL_COMPONENT}")
        endforeach()
    endif()

    # create copy target for directories
    foreach(res_folder ${RES_FOLDERS})
        get_filename_component(dir "${res_folder}" ABSOLUTE)

        if (NOT IS_DIRECTORY "${dir}")
            message(FATAL_ERROR "${RES_TARGET} has invalid RESOURCE_FOLDERS entry ${res_folder}")
        endif()

        # generate dir target name
        string(MD5 dir_hash "${dir}")
        set(target_name "rescopy-${dir_hash}")

        # collect files
        set(output_dir_base "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res")
        file(GLOB_RECURSE dir_files_rel RELATIVE "${dir}" "${dir}/*")
        set(dir_files_src)
        set(dir_files_dst)
        foreach (file ${dir_files_rel})
            list(APPEND dir_files_src "${dir}/${file}")
            list(APPEND dir_files_dst "${output_dir_base}/${file}")
        endforeach()

        # add files to target sources
        target_sources(${RES_TARGET} PRIVATE ${dir_files_src})

        # check if already copy target fir dir
        get_property(dir_copy_target DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTY ACME_DIR_COPY_${dir_hash})
        if (dir_copy_target)
            add_dependencies(${RES_TARGET} ${dir_copy_target})
        else()
            # no copy target yet, create one
            add_custom_command(
                OUTPUT ${dir_files_dst}
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${dir}" "${output_dir_base}"
                DEPENDS "${dir_files_src}"
                COMMENT "Copying ${dir} -> ${output_dir_base}"
                )

            add_custom_target(${target_name} DEPENDS ${dir_files_dst})
            set_property(TARGET ${target_name} PROPERTY FOLDER "CMakePredefinedTargets/rescopy")

            add_dependencies(${RES_TARGET} ${target_name})

            # store target name
            set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY ACME_DIR_COPY_${dir_hash} ${target_name})
        endif()

        # TODO check uniqueness (?)

    endforeach()
endfunction()
