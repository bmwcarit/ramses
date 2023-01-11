#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")

    # we always use the Khronos reference headers
    SET(OpenGL_INCLUDE_DIRS
        ${ramses-sdk_SOURCE_DIR}/external/khronos
    )

    # Windows has all OpenGL/WGL symbols in one lib - opengl32.lib
    SET(OpenGL_LIBRARIES opengl32)
    SET(OpenGL_DEFINITIONS "-DDESKTOP_GL")

    MARK_AS_ADVANCED(
        OpenGL_INCLUDE_DIRS
        OpenGL_LIBRARIES
        OpenGL_DEFINITIONS
    )

    SET(OpenGL_FOUND TRUE)

ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

    # we always use the Khronos reference headers
    SET(OpenGL_INCLUDE_DIRS
        ${ramses-sdk_SOURCE_DIR}/external/khronos
    )

    # only check, if systems has GLESv3 headers, if yes we
    # assume the system is GLESv3 enabled.
    # we still use the included khronos GLESv3 headers.
    FIND_PATH(GLESv3_INCLUDE_DIRS_IN_SYSROOT GLES3/gl3.h
        /usr/include
    )

    # GL ES 3 is implemented in GLESv2 (in mesa)
    FIND_LIBRARY(OpenGL_LIBRARIES
        NAMES GLESv2
        PATHS
    )

    IF(OpenGL_LIBRARIES AND GLESv3_INCLUDE_DIRS_IN_SYSROOT)
        SET(OpenGL_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        OpenGL_INCLUDE_DIRS
        OpenGL_LIBRARIES
    )

ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Android")

    # only check, if systems has GLESv3 headers, if yes we
    # assume the system is GLESv3 enabled.
    # we still use the included khronos GLESv3 headers.
    FIND_PATH(OpenGL_INCLUDE_DIRS_IN_SYSROOT GLES3/gl3.h
        /usr/include
    )

    FIND_LIBRARY(OpenGL_LIBRARIES
        NAMES GLESv3
        PATHS
    )

    IF(OpenGL_LIBRARIES AND OpenGL_INCLUDE_DIRS_IN_SYSROOT)
        SET(OpenGL_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        OpenGL_INCLUDE_DIRS
        OpenGL_LIBRARIES
    )

ELSEIF((CMAKE_SYSTEM_NAME MATCHES "Darwin"))
    execute_process(COMMAND "${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/FetchAndBuildMetalANGLE.sh"
                    WORKING_DIRECTORY "${ramses-sdk_SOURCE_DIR}/external/MetalANGLE")

    SET(OpenGL_INCLUDE_DIRS    
        ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/include
    )   

    FIND_LIBRARY(OpenGL_LIBRARIES MetalANGLE
        PATHS ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/MetalAngle.xcframework/macos-arm64_x86_64/
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

    IF(OpenGL_LIBRARIES)
        SET(OpenGL_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        OpenGL_INCLUDE_DIRS
        OpenGL_LIBRARIES
    )

ELSEIF((CMAKE_SYSTEM_NAME MATCHES "iOS"))
    execute_process(COMMAND "${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/FetchAndBuildMetalANGLE.sh"
        WORKING_DIRECTORY "${ramses-sdk_SOURCE_DIR}/external/MetalANGLE")

    SET(OpenGL_INCLUDE_DIRS
        ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/include
    )   

    IF(ramses-sdk_METALANGLE_IOS_SIMULATOR)
        FIND_LIBRARY(OpenGL_LIBRARIES MetalANGLE
            PATHS ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/MetalAngle.xcframework/ios-arm64_x86_64-simulator/
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    ELSE()
        FIND_LIBRARY(OpenGL_LIBRARIES MetalANGLE
            PATHS ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/MetalAngle.xcframework/ios-arm64/
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    ENDIF()
    IF(OpenGL_LIBRARIES)
        SET(OpenGL_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        OpenGL_INCLUDE_DIRS
        OpenGL_LIBRARIES
    )

ENDIF()

