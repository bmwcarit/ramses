#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    window-wayland-ivi-test
    TYPE                    BINARY
    SRC_FILES               *.cpp
                            *.h
    DEPENDENCIES            Platform
                            window-wayland-common
    )

makeTestFromTarget(
    TARGET window-wayland-ivi-test
    SUFFIX RNDSANDWICHTEST_SWRAST
    EXTRA_ARGS --gtest_filter=-*AWindowWaylandWithEventHandling*)
