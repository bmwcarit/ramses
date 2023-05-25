#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# let cmake know c++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# set additional flags dependent on used compiler and version

# TODO Violin/Tobias which of this stuff can be modernized?

# helper function to add flags
FUNCTION(ADD_FLAGS VAR)
    SET(TMP "${${VAR}}")
    FOREACH(flags ${ARGN})
        SET(TMP "${TMP} ${flags}")
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
SET(RLOGIC_C_CXX_FLAGS)
SET(RLOGIC_C_FLAGS)
SET(RLOGIC_CXX_FLAGS)
SET(RLOGIC_DEBUG_FLAGS)
SET(RLOGIC_DEBUG_INFO_FLAGS)
SET(RLOGIC_RELEASE_FLAGS)

# gcc OR clang (they share a lot)
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-fPIC -pthread")
    if (NOT ramses-logic_DISABLE_SYMBOL_VISIBILITY)
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-fvisibility=hidden")
    endif()
    ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Wall -Wextra -Wcast-align -Wshadow -Wformat -Wformat-security -Wvla -Wmissing-include-dirs")
    ADD_FLAGS(RLOGIC_CXX_FLAGS "-std=c++17 -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast")
    ADD_FLAGS(RLOGIC_C_FLAGS "-std=c11")
    ADD_FLAGS(RLOGIC_DEBUG_FLAGS "-ggdb -D_DEBUG -fno-omit-frame-pointer")
    ADD_FLAGS(RLOGIC_RELEASE_FLAGS "-O2 -DNDEBUG -fstack-protector-strong -D_FORTIFY_SOURCE=2")
    ADD_FLAGS(RLOGIC_DEBUG_INFO_FLAGS "-ggdb -fno-omit-frame-pointer")

    if (ramses-logic_WARNINGS_AS_ERRORS)
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Werror")
    endif()
ENDIF()

# gcc specific
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # optimize for debuggability
    ADD_FLAGS(RLOGIC_DEBUG_FLAGS "-Og")

    # remap GOT readonly after resolving
    ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Wl,-z,relro,-z,now")

    if (ramses-sdk_BUILD_WITH_LTO)
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-flto -Wodr -Wlto-type-mismatch")
    endif()

    # gcc specific warnings
    ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Wformat-signedness")
    # disable too crazy optimizations causing problems
    ADD_FLAGS(RLOGIC_RELEASE_FLAGS "-fno-ipa-cp-clone")

    IF(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        # disable unfixed warnings from gcc 7
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Wno-stringop-overflow -Wno-implicit-fallthrough")

        # enable more warnings on newer gcc
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Wformat-overflow -Wfree-nonheap-object")
    ENDIF()

    # disable unfixed warnings from gcc 8
    IF(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Wno-cast-function-type")
    ENDIF()
ENDIF()

# clang specific
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "Clang versions prior 10.0 are not supported")
    endif()

    ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-Wimplicit-fallthrough")

    #  do not optimize debug build at all (-Og is wrong on clang)
    ADD_FLAGS(RLOGIC_DEBUG_FLAGS "-O0")

    IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.5)
        # suppress missing override keyword warning
        ADD_FLAGS(RLOGIC_CXX_FLAGS "-Winconsistent-missing-override -Wmove")
    ENDIF()

    # handle enable coverage
    if (ramses-logic_ENABLE_TEST_COVERAGE)
        message(STATUS "+ Test coverage (clang)")
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-fprofile-instr-generate")
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "-fcoverage-mapping")
    endif()
ENDIF()

