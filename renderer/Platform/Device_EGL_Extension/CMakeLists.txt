#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

find_package(OpenGL)

createModule(
    NAME                    Device_EGL_Extension
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/Device_EGL_Extension/*.h
                            src/*.cpp
    DEPENDENCIES            Context_EGL
                            WaylandEGLExtensionProcs
                            libdrm
                            gbm
    INTERFACE_DEFINES       DEVICE_EGL_EXTENSION_SUPPORTED
    # TODO this should not be needed
    PUBLIC_DEFINES          ${OpenGL_DEFINITIONS}
)
