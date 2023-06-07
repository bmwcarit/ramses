#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# determine target os and architecture

# exclude unsupported platforms
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND MSVC)
    if (NOT MSVC_VERSION GREATER 1909)  # VS2017 is 1910 - 1919
        message(FATAL_ERROR "Visual Studio 2017 or later is required to build ramses")
    endif()
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
    message(FATAL_ERROR "GCC 7 or new required to build ramses")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
    message(FATAL_ERROR "Clang 6 or new required to build ramses")
endif()

if ((CMAKE_SYSTEM_NAME STREQUAL "Windows") OR (CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME STREQUAL "Darwin") OR (CMAKE_SYSTEM_NAME STREQUAL "iOS") OR (CMAKE_SYSTEM_NAME STREQUAL "Integrity") OR (CMAKE_SYSTEM_NAME STREQUAL "Android"))
    # We support these systems
else()
    message(FATAL_ERROR "Unknown CMAKE_SYSTEM_NAME '${CMAKE_SYSTEM_NAME}'")
endif()

# guess target bitness
if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    SET(TARGET_BITNESS 32)
ELSEIF(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    SET(TARGET_BITNESS 64)
else()
    message(FATAL_ERROR "Unknown sizeof(void*) '${CMAKE_SIZEOF_VOID_P}'")
endif()
