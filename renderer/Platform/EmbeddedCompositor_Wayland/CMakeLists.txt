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
    NAME                    EmbeddedCompositor_Wayland
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/EmbeddedCompositor_Wayland/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-lib
                            wayland-server
                            wayland-ivi-extension
                            wayland-zwp-linux-dmabuf-v1-extension
                            libdrm
                            WaylandUtilities
                            Context_EGL
)

if (TARGET EmbeddedCompositor_Wayland)
    # check for eglmesaext header and ensure its included when availble
    find_file(ramses-sdk_HAS_EGLMESAEXT "EGL/eglmesaext.h")
    if (ramses-sdk_HAS_EGLMESAEXT)
        target_compile_definitions(EmbeddedCompositor_Wayland PUBLIC "RAMSES_HAS_EGLMESAEXT=1")
    endif()
endif()

# For the test module see
# renderer/Platform/Platform_Wayland_IVI_EGL_ES_3_0/CMakeLists.txt
