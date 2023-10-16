#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(examples)

find_package(ramses-shared-lib-headless ${RAMSES_VERSION} REQUIRED NO_CMAKE_BUILDS_PATH)

if (NOT DEFINED ramses-sdk_VERSION)
    message(FATAL_ERROR "ramses-sdk_VERSION variable missing after find_package")
endif()

add_custom_target(run-all ALL)

# make sure the libraries are found without the need to set LD_LIBRARY_PATH
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${CMAKE_PREFIX_PATH}/lib")

# build with exported variables
add_executable(ramses-shared-lib-check "ramses-shared-lib-check.cpp")
target_link_libraries(ramses-shared-lib-check ${ramses-shared-lib-headless_LIBRARIES})
target_include_directories(ramses-shared-lib-check PRIVATE ${ramses-shared-lib-headless_INCLUDE_DIRS})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    get_filename_component(ramses-shared-lib_LIBDIR ${ramses-shared-lib-headless_LIBRARIES} DIRECTORY)
    target_link_libraries(ramses-shared-lib-check "-Wl,-rpath-link,${ramses-shared-lib_LIBDIR}")
endif()

add_custom_target(run-ramses-shared-lib-check
    COMMAND $<TARGET_FILE:ramses-shared-lib-check>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ramses-shared-lib-check
    COMMENT "Run executable ramses-shared-lib-check")
add_dependencies(run-all run-ramses-shared-lib-check)

# build with exported target
add_executable(tgt-ramses-shared-lib-check "ramses-shared-lib-check.cpp")
target_link_libraries(tgt-ramses-shared-lib-check ramses::headless)

add_custom_target(run-tgt-ramses-shared-lib-check
    COMMAND $<TARGET_FILE:tgt-ramses-shared-lib-check>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS tgt-ramses-shared-lib-check
    COMMENT "Run executable tgt-ramses-shared-lib-check")
add_dependencies(run-all run-tgt-ramses-shared-lib-check)
