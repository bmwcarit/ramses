#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(RAMSES_VERSION_MAJOR 0)
set(RAMSES_VERSION_MINOR 0)
set(RAMSES_VERSION_PATCH 0)
set(RAMSES_VERSION_POSTFIX "-devMaster")
set(RAMSES_VERSION "${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}${RAMSES_VERSION_POSTFIX}")

project(ramses-sdk
    VERSION         ${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}
    DESCRIPTION     "RAMSES - Rendering Architecture for Multi-Screen EnvironmentS"
    HOMEPAGE_URL    "https://ramses3d.org/"
    LANGUAGES       C CXX
)

# build options
option(ramses-sdk_WARNINGS_AS_ERRORS "If on, warnings are treated as errors during build." ON)
# TODO Building tests should be off when building ramses externally
option(ramses-sdk_BUILD_TESTS "Build ramses tests." ON)
option(ramses-sdk_BUILD_SMOKE_TESTS "Build smoke tests." ON) # TODO document how this interacts with the above option
option(ramses-sdk_BUILD_EXAMPLES "Build examples." ON)
option(ramses-sdk_BUILD_TOOLS "Build tool binaries." ON)
option(ramses-sdk_BUILD_DEMOS "Build demos." ON)
option(ramses-sdk_TEXT_SUPPORT "Enable/disable the ramses text API." ON)
option(ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB "Build client-only shared library." OFF)
option(ramses-sdk_BUILD_FULL_SHARED_LIB "Build per-renderer shared libraries." ON)
option(ramses-sdk_BUILD_DOCS "Build documentation (if Doxygen and Dot are found on the build host)." ON)
option(ramses-sdk_ENABLE_WAYLAND_IVI "Build a version of ramses renderer which uses wayland-ivi surfaces." ON)
option(ramses-sdk_ENABLE_WAYLAND_SHELL "Build a version of ramses renderer which uses wayland-shell surfaces." ON)
option(ramses-sdk_ENABLE_TCP_SUPPORT "Enable use of TCP communication." ON)
option(ramses-sdk_ENABLE_DLT "Enable DLT logging support." ON)
option(ramses-sdk_USE_LINUX_DEV_PTP "Enable support for synchronized PTP time on Linux." OFF)
option(ramses-sdk_BUILD_WITH_LTO "Build all targets with link time optimization enabled (supported only on some platforms)." OFF)
option(ramses-sdk_ENABLE_COVERAGE "Build with code coverage enabled (gcc: gcov, clang: source based coverage)." OFF)
option(ramses-sdk_USE_CCACHE "Enable ccache when configured and available" OFF)

set(ramses-sdk_CPP_VERSION "17" CACHE STRING "Set used c++ version, must be 17 or 20")

# find options
option(ramses-sdk_ALLOW_PLATFORM_FREETYPE "Enable to search for platform provided freetype and harfbuzz." ON)
option(ramses-sdk_ALLOW_CUSTOM_FREETYPE "Allow usage of custom freetype and harfbuzz if platform provided one was not found." ON)

set(ramses-sdk_CONSOLE_LOGLEVEL "" CACHE STRING "Console log level.")
set_property(CACHE ramses-sdk_CONSOLE_LOGLEVEL PROPERTY STRINGS "off" "fatal" "error" "warn" "info" "debug" "trace" "")

set(ramses-sdk_FOLDER_PREFIX "" CACHE STRING "Optional folder prefix for targets in visual studio.")

set(ramses-sdk_ENABLE_SANITIZER "" CACHE STRING "Enable build with a clang sanitizer.")
set_property(CACHE ramses-sdk_ENABLE_SANITIZER PROPERTY STRINGS "" "ubsan" "tsan" "asan" "asan+ubsan")

set(ramses-sdk_FOLDER_PREFIX "" CACHE STRING "Set a custom prefix for target folders in Visual Studio. If not set, will be set based on project's relative path.")
set(ramses-sdk_USE_LINKER_OVERWRITE "" CACHE STRING "Specify used linker (gcc/clang only).")
set(ramses-sdk_CPACK_GENERATOR "TGZ" CACHE STRING "CPack package type (default: TGZ).")
set(ramses-sdk_VERSION "${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}" CACHE STRING "Ramses version" FORCE)

