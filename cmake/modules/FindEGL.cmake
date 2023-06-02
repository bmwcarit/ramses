#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set( EGL_FOUND FALSE )

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    # Windows does not suppot EGL natively

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Android")
    find_path(EGL_INCLUDE_DIRS EGL/egl.h
        /usr/include
    )

    find_library(EGL_LIBRARY
        NAMES EGL
        PATHS
    )

    set( EGL_FOUND "NO" )
    if(EGL_LIBRARY)
        set( EGL_LIBRARIES ${EGL_LIBRARY} )
        set( EGL_FOUND TRUE )
    endif()

    mark_as_advanced(
        EGL_INCLUDE_DIRS
        EGL_LIBRARIES
        EGL_LIBRARY
        EGL_FOUND
    )

endif()