IF(ramses-logic_ENABLE_TEST_COVERAGE AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(FATAL_ERROR "Can't enable test coverage for compilers different than clang!")
ENDIF()

# flags for windows
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS}" "/W1;/W2;/W3;/W4" CMAKE_CXX_FLAGS)

    ADD_FLAGS(RLOGIC_C_CXX_FLAGS "/MP /DNOMINMAX")
    ADD_FLAGS(RLOGIC_CXX_FLAGS "/std:c++17 /W4 /wd4503 /wd4265 /wd4201 /wd4127 /wd4996 /bigobj")
    ADD_FLAGS(RLOGIC_RELEASE_FLAGS "/MD /O2 /Ob2 /DNDEBUG")
    ADD_FLAGS(RLOGIC_DEBUG_FLAGS "/MDd /Zi /Od /RTC1 /D_DEBUG")
    ADD_FLAGS(RLOGIC_DEBUG_INFO_FLAGS "/Zi")

    if (ramses-logic_WARNINGS_AS_ERRORS)
        ADD_FLAGS(RLOGIC_C_CXX_FLAGS "/WX")
    endif()
    ADD_DEFINITIONS("-D_WIN32_WINNT=0x0600" "-DWINVER=0x0600") # enable 'modern' windows APIs
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    SET(ENV{PKG_CONFIG_PATH} "")
    SET(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")
    SET(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
ENDIF()

# Special handling for C++17 filesystem
# Unfortunately not trivial, please keep this CMake config in one place!

function(check_handle_special_filesystem_lib libname)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_LIBRARIES ${libname})
    check_cxx_source_compiles("int main() {}" RLOGIC_HAS_STD_FS)

    if (RLOGIC_HAS_STD_FS)
        link_libraries(${libname})
    else()
        if (ramses-sdk_BUILD_TESTS)
            message(FATAL_ERROR "std::filesystem libary not found. Cannot use emulation with tests enabled")
        endif()
        if (NOT CMAKE_SYSTEM_NAME STREQUAL Linux)
            message(FATAL_ERROR "std::filesystem libary not found. Can use emulation only on Linux")
        endif()

        message(STATUS "std::filesystem libary not found, enable emulation")
        add_definitions("-DRLOGIC_STD_FILESYSTEM_EMULATION")
    endif()
endfunction()

# GCC prior version 9.1 puts filesystem in a separate static lib (stdc++fs) and potentially in the experimental namespace
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    check_handle_special_filesystem_lib(stdc++fs)
    # gcc prior version 8 puts symbols in the experimental namespace
    if(RLOGIC_HAS_STD_FS AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        add_definitions("-DRLOGIC_STD_FILESYSTEM_EXPERIMENTAL")
    endif()
endif()

# llvm prior version 9 puts filesystem in a separate static lib, similar but not exactly the same as GCC above
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    # libc++ requires different settings than stdlibc++
    # check if libc++ is used by inspecting global flags
    string(FIND "${CMAKE_CXX_FLAGS}" "-stdlib=libc++" USES_LIBCXX)
    if(USES_LIBCXX EQUAL -1)
        # Link stdc++fs from the std lib
        check_handle_special_filesystem_lib(stdc++fs)
        # llvm prior version 7 puts symbols in the experimental namespace
        if(RLOGIC_HAS_STD_FS AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
            add_definitions("-DRLOGIC_STD_FILESYSTEM_EXPERIMENTAL")
        endif()
    else()
        message(STATUS "Detected usage of libc++, using libc++ specific compiler flags")
        # See docs for more details https://libcxx.llvm.org/docs/UsingLibcxx.html#using-libc-experimental-and-experimental
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
            check_handle_special_filesystem_lib(c++experimental)
            if (RLOGIC_HAS_STD_FS)
                add_definitions("-DRLOGIC_STD_FILESYSTEM_EXPERIMENTAL")
            endif()
        else()
            link_libraries(libc++fs)
        endif()
    endif()
endif()

# distribute to the correct cmake variables
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RLOGIC_CXX_FLAGS} ${RLOGIC_C_CXX_FLAGS} ")
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${RLOGIC_C_FLAGS} ${RLOGIC_C_CXX_FLAGS} ")

SET(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${RLOGIC_DEBUG_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG} ${RLOGIC_DEBUG_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${RLOGIC_RELEASE_FLAGS}")
SET(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE} ${RLOGIC_RELEASE_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${RLOGIC_RELEASE_FLAGS} ${RLOGIC_DEBUG_INFO_FLAGS}")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${RLOGIC_RELEASE_FLAGS} ${RLOGIC_DEBUG_INFO_FLAGS}")
