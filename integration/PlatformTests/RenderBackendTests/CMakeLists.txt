#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RENDERER_MODULE_PER_CONFIG_STATIC(RenderBackendTests
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    FILES_PRIVATE_HEADER    src/*.h
    FILES_SOURCE            src/*.cpp

    DEPENDENCIES            RendererTestUtils
)

# Tests which are supposed to be run in the gate build job
SET(GATE_PLATFORMS
    "x11-egl-es-3-0"
    "wayland-ivi-egl-es-3-0"
)

IF (ramses-sdk_BUILD_TESTS)
    FOREACH(TEST_PLATFORM ${RENDERER_CONFIG_LIST})
        IF(TARGET platform-${TEST_PLATFORM})
            ACME_ADD_TEST(RenderBackendTests-${TEST_PLATFORM}
                RNDSANDWICHTEST
                -lid 3
            )
            # These tests don't work specifically on the build slaves, because binary shaders are not supported there
            SET(GATE_FILTER "-ADeviceSupportingBinaryShaders*:ADeviceSupportingGeometryShaders*")

            LIST(FIND GATE_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                ACME_ADD_TEST(RenderBackendTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_SWRAST
                    --gtest_filter=${GATE_FILTER} -lid 3
                )
            ENDIF()
        ENDIF()
    ENDFOREACH()
ENDIF()
