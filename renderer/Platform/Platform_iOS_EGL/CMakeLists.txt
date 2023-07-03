#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    Platform_iOS_EGL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               src/*.cpp
                            src/*.mm
    DEPENDENCIES            Platform_EGL
                            QuartzCore
)
