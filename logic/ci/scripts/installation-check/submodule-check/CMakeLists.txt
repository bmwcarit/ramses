#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(submodule_check)

#######################################################################
### Find ramses from installation and simulate custom ramses target
#######################################################################

find_package(ramses-shared-lib REQUIRED NO_CMAKE_BUILDS_PATH COMPONENTS LINUX-X11)

add_library(custom-ramses SHARED IMPORTED GLOBAL)
set_target_properties(custom-ramses PROPERTIES IMPORTED_LOCATION ${ramses-shared-lib_LIBRARIES})
if (MSVC)
    set_target_properties(custom-ramses PROPERTIES IMPORTED_IMPLIB ${ramses-shared-lib_LIBRARIES})
endif()
target_include_directories(custom-ramses INTERFACE ${ramses-shared-lib_INCLUDE_DIRS})
add_library(my::custom-ramses ALIAS custom-ramses)

#######################################################################
### Tell ramses logic to use custom ramses and force error if not found
#######################################################################

set(ramses-logic_RAMSES_TARGET "my::custom-ramses" CACHE STRING "" FORCE)
set(ramses-logic_ALLOW_RAMSES_BUILD OFF CACHE BOOL "" FORCE)

#######################################################################
### Enable static build, disable shared build (exactly the opposite than the defaults)
#######################################################################

set(ramses-logic_BUILD_STATIC_LIB ON CACHE BOOL "" FORCE)
set(ramses-logic_BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)

#######################################################################
### Add ramses logic as a subdirectory and build using existing ramses target
#######################################################################

# This directory is injected by the test script
# and acts as if ramses logic was a submodule of another project
add_subdirectory(ramses-logic)

add_executable(submodule-check ramses-logic-submodule-check.cpp)

target_link_libraries(submodule-check rlogic::ramses-logic-static)

add_custom_target(run-submodule-check
    COMMAND $<TARGET_FILE:submodule-check>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS submodule-check
    COMMENT "Run executable submodule-check")

add_custom_target(run-all ALL)
add_dependencies(run-all run-submodule-check)

# Do some extra checks on publicly exported targets/variables

if(NOT DEFINED ramses-logic_VERSION)
    message(FATAL_ERROR "ramses-logic_VERSION is not exported correctly!")
endif()

if(TARGET rlogic::ramses-logic)
    message(FATAL_ERROR "ramses-logic shared lib target is wrongly exported (should export only static lib)")
endif()
