#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET(WGL_FOUND FALSE)

# Find WGL, which is Windows-only

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")

    SET(WGL_INCLUDE_DIRS
        ${ramses-sdk_SOURCE_DIR}/external/khronos
    )

    # Windows has all OpenGL/WGL symbols in one lib - opengl32.lib
    SET(WGL_LIBRARIES opengl32)
    SET(WGL_FOUND TRUE)

    MARK_AS_ADVANCED(
        WGL_INCLUDE_DIRS
        WGL_LIBRARIES
    )

ENDIF()
