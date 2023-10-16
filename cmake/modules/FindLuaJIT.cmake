#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
find_path(LUAJIT_INCLUDE_DIR lua.h
    HINTS
    $ENV{LUAJIT_DIR}
    PATH_SUFFIXES luajit-2.1 luajit
    PATHS
    /usr/local
    /usr
    /opt
    )

find_library(LUAJIT_LIBRARY
    NAMES luajit-5.1 lua51
    HINTS
    $ENV{LUAJIT_DIR}
    PATH_SUFFIXES lib64 lib
    PATHS
    /usr/local
    /usr
    /opt
    )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LuaJIT DEFAULT_MSG LUAJIT_LIBRARY LUAJIT_INCLUDE_DIR)

if(NOT TARGET lua::lua)
    add_library(lua::lua UNKNOWN IMPORTED)
    set_target_properties(lua::lua PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LUAJIT_INCLUDE_DIR}"
        IMPORTED_LOCATION "${LUAJIT_LIBRARY}")
endif()

mark_as_advanced(LUAJIT_INCLUDE_DIR LUAJIT_LIBRARY)

