#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# check selected c++ version
set(ramses-sdk_ALLOWED_CPP_VERSIONS 17 20)
if (NOT ramses-sdk_CPP_VERSION IN_LIST ramses-sdk_ALLOWED_CPP_VERSIONS)
    message(FATAL_ERROR "ramses-sdk_CPP_VERSION=${ramses-sdk_CPP_VERSION} must be in ${ramses-sdk_ALLOWED_CPP_VERSIONS}")
endif()

# let cmake know c++ version
set(CMAKE_CXX_STANDARD ${ramses-sdk_CPP_VERSION})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# set additional flags dependent on used compiler and version

# interface library for all warning flags
add_library(ramses-build-options-base INTERFACE)

# helper function to add and remove flags
function(addFlags VAR)
    set(TMP "${${VAR}}")
    foreach(flags ${ARGN})
        set(TMP "${TMP} ${flags}")
    endforeach()
    set(${VAR} ${TMP} PARENT_SCOPE)
endfunction()

function(removeFlags flags toRemoveList outVar)
    string(REGEX REPLACE " +" ";" flags_LIST "${flags}")   # to list
    list(REMOVE_ITEM flags_LIST ${toRemoveList})           # filter list
    string(REPLACE ";" " " flags_filtered "${flags_LIST}") # to string
    set(${outVar} "${flags_filtered}" PARENT_SCOPE)
endfunction()


# variables to fill
set(RAMSES_C_CXX_FLAGS)
set(RAMSES_C_FLAGS)
set(RAMSES_CXX_FLAGS)
set(RAMSES_DEBUG_FLAGS)
set(RAMSES_DEBUG_INFO_FLAGS)
set(RAMSES_RELEASE_FLAGS)

# gcc OR clang (they share a lot)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    addFlags(RAMSES_C_CXX_FLAGS "-fPIC -pthread -fvisibility=hidden")
    addFlags(RAMSES_CXX_FLAGS "-std=c++${ramses-sdk_CPP_VERSION}")
    addFlags(RAMSES_C_FLAGS "-std=c11")
    addFlags(RAMSES_DEBUG_FLAGS "-ggdb -D_DEBUG -fno-omit-frame-pointer")
    addFlags(RAMSES_RELEASE_FLAGS "-O2 -DNDEBUG -fstack-protector-strong -D_FORTIFY_SOURCE=2")
    addFlags(RAMSES_DEBUG_INFO_FLAGS "-ggdb -fno-omit-frame-pointer")

    target_compile_options(ramses-build-options-base INTERFACE
        -Wall -Wextra -Wcast-align -Wshadow -Wformat -Wformat-security -Wvla -Wmissing-include-dirs
        -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast -Wunused)

    if (ramses-sdk_WARNINGS_AS_ERRORS)
        target_compile_options(ramses-build-options-base INTERFACE -Werror)
    endif()

    if (ramses-sdk_USE_LINKER_OVERWRITE)
        message(STATUS "+ Use linker '${ramses-sdk_USE_LINKER_OVERWRITE}'")
        set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=${ramses-sdk_USE_LINKER_OVERWRITE}")
    endif()
endif()

# gcc specific
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # optimize for debuggability
    addFlags(RAMSES_DEBUG_FLAGS "-Og")

    # remap GOT readonly after resolving
    addFlags(RAMSES_C_CXX_FLAGS "-Wl,-z,relro,-z,now")

    if (ramses-sdk_BUILD_WITH_LTO)
        target_compile_options(ramses-build-options-base INTERFACE -flto -Wodr -Wlto-type-mismatch)
    endif()

    # gcc specific warnings
    target_compile_options(ramses-build-options-base INTERFACE
        -Wformat-signedness -Wformat-overflow -Wfree-nonheap-object)

    # disable unfixed warnings from gcc 7
    target_compile_options(ramses-build-options-base INTERFACE -Wno-implicit-fallthrough)

    # disable unfixed warnings from gcc 8
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
        target_compile_options(ramses-build-options-base INTERFACE -Wno-cast-function-type)
    endif()

    # disable stringop overflow from gcc 11 - needed for lodepnd
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
        target_compile_options(ramses-build-options-base INTERFACE -Wno-stringop-overflow)
    endif()


    # handle enable coverage
    if (ramses-sdk_ENABLE_COVERAGE)
        message(STATUS "+ gcov coverage enabled")
        target_compile_options(ramses-build-options-base INTERFACE --coverage)
        target_link_options(ramses-build-options-base INTERFACE --coverage)
    endif()
endif()

