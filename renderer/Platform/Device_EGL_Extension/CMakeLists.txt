#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

FIND_PACKAGE(OpenGL)

ACME_MODULE(
    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    Device_EGL_Extension
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/Device_EGL_Extension/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            Context_EGL
                            WaylandEGLExtensionProcs
                            libdrm
                            gbm
)

# ACME doesn't pass on definitions, so forced to do it this way
if (TARGET Device_EGL_Extension)
    target_compile_definitions(Device_EGL_Extension PUBLIC ${OpenGL_DEFINITIONS})
    target_compile_definitions(Device_EGL_Extension INTERFACE DEVICE_EGL_EXTENSION_SUPPORTED)
endif()
