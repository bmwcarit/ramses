#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    Window_Wayland_IVI
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/Window_Wayland_IVI/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            Window_Wayland
                            wayland-ivi-extension
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    Window_Wayland_IVI_Test
    TYPE                    TEST
    TEST_SUFFIX             RNDSANDWICHTEST

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_SOURCE            test/*.cpp
                            test/*.h

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            Window_Wayland_IVI
                            Window_Wayland_Test
                            SystemCompositorController_Wayland_IVI
)

# This module is tested in the renderer sandwich tests (<ramses>/integration/ramses-renderer-tests)
