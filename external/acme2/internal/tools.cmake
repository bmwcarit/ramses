############################################################################
#
# Copyright (C) 2014 BMW Car IT GmbH
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

#==============================================================================
# logging
#==============================================================================

MACRO(ACME_INFO)
    MESSAGE(STATUS "${ARGV}")
ENDMACRO()

MACRO(ACME_ERROR)
    message(FATAL_ERROR "ERROR: ${ARGV}")
ENDMACRO()

#==============================================================================
# add_test helper
#==============================================================================

MACRO(ACME_ADD_TEST test_target test_suffix)
    if (NOT TARGET ${test_target})
        ACME_ERROR("ACME_ADD_TEST: Target ${test_target} not found")
    endif()

    list(FIND PROJECT_ALLOWED_TEST_SUFFIXES ${test_suffix} ACME_ADD_TEST_SUFFIX_FOUND)
    if (ACME_ADD_TEST_SUFFIX_FOUND EQUAL -1)
        ACME_ERROR("ACME_ADD_TEST: Test suffix ${test_suffix} invalid. Allowed are ${PROJECT_ALLOWED_TEST_SUFFIXES}")
    endif()

    ADD_TEST(
        NAME ${test_target}_${test_suffix}
        COMMAND ${test_target} --gtest_output=xml:${test_target}_${test_suffix}.xml ${ARGN}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )

    if (${ACME_ENABLE_TEST_PROPERTIES})
        # attach environment variable for clang coverage
        set_tests_properties(${test_target}_${test_suffix} PROPERTIES
            ENVIRONMENT LLVM_PROFILE_FILE=${test_target}_${test_suffix}_%p.profraw)
    endif()
ENDMACRO()

#==============================================================================
# postprocessing of target
#==============================================================================

function(ACME_CURRENT_FOLDER_PATH OUT)
    # extract and set folder name from path
    # first get path relative to ramses root dir
    string(REGEX REPLACE "${PROJECT_SOURCE_DIR}/" "" ACME_relative_path "${CMAKE_CURRENT_SOURCE_DIR}")
    string(REGEX REPLACE "/[^/]*$" "" ACME_folder_path "${ACME_relative_path}")
    if (NOT CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        # optionally adjust path for ramses used as subdirectory
        string(REGEX REPLACE "${CMAKE_SOURCE_DIR}/" "" ACME_folder_prefix_path "${PROJECT_SOURCE_DIR}")
        set(ACME_folder_path "${ACME_folder_prefix_path}/${ACME_folder_path}")
    endif()
    set(${OUT} ${ACME_folder_path} PARENT_SCOPE)
endfunction()

function(ACME_FOLDERIZE_TARGET tgt)
    # skip interface libs because VS generator ignore INTERFACE_FOLDER property
    get_target_property(tgt_type ${tgt} TYPE)
    if (tgt_type STREQUAL INTERFACE_LIBRARY)
        return()
    endif()

    ACME_CURRENT_FOLDER_PATH(ACME_folder_path)
    set_property(TARGET ${tgt} PROPERTY FOLDER "${ACME_folder_path}")

    # sort sources in goups
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

function(ACME_FOLDERIZE_TARGETS)
    foreach (tgt ${ARGV})
        ACME_FOLDERIZE_TARGET(${tgt})
    endforeach()
endfunction()

#==============================================================================
# resource copying to buildfolder + install
#==============================================================================

function(ACME_COPY_RESOURCES_FOR_TARGET tgt)
    if (NOT TARGET ${tgt})
        message(FATAL_ERROR "Target ${tgt} does not exist")
    endif()

    # parse args
    cmake_parse_arguments(RES "" "ENABLE_INSTALL" "" ${ARGN})
    set(res_folders ${RES_UNPARSED_ARGUMENTS})

    # install whole folders if requested
    if (RES_ENABLE_INSTALL)
        foreach(user_dir ${res_folders})
            install(DIRECTORY ${user_dir}/ DESTINATION ${ACME_INSTALL_RESOURCE} COMPONENT "${ACME_PACKAGE_NAME}")
        endforeach()
    endif()

    # create copy target for directories
    foreach(user_dir ${res_folders})
        get_filename_component(dir "${user_dir}" ABSOLUTE)

        if (NOT IS_DIRECTORY "${dir}")
            message(FATAL_ERROR "${tgt} has invalid RESOURCE_FOLDER ${user_dir}")
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
        target_sources(${tgt} PRIVATE ${dir_files_src})

        # check if already copy target fir dir
        get_property(dir_copy_target DIRECTORY "${PROJECT_BASE_DIR}" PROPERTY ACME_DIR_COPY_${dir_hash})
        if (dir_copy_target)
            add_dependencies(${tgt} ${dir_copy_target})
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

            add_dependencies(${tgt} ${target_name})

            # store target name
            set_property(DIRECTORY ${PROJECT_BASE_DIR} PROPERTY ACME_DIR_COPY_${dir_hash} ${target_name})
        endif()

        # TODO check uniqueness (?)

    endforeach()
endfunction()
