#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(ramses-sdk_ENABLE_WAYLAND_SHELL)
    ACME_MODULE(
        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    Window_Wayland_Shell
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

        #==========================================================================
        # files of this module
        #==========================================================================
        FILES_PRIVATE_HEADER    include/Window_Wayland_Shell/*.h
        FILES_SOURCE            src/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            Window_Wayland
    )
ENDIF()

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    Window_Wayland_Shell_Test
    TYPE                    TEST
    TEST_SUFFIX             RNDSANDWICHTEST_SWRAST

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_SOURCE            test/*.cpp
                            test/*.h

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            Window_Wayland_Shell
                            Window_Wayland_Test
)

# This module is tested in the renderer sandwich tests (<ramses>/integration/ramses-renderer-tests)
