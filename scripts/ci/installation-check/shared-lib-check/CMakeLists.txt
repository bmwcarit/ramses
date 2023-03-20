#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(examples)

if (NOT DEFINED RAMSES_RENDERER_PLATFORM)
    message(FATAL_ERROR "Must set RAMSES_RENDERER_PLATFORM")
endif()

add_custom_target(run-all ALL)

if (NOT DEFINED GL_VERSION OR GL_VERSION STREQUAL "-")
    # default none
    set(GL_VERSION "")
endif()

unset(ramses-shared-lib_FOUND CACHE)
unset(ramses-shared-lib_LIBRARIES CACHE)
unset(ramses-shared-lib_INCLUDE_DIRS CACHE)
unset(ramses-sdk_VERSION CACHE)

set(name ramses-shared-lib-check)
if (GL_VERSION)
    set(name ${name}-${GL_VERSION})
endif()
message(STATUS "Build '${name}'")

find_package(ramses-shared-lib ${RAMSES_VERSION} REQUIRED COMPONENTS ${RAMSES_RENDERER_PLATFORM} ${GL_VERSION} NO_CMAKE_BUILDS_PATH)

if (NOT DEFINED ramses-sdk_VERSION)
    message(FATAL_ERROR "ramses-sdk_VERSION variable missing after find_package")
endif()

# build with exported variables
add_executable(${name} "ramses-shared-lib-check.cpp")
target_link_libraries(${name} ${ramses-shared-lib_LIBRARIES})
target_include_directories(${name} PRIVATE ${ramses-shared-lib_INCLUDE_DIRS})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    get_filename_component(ramses-shared-lib_LIBDIR ${ramses-shared-lib_LIBRARIES} DIRECTORY)
    target_link_libraries(${name} "-Wl,-rpath-link,${ramses-shared-lib_LIBDIR}")
endif()

add_custom_target(run-${name}
    COMMAND $<TARGET_FILE:${name}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${name}
    COMMENT "Run executable ${name}")
add_dependencies(run-all run-${name})

# build with exported target
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13")
    set(tgtname "tgt-${name}")
    add_executable(${tgtname} "ramses-shared-lib-check.cpp")
    target_link_libraries(${tgtname} ramses::ramses)

    add_custom_target(run-${tgtname}
        COMMAND $<TARGET_FILE:${tgtname}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${tgtname}
        COMMENT "Run executable ${tgtname}")
    add_dependencies(run-all run-${tgtname})
endif()
