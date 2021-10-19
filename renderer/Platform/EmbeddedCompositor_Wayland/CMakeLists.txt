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
                            WaylandEGLExtensionProcs
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    EmbeddedCompositor_Wayland_Test
    TYPE                    TEST
    TEST_SUFFIX             RNDSANDWICHTEST_SWRAST
    ENABLE_INSTALL          ON

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_SOURCE            ../EmbeddedCompositor_Wayland/test/*.cpp
                            ../EmbeddedCompositor_Wayland/test/*.h

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            EmbeddedCompositor_Wayland
                            wayland-client
                            Device_GL
                            RendererTestUtils
                            WaylandTestUtils
                            WaylandUtilities
)
