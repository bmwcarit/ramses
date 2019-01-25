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

MACRO(ACME_DEBUG)
    IF(${ACME_DEBUG_ENABLED})
        MESSAGE(STATUS "${ARGV}")
    ENDIF()
ENDMACRO()

MACRO(ACME_WARNING)
    IF(${ACME_WARNING_AS_ERROR})
        ACME_ERROR("${ARGV}")
    ELSE()
        MESSAGE("        ${ARGV}")
    ENDIF()
ENDMACRO()

MACRO(ACME_ERROR)
    ACME_INFO("ERROR: ${ARGV}")
    MESSAGE(FATAL_ERROR "Cancel Build.")
ENDMACRO()


#==============================================================================
# argument splitter
#==============================================================================

# Split arguments passed to a function into several lists separated by
# specified identifiers that do not have an associated list e.g.:
#
# SET(arguments
#   hello world
#   LIST3 foo bar
#   LIST1 fuz baz
#   )
# ARGUMENT_SPLITTER("${arguments}" "LIST1 LIST2 LIST3" ARG)
#
# results in 8 distinct variables:
#  * ARG_DEFAULT_FOUND: 1
#  * ARG_DEFAULT: hello;world
#  * ARG_LIST1_FOUND: 1
#  * ARG_LIST1: fuz;baz
#  * ARG_LIST2_FOUND: 0
#  * ARG_LIST2:
#  * ARG_LIST3_FOUND: 1
#  * ARG_LIST3: foo;bar

MACRO(ARGUMENT_SPLITTER argInput argKeywordList argPrefix)

    SET(inputTokenList "${argInput}")
    SET(keywordList ${argKeywordList})
    SET(splitter_prefix ${argPrefix})

    # initialize all out variables to default values
    FOREACH(keyword ${keywordList})
        SET(${splitter_prefix}${keyword}_FOUND 0)
        SET(${splitter_prefix}${keyword} "")
    ENDFOREACH()

    # iterate all tokens of provided input
    FOREACH(token ${inputTokenList})
        # check if current token is a keyword
        LIST(FIND keywordList ${token} tokenIsKeyword)

        IF (NOT ${tokenIsKeyword} EQUAL -1)
            # if current token is keyword, set found variable to true
            SET(lastKeyword ${token})
            SET(${splitter_prefix}${token}_FOUND 1)
        ELSE ()
            # if current token is not keyword, append token to variable of last found keyword
            LIST(APPEND ${splitter_prefix}${lastKeyword} ${token})
        ENDIF ()
    ENDFOREACH()

ENDMACRO(ARGUMENT_SPLITTER)

#==============================================================================
# file list resolver
#==============================================================================
MACRO(GET_ALL_FILES var_name directory_list)
    SET(file_list "")
    FOREACH(directory ${directory_list})
        FILE(GLOB directory_content "${directory}")
        # Only add files to the list, a included folder will cause INSTALL command to produce a cmake error
        FOREACH(fileOrDir ${directory_content})
            IF(NOT IS_DIRECTORY "${fileOrDir}/")
                LIST(APPEND file_list ${fileOrDir})
            ENDIF()
        ENDFOREACH()
    ENDFOREACH()
    SET(${var_name} ${file_list})
ENDMACRO(GET_ALL_FILES)

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
ENDMACRO()

#==============================================================================
# postprocessing of target
#==============================================================================

function(ACME_FOLDERIZE_TARGET tgt)
    # extract and set folder name from path
    # first get path relative to ramses root dir
    string(REGEX REPLACE "${ramses-sdk_ROOT_CMAKE_PATH}/" "" ACME_relative_path "${CMAKE_CURRENT_SOURCE_DIR}")
    string(REGEX REPLACE "/[^/]*$" "" ACME_folder_path "${ACME_relative_path}")
    if (NOT CMAKE_SOURCE_DIR STREQUAL ramses-sdk_ROOT_CMAKE_PATH)
        # optionally adjust path for ramses used as subdirectory
        string(REGEX REPLACE "${CMAKE_SOURCE_DIR}/" "" ACME_folder_prefix_path "${ramses-sdk_ROOT_CMAKE_PATH}")
        set(ACME_folder_path "${ACME_folder_prefix_path}/${ACME_folder_path}")
    endif()
    set_property(TARGET ${tgt} PROPERTY FOLDER "${ACME_folder_path}")

    # sort sources in goups
    get_target_property(tgt_content ${tgt} SOURCES)
    foreach(file_iter ${tgt_content})
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" tmp1 "${file_iter}")
        string(REGEX REPLACE "/[^/]*$" "" tmp2 "${tmp1}")
        string(REPLACE "/" "\\" module_internal_path "${tmp2}")
        source_group(${module_internal_path} FILES ${file_iter})
    endforeach()
endfunction()

function(ACME_FOLDERIZE_TARGETS)
    foreach (tgt ${ARGV})
        ACME_FOLDERIZE_TARGET(${tgt})
    endforeach()
endfunction()
