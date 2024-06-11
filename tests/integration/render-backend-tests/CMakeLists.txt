#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModuleWithRenderer(
    NAME            render-backend-tests
    TYPE            BINARY
    ENABLE_INSTALL  ON
    SRC_FILES       *.h
                    *.cpp
    DEPENDENCIES    RendererTestUtils
                    ramses-cli
                    glslang-init-gtest-env
)

# Tests which are supposed to be run in the gate build job
SET(GATE_WINDOW_PLATFORMS
    "x11"
    "wayland-ivi"
)

IF (ramses-sdk_BUILD_TESTS)
    makeTestPerWindowTypeFromTarget(
        TARGET render-backend-tests
        SUFFIX RNDSANDWICHTEST
        EXTRA_ARGS --ivi-layer 3
    )

    # These tests don't work specifically on the build slaves, because binary shaders are not supported there
    SET(GATE_FILTER "-ADeviceSupportingBinaryShaders*:ADeviceSupportingGeometryShaders*")

    makeTestPerWindowTypeFromTarget(
        TARGET render-backend-tests
        SUFFIX RNDSANDWICHTEST_SWRAST
        EXTRA_ARGS --gtest_filter=${GATE_FILTER} --ivi-layer 3
        WINDOW_TYPE_FILTER ${GATE_WINDOW_PLATFORMS}
    )

    # Vulkan tests
    if(ramses-sdk_ENABLE_DEVICE_TYPE_VULKAN)
        set(VULKAN_FILTER "${GATE_FILTER}")

        if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
            makeTestFromTarget(TARGET render-backend-tests
                    SUFFIX x11-vulkan_RNDSANDWICHTEST_SWRAST
                    EXTRA_ARGS --gtest_filter=${VULKAN_FILTER} --device-type vulkan --window-type x11
        )
        endif()

        if(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS)
            makeTestFromTarget(TARGET render-backend-tests
                    SUFFIX windows-vulkan_RNDSANDWICHTEST
                    EXTRA_ARGS --gtest_filter=${VULKAN_FILTER} --device-type vulkan --window-type windows
        )
        endif()
    endif()
ENDIF()
