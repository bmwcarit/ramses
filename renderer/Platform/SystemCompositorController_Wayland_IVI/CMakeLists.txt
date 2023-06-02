#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    SystemCompositorController_Wayland_IVI
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/SystemCompositorController_Wayland_IVI/*.h
                            src/*.cpp
    DEPENDENCIES            ramses-renderer-lib
                            WaylandUtilities
                            wayland-ivi-extension
                            wayland-client
)

if (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    SystemCompositorController_Wayland_IVI_Test
        TYPE                    BINARY
        SRC_FILES            test/*.cpp
                                test/*.h
        DEPENDENCIES            SystemCompositorController_Wayland_IVI
                                WaylandTestUtils
                                RendererTestCommon
                                WaylandUtilities
    )

    makeTestFromTarget(
        TARGET SystemCompositorController_Wayland_IVI_Test
        SUFFIX RNDSANDWICHTEST_SWRAST
        SKIPPABLE)
endif()
