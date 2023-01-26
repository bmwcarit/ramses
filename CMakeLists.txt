#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(RAMSES_VERSION_MAJOR 27)
SET(RAMSES_VERSION_MINOR 0)
SET(RAMSES_VERSION_PATCH 129)
SET(RAMSES_VERSION_POSTFIX "")

CMAKE_POLICY(SET CMP0048 NEW)
PROJECT(ramses-sdk VERSION ${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH})


# TODO(tobias) must be here because used inside platformConfig, rework order of all options to solve properly
OPTION(ramses-sdk_WARNINGS_AS_ERRORS "Enable warnings as errors during build" OFF)
set(ramses-sdk_CPP_VERSION "14" CACHE STRING "Set used c++ version, must be 14, 17 or 20")

include(cmake/ramses/setCmakePolicies.cmake NO_POLICY_SCOPE)  # we want to propagate policy settings out!
include(cmake/ramses/platformDetection.cmake)
include(cmake/ramses/platformConfig.cmake)
include(cmake/ramses/getGitInformation.cmake)
include(cmake/ramses/externaltools.cmake)
include(cmake/ramses/tools.cmake)
include(cmake/ramses/testConfig.cmake)
include(cmake/ramses/buildConfig.cmake)

SET(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

INCLUDE(external/acme2/acme2.cmake)

# if no build configuration is defined, use build config 'Release'
IF (NOT CMAKE_BUILD_TYPE)
    ACME_INFO("Build type not defined. Using default build type 'Release'.")
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug, Release." FORCE)
ELSE ()
    # Case-insensitive comparison required, built type is not case-sensitive
    STRING(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER_CASE)
    IF(NOT("${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "debug" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "release" OR
           "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "relwithdebinfo" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "minsizerel"))
        ACME_ERROR("Build type set to unsupported type ${CMAKE_BUILD_TYPE}.")
    ENDIF ()
ENDIF (NOT CMAKE_BUILD_TYPE)

MESSAGE(STATUS "Ramses Build Config: SystemName ${CMAKE_SYSTEM_NAME}, SystemVersion ${CMAKE_SYSTEM_VERSION}, CompilerID ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, TargetOS ${TARGET_OS}, BuildType ${CMAKE_BUILD_TYPE}, TargetBitness ${TARGET_BITNESS} c++${ramses-sdk_CPP_VERSION}")

# find options
OPTION(ramses-sdk_ALLOW_PLATFORM_FREETYPE "Enable to search for platform provided freetype and harfbuzz" ON)
OPTION(ramses-sdk_ALLOW_CUSTOM_FREETYPE "Allow usage of custom freetype and harfbuzz if platform provided one  was not found" ON)

# build options
OPTION(ramses-sdk_BUILD_EXAMPLES "Build Example targets: ON, OFF" ON)
OPTION(ramses-sdk_BUILD_TOOLS "Build tools: ON, OFF" ON)
OPTION(ramses-sdk_BUILD_SMOKE_TESTS "Build smoke test targets: ON, OFF" ON)
OPTION(ramses-sdk_BUILD_DEMOS "Build demo targets: ON, OFF" ON)
OPTION(ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB "Build client only shared library" OFF)
OPTION(ramses-sdk_BUILD_FULL_SHARED_LIB "Build per renderer shared libraries" ON)
OPTION(ramses-sdk_BUILD_DOCS "Build documentation if Doxygen is found" ON)
OPTION(ramses-sdk_ENABLE_WAYLAND_IVI "Build a version of ramses renderer which uses wayland ivi surfaces" ON)
OPTION(ramses-sdk_ENABLE_WAYLAND_SHELL "Build a version of ramses renderer which uses wayland shell surfaces" ON)
OPTION(ramses-sdk_ENABLE_TCP_SUPPORT "Enable use of TCP communication" ON)
OPTION(ramses-sdk_ENABLE_FALLBACK_FAKE_COMMUNICATION_SYSTEM "Allow fake communication system as last resort" ON)
OPTION(ramses-sdk_ENABLE_DLT "Enable dlt support" ON)
OPTION(ramses-sdk_USE_LINUX_DEV_PTP "Enable support for synchronized ptp time on linux" OFF)
OPTION(ramses-sdk_BUILD_WITH_LTO "Build all targets with link time optimization enabled (not supported on all platforms)" OFF)
option(ramses-sdk_ENABLE_COVERAGE "Build withcode coverage enabled (gcc: gcov, clang: source based coverage)" OFF)

SET(ramses-sdk_CONSOLE_LOGLEVEL "" CACHE STRING "Console log level.")
SET_PROPERTY(CACHE ramses-sdk_CONSOLE_LOGLEVEL PROPERTY STRINGS "off" "fatal" "error" "warn" "info" "debug" "trace" "")

SET(ramses-sdk_ENABLE_SANITIZER "" CACHE STRING "Enable build with a clang sanitizer.")
SET_PROPERTY(CACHE ramses-sdk_ENABLE_SANITIZER PROPERTY STRINGS "" "ubsan" "tsan" "asan" "asan+ubsan")

set(ramses-sdk_FOLDER_PREFIX "" CACHE STRING "Set a custom prefix for target folders in Visual Studio. If not set, will be set based on project's relative path")

set(ramses-sdk_USE_LINKER_OVERWRITE "" CACHE STRING "Specify used linker (gcc/clang only)")

set(ramses-sdk_CPACK_GENERATOR "TGZ" CACHE STRING "CPack package type (default: TGZ)")

SET(RAMSES_VERSION "${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}${RAMSES_VERSION_POSTFIX}")
set(ramses-sdk_VERSION "${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}" CACHE STRING "Ramses version" FORCE)

MESSAGE(STATUS "RAMSES_VERSION=${RAMSES_VERSION}")

SET(ramses-sdk_VERSION_DIR "ramses-${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}")
IF (ramses-sdk_USE_VERSIONED_BINARIES)
    SET(ramses-sdk_BINARY_DIR_BASE "bin/${ramses-sdk_VERSION_DIR}")
ELSE()
    SET(ramses-sdk_BINARY_DIR_BASE "bin")
ENDIF()

include(cmake/ramses/rendererModulePerConfig.cmake)

ADD_DEFINITIONS(-DRAMSES_LINK_STATIC)


if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    message(STATUS "Redirect ramses library output to ${CMAKE_CURRENT_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    message(STATUS "Redirect ramses runtime output to ${CMAKE_CURRENT_BINARY_DIR}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()
set(CMAKE_DEBUG_POSTFIX "") # no debug suffix in this project

set(ramses-shared-lib-MIXIN CACHE INTERNAL "")
set(ramses-shared-lib-renderer-MIXIN CACHE INTERNAL "")

include(cmake/ramses/platformTargets.cmake)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    SET(ENV{PKG_CONFIG_PATH} "")
    SET(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")
    SET(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
ENDIF()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ramses/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(AndroidSDK)

################################################
################     Ramses   ##################
################################################
set(ramses-sdk_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_IVI_TEST_APPS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_DEMO_ECM OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_SMOKE_TESTS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_ENABLE_DLT OFF CACHE BOOL "" FORCE)
set(ramses-sdk_FORCE_OFF_SOMEIP_HU ON CACHE BOOL "" FORCE)
set(ramses-sdk_FORCE_OFF_SOMEIP_IC ON CACHE BOOL "" FORCE)
set(ramses-sdk_ENABLE_TCP_SUPPORT OFF CACHE BOOL "" FORCE)

# Disable some things which don't have a CMake option
set(ramses-sdk_ramses-daemon OFF CACHE BOOL "" FORCE)

set(RAMSES_TARGET "ramses-shared-lib-android-egl-es-3-0")
set(ramses-sdk_BUILD_FULL_SHARED_LIB ON CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB OFF CACHE BOOL "" FORCE)

################################################
###############     Android   ##################
################################################
#add_subdirectory(android)



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
    URL                         "https://github.com/bmwcarit/ramses"
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
    # content
    #==========================================================================
    CONTENT                     AUTO external
                                ON   framework

                                ON   client/ramses-client

                                ON   ramses-daemon

                                ON   renderer/RendererLib

                                # Auxilary platform components
                                AUTO renderer/Platform/WaylandEGLExtensionProcs
                                AUTO renderer/Platform/WaylandUtilities
                                AUTO renderer/Platform/SystemCompositorController_Wayland_IVI

                                # Window Components
                                AUTO renderer/Platform/Window_Windows
                                AUTO renderer/Platform/Window_Wayland

                                # Context Components
                                AUTO renderer/Platform/Context_WGL
                                AUTO renderer/Platform/Context_EGL

                                # Device Components
                                AUTO renderer/Platform/Device_GL
                                AUTO renderer/Platform/Device_EGL_Extension

                                # Other Components
                                # TODO Mohamed: those components should be possible to move before window components (have less deps)
                                AUTO renderer/Platform/EmbeddedCompositor_Wayland
                                AUTO renderer/Platform/Window_Wayland_Test
                                AUTO renderer/Platform/Window_Wayland_IVI
                                AUTO renderer/Platform/Window_Wayland_Shell

                                # Platforms
                                AUTO renderer/Platform/Platform_Windows_WGL
                                AUTO renderer/Platform/Platform_Windows_WGL_4_2_core
                                AUTO renderer/Platform/Platform_Windows_WGL_4_5
                                AUTO renderer/Platform/Platform_Windows_WGL_ES_3_0
                                AUTO renderer/Platform/Platform_EGL
                                AUTO renderer/Platform/Platform_Wayland_EGL
                                AUTO renderer/Platform/Platform_Wayland_IVI_EGL
                                AUTO renderer/Platform/Platform_Wayland_Shell_EGL
                                AUTO renderer/Platform/Platform_X11
                                AUTO renderer/Platform/Platform_Integrity_RGL
                                AUTO renderer/Platform/Platform_Android

                                AUTO ramses-shared-lib

                                AUTO renderer/ramses-renderer-main

                                AUTO examples

                                AUTO integration

                                AUTO utils


                                AUTO demo
)

#==========================================================================
# target: code style checker
#==========================================================================
include(cmake/ramses/addCheckerTargets.cmake)

#==========================================================================
# build and install documentation
#==========================================================================
if (ramses-sdk_BUILD_DOCS)
    add_subdirectory(doc)
endif()

INSTALL(FILES       README.md CHANGELOG.txt LICENSE.txt
        DESTINATION share/doc/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        COMPONENT   ${PROJECT_NAME}-${PROJECT_VERSION}
    )

#==========================================================================
# generate CMake config files
#==========================================================================
INCLUDE(CMakePackageConfigHelpers)

SET(configmodules) # initially empty

IF (ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB)
    LIST(APPEND configmodules ramses-shared-lib-client-only)
ENDIF()
IF (ramses-sdk_BUILD_FULL_SHARED_LIB)
    # only install full shared lib cmake find script if built at least one
    foreach (PLATFORM_NAME ${RENDERER_CONFIG_LIST})
        if (TARGET ramses-shared-lib-${PLATFORM_NAME})
            LIST(APPEND configmodules ramses-shared-lib)
            break()
        endif()
    endforeach()
ENDIF()

FOREACH(configmodule ${configmodules})

    # install paths for find/config script must differ on windows and linux because of different search
    # rules used by find_package. See https://cmake.org/cmake/help/git-master/command/find_package.html
    # for details
    SET(configmodule-VERSION_DIR "${configmodule}-${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}")
    IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        SET(ramses-sdk_FIND_SCRIPT_INSTALL_DIR "lib/${configmodule-VERSION_DIR}/cmake")
    ELSE()
        SET(ramses-sdk_FIND_SCRIPT_INSTALL_DIR "lib/cmake/${configmodule-VERSION_DIR}")
    ENDIF()

    # generate CMake config file (use ${configmodule}Template.cmake.in as base)
    CONFIGURE_PACKAGE_CONFIG_FILE(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/${configmodule}Template.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${configmodule}Config.cmake"
        INSTALL_DESTINATION "${ramses-sdk_FIND_SCRIPT_INSTALL_DIR}"
        PATH_VARS PROJECT_INSTALL_HEADER PROJECT_INSTALL_SHARED_LIB PROJECT_INSTALL_BINARY
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
