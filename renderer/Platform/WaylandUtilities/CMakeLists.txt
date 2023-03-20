#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    WaylandUtilities
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/WaylandUtilities/*.h
                            src/*.cpp
    DEPENDENCIES            ramses-renderer-lib
                            LinuxInput
                            wayland-server
)



IF (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    WaylandTestUtils
        TYPE                    STATIC_LIBRARY
        INCLUDE_PATHS           WaylandTestUtils
        SRC_FILES               WaylandTestUtils/*.h
                                WaylandTestUtils/*.cpp
        DEPENDENCIES            WaylandUtilities
                                ramses-gmock
    )
ENDIF()
