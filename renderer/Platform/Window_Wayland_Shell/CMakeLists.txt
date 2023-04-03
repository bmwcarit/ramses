#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(ramses-sdk_ENABLE_WAYLAND_SHELL)
    createModule(
        NAME                    Window_Wayland_Shell
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF
        INCLUDE_PATHS           include
        SRC_FILES               include/Window_Wayland_Shell/*.h
                                src/*.cpp
        DEPENDENCIES            Window_Wayland
    )

    if (${ramses-sdk_BUILD_TESTS})
        createModule(
            NAME                    Window_Wayland_Shell_Test
            TYPE                    BINARY
            SRC_FILES            test/*.cpp
                                    test/*.h
            DEPENDENCIES            Window_Wayland_Shell
                                    Window_Wayland_Test
        )

        makeTestFromTarget(
            TARGET Window_Wayland_Shell_Test
            SUFFIX RNDSANDWICHTEST_SWRAST
            SKIPPABLE)
    endif()

endif()
