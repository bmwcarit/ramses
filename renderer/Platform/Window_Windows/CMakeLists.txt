#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    createModule(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    Window_Windows
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        #==========================================================================
        # files of this module
        #==========================================================================
        INCLUDE_PATHS           include
        SRC_FILES               include/Window_Windows/*.h
                                src/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            ramses-renderer-lib
    )

    if (${ramses-sdk_BUILD_TESTS})
        createModule(

            #==========================================================================
            # general module information
            #==========================================================================
            NAME                    Window_Windows_Test
            TYPE                    BINARY

            #==========================================================================
            # files of this module
            #==========================================================================
            SRC_FILES            test/*.cpp
                                    test/*.h

            #==========================================================================
            # dependencies
            #==========================================================================
            DEPENDENCIES            Window_Windows
                                    RendererTestUtils
                                    ramses-gmock-main
        )

        makeTestFromTarget(
            TARGET Window_Windows_Test
            SUFFIX RNDSANDWICHTEST
            SKIPPABLE)
    endif()
ENDIF()
