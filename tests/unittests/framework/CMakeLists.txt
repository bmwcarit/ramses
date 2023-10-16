#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if (ramses-sdk_ENABLE_TCP_SUPPORT)
    set(ramses-framework-test-TCP_MIXIN
    INCLUDE_PATHS           Communication/TransportTCP
    SRC_FILES               Communication/TransportTCP/*.h
                            Communication/TransportTCP/*.cpp)
endif()

createModule(
    NAME                    ramses-framework-test
    TYPE                    BINARY

    INCLUDE_PATHS           PlatformAbstraction
    SRC_FILES               PlatformAbstraction/*.h
                            PlatformAbstraction/*.cpp
    INCLUDE_PATHS           ramses-framework
    SRC_FILES               ramses-framework/*.h
                            ramses-framework/*.cpp
    INCLUDE_PATHS           Core/Math3d
    SRC_FILES               Core/Math3d/*.h
                            Core/Math3d/*.cpp
    INCLUDE_PATHS           Core/Utils
    SRC_FILES               Core/Utils/*.h
                            Core/Utils/*.cpp
    SRC_FILES               Core/Common/*.h
                            Core/Common/*.cpp
    INCLUDE_PATHS           Core/TaskFramework
    SRC_FILES               Core/TaskFramework/*.h
                            Core/TaskFramework/*.cpp
    INCLUDE_PATHS           SceneGraph/Scene
    SRC_FILES               SceneGraph/Scene/*.h
                            SceneGraph/Scene/*.cpp
    INCLUDE_PATHS           SceneGraph/Resource
    SRC_FILES               SceneGraph/Resource/*.h
                            SceneGraph/Resource/*.cpp
    INCLUDE_PATHS           SceneGraph/SceneAPI
    SRC_FILES               SceneGraph/SceneAPI/*.h
                            SceneGraph/SceneAPI/*.cpp
    INCLUDE_PATHS           Communication/TransportCommon
    SRC_FILES               Communication/TransportCommon/*.h
                            Communication/TransportCommon/*.cpp
    INCLUDE_PATHS           Ramsh
    SRC_FILES               Ramsh/*.h
                            Ramsh/*.cpp
    INCLUDE_PATHS           Components
    SRC_FILES               Components/*.h
                            Components/*.cpp
    INCLUDE_PATHS           DltLogAppender
    SRC_FILES               DltLogAppender/*.h
                            DltLogAppender/*.cpp
    INCLUDE_PATHS           Watchdog
    SRC_FILES               Watchdog/*.h
                            Watchdog/*.cpp
    INCLUDE_PATHS           SceneReferencing
    SRC_FILES               SceneReferencing/*.h
                            SceneReferencing/*.cpp

    ${ramses-framework-test-TCP_MIXIN}

    SRC_FILES               main.cpp

    DEPENDENCIES            ramses-framework
                            framework-test-utils
                            ramses-gmock-main
)

# disable maybe uninitialized
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
        target_compile_options(ramses-framework-test PRIVATE -Wno-maybe-uninitialized)
    endif()
endif()

makeTestFromTarget(TARGET ramses-framework-test SUFFIX UNITTEST)
