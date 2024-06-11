#  -------------------------------------------------------------------------
#  Copyright (C) 2024 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# This target is designed to automatically add gtest environment
# that initializes glslang, to avoid having glslang initialized
# for every test
# NOTE: this target must be added as a DIRECT dependency
# to the cmake module that needs this functionality

createModule(
    NAME                    glslang-init-gtest-env
    TYPE                    OBJECT
    INCLUDE_PATHS           .
    SRC_FILES               *.h
                            *.cpp
    DEPENDENCIES            ramses-client
                            ramses-gmock
)
