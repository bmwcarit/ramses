#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    Window_Wayland_Test
    TYPE                    STATIC_LIBRARY
    INCLUDE_PATHS           include
    SRC_FILES               include/Window_Wayland_Test/*.h
    DEPENDENCIES            Window_Wayland
                            EGL
                            LinuxInput
                            RendererTestUtils
                            WaylandTestUtils
)
