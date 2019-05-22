#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_STANDARD 11)

project(examples)

if (NOT DEFINED RAMSES_RENDERER_PLATFORM)
    set(RAMSES_RENDERER_PLATFORM "LINUX-X11")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(PLATFORM_DEPENDANT_DEPENDENCIES rt pthread)
else()
    set(PLATFORM_DEPENDANT_DEPENDENCIES)
endif()

add_custom_target(run-all ALL)

foreach(glversion "" ${GL_VERSION})
    unset(ramses-static-lib_FOUND CACHE)
    unset(ramses-static-lib_LIBRARIES CACHE)
    unset(ramses-static-lib_INCLUDE_DIRS CACHE)

    set(name ramses-static-lib-check)
    if (glversion)
        set(name ${name}-${glversion})
    endif()
    message(STATUS "Build '${name}'")

    find_package(ramses-static-lib ${RAMSES_VERSION} REQUIRED COMPONENTS ${RAMSES_RENDERER_PLATFORM} ${glversion} NO_CMAKE_BUILDS_PATH)

    add_executable(${name} "ramses-static-lib-check.cpp")
    target_link_libraries(${name} ${ramses-static-lib_LIBRARIES} ${PLATFORM_DEPENDANT_DEPENDENCIES})
    target_include_directories(${name} PRIVATE ${ramses-static-lib_INCLUDE_DIRS})

    add_custom_target(run-${name}
        COMMAND $<TARGET_FILE:${name}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${name}
        COMMENT "Run executable ${name}")
    add_dependencies(run-all run-${name})
endforeach()
