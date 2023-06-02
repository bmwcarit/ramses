#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

expandModuleByPlatform(
    NAME            RenderBackendTests
    TYPE            BINARY
    LINKED_TYPE     STATIC
    ENABLE_INSTALL  ON
    SRC_FILES       src/*.h
                    src/*.cpp
    DEPENDENCIES    RendererTestUtils ramses-cli
)

# Tests which are supposed to be run in the gate build job
SET(GATE_WINDOW_PLATFORMS
    "x11"
    "wayland-ivi"
)

IF (ramses-sdk_BUILD_TESTS)
    makeTestPerWindowTypeFromTarget(
        TARGET RenderBackendTests
        SUFFIX RNDSANDWICHTEST
        EXTRA_ARGS --ivi-layer 3
        SKIPPABLE
    )

    # These tests don't work specifically on the build slaves, because binary shaders are not supported there
    SET(GATE_FILTER "-ADeviceSupportingBinaryShaders*:ADeviceSupportingGeometryShaders*")

    makeTestPerWindowTypeFromTarget(
        TARGET RenderBackendTests
        SUFFIX RNDSANDWICHTEST_SWRAST
        EXTRA_ARGS --gtest_filter=${GATE_FILTER} --ivi-layer 3
        WINDOW_TYPE_FILTER ${GATE_WINDOW_PLATFORMS}
        SKIPPABLE
    )
ENDIF()
