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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(ACME_DEBUG_ENABLED            "enable debug output of acme"                        OFF)
OPTION(ACME_WARNING_AS_ERROR         "treat warnings as errors"                           OFF)
OPTION(ACME_CREATE_PACKAGE           "include CPack in order to create a 'package' target" ON)

SET(ACME2_BASE_DIR   ${CMAKE_CURRENT_LIST_DIR})
SET(ACME2_PLUGIN_DIR ${ACME2_PROJECT_PLUGIN_DIR} ${ACME2_BASE_DIR}/plugins)
MESSAGE(STATUS "Using ${ACME2_PLUGIN_DIR} as plugin dirs")

INCLUDE(${ACME2_BASE_DIR}/internal/tools.cmake)
INCLUDE(${ACME2_BASE_DIR}/internal/plugins.cmake)
INCLUDE(${ACME2_BASE_DIR}/internal/api.cmake)

ENABLE_TESTING()    # use CTest

IF (POLICY CMP0054)
    CMAKE_POLICY(SET CMP0054 NEW)
ENDIF()

IF (POLICY CMP0022)
    CMAKE_POLICY(SET CMP0022 NEW)
ENDIF()

ACME_DEBUG("Using ${ACME2_BASE_DIR}/acme2.cmake")
ACME_DEBUG("CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}'")

MACRO(ACME2_PROJECT)

    SET(PROJECT_SETTINGS "${ARGV}")

    SET(SUPER_PROJECT_NAME ${PROJECT_NAME})
    SET(PROJECT_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

    set_property(DIRECTORY "${PROJECT_BASE_DIR}" APPEND PROPERTY ACME_COPIED_RESOURCE_NAMES_TO_DIRECTORY "")
    set_property(DIRECTORY "${PROJECT_BASE_DIR}" APPEND PROPERTY ACME_COPIED_RESOURCE_PATHS_TO_DIRECTORY "")

    INCLUDE(${ACME2_BASE_DIR}/internal/init_project_settings.cmake)

    # setup project
    IF (POLICY CMP0048)
        CMAKE_POLICY(SET CMP0048 NEW)
        SET(version_string ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
        PROJECT(${PROJECT_NAME} VERSION ${version_string})
    ELSE()
        PROJECT(${PROJECT_NAME})
    ENDIF()

    ACME_CALL_PLUGIN_HOOK(init)

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

    # install project documentation
    IF(PROJECT_ENABLE_INSTALL_DOCUMENTATION)
        FOREACH(DOC ${PROJECT_FILES_DOCUMENTATION})
            INSTALL(FILES       ${DOC}
                    DESTINATION ${PROJECT_INSTALL_DOCUMENTATION}
                    COMPONENT   ${PROJECT_NAME}-${PROJECT_VERSION}
            )
        ENDFOREACH()
    ENDIF()

    IF(ACME_CREATE_PACKAGE)
        INCLUDE(${ACME2_BASE_DIR}/internal/create_package.cmake)
    ENDIF()

    ACME_CALL_PLUGIN_HOOK(on_project_setup_complete)

ENDMACRO(ACME2_PROJECT)

MACRO(ACME_MODULE)

    SET(MODULE_SETTINGS "${ARGV}")
    SET(BUILD_ENABLED TRUE)
    SET(STATUS "ON      ")

    INCLUDE(${ACME2_BASE_DIR}/internal/init_module_settings.cmake)
    INCLUDE(${ACME2_BASE_DIR}/internal/init_acme_settings.cmake)

    SET(MSG "")
    # check, if module contains files
    IF ("${ACME_FILES_SOURCE}${ACME_FILES_PRIVATE_HEADER}${ACME_FILES_RESOURCE}" STREQUAL "")
        LIST(APPEND MSG "does not contain files")
        SET(BUILD_ENABLED FALSE)
    ENDIF()

    # check, if all dependencies can be resolved
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

    IF (POLICY CMP0054)
        CMAKE_POLICY(SET CMP0054 NEW)
    ENDIF()
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
            ACME_WARNING("build enabled, but")
            FOREACH(M ${MSG})
                ACME_WARNING("- ${M}")
            ENDFOREACH()
            ACME_ERROR("aborting configuration")
        ENDIF()

        ACME_INFO("+ ${ACME_NAME} (${ACME_TYPE})")

        # build module
        INCLUDE(${ACME2_BASE_DIR}/internal/module_template.cmake)

        IF (TARGET ${ACME_NAME})
            ACME_CALL_PLUGIN_HOOK(on_target_created)
            ACME_CALL_PLUGIN_HOOK(on_${ACME_TYPE}_created)
        ENDIF()

    ENDIF()

ENDMACRO(ACME_MODULE)
