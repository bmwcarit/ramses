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
    NAME                    platform-wayland-ivi-egl-es-3-0
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/Platform_Wayland_IVI_EGL_ES_3_0/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            PlatformFactory_Wayland_IVI_EGL
                            Device_GL
)

# TODO: Michael Move EmbeddedCompositor_Wayland_Test to EmbeddedCompositor_Wayland
#       directory where it belongs. It is here because of dependency to
#       platform-wayland-ivi-egl-es-3-0 and order of module evaluation

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    EmbeddedCompositor_Wayland_Test
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_SOURCE            ../EmbeddedCompositor_Wayland/test/*.cpp
                            ../EmbeddedCompositor_Wayland/test/*.h

    #==========================================================================
    # dependencies
    #==========================================================================
    # TODO: Michael fix dependencies for these tests, e.g. WaylandEGLExtensionProcs.h
    DEPENDENCIES            EmbeddedCompositor_Wayland
                            RendererTestUtils
                            WaylandTestUtils
                            platform-wayland-ivi-egl-es-3-0
                            WaylandUtilities
)

IF (TARGET EmbeddedCompositor_Wayland_Test)
    # Tests that make connection and an event loop dispatch currently report potential leaks by address sanitizer
    # Wayland library seems to internally allocate some resources associated to the connected client, it is unclear at the moment how those should be freed
    ACME_ADD_TEST(EmbeddedCompositor_Wayland_Test
        RNDSANDWICHTEST_SWRAST
        --gtest_filter=-AEmbeddedCompositor_Wayland.*_ClientConnectionTest
    )
ENDIF()
