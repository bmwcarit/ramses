#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# use predefined values in dependant repositories
IF (EXISTS "${PROJECT_SOURCE_DIR}/originalGitInformation.txt")
    FILE(READ "${PROJECT_SOURCE_DIR}/originalGitInformation.txt" GIT_INFO_FILE_CONTENT)
    STRING(REGEX REPLACE "\n" ";" GIT_INFO_FILE_CONTENT "${GIT_INFO_FILE_CONTENT}")
    LIST(GET GIT_INFO_FILE_CONTENT 0 GIT_COMMIT_COUNT)
    LIST(GET GIT_INFO_FILE_CONTENT 1 GIT_COMMIT_HASH)
    MESSAGE(STATUS "Get git info from originalGitInformation.txt: GIT_COMMIT_HASH=${GIT_COMMIT_HASH} GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT}")

ELSE()
    FIND_PACKAGE(Git QUIET)

    IF(GIT_FOUND)
        # try get hash and commit count
        EXEC_PROGRAM(${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
            ARGS rev-list HEAD --count
            OUTPUT_VARIABLE GIT_COMMIT_COUNT_TMP
            RETURN_VALUE GIT_COUNT_RETURN_VALUE)
        EXEC_PROGRAM(${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
            ARGS rev-parse --short HEAD
            OUTPUT_VARIABLE GIT_COMMIT_HASH_TMP
            RETURN_VALUE GIT_HASH_RETURN_VALUE)

        # optionally get branch
        EXEC_PROGRAM(${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
            ARGS rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE GIT_COMMIT_BRANCH_TMP
            RETURN_VALUE GIT_BRANCH_RETURN_VALUE)

        if (GIT_COUNT_RETURN_VALUE EQUAL 0 AND GIT_HASH_RETURN_VALUE EQUAL 0)
            set(GIT_COMMIT_COUNT ${GIT_COMMIT_COUNT_TMP})
            set(GIT_COMMIT_HASH ${GIT_COMMIT_HASH_TMP})
            if (GIT_BRANCH_RETURN_VALUE EQUAL 0)
                set(GIT_COMMIT_BRANCH ${GIT_COMMIT_BRANCH_TMP})
            endif()

            message(STATUS "Get git info from repo: GIT_COMMIT_HASH=${GIT_COMMIT_HASH} GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT} GIT_COMMIT_BRANCH=${GIT_COMMIT_BRANCH}")
        endif()
    ENDIF()

ENDIF()

if (NOT GIT_COMMIT_HASH OR NOT GIT_COMMIT_COUNT)
    message(STATUS "Could not determin git info. Use default values.")
    set(GIT_COMMIT_HASH "(unknown)")
    set(GIT_COMMIT_COUNT "0")
endif()
