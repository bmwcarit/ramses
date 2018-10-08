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
    NAME                    WaylandUtilities
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/WaylandUtilities/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-lib
                            LinuxInput
                            wayland-server
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    WaylandUtilities_Test
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
    DEPENDENCIES            WaylandUtilities
                            RendererTestUtils
                            UnixUtilities
                            gmock_main
)
