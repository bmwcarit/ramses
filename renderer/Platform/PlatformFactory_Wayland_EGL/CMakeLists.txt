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
    NAME                    PlatformFactory_Wayland_EGL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/PlatformFactory_Wayland_EGL/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            Surface_Wayland_EGL
                            Surface_EGL_Offscreen
                            Context_EGL
                            Window_Wayland
                            EmbeddedCompositor_Dummy
                            EmbeddedCompositor_Wayland
                            TextureUploadingAdapter_Wayland
                            WindowEventsPollingManager_Wayland
)

# This module is tested in the renderer sandwich tests (<ramses>/integration/ramses-renderer-tests)
