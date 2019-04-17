#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    Window_Windows
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

        #==========================================================================
        # files of this module
        #==========================================================================
        FILES_PRIVATE_HEADER    include/Window_Windows/*.h
        FILES_SOURCE            src/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            ramses-renderer-lib
    )

    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    Window_Windows_Test
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
        DEPENDENCIES            Window_Windows
                                RendererTestUtils
                                ramses-gmock-main
    )
ENDIF()
