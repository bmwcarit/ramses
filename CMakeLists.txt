#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

set(RAMSES_VERSION_MAJOR 28)
set(RAMSES_VERSION_MINOR 0)
set(RAMSES_VERSION_PATCH 0)
set(RAMSES_VERSION_POSTFIX "-rc5")
set(RAMSES_VERSION "${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}${RAMSES_VERSION_POSTFIX}")

project(ramses-sdk
    VERSION         ${RAMSES_VERSION_MAJOR}.${RAMSES_VERSION_MINOR}.${RAMSES_VERSION_PATCH}
    DESCRIPTION     "RAMSES - Rendering Architecture for Multi-Screen EnvironmentS"
    HOMEPAGE_URL    "https://ramses3d.org/"
    LANGUAGES       C CXX
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(RAMSES_TOPLEVEL ON)
else()
    set(RAMSES_TOPLEVEL OFF)
endif()


# build options
set(ramses-sdk_CPP_VERSION "17" CACHE STRING            "Set used c++ version, must be 17 or 20")
option(ramses-sdk_WARNINGS_AS_ERRORS                    "If on, warnings are treated as errors during build." ON)
# TODO let all ramses components which are getting installed to respect this option
option(ramses-sdk_ENABLE_INSTALL                        "Enable/disable installation of Ramses" ON)
option(ramses-sdk_BUILD_WITH_LTO                        "Build all targets with link time optimization enabled (supported only on some platforms)." OFF)

# window type configuration. Several window types can be enabled in same build
option(ramses-sdk_ENABLE_DEFAULT_WINDOW_TYPE            "Enable a default window type if none explicitly enabled" ON)
option(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS            "Enable building for Windows window" OFF)
option(ramses-sdk_ENABLE_WINDOW_TYPE_X11                "Enable building for X11 window" OFF)
option(ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID            "Enable building for Android window" OFF)
option(ramses-sdk_ENABLE_WINDOW_TYPE_IOS                "Enable building for iOS window" OFF)
option(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI        "Enable building for Wayland ivi window" OFF)
option(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL   "Enable building for Wayland wl_shell window" OFF)

# shared lib options
option(ramses-sdk_BUILD_FULL_SHARED_LIB                 "Build full shared libraries (with renderer)." ON)

# optional components
option(ramses-sdk_ENABLE_LOGIC                          "Enable ramses logic - a component for scripting and animation." ON)
option(ramses-sdk_TEXT_SUPPORT                          "Enable/disable the ramses text API." ON)
option(ramses-sdk_ENABLE_TCP_SUPPORT                    "Enable use of TCP communication." ON)
option(ramses-sdk_ENABLE_DLT                            "Enable DLT logging support." ON)

option(ramses-sdk_BUILD_EXAMPLES                        "Build examples." ${RAMSES_TOPLEVEL})
option(ramses-sdk_BUILD_DEMOS                           "Build demos." ${RAMSES_TOPLEVEL})
option(ramses-sdk_BUILD_TOOLS                           "Build tool binaries." ${RAMSES_TOPLEVEL})
option(ramses-sdk_BUILD_TESTS                           "Build ramses tests." ${RAMSES_TOPLEVEL})

option(ramses-sdk_FORCE_BUILD_DOCS                      "Build documentation, report error and abort if Doxygen and Dot are found on the system." OFF)
option(ramses-sdk_USE_LINUX_DEV_PTP                     "Enable support for synchronized PTP time on Linux." OFF)
option(ramses-sdk_ENABLE_COVERAGE                       "Build with code coverage enabled (gcc: gcov, clang: source based coverage)." OFF)
option(ramses-sdk_USE_CCACHE                            "Enable ccache when configured and available" OFF)
option(ramses-sdk_ENABLE_FLATBUFFERS_GENERATION         "Enable generation of flatbuffers code" ${RAMSES_TOPLEVEL})
option(ramses-sdk_USE_IMAGEMAGICK                       "Enable tests depending on image magick compare" OFF)

# find options
option(ramses-sdk_ALLOW_PLATFORM_GLM                    "Enable to search for platform provided OpenGL Math library (glm)." ON)
option(ramses-sdk_ALLOW_PLATFORM_FREETYPE               "Enable to search for platform provided freetype and harfbuzz." ON)
option(ramses-sdk_ALLOW_CUSTOM_FREETYPE                 "Allow usage of custom freetype and harfbuzz if platform provided one was not found." ON)
option(ramses-sdk_ALLOW_PLATFORM_LZ4                    "Enable to search for platform provided lz4" ON)
option(ramses-sdk_USE_PLATFORM_LUAJIT                   "Uses platform provided luajit instead of internal lua" OFF)

# other options
set(ramses-sdk_FOLDER_PREFIX "" CACHE STRING            "Optional folder prefix for targets in visual studio.")

set(ramses-sdk_ENABLE_SANITIZER "" CACHE STRING         "Enable build with a clang sanitizer.")
set_property(CACHE ramses-sdk_ENABLE_SANITIZER PROPERTY STRINGS "" "ubsan" "tsan" "asan" "asan+ubsan")

set(ramses-sdk_FOLDER_PREFIX "" CACHE STRING            "Set a custom prefix for target folders in Visual Studio. If not set, will be set based on project's relative path.")
set(ramses-sdk_USE_LINKER_OVERWRITE "" CACHE STRING     "Specify used linker (gcc/clang only).")
set(ramses-sdk_CPACK_GENERATOR "TGZ" CACHE STRING       "CPack package type (default: TGZ).")

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

if(ramses-sdk_ENABLE_FLATBUFFERS_GENERATION)
    include(cmake/ramses/flatbuffersGeneration.cmake)
endif()

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

# 32-bit builds are not supported
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "32-bit architectures not supported.")
endif()

