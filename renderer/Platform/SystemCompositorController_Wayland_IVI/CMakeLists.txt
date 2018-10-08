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
    NAME                    SystemCompositorController_Wayland_IVI
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/SystemCompositorController_Wayland_IVI/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-lib
                            WaylandUtilities
                            wayland-ivi-extension
                            wayland-client
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    SystemCompositorController_Wayland_IVI_Test
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
    DEPENDENCIES            SystemCompositorController_Wayland_IVI
                            RendererTestUtils
                            UnixUtilities
)
