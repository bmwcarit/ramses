#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    Window_Wayland_IVI
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/Window_Wayland_IVI/*.h
                            src/*.cpp
    DEPENDENCIES            Window_Wayland
                            wayland-ivi-extension
)

if (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    Window_Wayland_IVI_Test
        TYPE                    BINARY
        SRC_FILES            test/*.cpp
                                test/*.h
        DEPENDENCIES            Window_Wayland_IVI
                                Window_Wayland_Test
                                SystemCompositorController_Wayland_IVI
        )

        # This module is tested in the renderer sandwich tests (<ramses>/integration/ramses-renderer-tests)

    makeTestFromTarget(
        TARGET Window_Wayland_IVI_Test
        SUFFIX RNDSANDWICHTEST_SWRAST
        SKIPPABLE
        EXTRA_ARGS --gtest_filter=-*AWindowWaylandWithEventHandling*)
endif()