if(CMAKE_BUILD_TYPE)
    # Case-insensitive comparison required, build type is not case-sensitive
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER_CASE)
    if(NOT("${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "debug" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "release" OR
           "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "relwithdebinfo" OR "${CMAKE_BUILD_TYPE_LOWER_CASE}" STREQUAL "minsizerel"))
        message(FATAL_ERROR "Build type set to unsupported type ${CMAKE_BUILD_TYPE}.")
    endif()
endif()

message(STATUS "Ramses Build Config: SystemName ${CMAKE_SYSTEM_NAME}, SystemVersion ${CMAKE_SYSTEM_VERSION}, CompilerID ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, BuildType ${CMAKE_BUILD_TYPE}, TargetBitness ${TARGET_BITNESS} c++${ramses-sdk_CPP_VERSION}")

if((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "iOS") AND ramses-sdk_TEXT_SUPPORT)
    message(WARNING "Text rendering is not supported on iOS and MacOS currently! Disabling it!")
    set(ramses-sdk_TEXT_SUPPORT OFF CACHE STRING "Enable/disable the ramses text API." FORCE)
endif()

IF (NOT DEFINED ramses-sdk_ENABLE_HARFBUZZ_LEGACY_SHAPING)
    IF ("${TARGET_OS}" STREQUAL "Integrity")
        SET(ramses-sdk_ENABLE_HARFBUZZ_LEGACY_SHAPING ON CACHE BOOL "Enable old shaping behavior for text blocks with special characters, e.g. for compatibility reasons")
    ENDIF()
ENDIF()

# check if valid configuration of window types in relation with shared libs
set(ANY_WINDOW_TYPE_ENABLED OFF)

if(    ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS
    OR ramses-sdk_ENABLE_WINDOW_TYPE_X11
    OR ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID
    OR ramses-sdk_ENABLE_WINDOW_TYPE_IOS
    OR ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI
    OR ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL)
        set(ANY_WINDOW_TYPE_ENABLED ON)
endif()

if(ANY_WINDOW_TYPE_ENABLED AND ramses-sdk_ENABLE_DEFAULT_WINDOW_TYPE)
    message(WARNING "Default window type setting will be ignored since a window type is enabled explicitly.")
endif()

#if no window type explicitly enabled, check if default window type is (implicitly) enabled
if(NOT ANY_WINDOW_TYPE_ENABLED)
    if(ramses-sdk_ENABLE_DEFAULT_WINDOW_TYPE)

        macro(setDefaultWindowType cacheVariable windowTypeName)
            message(STATUS "Setting ${windowTypeName} as default window type. Value set to ${cacheVariable} will be overridden")

            # This overrides the cache variable with a non-cache cmake variable of the same name to avoid over writing value
            # set by the user in gui or ccache if any
            set(${cacheVariable} ON)
            set(ANY_WINDOW_TYPE_ENABLED ON)
        endmacro()

        if(CMAKE_SYSTEM_NAME  STREQUAL "Windows")
            setDefaultWindowType(ramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS "Windows")
        elseif(CMAKE_SYSTEM_NAME  STREQUAL "Linux")
            setDefaultWindowType(ramses-sdk_ENABLE_WINDOW_TYPE_X11 "X11")
        elseif(CMAKE_SYSTEM_NAME  STREQUAL "Android")
            setDefaultWindowType(ramses-sdk_ENABLE_WINDOW_TYPE_ANDROID "Android")
        elseif(CMAKE_SYSTEM_NAME  STREQUAL "iOS")
            setDefaultWindowType(ramses-sdk_ENABLE_WINDOW_TYPE_IOS "iOS")
        else()
            message(WARNING "No default window type is known for the build operating system '${CMAKE_SYSTEM_NAME}':\n"
                             "* Either disable default window type by setting ramses-sdk_ENABLE_DEFAULT_WINDOW_TYPE=OFF\n"
                             "* or enable a window type explicitly by setting any of the options ramses-sdk_ENABLE_WINDOW_TYPE_* to ON")
        endif()
    endif()
endif()

if(NOT ANY_WINDOW_TYPE_ENABLED AND ramses-sdk_BUILD_FULL_SHARED_LIB)
    message(WARNING "Incorrect configuration. Trying to build full shared lib with renderer while no window type is enabled:\n"
                    "* If trying to build renderer enable at least one window type by setting any of the options ramses-sdk_ENABLE_WINDOW_TYPE_* to ON\n"
                    "* Otherwise disable building full shared lib (with renderer) by setting ramses-sdk_BUILD_FULL_SHARED_LIB=OFF\n"
                    "For current configuration ramses-sdk_BUILD_FULL_SHARED_LIB=ON will be ignored!")

    # Override cmake option (with normal/non-cached cmake variable)
    set(ramses-sdk_BUILD_FULL_SHARED_LIB OFF)
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

include(cmake/ramses/platformTargets.cmake)
include(cmake/ramses/makeTestFromTarget.cmake)
include(cmake/ramses/resourceCopy.cmake)

if(ramses-sdk_ENABLE_FLATBUFFERS_GENERATION)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
        message(FATAL_ERROR "The Android build systems don't allow modifying the source tree. Please set ramses-sdk_ENABLE_FLATBUFFERS_GENERATION to OFF.")
    endif()
endif()

set(RAMSES_INSTALL_HEADERS_PATH     include)
set(RAMSES_INSTALL_RUNTIME_PATH     bin)
set(RAMSES_INSTALL_LIBRARY_PATH     lib)
set(RAMSES_INSTALL_ARCHIVE_PATH     lib)
set(RAMSES_INSTALL_RESOURCES_PATH   "bin/res")

if (ramses-sdk_ALLOW_PLATFORM_GLM)
    find_package(glm QUIET)
    if (glm_FOUND AND NOT TARGET glm::glm)
        # older versions of glm may not export the "glm::glm" target, but "glm"
        add_library(glm::glm ALIAS glm)
    endif()
endif()
if (ramses-sdk_USE_PLATFORM_LUAJIT)
    if (TARGET lua::lua)
        message(WARNING "ramses-sdk_USE_PLATFORM_LUAJIT is ON, but `lua::lua` target is already defined. Existing `lua::lua` target will be used.")
    endif()
    find_package(LuaJIT REQUIRED)
endif()

createBuildConfig()

add_subdirectory(external)

add_subdirectory(include)
add_subdirectory(src)

if(ramses-sdk_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(ramses-sdk_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(ramses-sdk_BUILD_FULL_SHARED_LIB)
    if(ramses-sdk_BUILD_DEMOS)
        add_subdirectory(demo)
    endif()
endif()

if(ramses-sdk_BUILD_TESTS)
    add_subdirectory(tests)
endif()

include(cmake/ramses/createPackage.cmake)
include(cmake/ramses/addCheckerTargets.cmake)

if(RAMSES_TOPLEVEL OR ramses-sdk_FORCE_BUILD_DOCS)
    add_subdirectory(doc)
endif()

if(ramses-sdk_ENABLE_INSTALL)
    install(FILES       README.md CHANGELOG.md LICENSE.txt CONTRIBUTING.rst
            DESTINATION share/doc/ramses-sdk-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
            COMPONENT   ramses-sdk-${PROJECT_VERSION}
        )

    include(CMakePackageConfigHelpers)

    list(APPEND configmodules ramses-shared-lib-headless)
    if (ramses-sdk_BUILD_FULL_SHARED_LIB)
        list(APPEND configmodules ramses-shared-lib)
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
endif()
