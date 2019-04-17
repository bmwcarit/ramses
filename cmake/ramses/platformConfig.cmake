#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# set additional flags dependent on used compiler and version

# helper function to add flags
FUNCTION(ADD_FLAGS VAR)
    SET(TMP "${${VAR}}")
    FOREACH(flags ${ARGN})
        SET(TMP "${TMP} ${flags}")
    ENDFOREACH()
    SET(${VAR} ${TMP} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(REMOVE_FLAGS VAR)
    SET(TMP "${${VAR}}")
    FOREACH(flags ${ARGN})
        STRING(REPLACE "${flags}" "" TMP "${TMP}")
    ENDFOREACH()
    SET(${VAR} ${TMP} PARENT_SCOPE)
ENDFUNCTION()

# variables to fill
SET(RAMSES_C_CXX_FLAGS)
SET(RAMSES_C_FLAGS)
SET(RAMSES_CXX_FLAGS)
SET(RAMSES_DEBUG_FLAGS)
SET(RAMSES_DEBUG_INFO_FLAGS)
SET(RAMSES_RELEASE_FLAGS)

# gcc OR clang (they share a lot)
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    ADD_FLAGS(RAMSES_C_CXX_FLAGS "-fPIC -pthread -fvisibility=hidden")
    ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wall -Wextra -Wcast-align -Wshadow -Wformat -Wformat-security -Wvla -Wmissing-include-dirs")
    ADD_FLAGS(RAMSES_CXX_FLAGS "-std=c++14 -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast")
    ADD_FLAGS(RAMSES_C_FLAGS "-std=c11")
    ADD_FLAGS(RAMSES_DEBUG_FLAGS "-ggdb -D_DEBUG -fno-omit-frame-pointer")
    ADD_FLAGS(RAMSES_RELEASE_FLAGS "-O3 -DNDEBUG")
    ADD_FLAGS(RAMSES_DEBUG_INFO_FLAGS "-ggdb -fno-omit-frame-pointer")

    if (ramses-sdk_WARNINGS_AS_ERRORS)
        ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Werror")
    endif()

    IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows") # no fortify on mingw
        ADD_FLAGS(RAMSES_RELEASE_FLAGS "-fstack-protector-strong -D_FORTIFY_SOURCE=2")
    ENDIF()
ENDIF()

# gcc specific
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # remap GOT readonly after resolving
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") # mingw case
        ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wa,-mbig-obj")
    ELSE()
        ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wl,-z,relro,-z,now")
    ENDIF()

    if (ramses-sdk_BUILD_WITH_LTO)
        ADD_FLAGS(RAMSES_C_CXX_FLAGS "-flto -Wodr -Wlto-type-mismatch")
    endif()

    # gcc specific warnings
    ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wformat-signedness")
    # disable too crazy optimizations causing problems
    ADD_FLAGS(RAMSES_RELEASE_FLAGS "-fno-ipa-cp-clone")

    IF(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        # disable unfixed warnings from gcc 7
        ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wno-stringop-overflow -Wno-implicit-fallthrough")

        # enable more warnings on newer gcc
        ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wformat-overflow -Wfree-nonheap-object")
    ENDIF()

    # disable unfixed warnings from gcc 8
    IF(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
        ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wno-cast-function-type")
    ENDIF()
ENDIF()

# clang specific
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.5)
        # suppress missing override keyword warning
        ADD_FLAGS(RAMSES_CXX_FLAGS "-Wno-inconsistent-missing-override -Wmove")
    ENDIF()
ENDIF()

# flags for integrity
IF(${CMAKE_SYSTEM_NAME} MATCHES "Integrity")
    ADD_FLAGS(RAMSES_C_CXX_FLAGS "--diag_suppress=381,111,2008,620,82,1974")
    ADD_FLAGS(CMAKE_EXE_LINKER_FLAGS "--c++14")

    # integrity is an unknown system to the eglplatform.h header
    # so we manually define the correct choice for integrity here
    ADD_DEFINITIONS("-D__WINSCW__" "-DGTEST_HAS_TYPED_TEST=1" "-DGTEST_HAS_TYPED_TEST_P=1")
ENDIF()

# flags for windows
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    IF(MSVC)
        REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS}" "/W1;/W2;/W3;/W4" CMAKE_CXX_FLAGS)

        ADD_FLAGS(RAMSES_C_CXX_FLAGS "/MP /DNOMINMAX")
        ADD_FLAGS(RAMSES_CXX_FLAGS "/std:c++14 /W4 /wd4503 /wd4265 /wd4201 /wd4127 /wd4996 /bigobj")
        ADD_FLAGS(RAMSES_RELEASE_FLAGS "/MD /O2 /Ob2 /DNDEBUG")
        ADD_FLAGS(RAMSES_DEBUG_FLAGS "/MDd /Zi /Od /RTC1 /D_DEBUG")
        ADD_FLAGS(RAMSES_DEBUG_INFO_FLAGS "/Zi")

        if (ramses-sdk_WARNINGS_AS_ERRORS)
            ADD_FLAGS(RAMSES_C_CXX_FLAGS "/WX")
        endif()
    ENDIF()
    ADD_DEFINITIONS("-D_WIN32_WINNT=0x0600" "-DWINVER=0x0600") # enable 'modern' windows APIs
ENDIF()

# distribute to the correct cmake variables
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RAMSES_CXX_FLAGS} ${RAMSES_C_CXX_FLAGS} ")
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${RAMSES_C_FLAGS} ${RAMSES_C_CXX_FLAGS} ")

SET(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${RAMSES_DEBUG_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG} ${RAMSES_DEBUG_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${RAMSES_RELEASE_FLAGS}")
SET(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE} ${RAMSES_RELEASE_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${RAMSES_RELEASE_FLAGS} ${RAMSES_DEBUG_INFO_FLAGS}")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${RAMSES_RELEASE_FLAGS} ${RAMSES_DEBUG_INFO_FLAGS}")
