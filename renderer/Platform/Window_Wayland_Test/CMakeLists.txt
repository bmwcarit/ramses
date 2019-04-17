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
    NAME                    Window_Wayland_Test
    TYPE                    STATIC_LIBRARY

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/Window_Wayland_Test/*.h

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            Window_Wayland
                            EGL
                            LinuxInput
                            RendererTestUtils
                            WaylandTestUtils
                            WindowEventsPollingManager_Wayland
)