message(STATUS "RAMSES_VERSION=${RAMSES_VERSION}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/ramses/setCmakePolicies.cmake NO_POLICY_SCOPE)  # we want to propagate policy settings out!
include(cmake/ramses/platformDetection.cmake)
include(cmake/ramses/platformConfig.cmake)
include(cmake/ramses/getGitInformation.cmake)
include(cmake/ramses/externaltools.cmake)
include(cmake/ramses/folderize.cmake)
include(cmake/ramses/testConfig.cmake)
include(cmake/ramses/buildConfig.cmake)
include(cmake/ramses/createTarget.cmake)
include(cmake/ramses/addSubdirectory.cmake)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

# if no build configuration is defined, use build config 'Release'
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not defined. Using default build type 'Release'.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug, Release." FORCE)
else ()
    # Case-insensitive comparison required, build type is not case-sensitive
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER_CASE)
    if(NOT("${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "debug" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "release" OR
           "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "relwithdebinfo" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "minsizerel"))
        message(FATAL_ERROR "Build type set to unsupported type ${CMAKE_BUILD_TYPE}.")
    endif ()
endif ()

message(STATUS "Ramses Build Config: SystemName ${CMAKE_SYSTEM_NAME}, SystemVersion ${CMAKE_SYSTEM_VERSION}, CompilerID ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, BuildType ${CMAKE_BUILD_TYPE}, TargetBitness ${TARGET_BITNESS} c++${ramses-sdk_CPP_VERSION}")

set(ramses-sdk_VERSION_DIR "ramses-${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}")
if (ramses-sdk_USE_VERSIONED_BINARIES)
    set(ramses-sdk_BINARY_DIR_BASE "bin/${ramses-sdk_VERSION_DIR}")
else()
    set(ramses-sdk_BINARY_DIR_BASE "bin")
endif()

if((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "iOS") AND ramses-sdk_TEXT_SUPPORT)
    message(WARNING "Text rendering is not supported on iOS and MacOS currently! Disabling it!")
    set(ramses-sdk_TEXT_SUPPORT OFF CACHE STRING "Enable/disable the ramses text API." FORCE)
endif()

# TODO this should not be here
add_definitions(-DRAMSES_LINK_STATIC)

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
include(cmake/ramses/makeTestFromTarget.cmake)
include(cmake/ramses/resourceCopy.cmake)

set(RAMSES_INSTALL_HEADERS_PATH     "include/${ramses-sdk_VERSION_DIR}")
set(RAMSES_INSTALL_RUNTIME_PATH     "${ramses-sdk_BINARY_DIR_BASE}")
set(RAMSES_INSTALL_LIBRARY_PATH     lib)
set(RAMSES_INSTALL_RESOURCES_PATH   "${ramses-sdk_BINARY_DIR_BASE}/res")

createBuildConfig()

addSubdirectory(MODE AUTO PATH external)
addSubdirectory(MODE ON PATH framework)

addSubdirectory(MODE ON PATH client/ramses-client)

addSubdirectory(MODE ON PATH ramses-daemon)

addSubdirectory(MODE ON PATH renderer/RendererLib)

addSubdirectory(MODE AUTO PATH renderer/Platform/WaylandEGLExtensionProcs)
addSubdirectory(MODE AUTO PATH renderer/Platform/WaylandUtilities)
addSubdirectory(MODE AUTO PATH renderer/Platform/SystemCompositorController_Wayland_IVI)

addSubdirectory(MODE AUTO PATH renderer/Platform/Window_Windows)
addSubdirectory(MODE AUTO PATH renderer/Platform/Window_Wayland)

addSubdirectory(MODE AUTO PATH renderer/Platform/Context_WGL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Context_EGL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Device_GL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Device_EGL_Extension)

# TODO Mohamed: those components should be possible to move before window components (have less deps)
addSubdirectory(MODE AUTO PATH renderer/Platform/EmbeddedCompositor_Wayland)
addSubdirectory(MODE AUTO PATH renderer/Platform/Window_Wayland_Test)
addSubdirectory(MODE AUTO PATH renderer/Platform/Window_Wayland_IVI)
addSubdirectory(MODE AUTO PATH renderer/Platform/Window_Wayland_Shell)

addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Windows_WGL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Windows_WGL_4_2_core)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Windows_WGL_4_5)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Windows_WGL_ES_3_0)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_EGL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Wayland_EGL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Wayland_IVI_EGL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Wayland_Shell_EGL)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_X11)
addSubdirectory(MODE AUTO PATH renderer/Platform/Platform_Android)

