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
    NAME                    DisplayManager
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/DisplayManager/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-api
                            ramses-renderer-lib
                            ramses-capu
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    DisplayManagerTest
    TYPE                    TEST

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            test
    FILES_PRIVATE_HEADER    test/*.h
    FILES_SOURCE            test/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            DisplayManager
                            RendererTestUtils
                            gmock_main
)
