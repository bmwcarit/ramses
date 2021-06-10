#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

FIND_PACKAGE(PkgConfig QUIET)

IF (NOT WIN32 AND PKG_CONFIG_FOUND)
    SET(harfbuzz_minversion 1.2.7)

    PKG_CHECK_MODULES(freetype freetype2)
    PKG_CHECK_MODULES(harfbuzz harfbuzz>=${harfbuzz_minversion})

    IF(NOT harfbuzz_FOUND)
        # try to find directly
        FIND_PATH(harfbuzz_INCLUDE_DIRS hb.h
            /usr/include/harfbuzz
        )
        FIND_LIBRARY(harfbuzz_LIBRARIES
            NAMES harfbuzz
        )

        IF(harfbuzz_LIBRARIES AND harfbuzz_INCLUDE_DIRS)
            SET(harfbuzz_FOUND 1 CACHE INTERNAL "" FORCE)
        ELSE()
            MESSAGE(STATUS "Could not find harfbuzz")
        ENDIF()

    ENDIF()

    IF(NOT(freetype_FOUND AND harfbuzz_FOUND))
        SET(freetype_FOUND 0)
        UNSET(freetype_LIBRARIES)
        UNSET(freetype_LIBRARY_DIRS)
        UNSET(freetype_LDFLAGS)
        UNSET(freetype_LDFLAGS_OTHER)
        UNSET(freetype_INCLUDE_DIRS)
        UNSET(freetype_CFLAGS)
        UNSET(freetype_CFLAGS_OTHER)

        SET(harfbuzz_FOUND 0)
        UNSET(harfbuzz_LIBRARIES)
        UNSET(harfbuzz_LIBRARY_DIRS)
        UNSET(harfbuzz_LDFLAGS)
        UNSET(harfbuzz_LDFLAGS_OTHER)
        UNSET(harfbuzz_INCLUDE_DIRS)
        UNSET(harfbuzz_CFLAGS)
        UNSET(harfbuzz_CFLAGS_OTHER)
    ENDIF()
ENDIF()

