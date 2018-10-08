#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# from file
IF (EXISTS "${ramses-sdk_ROOT_CMAKE_PATH}/originalGitInformation.txt")
    FILE(READ "${ramses-sdk_ROOT_CMAKE_PATH}/originalGitInformation.txt" GIT_INFO_FILE_CONTENT)
    STRING(REGEX REPLACE "\n" ";" GIT_INFO_FILE_CONTENT "${GIT_INFO_FILE_CONTENT}")
    LIST(GET GIT_INFO_FILE_CONTENT 0 GIT_COMMIT_COUNT)
    LIST(GET GIT_INFO_FILE_CONTENT 1 GIT_COMMIT_HASH)
    MESSAGE("Using originalGitInformation.txt override for variable: GIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
    MESSAGE("Using originalGitInformation.txt override for variable: GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT}")
ENDIF()

# env variables
IF (DEFINED ENV{GIT_COMMIT_COUNT} AND DEFINED ENV{GIT_COMMIT_HASH} AND NOT "$ENV{GIT_COMMIT_COUNT}" STREQUAL "" AND NOT "$ENV{GIT_COMMIT_HASH}" STREQUAL "")
    SET(GIT_COMMIT_COUNT $ENV{GIT_COMMIT_COUNT})
    SET(GIT_COMMIT_HASH $ENV{GIT_COMMIT_HASH})
    MESSAGE("Using env variable override for variable: GIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
    MESSAGE("Using env variable override for variable: GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT}")
ENDIF()

# set scm version
IF (DEFINED GIT_COMMIT_COUNT AND DEFINED GIT_COMMIT_HASH AND NOT "${GIT_COMMIT_COUNT}" STREQUAL "" AND NOT "${GIT_COMMIT_HASH}" STREQUAL "")
    SET(SCM_VERSION ${GIT_COMMIT_COUNT}-${GIT_COMMIT_HASH})
    MESSAGE("Using GIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
    MESSAGE("Using GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT}")
    MESSAGE("Using SCM_VERSION=${SCM_VERSION}")
ENDIF()
