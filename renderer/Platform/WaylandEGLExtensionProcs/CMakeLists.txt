#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    WaylandEGLExtensionProcs
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/WaylandEGLExtensionProcs/*.h
                            src/*.cpp
    DEPENDENCIES            ramses-renderer-lib
                            wayland-zwp-linux-dmabuf-v1-extension
                            EGL
)


if (TARGET WaylandEGLExtensionProcs)
    # check for eglmesaext header and ensure its included when availble
    find_file(ramses-sdk_HAS_EGLMESAEXT "EGL/eglmesaext.h")
    if (ramses-sdk_HAS_EGLMESAEXT)
        target_compile_definitions(WaylandEGLExtensionProcs PUBLIC "RAMSES_HAS_EGLMESAEXT=1")
    endif()
endif()
