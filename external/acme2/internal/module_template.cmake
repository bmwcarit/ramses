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

SET(TARGET_CONTENT
    ${ACME_FILES_SOURCE}
)

IF (TARGET ${ACME_NAME})
    ACME_ERROR("Target ${ACME_NAME} already exists")
ENDIF()

IF("${TARGET_CONTENT}" STREQUAL "")
    ACME_ERROR("Target ${ACME_NAME} has no files")
ENDIF()

if (NOT "${ACME_FILES_RESOURCE}" STREQUAL "")
    message(FATAL_ERROR "Replace use of FILES_RESOURCE in target '${ACME_NAME}' with RESOURCE_FOLDER")
endif()

#==============================================================================================
IF("${ACME_TYPE}" STREQUAL "STATIC_LIBRARY")
    #==============================================================================================
    ADD_LIBRARY(${ACME_NAME} STATIC ${TARGET_CONTENT})

    #==============================================================================================
ELSEIF("${ACME_TYPE}" STREQUAL "BINARY")
    #==============================================================================================
    ADD_EXECUTABLE(${ACME_NAME} ${TARGET_CONTENT})
    set_target_properties(${ACME_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    IF(ACME_ENABLE_INSTALL)
        INSTALL(TARGETS ${ACME_NAME} DESTINATION ${ACME_INSTALL_BINARY} COMPONENT "${ACME_PACKAGE_NAME}")
        if (MSVC)
            INSTALL(FILES $<TARGET_PDB_FILE:${ACME_NAME}> DESTINATION ${ACME_INSTALL_BINARY} CONFIGURATIONS Debug RelWithDebInfo)
        endif()
    ENDIF()

    #==============================================================================================
ELSEIF("${ACME_TYPE}" STREQUAL "TEST")
    #==============================================================================================
    LIST(FIND ACME_ALLOWED_TEST_SUFFIXES ${ACME_TEST_SUFFIX} _INDEX_OF_SUFFIX)
    IF(${_INDEX_OF_SUFFIX} EQUAL -1)
        ACME_ERROR("Your test module has invalid suffix: '${ACME_TEST_SUFFIX}'")
    ENDIF()

    ADD_EXECUTABLE(${ACME_NAME} ${TARGET_CONTENT})
    set_target_properties(${ACME_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    ADD_TEST(
        NAME "${ACME_NAME}_${ACME_TEST_SUFFIX}"
        COMMAND ${ACME_NAME} --gtest_output=xml:TestResult_${ACME_NAME}.xml
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )

    # TODO(tobias) stay compatible: tests are always installed when they are built
    INSTALL(TARGETS ${ACME_NAME} DESTINATION ${ACME_INSTALL_BINARY} COMPONENT "${ACME_PACKAGE_NAME}")
    if (MSVC)
        INSTALL(FILES $<TARGET_PDB_FILE:${ACME_NAME}> DESTINATION ${ACME_INSTALL_BINARY} CONFIGURATIONS Debug RelWithDebInfo)
    endif()

    if (${ACME_ENABLE_TEST_PROPERTIES})
        # attach environment variable for clang coverage
        set_tests_properties(${ACME_NAME}_${ACME_TEST_SUFFIX} PROPERTIES
            ENVIRONMENT LLVM_PROFILE_FILE=${ACME_NAME}_${ACME_TEST_SUFFIX}_%p.profraw)
    endif()

    #==============================================================================================
ELSEIF("${ACME_TYPE}" STREQUAL "SHARED_LIBRARY")
    #==============================================================================================
    ADD_LIBRARY(${ACME_NAME} SHARED ${TARGET_CONTENT})
    IF(ACME_ENABLE_INSTALL)
        if (MSVC)
            INSTALL(TARGETS ${ACME_NAME} DESTINATION ${ACME_INSTALL_BINARY} COMPONENT "${ACME_PACKAGE_NAME}")
            INSTALL(FILES $<TARGET_PDB_FILE:${ACME_NAME}> DESTINATION ${ACME_INSTALL_BINARY} CONFIGURATIONS Debug RelWithDebInfo)
        else()
            INSTALL(TARGETS ${ACME_NAME} DESTINATION ${ACME_INSTALL_SHARED_LIB} COMPONENT "${ACME_PACKAGE_NAME}")
        endif()
    ENDIF()

    #==============================================================================================
ELSEIF()
    #==============================================================================================
    ACME_ERROR("Your module has invalid type '${ACME_TYPE}'")
ENDIF()

#==============================================================================================
# set common ACME2 project properties
#==============================================================================================
SET_PROPERTY(TARGET ${ACME_NAME} PROPERTY LINKER_LANGUAGE     CXX)
SET_PROPERTY(TARGET ${ACME_NAME} PROPERTY VERSION             ${ACME_VERSION})


#==============================================================================================
if(ACME_RESOURCE_FOLDER)
#==============================================================================================
    # install directories
    if(${ACME_ENABLE_INSTALL} OR "${ACME_TYPE}" STREQUAL "TEST")
        foreach(user_dir ${ACME_RESOURCE_FOLDER})
            install(DIRECTORY ${user_dir}/ DESTINATION ${ACME_INSTALL_RESOURCE} COMPONENT "${ACME_PACKAGE_NAME}")
        endforeach()
    endif()

    # create copy target for directories
    foreach(user_dir ${ACME_RESOURCE_FOLDER})
        get_filename_component(dir "${user_dir}" ABSOLUTE)

        if (NOT IS_DIRECTORY "${dir}")
            message(FATAL_ERROR "${ACME_NAME} has invalid RESOURCE_FOLDER ${user_dir}")
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
        target_sources(${ACME_NAME} PRIVATE ${dir_files_src})

        # check if already copy target fir dir
        get_property(dir_copy_target DIRECTORY "${PROJECT_BASE_DIR}" PROPERTY ACME_DIR_COPY_${dir_hash})
        if (dir_copy_target)
            add_dependencies(${ACME_NAME} ${dir_copy_target})
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

            add_dependencies(${ACME_NAME} ${target_name})

            # store target name
            set_property(DIRECTORY ${PROJECT_BASE_DIR} PROPERTY ACME_DIR_COPY_${dir_hash} ${target_name})
        endif()

        # TODO check uniqueness (?)

    endforeach()
ENDIF()

#==============================================================================================
# add include dirs
#==============================================================================================
if (NOT "${ACME_INCLUDE_BASE}" STREQUAL "")
    target_include_directories(${ACME_NAME} PUBLIC "${ACME_INCLUDE_BASE}")
endif()

#==============================================================================================
# find and link dependencies
#==============================================================================================
FOREACH(DEPENDENCY ${ACME_DEPENDENCIES})

    IF(TARGET ${DEPENDENCY})
        # link target directly
        IF ("${ACME_TYPE}" STREQUAL "SHARED_LIBRARY")
            TARGET_LINK_LIBRARIES(${ACME_NAME} PRIVATE ${DEPENDENCY})
        ELSE()
            TARGET_LINK_LIBRARIES(${ACME_NAME} PUBLIC ${DEPENDENCY})
        ENDIF()

    ELSE()
        # ensure it was already found by outside dependency checker
        if (NOT ${DEPENDENCY}_FOUND)
            ACME_ERROR("${ACME_NAME}: Missing dependency ${DEPENDENCY}")
        endif()

        # link includes and libs from vars
        if (${DEPENDENCY}_INCLUDE_DIRS)
            target_include_directories(${ACME_NAME} PUBLIC ${${DEPENDENCY}_INCLUDE_DIRS})
        endif()

        if(${DEPENDENCY}_LIBRARIES)
            target_link_libraries(${ACME_NAME} PUBLIC ${${DEPENDENCY}_LIBRARIES})
        endif()
    ENDIF()

ENDFOREACH()

#==============================================================================================
# ensure VS IDE folder structure
#==============================================================================================
ACME_FOLDERIZE_TARGET(${ACME_NAME})
