#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    test-asset-producer
    TYPE                    BINARY
    ENABLE_INSTALL          OFF
    SRC_FILES               main.cpp
    DEPENDENCIES            ramses-shared-lib-headless
)

add_custom_target(RL_REGEN_TEST_ASSETS
    COMMAND test-asset-producer ${PROJECT_SOURCE_DIR}/tests/unittests/client/res)
set_property(TARGET RL_REGEN_TEST_ASSETS PROPERTY FOLDER "CMakePredefinedTargets")
