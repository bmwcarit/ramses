############################################################################
#
# Copyright 2014 BMW Car IT GmbH
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
SET(TARGET_DIRECTORY ${CMAKE_BINARY_DIR}/BuildConfig)

SET(EXPORTED_VARIABLES
    PROJECT_NAME
    PROJECT_DESCRIPTION
    PROJECT_URL
    PROJECT_VERSION_STRING
    PROJECT_VERSION_MAJOR
    PROJECT_VERSION_MINOR
    PROJECT_VERSION_PATCH
    CMAKE_BUILD_TYPE
    CMAKE_CXX_COMPILER
    CMAKE_CXX_COMPILER_ID
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_SYSTEM_NAME
    CMAKE_SYSTEM_VERSION
    CMAKE_TOOLCHAIN_FILE
    CMAKE_VERSION
    GIT_COMMIT_COUNT
    GIT_COMMIT_HASH
    TARGET_OS
    BUILD_ENV_VERSION_INFO_FULL
)
SET(EXPORTED_INT_VARIABLES
    PROJECT_VERSION_MAJOR
    PROJECT_VERSION_MINOR
    PROJECT_VERSION_PATCH)

IF (NOT DEFINED GIT_COMMIT_COUNT OR NOT DEFINED GIT_COMMIT_HASH)
    message(FATAL_ERROR "GIT_COMMIT_COUNT and GIT_COMMIT_HASH must be set")
ENDIF()

STRING(REPLACE "-" "_" PROJECT_NAME_NO_DASH ${PROJECT_NAME})
STRING(TOUPPER ${PROJECT_NAME_NO_DASH} PROJECT_NAME_CAPITAL)
STRING(TOLOWER ${PROJECT_NAME_NO_DASH} PROJECT_NAME_LOWERCASE)

SET(EXPORTED_ACME2_SYMBOLS "")

FOREACH(VAR ${EXPORTED_VARIABLES})
    IF(NOT DEFINED ${VAR})
        SET(${VAR} "(unknown)")
    ENDIF()

    # escape '\'
    string(REPLACE "\\" "\\\\" TMP "${${VAR}}")
    string(REPLACE "\"" "\\\"" TMP "${TMP}")
    set(ESCAPED_${VAR} "${TMP}")

    SET(EXPORTED_ACME2_SYMBOLS "${EXPORTED_ACME2_SYMBOLS}\nconst char* const ${PROJECT_NAME_CAPITAL}_${VAR} = \"@ESCAPED_${VAR}@\";")
ENDFOREACH()

FOREACH(VAR ${EXPORTED_INT_VARIABLES})
    IF(DEFINED ${VAR})
        SET(EXPORTED_ACME2_SYMBOLS "${EXPORTED_ACME2_SYMBOLS}\nconst int ${PROJECT_NAME_CAPITAL}_${VAR}_INT = @${VAR}@;")
    ENDIF()
ENDFOREACH()

CONFIGURE_FILE(
    ${CMAKE_CURRENT_LIST_DIR}/build-config.h.in
    ${TARGET_DIRECTORY}/${PROJECT_NAME}-build-config.h.in
)

CONFIGURE_FILE(
    ${TARGET_DIRECTORY}/${PROJECT_NAME}-build-config.h.in
    ${TARGET_DIRECTORY}/${PROJECT_NAME}-build-config.h
)

ACME_INFO("G ${PROJECT_NAME}BuildConfig.h")

# add this folder to global include list
INCLUDE_DIRECTORIES(${TARGET_DIRECTORY})
