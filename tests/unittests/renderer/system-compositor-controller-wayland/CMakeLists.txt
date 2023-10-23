#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    system-compositor-controller-wayland-test
    TYPE                    BINARY
    SRC_FILES               *.cpp
                            *.h
    DEPENDENCIES            Platform
                            wayland-test-utils
                            renderer-test-common
)

makeTestFromTarget(
    TARGET system-compositor-controller-wayland-test
    SUFFIX RNDSANDWICHTEST_SWRAST
    )
