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

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

OPTION(ACME_CREATE_PACKAGE           "include CPack in order to create a 'package' target" ON)
OPTION(ACME_ENABLE_TEST_PROPERTIES   "Enable use of test properties" ON)

SET(ACME2_BASE_DIR   ${CMAKE_CURRENT_LIST_DIR})

INCLUDE(${ACME2_BASE_DIR}/internal/tools.cmake)
INCLUDE(${ACME2_BASE_DIR}/internal/api.cmake)

CMAKE_POLICY(SET CMP0054 NEW)
CMAKE_POLICY(SET CMP0022 NEW)

MACRO(ACME2_PROJECT)
    SET(PROJECT_SETTINGS "${ARGV}")

    SET(SUPER_PROJECT_NAME ${PROJECT_NAME})
    SET(PROJECT_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

    set_property(DIRECTORY "${PROJECT_BASE_DIR}" APPEND PROPERTY ACME_COPIED_RESOURCE_NAMES_TO_DIRECTORY "")
    set_property(DIRECTORY "${PROJECT_BASE_DIR}" APPEND PROPERTY ACME_COPIED_RESOURCE_PATHS_TO_DIRECTORY "")

    # reset PROJECT_<value>
    FOREACH(PROPERTY ${ACME2_API})
        SET(PROJECT_${PROPERTY} "")
    ENDFOREACH()

    # values provided by ACME_PROJECT call are in PROJECT_<value>
    cmake_parse_arguments(PROJECT "" "" "${ACME2_API}" ${PROJECT_SETTINGS})

    # apply project settings or use externally provided values or use default values
    FOREACH(PROPERTY ${ACME2_API})
        IF("${PROJECT_${PROPERTY}}" STREQUAL "")
            IF ("${${PROJECT_NAME}_${PROPERTY}}" STREQUAL "")
                SET(PROJECT_${PROPERTY} ${DEFAULT_${PROPERTY}})
            ELSE()
                SET(PROJECT_${PROPERTY} ${${PROJECT_NAME}_${PROPERTY}})
            ENDIF()
        ENDIF()
    ENDFOREACH()

    include(${ACME2_BASE_DIR}/internal/create_build_config.cmake)

    OPTION(${PROJECT_NAME}_BUILD_TESTS "build unit tests for project '${PROJECT_NAME}'" ON)

    SET(${PROJECT_NAME}_WILL_NOT_FIND "" CACHE INTERNAL "")

    SET(LAST_STATE "ON")
    FOREACH(CONTENT ${PROJECT_CONTENT})
        IF(("${CONTENT}" STREQUAL "ON") OR ("${CONTENT}" STREQUAL "OFF") OR ("${CONTENT}" STREQUAL "AUTO"))
            SET(LAST_STATE "${CONTENT}")
        ELSE()
            STRING(REPLACE "/" "_" CONTENT_STRING "${CONTENT}")
            SET(${PROJECT_NAME}_${CONTENT_STRING} "${LAST_STATE}" CACHE STRING "")
            SET_PROPERTY(CACHE ${PROJECT_NAME}_${CONTENT_STRING} PROPERTY STRINGS ON OFF AUTO)

            IF("${${PROJECT_NAME}_${CONTENT_STRING}}" STREQUAL "OFF")
                ACME_INFO("- ${CONTENT}")
            ENDIF()

            IF("${${PROJECT_NAME}_${CONTENT_STRING}}" STREQUAL "AUTO")
                IF (EXISTS "${PROJECT_BASE_DIR}/${CONTENT}")
                    SET(ACME_ENABLE_DEPENDENCY_CHECK ON)
                    ADD_SUBDIRECTORY(${CONTENT})
                ENDIF()
            ENDIF()

            IF("${${PROJECT_NAME}_${CONTENT_STRING}}" STREQUAL "ON")
                SET(ACME_ENABLE_DEPENDENCY_CHECK OFF)
                ADD_SUBDIRECTORY(${CONTENT})
            ENDIF()
        ENDIF()
    ENDFOREACH()

    IF(ACME_CREATE_PACKAGE)
        INCLUDE(${ACME2_BASE_DIR}/internal/create_package.cmake)
    ENDIF()
ENDMACRO(ACME2_PROJECT)

MACRO(ACME_MODULE)

    CMAKE_POLICY(SET CMP0054 NEW)

    SET(MODULE_SETTINGS "${ARGV}")
    SET(BUILD_ENABLED TRUE)

    # translate all MODULE_* provided by ACME2 module
    # to checked and preprocessed ACME_* variables

    # reset MODULE_<value>
    FOREACH(PROPERTY ${ACME2_API})
        SET(MODULE_${PROPERTY} "")
    ENDFOREACH()

    # values provided by ACME_MODULE call are in MOUDLE_<value>
    cmake_parse_arguments(MODULE "" "" "${ACME2_API}" ${MODULE_SETTINGS})

    # apply module settings or use project values,
    # if property was was not provided
    FOREACH(PROPERTY ${ACME2_API})
        IF("${MODULE_${PROPERTY}}" STREQUAL "")
            SET(MODULE_${PROPERTY} ${PROJECT_${PROPERTY}})
        ENDIF()
    ENDFOREACH()

    # do not use CONTENT, this is project specific
    SET(MODULE_CONTENT "")

    # init acme settings with module settings
    FOREACH(PROPERTY ${ACME2_API})
        SET(ACME_${PROPERTY} ${MODULE_${PROPERTY}})
    ENDFOREACH()

    # resolve file wildcards
    file(GLOB ACME_FILES_SOURCE LIST_DIRECTORIES false ${MODULE_FILES_SOURCE} ${MODULE_FILES_PRIVATE_HEADER})

    # use glob to make directories absolute
    file(GLOB ACME_INCLUDE_BASE ${MODULE_INCLUDE_BASE})

    SET(ACME_PACKAGE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

    # check, if module contains files
    IF ("${ACME_FILES_SOURCE}" STREQUAL "")
        message(FATAL_ERROR "${ACME_NAME} does not have any files")
    ENDIF()

    # check, if all dependencies can be resolved
    SET(MSG "")
    FOREACH(ACME_DEPENDENCY ${ACME_DEPENDENCIES})
        IF(NOT TARGET ${ACME_DEPENDENCY})
            list(FIND ${PROJECT_NAME}_WILL_NOT_FIND ${ACME_DEPENDENCY} SKIP_FIND)

            IF(NOT ${ACME_DEPENDENCY}_FOUND AND SKIP_FIND EQUAL -1)
                FIND_PACKAGE(${ACME_DEPENDENCY} QUIET)
            ENDIF()
            IF(NOT ${ACME_DEPENDENCY}_FOUND)
                SET(${PROJECT_NAME}_WILL_NOT_FIND "${ACME_DEPENDENCY};${${PROJECT_NAME}_WILL_NOT_FIND}" CACHE INTERNAL "")

                LIST(APPEND MSG "missing ${ACME_DEPENDENCY}")
                IF(ACME_ENABLE_DEPENDENCY_CHECK)
                    SET(BUILD_ENABLED FALSE)
                ENDIF()
            ENDIF()
        ENDIF()
    ENDFOREACH()

    # check if module is test and disable, if tests are disabled
    IF("${ACME_TYPE}" STREQUAL "TEST" AND NOT ${PROJECT_NAME}_BUILD_TESTS)
        SET(MSG "tests disabled") # overwrite missing dependencies
        SET(BUILD_ENABLED FALSE)
    ENDIF()

    IF(NOT BUILD_ENABLED)
        ACME_INFO("- ${ACME_NAME} [${MSG}]")
        SET(${PROJECT_NAME}_WILL_NOT_FIND "${ACME_NAME};${${PROJECT_NAME}_WILL_NOT_FIND}" CACHE INTERNAL "")
    ELSE()
        IF(NOT "${MSG}" STREQUAL "")
            message("        build enabled, but")
            FOREACH(M ${MSG})
                message("        - ${M}")
            ENDFOREACH()
            ACME_ERROR("aborting configuration")
        ENDIF()

        ACME_INFO("+ ${ACME_NAME} (${ACME_TYPE})")

        # build module
        INCLUDE(${ACME2_BASE_DIR}/internal/module_template.cmake)
    ENDIF()

ENDMACRO(ACME_MODULE)
