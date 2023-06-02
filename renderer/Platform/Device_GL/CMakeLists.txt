#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    Device_GL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/Device_GL/*.h
                            src/*.cpp
    DEPENDENCIES            ramses-renderer-lib
                            OpenGL
    # TODO move this to the OpenGL target where it belongs
    PUBLIC_DEFINES          ${OpenGL_DEFINITIONS}
)
