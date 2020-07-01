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
    ENABLE_INSTALL          OFF

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



IF (${ramses-sdk_BUILD_TESTS})
    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    WaylandTestUtils
        TYPE                    STATIC_LIBRARY

        #==========================================================================
        # files of this module
        #==========================================================================

        INCLUDE_BASE            WaylandTestUtils
        FILES_PRIVATE_HEADER    WaylandTestUtils/*.h
        FILES_SOURCE            WaylandTestUtils/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            WaylandUtilities
                                ramses-gmock
    )
ENDIF()
