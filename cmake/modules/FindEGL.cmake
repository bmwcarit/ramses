#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET( EGL_FOUND FALSE )

IF (TARGET_OS MATCHES "Windows")

    # Windows does not suppot EGL natively

ELSEIF(TARGET_OS MATCHES "Linux" OR TARGET_OS MATCHES "Android")
    FIND_PATH(EGL_INCLUDE_DIRS EGL/egl.h
        /usr/include
    )

    FIND_LIBRARY(EGL_LIBRARY
        NAMES EGL
        PATHS
    )

    SET( EGL_FOUND "NO" )
    IF(EGL_LIBRARY)
        SET( EGL_LIBRARIES ${EGL_LIBRARY} )
        SET( EGL_FOUND TRUE )
        #message(STATUS "Found EGL libs: ${EGL_LIBRARIES}")
        #message(STATUS "Found EGL includes: ${EGL_INCLUDE_DIRS}")
    ENDIF(EGL_LIBRARY)

    MARK_AS_ADVANCED(
        EGL_INCLUDE_DIRS
        EGL_LIBRARIES
        EGL_LIBRARY
        EGL_FOUND
    )

ELSEIF((TARGET_OS MATCHES "Darwin"))
    SET(EGL_INCLUDE_DIRS
        ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/include
    )   

    FIND_LIBRARY(EGL_LIBRARIES MetalANGLE
     PATHS ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/MetalAngle.xcframework/macos-arm64_x86_64/
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

    IF(EGL_LIBRARIES)
        SET(EGL_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        EGL_INCLUDE_DIRS
        EGL_LIBRARIES
        EGL_FOUND
    )

ELSEIF((TARGET_OS MATCHES "iOS"))

    SET(EGL_INCLUDE_DIRS
        ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/include
    )   
   
    IF(ramses-sdk_METALANGLE_IOS_SIMULATOR)
        FIND_LIBRARY(EGL_LIBRARIES MetalANGLE
            PATHS ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/MetalAngle.xcframework/ios-arm64_x86_64-simulator/
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    ELSE()
        FIND_LIBRARY(EGL_LIBRARIES MetalANGLE
            PATHS ${ramses-sdk_SOURCE_DIR}/external/MetalANGLE/MetalAngle.xcframework/ios-arm64/
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    ENDIF()

    IF(EGL_LIBRARIES)
        SET(EGL_FOUND TRUE)
    ENDIF()
    
    MARK_AS_ADVANCED(
        EGL_INCLUDE_DIRS
        EGL_LIBRARIES
        EGL_FOUND
    )

ENDIF()
