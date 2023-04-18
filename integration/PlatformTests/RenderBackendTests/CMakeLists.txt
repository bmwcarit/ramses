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
SET(GATE_PLATFORMS
    "x11-egl-es-3-0"
    "wayland-ivi-egl-es-3-0"
)

IF (ramses-sdk_BUILD_TESTS)
    foreach(TEST_PLATFORM ${PLATFORM_LIST})
        IF(TARGET platform-${TEST_PLATFORM})
            makeTestFromTarget(
                TARGET RenderBackendTests-${TEST_PLATFORM}
                SUFFIX RNDSANDWICHTEST
                EXTRA_ARGS --ivi-layer 3
                SKIPPABLE
            )
            # These tests don't work specifically on the build slaves, because binary shaders are not supported there
            SET(GATE_FILTER "-ADeviceSupportingBinaryShaders*:ADeviceSupportingGeometryShaders*")

            LIST(FIND GATE_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                makeTestFromTarget(
                    TARGET RenderBackendTests-${TEST_PLATFORM}
                    SUFFIX RNDSANDWICHTEST_SWRAST
                    EXTRA_ARGS --gtest_filter=${GATE_FILTER} --ivi-layer 3
                    SKIPPABLE
                )
            ENDIF()
        ENDIF()
    ENDFOREACH()
ENDIF()
