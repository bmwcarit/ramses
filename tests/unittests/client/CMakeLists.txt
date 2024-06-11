#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


if(ramses-sdk_TEXT_SUPPORT)
    file(GLOB
        RAMSES_CLIENT_TEXT_TEST_FILES
        text/*.h
        text/*.cpp)
endif()

file(GLOB_RECURSE RAMSES_CLIENT_LOGIC_TEST_FILES
    logic/*.cpp
    logic/*.h
)

createModule(
    NAME                    ramses-client-test
    TYPE                    BINARY
    INCLUDE_PATHS           .
                            utils
    SRC_FILES               *.h
                            *.cpp
                            utils/*.h
                            utils/*.cpp
                            ${RAMSES_CLIENT_TEXT_TEST_FILES}
                            ${RAMSES_CLIENT_LOGIC_TEST_FILES}
    RESOURCE_FOLDERS        res
    DEPENDENCIES            ramses-client
                            framework-test-utils
                            ramses-gmock-main
                            glslang-init-gtest-env
)

makeTestFromTarget(
    TARGET ramses-client-test
    SUFFIX UNITTEST)