# clang specific
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(ramses-build-options-base INTERFACE
        -Wimplicit-fallthrough -Winconsistent-missing-override -Wmove
        -Winconsistent-missing-destructor-override)

    #  do not optimize debug build at all (-Og is wrong on clang)
    addFlags(RAMSES_DEBUG_FLAGS "-O0")

    # handle enable coverage
    if (ramses-sdk_ENABLE_COVERAGE)
        message(STATUS "+ clang source based coverage enabled")
        target_compile_options(ramses-build-options-base INTERFACE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(ramses-build-options-base INTERFACE -fprofile-instr-generate -fcoverage-mapping)
    endif()

    # handle sanitizers
    if (ramses-sdk_ENABLE_SANITIZER AND NOT ramses-sdk_ENABLE_SANITIZER STREQUAL "")

        set(ubsan_checks "bool,bounds,enum,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,vla-bound,float-cast-overflow,vptr,alignment,function")
        set(ubsan_options "-fsanitize=undefined -fno-sanitize-recover=${ubsan_checks}")
        set(tsan_options "-fsanitize=thread")
        set(asan_options "-fsanitize=address")

        if (ramses-sdk_ENABLE_SANITIZER STREQUAL "ubsan")
            message(STATUS "+ Enable undefined behavior sanitizer (ubsan)")
            set(sanitizer_options "${ubsan_options}")
        elseif(ramses-sdk_ENABLE_SANITIZER STREQUAL "tsan")
            message(STATUS "+ Enable thread sanitizer (tsan)")
            set(sanitizer_options "${tsan_options}")
        elseif(ramses-sdk_ENABLE_SANITIZER STREQUAL "asan")
            message(STATUS "+ Enable address sanitizer (asan)")
            set(sanitizer_options "${asan_options}")
        elseif(ramses-sdk_ENABLE_SANITIZER STREQUAL "asan+ubsan")
            message(STATUS "+ Enable address and undefined behavior sanitizers (asan+ubsan)")
            set(sanitizer_options "${asan_options} ${ubsan_options}")
        else()
            message(FATAL_ERROR "Unknown value for ramses-sdk_ENABLE_SANITIZER '${ramses-sdk_ENABLE_SANITIZER}'")
        endif()
        addFlags(RAMSES_C_CXX_FLAGS "-fno-omit-frame-pointer ${sanitizer_options}")
    endif()
endif()

# flags for windows
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    removeFlags("${CMAKE_CXX_FLAGS}" "/W1;/W2;/W3;/W4" CMAKE_CXX_FLAGS)

    addFlags(RAMSES_C_CXX_FLAGS "/MP /DNOMINMAX")
    addFlags(RAMSES_CXX_FLAGS "/std:c++${ramses-sdk_CPP_VERSION} /bigobj")
    target_compile_options(ramses-build-options-base INTERFACE /W4 /wd4503 /wd4265 /wd4201 /wd4127 /wd4996 /wd4702)
    addFlags(RAMSES_RELEASE_FLAGS "/MD /O2 /Ob2 /DNDEBUG")
    addFlags(RAMSES_DEBUG_FLAGS "/MDd /Zi /Od /D_DEBUG")
    target_compile_options(ramses-build-options-base INTERFACE $<$<CONFIG:Debug>:/RTC1>)
    addFlags(RAMSES_DEBUG_INFO_FLAGS "/Zi")

    if (ramses-sdk_WARNINGS_AS_ERRORS)
        target_compile_options(ramses-build-options-base INTERFACE /WX)
    endif()
    add_definitions("-D_WIN32_WINNT=0x0600" "-DWINVER=0x0600") # enable 'modern' windows APIs

    if (${MSVC_TOOLSET_VERSION} LESS_EQUAL "141") # disable warning unreachable code for VS 2017 due to false positives
        addFlags(RAMSES_C_CXX_FLAGS "/wd4702")
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(ENV{PKG_CONFIG_PATH} "")
    set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")
    set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
endif()

if((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "iOS"))
    addFlags(RAMSES_C_FLAGS "-x objective-c")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")
    set(CMAKE_XCODE_GENERATE_SCHEME ON)
endif()

# distribute to the correct cmake variables
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RAMSES_CXX_FLAGS} ${RAMSES_C_CXX_FLAGS} ")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${RAMSES_C_FLAGS} ${RAMSES_C_CXX_FLAGS} ")

set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${RAMSES_DEBUG_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG} ${RAMSES_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${RAMSES_RELEASE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE} ${RAMSES_RELEASE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${RAMSES_RELEASE_FLAGS} ${RAMSES_DEBUG_INFO_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${RAMSES_RELEASE_FLAGS} ${RAMSES_DEBUG_INFO_FLAGS}")
