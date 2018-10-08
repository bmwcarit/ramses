#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

INCLUDE(FindPackageHandleStandardArgs)

# guess vtune install dir if not given
IF(NOT ramses-sdk_BUILD_PERFORMANCE_PROFILER_VTUNE_INSTALL_DIR)
    IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        SET(ramses-sdk_BUILD_PERFORMANCE_PROFILER_VTUNE_INSTALL_DIR "/opt/intel/")
    ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        SET(ramses-sdk_BUILD_PERFORMANCE_PROFILER_VTUNE_INSTALL_DIR "C:/Program Files (x86)/IntelSWTools/VTune Amplifier XE 2017" )
    ENDIF()
ENDIF()

# find include path
FIND_PATH(VTune_INCLUDE_DIRS
    NAMES ittnotify.h
    PATHS ${ramses-sdk_BUILD_PERFORMANCE_PROFILER_VTUNE_INSTALL_DIR}
    PATH_SUFFIXES include)

# find library
IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(vtune_library_name "libittnotify.a")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(vtune_library_name "libittnotify")
ENDIF()

IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(VTune_LIB_DIR lib64)
ELSE()
    SET(VTune_LIB_DIR lib32)
ENDIF()

FIND_LIBRARY(VTune_LIBRARIES
    ${vtune_library_name}
    HINTS "${VTune_INCLUDE_DIRS}/.."
    PATHS ${ramses-sdk_BUILD_PERFORMANCE_PROFILER_VTUNE_INSTALL_DIR}
    PATH_SUFFIXES ${VTune_LIB_DIR})

# check found
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VTune
    FOUND_VAR VTune_FOUND
    REQUIRED_VARS VTune_LIBRARIES VTune_INCLUDE_DIRS)

MESSAGE("+ Intel VTune (Include: ${VTune_INCLUDE_DIRS} Libs: ${VTune_LIBRARIES})")

MARK_AS_ADVANCED(
    VTune_INCLUDE_DIRS
    VTune_LIBRARIES
    VTune_FOUND)
