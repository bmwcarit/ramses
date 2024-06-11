#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    ramses-renderer-test
    TYPE                    BINARY
    SRC_FILES               *.h
                            *.cpp
    DEPENDENCIES            ramses-client
                            ramses-renderer-impl
                            PlatformFactoryFake
                            ramses-gmock-main
                            renderer-test-common
)

makeTestFromTarget(
    TARGET ramses-renderer-test
    SUFFIX UNITTEST)
