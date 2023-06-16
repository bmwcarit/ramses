#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    Platform_X11_EGL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/Platform_X11_EGL/*.h
                            src/*.cpp
    DEPENDENCIES            X11
                            Platform_EGL
)

if (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    platform-x11-test
        TYPE                    BINARY
        SRC_FILES               test/*.cpp
                                test/*.h
        DEPENDENCIES            Platform_X11_EGL
                                ramses-gmock-main
                                RendererTestCommon
    )

    makeTestFromTarget(
        TARGET platform-x11-test
        SUFFIX RNDSANDWICHTEST_SWRAST
        SKIPPABLE)
endif()