addSubdirectory(MODE AUTO PATH ramses-shared-lib)
addSubdirectory(MODE AUTO PATH renderer/ramses-renderer-main)
addSubdirectory(MODE AUTO PATH examples)
addSubdirectory(MODE AUTO PATH integration)
addSubdirectory(MODE AUTO PATH utils)
addSubdirectory(MODE AUTO PATH demo)

include(cmake/ramses/createPackage.cmake)
include(cmake/ramses/addCheckerTargets.cmake)

if (ramses-sdk_BUILD_DOCS)
    add_subdirectory(doc)
endif()

install(FILES       README.md CHANGELOG.txt LICENSE.txt
        DESTINATION share/doc/ramses-sdk-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        COMPONENT   ramses-sdk-${PROJECT_VERSION}
    )

include(CMakePackageConfigHelpers)

set(configmodules) # initially empty
if (ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB)
    list(APPEND configmodules ramses-shared-lib-client-only)
endif()
if (ramses-sdk_BUILD_FULL_SHARED_LIB)
    # only install full shared lib cmake find script if built at least one
    foreach (PLATFORM_NAME ${PLATFORM_LIST})
        if (TARGET ramses-shared-lib-${PLATFORM_NAME})
            list(APPEND configmodules ramses-shared-lib)
            break()
        endif()
    endforeach()
endif()

foreach(configmodule ${configmodules})
    # install paths for find/config script must differ on windows and linux because of different search
    # rules used by find_package. See https://cmake.org/cmake/help/git-master/command/find_package.html
    # for details
    set(configmodule-VERSION_DIR "${configmodule}-${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}")
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(ramses-sdk_FIND_SCRIPT_INSTALL_DIR "lib/${configmodule-VERSION_DIR}/cmake")
    else()
        set(ramses-sdk_FIND_SCRIPT_INSTALL_DIR "lib/cmake/${configmodule-VERSION_DIR}")
    endif()

    # generate CMake config file (use ${configmodule}Template.cmake.in as base)
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/${configmodule}Template.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${configmodule}Config.cmake"
        INSTALL_DESTINATION "${ramses-sdk_FIND_SCRIPT_INSTALL_DIR}"
        PATH_VARS RAMSES_INSTALL_HEADERS_PATH RAMSES_INSTALL_LIBRARY_PATH RAMSES_INSTALL_RUNTIME_PATH
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )
    install(
        FILES       ${CMAKE_CURRENT_BINARY_DIR}/${configmodule}Config.cmake
        DESTINATION ${ramses-sdk_FIND_SCRIPT_INSTALL_DIR}
    )
    message(STATUS "G ${configmodule}Config.cmake")

    # generate CMake version file
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${configmodule}ConfigVersion.cmake"
        VERSION ${RAMSES_VERSION}
        COMPATIBILITY SameMajorVersion)
    install(
        FILES       ${CMAKE_CURRENT_BINARY_DIR}/${configmodule}ConfigVersion.cmake
        DESTINATION ${ramses-sdk_FIND_SCRIPT_INSTALL_DIR}
    )
    message(STATUS "G ${configmodule}ConfigVersion.cmake")

endforeach()

# produce a version file
string(TIMESTAMP RAMSES_BUILD_TIME UTC)
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/ramses-version.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ramses-version"
        @ONLY
    )
install(
        FILES       "${CMAKE_CURRENT_BINARY_DIR}/ramses-version"
        DESTINATION ${RAMSES_INSTALL_HEADERS_PATH}
    )
