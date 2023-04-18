#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_library(ramses-cli INTERFACE)
target_include_directories(ramses-cli INTERFACE include)
target_link_libraries(ramses-cli INTERFACE
    ramses-framework-api
    ramses-renderer-api
    CLI11::CLI11
)

if (${ramses-sdk_BUILD_TESTS})
    add_executable(ramses-cli-test
        test/ramses-cli-test.cpp
        include/ramses-cli.h
    )
    target_link_libraries(ramses-cli-test PRIVATE
        ramses-cli
        ramses-gmock-main
        ramses-framework
        ramses-renderer-lib
    )
    makeTestFromTarget(
        TARGET ramses-cli-test
        SUFFIX UNITTEST)
    folderizeTarget(ramses-cli-test)
endif()
