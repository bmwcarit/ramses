#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(framework-test-utils)
add_subdirectory(framework)
add_subdirectory(glslang-init-gtest-env)
add_subdirectory(client)

if(ANY_WINDOW_TYPE_ENABLED)
    add_subdirectory(renderer)
    add_subdirectory(ramses-cli)
endif()

if(ramses-sdk_BUILD_TOOLS)
    add_subdirectory(tools)
endif()
