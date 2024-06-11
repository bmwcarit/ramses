#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

find_path(GLES3_INCLUDE_DIRS GLES3/gl3.h
    PATHS
    /usr/include
)

find_library(GLES3_LIBRARIES
    NAMES GLESv3 GLESv2
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGL DEFAULT_MSG GLES3_LIBRARIES GLES3_INCLUDE_DIRS)

if (OpenGL_FOUND)
    add_library(OpenGL::GLES3 UNKNOWN IMPORTED)
    set_target_properties(OpenGL::GLES3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GLES3_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${GLES3_LIBRARIES}"
        )

    mark_as_advanced(
        GLES3_INCLUDE_DIRS
        GLES3_LIBRARIES
    )
endif()
