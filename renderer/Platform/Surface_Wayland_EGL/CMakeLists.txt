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
    NAME                    Surface_Wayland_EGL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/Surface_Wayland_EGL/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            Context_EGL
                            Window_Wayland
                            EmbeddedCompositor_Dummy
                            Logger_Wayland
)

# This module is tested in the renderer sandwich tests (<ramses>/integration/ramses-renderer-tests)
