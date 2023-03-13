#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(ramses-sdk_ENABLE_WAYLAND_IVI)
    createModule(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    Window_Wayland_IVI
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        #==========================================================================
        # files of this module
        #==========================================================================
        INCLUDE_PATHS           include
        SRC_FILES               include/Window_Wayland_IVI/*.h
                                src/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            Window_Wayland
                                wayland-ivi-extension
    )

    if (${ramses-sdk_BUILD_TESTS})
        createModule(

            #==========================================================================
            # general module information
            #==========================================================================
            NAME                    Window_Wayland_IVI_Test
            TYPE                    BINARY

            #==========================================================================
            # files of this module
            #==========================================================================
            SRC_FILES            test/*.cpp
                                    test/*.h

            #==========================================================================
            # dependencies
            #==========================================================================
            DEPENDENCIES            Window_Wayland_IVI
                                    Window_Wayland_Test
                                    SystemCompositorController_Wayland_IVI
            )

            # This module is tested in the renderer sandwich tests (<ramses>/integration/ramses-renderer-tests)

        makeTestFromTarget(
            TARGET Window_Wayland_IVI_Test
            SUFFIX RNDSANDWICHTEST
            SKIPPABLE)
    endif()

endif()
