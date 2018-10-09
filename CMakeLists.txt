#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED(VERSION 3.3)

SET(ramses-sdk_ROOT_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/ramses/setCmakePolicies.cmake NO_POLICY_SCOPE)  # we want to propagate policy settings out!
include(cmake/ramses/platformDetection.cmake)
include(cmake/ramses/platformConfig.cmake)

SET(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

SET (ACME2_PROJECT_PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake/acme2_plugins)
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

INCLUDE(external/acme2/acme2.cmake)

# if no build configuration is defined, use build config 'Release'
IF (NOT CMAKE_BUILD_TYPE)
    ACME_INFO("Build type not defined. Using default build type 'Release'.")
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug, Release." FORCE)
ELSE ()
    # Case-insensitive comparison required, built type is not case-sensitive
    STRING(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER_CASE)
    IF(NOT("${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "debug" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "release" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "relwithdebinfo"))
        ACME_ERROR("Build type set to unsupported type ${CMAKE_BUILD_TYPE}.")
    ENDIF ()
ENDIF (NOT CMAKE_BUILD_TYPE)

MESSAGE(STATUS "Ramses Build Config: SystemName ${CMAKE_SYSTEM_NAME}, SystemVersion ${CMAKE_SYSTEM_VERSION}, CompilerID ${CMAKE_CXX_COMPILER_ID}, TargetOS ${TARGET_OS}, TargetArch ${TARGET_ARCH}, BuildType ${CMAKE_BUILD_TYPE}, TargetBitness ${TARGET_BITNESS}")

# find options
OPTION(ramses-sdk_ALLOW_PLATFORM_FREETYPE "Enable to search for platform provided freetype and harfbuzz" ON)
OPTION(ramses-sdk_ALLOW_CUSTOM_FREETYPE "Allow usage of custom freetype and harfbuzz if platform provided one  was not found" ON)

# build options
OPTION(ramses-sdk_BUILD_EXAMPLES "Build Example targets: ON, OFF" ON)
OPTION(ramses-sdk_BUILD_SMOKE_TESTS "Build smoke test targets: ON, OFF" ON)
OPTION(ramses-sdk_BUILD_DEMOS "Build demo targets: ON, OFF" ON)
OPTION(ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB "Build client only shared library instead of per renderer libraries" OFF)
OPTION(ramses-sdk_ENABLE_WAYLAND_SHELL "Build a version of ramses renderer which uses wayland shell surfaces instead of IVI surfaces" OFF)
OPTION(ramses-sdk_ENABLE_TCP_SUPPORT "Enable use of TCP communication" ON)
OPTION(ramses-sdk_ENABLE_DLT "Enable dlt support" ON)
OPTION(ramses-sdk_USE_LINUX_DEV_PTP "Enable support for synchronized ptp time on linux" OFF)

SET(ramses-sdk_CONSOLE_LOG_LEVEL "" CACHE STRING "Console log level.")
SET_PROPERTY(CACHE ramses-sdk_CONSOLE_LOG_LEVEL PROPERTY STRINGS "off" "fatal" "error" "warn" "info" "debug" "trace" "")

# performance profiler options
SET(ramses-sdk_BUILD_PERFORMANCE_PROFILER_SUPPORT "OFF" CACHE STRING "Include performance profiler support")
SET_PROPERTY(CACHE ramses-sdk_BUILD_PERFORMANCE_PROFILER_SUPPORT PROPERTY STRINGS OFF LOG_OUTPUT VTUNE)
SET(ramses-sdk_BUILD_PERFORMANCE_PROFILER_VTUNE_INSTALL_DIR "" CACHE STRING "Install directory of Intel VTune")

#install options
OPTION(ramses-sdk_INSTALL_STATIC_LIBS "Install static libraries: ON, OFF" ON)

# enable default ACME2 plugins
OPTION(ACME_ENABLE_PLUGIN_PROJECT_CONFIG_HEADER "ACME2 plugin for build config header generation" ON)

include(cmake/ramses/checkForOverrideGitCommitInfo.cmake)

SET(RAMSES_VERSION_MAJOR 0)
SET(RAMSES_VERSION_MINOR 0)
SET(RAMSES_VERSION_PATCH 0)
SET(RAMSES_VERSION_POSTFIX "-devMaster")

SET(RAMSES_VERSION "${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}${RAMSES_VERSION_POSTFIX}")
IF (BUILD_ENV_VERSION_INFO_FULL)
    string(REPLACE "\\" "\\\\" BUILD_ENV_VERSION_INFO_FULL_ESCAPED ${BUILD_ENV_VERSION_INFO_FULL})
    string(REPLACE "\"" "\\\"" BUILD_ENV_VERSION_INFO_FULL_ESCAPED ${BUILD_ENV_VERSION_INFO_FULL_ESCAPED})
endif (BUILD_ENV_VERSION_INFO_FULL)

MESSAGE("RAMSES_VERSION=${RAMSES_VERSION}")

SET(ramses-sdk_VERSION_DIR "ramses-${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}")
IF (ramses-sdk_USE_VERSIONED_BINARIES)
    SET(ramses-sdk_BINARY_DIR_BASE "bin/${ramses-sdk_VERSION_DIR}")
ELSE()
    SET(ramses-sdk_BINARY_DIR_BASE "bin")
ENDIF()

include(cmake/ramses/rendererModulePerConfig.cmake)

ADD_DEFINITIONS(-DRAMSES_LINK_STATIC)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_DEBUG_POSTFIX "") # no debug suffix in this project

set(ramses-shared-lib-MIXIN CACHE INTERNAL "")

ACME2_PROJECT(
    #==========================================================================
    # project metadata
    #==========================================================================
    NAME                        ramses-sdk
    VERSION_MAJOR               ${RAMSES_VERSION_MAJOR}
    VERSION_MINOR               ${RAMSES_VERSION_MINOR}
    VERSION_PATCH               ${RAMSES_VERSION_PATCH}
    VERSION_STRING              ${RAMSES_VERSION}
    DESCRIPTION                 "RAMSES - Rendering Architecture for Multi-Screen EnvironmentS"
    URL                         "https://github.com/GENIVI/ramses"
    ENABLE_INSTALL              OFF

    #==========================================================================
    # Test partitioned into following categories:
    # UNITTEST                     = default - generally, test for classes (Units)
    # RNDSANDWICHTEST              = sandwich tests, which need a rendering backend (can draw pixels)
    # RNDSANDWICHTEST_SWRAST       = subset of RNDSANDWICHTEST which can be executed on a sw. rasterizer (Mesa in docker)
    # RNDSANDWICHTEST_VALGRINDGATE = subset of RNDSANDWICHTEST which are executed with valgrind in the gate
    #==========================================================================
    ALLOWED_TEST_SUFFIXES       UNITTEST RNDSANDWICHTEST RNDSANDWICHTEST_SWRAST RNDSANDWICHTEST_VALGRINDGATE

    #==========================================================================
    # install path setup
    #==========================================================================
    INSTALL_HEADER              "include/${ramses-sdk_VERSION_DIR}"
    INSTALL_BINARY              "${ramses-sdk_BINARY_DIR_BASE}"
    INSTALL_RESOURCE            "${ramses-sdk_BINARY_DIR_BASE}/res"
    INSTALL_STATIC_LIB          "lib/${ramses-sdk_VERSION_DIR}"

    #==========================================================================
    # documentation files, will be installed and packaged
    #==========================================================================
    FILES_DOCUMENTATION         README.md
                                LICENSE.txt
    ENABLE_INSTALL_DOCUMENTATION ON

    #==========================================================================
    # content
    #==========================================================================
    CONTENT                     ON   external
                                ON   framework

                                ON   client/ramses-client-api
                                ON   client/ramses-client
                                ON   client/ramses-text-api
                                ON   client/ramses-text

                                ON   ramses-daemon

                                ON   renderer/ramses-renderer-api
                                ON   renderer/RendererLib

                                # Auxilary platform components
                                AUTO renderer/Platform/UnixUtilities
                                AUTO renderer/Platform/WaylandUtilities
                                AUTO renderer/Platform/SystemCompositorController_Wayland_IVI

                                # Window Components
                                AUTO renderer/Platform/Window_Windows
                                AUTO renderer/Platform/Window_X11
                                AUTO renderer/Platform/Window_Wayland

                                # Context Components
                                AUTO renderer/Platform/Context_WGL
                                AUTO renderer/Platform/Context_EGL

                                # Device Components
                                AUTO renderer/Platform/Device_GL

                                # Other Components
                                # TODO Mohamed: those components should be possible to move before window components (have less deps)
                                AUTO renderer/Platform/WindowEventsPollingManager_Wayland
                                AUTO renderer/Platform/TextureUploadingAdapter_Wayland
                                AUTO renderer/Platform/EmbeddedCompositor_Dummy
                                AUTO renderer/Platform/EmbeddedCompositor_Wayland
                                AUTO renderer/Platform/Logger_Wayland
                                AUTO renderer/Platform/Window_Wayland_Test
                                AUTO renderer/Platform/Window_Wayland_IVI
                                AUTO renderer/Platform/Window_Wayland_Shell
                                AUTO renderer/Platform/Window_Android

                                # Surface Components
                                AUTO renderer/Platform/Surface_Windows_WGL
                                AUTO renderer/Platform/Surface_X11_EGL
                                AUTO renderer/Platform/Surface_Wayland_EGL
                                AUTO renderer/Platform/Surface_EGL_Offscreen
                                AUTO renderer/Platform/Surface_Android_EGL
                                AUTO renderer/Platform/PlatformFactory_Wayland_EGL
                                AUTO renderer/Platform/PlatformFactory_Wayland_IVI_EGL
                                AUTO renderer/Platform/PlatformFactory_Wayland_Shell_EGL

                                # Platforms
                                AUTO renderer/Platform/Platform_Windows_WGL_4_2_core
                                AUTO renderer/Platform/Platform_Windows_WGL_4_5
                                AUTO renderer/Platform/Platform_Windows_WGL_ES_3_0
                                AUTO renderer/Platform/Platform_Wayland_IVI_EGL_ES_3_0
                                AUTO renderer/Platform/Platform_Wayland_Shell_EGL_ES_3_0
                                AUTO renderer/Platform/Platform_X11_EGL_ES_3_0
                                AUTO renderer/Platform/Platform_Android_EGL_ES_3_0

                                AUTO renderer/DisplayManager

                                AUTO ramses-shared-lib

                                AUTO renderer/ramses-renderer-main

                                AUTO examples

                                AUTO integration

                                AUTO utils


                                AUTO demo

                                AUTO external/wayland-ivi-example-client
)

#==========================================================================
# target: code style checker
#==========================================================================
include(cmake/ramses/addCheckerTargets.cmake)

#==========================================================================
# build documentation
#==========================================================================
ADD_SUBDIRECTORY(doc)

#==========================================================================
# generate CMake config files
#==========================================================================
INCLUDE(CMakePackageConfigHelpers)

SET(configmodules) # initially empty

IF (ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB)
    LIST(APPEND configmodules ramses-shared-lib-client-only)
ELSE()
    LIST(APPEND configmodules ramses-shared-lib)
ENDIF()


IF (ramses-sdk_INSTALL_STATIC_LIBS)
    LIST(APPEND configmodules ramses-static-lib)
ENDIF()


FOREACH(configmodule ${configmodules})

    SET(configmodule-VERSION_DIR "${configmodule}-${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}")

    # install paths for find/config script must differ on windows and linux because of different search
    # rules used by find_package. See https://cmake.org/cmake/help/git-master/command/find_package.html
    # for details
    IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        SET(ramses-sdk_FIND_SCRIPT_INSTALL_DIR "lib/${configmodule-VERSION_DIR}/cmake")
    ELSE()
        SET(ramses-sdk_FIND_SCRIPT_INSTALL_DIR "lib/cmake/${configmodule-VERSION_DIR}")
    ENDIF()

    # preapare vtune profiler options to forward to config script
    SET(ramses-sdk_VTUNE_LIBRARY_NAME "")
    IF(configmodule MATCHES "static" AND ramses-sdk_BUILD_PERFORMANCE_PROFILER_SUPPORT MATCHES VTUNE)
        FIND_PACKAGE(VTune QUIET)
        IF (VTune_FOUND)
            IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
                SET(ramses-sdk_VTUNE_LIBRARY_NAME "libittnotify.a")
            ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
                SET(ramses-sdk_VTUNE_LIBRARY_NAME "libittnotify")
            ENDIF()
        ENDIF()
    ENDIF()

    # generate CMake config file (use ${configmodule}Template.cmake.in as base)
    CONFIGURE_PACKAGE_CONFIG_FILE(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/${configmodule}Template.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${configmodule}Config.cmake"
        INSTALL_DESTINATION "${ramses-sdk_FIND_SCRIPT_INSTALL_DIR}"
        PATH_VARS PROJECT_INSTALL_HEADER PROJECT_INSTALL_STATIC_LIB PROJECT_INSTALL_SHARED_LIB ramses-sdk_VTUNE_LIBRARY_NAME
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )
    INSTALL(
        FILES       ${CMAKE_CURRENT_BINARY_DIR}/${configmodule}Config.cmake
        DESTINATION ${ramses-sdk_FIND_SCRIPT_INSTALL_DIR}
    )
    ACME_INFO("G ${configmodule}Config.cmake")

    # generate CMake version file
    WRITE_BASIC_PACKAGE_VERSION_FILE("${CMAKE_CURRENT_BINARY_DIR}/${configmodule}ConfigVersion.cmake"
        VERSION ${RAMSES_VERSION}
        COMPATIBILITY SameMajorVersion)
    INSTALL(
        FILES       ${CMAKE_CURRENT_BINARY_DIR}/${configmodule}ConfigVersion.cmake
        DESTINATION ${ramses-sdk_FIND_SCRIPT_INSTALL_DIR}
    )
    ACME_INFO("G ${configmodule}ConfigVersion.cmake")

ENDFOREACH()

# produce a version file
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ramses-version")
# case when using a ascent source package already containing the originally configured file
    INSTALL(
            FILES       "${CMAKE_CURRENT_SOURCE_DIR}/ramses-version"
            DESTINATION ${PROJECT_INSTALL_HEADER}
        )
ELSE()
    SET(RAMSES_COMMIT_COUNT "${GIT_COMMIT_COUNT}")
    SET(RAMSES_GIT_HASH "${GIT_COMMIT_HASH}")
    SET(RAMSES_BRANCH "${GIT_COMMIT_BRANCH}")
    STRING(TIMESTAMP RAMSES_BUILD_TIME UTC)
    CONFIGURE_FILE(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/ramses-version.in"
            "${CMAKE_CURRENT_BINARY_DIR}/ramses-version"
            @ONLY
        )
    INSTALL(
            FILES       "${CMAKE_CURRENT_BINARY_DIR}/ramses-version"
            DESTINATION ${PROJECT_INSTALL_HEADER}
        )
ENDIF()
