#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# determine target os and architecture

# exclude unsupported platforms
IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND MSVC)
    IF (NOT MSVC_VERSION GREATER 1899)
        MESSAGE(FATAL_ERROR "Only Visual Studio 2015 and later supported")
    ENDIF()
ENDIF()

# guess TARGET_OS if not given
IF (NOT TARGET_OS)
    IF ((CMAKE_SYSTEM_NAME STREQUAL "Windows") OR (CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME STREQUAL "Integrity") OR (CMAKE_SYSTEM_NAME STREQUAL "Android"))
        SET(TARGET_OS "${CMAKE_SYSTEM_NAME}")
    ELSE()
        MESSAGE(FATAL_ERROR "No TARGET_OS given and unknown CMAKE_SYSTEM_NAME '${CMAKE_SYSTEM_NAME}'")
    ENDIF()
ENDIF()

# guess target bitness
IF (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    SET(TARGET_BITNESS 32)
ELSEIF(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    SET(TARGET_BITNESS 64)
ELSE()
    MESSAGE(FATAL_ERROR "Unknown sizeof(void*) '${CMAKE_SIZEOF_VOID_P}'")
ENDIF()
