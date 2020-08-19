#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# let cmake know c++ version
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# set additional flags dependent on used compiler and version

# interface library for all warning flags
add_library(ramses-build-options-base INTERFACE)

# helper function to add and remove flags
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

FUNCTION(REMOVE_FROM_FLAGS flags toRemoveList outVar)
    string(REGEX REPLACE " +" ";" flags_LIST "${flags}")   # to list
    list(REMOVE_ITEM flags_LIST ${toRemoveList})           # filter list
    string(REPLACE ";" " " flags_filtered "${flags_LIST}") # to string
    set(${outVar} "${flags_filtered}" PARENT_SCOPE)
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
    ADD_FLAGS(RAMSES_CXX_FLAGS "-std=c++14")
    ADD_FLAGS(RAMSES_C_FLAGS "-std=c11")
    ADD_FLAGS(RAMSES_DEBUG_FLAGS "-ggdb -D_DEBUG -fno-omit-frame-pointer")
    ADD_FLAGS(RAMSES_RELEASE_FLAGS "-O2 -DNDEBUG -fstack-protector-strong -D_FORTIFY_SOURCE=2")
    ADD_FLAGS(RAMSES_DEBUG_INFO_FLAGS "-ggdb -fno-omit-frame-pointer")

    target_compile_options(ramses-build-options-base INTERFACE
        -Wall -Wextra -Wcast-align -Wshadow -Wformat -Wformat-security -Wvla -Wmissing-include-dirs
        -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast)

    if (ramses-sdk_WARNINGS_AS_ERRORS)
        target_compile_options(ramses-build-options-base INTERFACE -Werror)
    endif()
ENDIF()

# gcc specific
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # optimize for debuggability
    ADD_FLAGS(RAMSES_DEBUG_FLAGS "-Og")

    # remap GOT readonly after resolving
    ADD_FLAGS(RAMSES_C_CXX_FLAGS "-Wl,-z,relro,-z,now")

    if (ramses-sdk_BUILD_WITH_LTO)
        target_compile_options(ramses-build-options-base INTERFACE -flto -Wodr -Wlto-type-mismatch)
    endif()

    # gcc specific warnings
    target_compile_options(ramses-build-options-base INTERFACE -Wformat-signedness)

    IF(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        # disable unfixed warnings from gcc 7
        target_compile_options(ramses-build-options-base INTERFACE -Wno-stringop-overflow -Wno-implicit-fallthrough)

        # enable more warnings on newer gcc
        target_compile_options(ramses-build-options-base INTERFACE -Wformat-overflow -Wfree-nonheap-object)
    ENDIF()

    # disable unfixed warnings from gcc 8
    IF(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
        target_compile_options(ramses-build-options-base INTERFACE -Wno-cast-function-type)
    ENDIF()

    # handle enable coverage
    if (ramses-sdk_ENABLE_COVERAGE)
        message(STATUS "+ gcov coverage enabled")
        target_compile_options(ramses-build-options-base INTERFACE --coverage)
        target_link_options(ramses-build-options-base INTERFACE --coverage)
    endif()
ENDIF()

# clang specific
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #  do not optimize debug build at all (-Og is wrong on clang)
    ADD_FLAGS(RAMSES_DEBUG_FLAGS "-O0")

    IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.5)
        # suppress missing override keyword warning
        target_compile_options(ramses-build-options-base INTERFACE -Winconsistent-missing-override -Wmove)
    ENDIF()

    # handle enable coverage
    if (ramses-sdk_ENABLE_COVERAGE)
        message(STATUS "+ clang source based coverage enabled")
        target_compile_options(ramses-build-options-base INTERFACE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(ramses-build-options-base INTERFACE -fprofile-instr-generate -fcoverage-mapping)
    endif()
ENDIF()

# flags for integrity
IF(${CMAKE_SYSTEM_NAME} MATCHES "Integrity")
    target_compile_options(ramses-build-options-base INTERFACE --diag_suppress=381,111,2008,620,82,1974,1932,1721,1704,540,68,991,177,174)
    ADD_FLAGS(CMAKE_EXE_LINKER_FLAGS "--c++14")
    ADD_FLAGS(RAMSES_RELEASE_FLAGS "-DNDEBUG")

    if (ramses-sdk_WARNINGS_AS_ERRORS)
        target_compile_options(ramses-build-options-base INTERFACE --quit_after_warnings)
    endif()

    # integrity is an unknown system to the eglplatform.h header
    # so we manually define the correct choice for integrity here
    ADD_DEFINITIONS("-D__WINSCW__" "-DGTEST_HAS_TYPED_TEST=1" "-DGTEST_HAS_TYPED_TEST_P=1")
ENDIF()

# flags for windows
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS}" "/W1;/W2;/W3;/W4" CMAKE_CXX_FLAGS)

    ADD_FLAGS(RAMSES_C_CXX_FLAGS "/MP /DNOMINMAX")
    ADD_FLAGS(RAMSES_CXX_FLAGS "/std:c++14 /bigobj")
    target_compile_options(ramses-build-options-base INTERFACE /W4 /wd4503 /wd4265 /wd4201 /wd4127 /wd4996)
    ADD_FLAGS(RAMSES_RELEASE_FLAGS "/MD /O2 /Ob2 /DNDEBUG")
    ADD_FLAGS(RAMSES_DEBUG_FLAGS "/MDd /Zi /Od /D_DEBUG")
    target_compile_options(ramses-build-options-base INTERFACE $<$<CONFIG:Debug>:/RTC1>)
    ADD_FLAGS(RAMSES_DEBUG_INFO_FLAGS "/Zi")

    if (ramses-sdk_WARNINGS_AS_ERRORS)
        target_compile_options(ramses-build-options-base INTERFACE /WX)
    endif()
    ADD_DEFINITIONS("-D_WIN32_WINNT=0x0600" "-DWINVER=0x0600") # enable 'modern' windows APIs
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    SET(ENV{PKG_CONFIG_PATH} "")
    SET(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")
    SET(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
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
