if(POLICY CMP0054)
   # skip policies not known to earlier cmake versions
   cmake_policy(SET CMP0054 NEW)
endif(POLICY CMP0054)

find_package(PkgConfig)

function(ensureSubmoduleExists SUBMOD_NAME)
    if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${SUBMOD_NAME}")
        message(FATAL_ERROR "Missing submodule /external/${SUBMOD_NAME}. Please fetch Git submodules! (see build documentation)")
    endif()
endfunction()

# Ensure all external dependencies are built as static libs when not explicitly specified
# otherwise using ramses will lead to issues when BUILD_SHARED_LIBS in ON
set(BUILD_SHARED_LIBS OFF)

# #### externals that are build with normal compiler flags ####

#project specific setup for gtest/gmock
set(gtest_force_shared_crt true CACHE BOOL "Must be enabled for linking our unit tests" FORCE)
IF(ramses-sdk_BUILD_TESTS AND NOT TARGET gmock)
    set(INSTALL_GTEST OFF CACHE INTERNAL "")
    set(INSTALL_GMOCK OFF CACHE INTERNAL "")

    ADD_SUBDIRECTORY(googletest)
    target_include_directories(gmock INTERFACE "googletest/googletest/include")
    target_include_directories(gmock_main INTERFACE "googletest/googletest/include")
    target_compile_options(gtest PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>)
    target_compile_options(gmock PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>)

    folderizeTarget(gmock)
    folderizeTarget(gmock_main)
    folderizeTarget(gtest)
    folderizeTarget(gtest_main)
    message(STATUS "+ googletest (custom)")
ELSE()
    message(STATUS "+ googletest (external)")
ENDIF()

add_library(ramses-gmock INTERFACE)
add_library(ramses-gmock-main INTERFACE)
target_link_libraries(ramses-gmock INTERFACE gmock gtest)
target_link_libraries(ramses-gmock-main INTERFACE gmock_main gtest)


# fmt string formatting library
if (TARGET fmt::fmt)
    message(STATUS "+ fmt (existing target)")
else()
    set(FMT_INSTALL OFF)
    add_subdirectory("fmt")
    target_compile_definitions(fmt PUBLIC "-DFMT_EXCEPTIONS=0")
    folderizeTarget(fmt)
    message(STATUS "+ fmt")
endif()


# CLI11 command line parsing library
if (TARGET CLI11::CLI11)
    message(STATUS "+ CLI11 (existing target)")
else()
    add_subdirectory(cli11)

    # show CLI11 INTERFACE library in VS
    file(GLOB CLI11_HEADER
        ${CMAKE_CURRENT_SOURCE_DIR}/cli11/include/CLI/*.hpp
        )
    add_custom_target(cli11 SOURCES ${CLI11_HEADER})
    folderizeTarget(cli11)
    message(STATUS "+ CLI11")
endif()

# OpenGL Mathematics (glm)
if (TARGET glm::glm)
    get_target_property(glm_includes glm::glm INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "+ glm (existing target): ${glm_includes}")
else()
    add_subdirectory(glm)
    message(STATUS "+ glm (internal)")
    if(ramses-sdk_ENABLE_INSTALL)
        install(DIRECTORY glm/glm DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
    endif()
endif()

add_library(ramses-glad
    glad/src/gles2.c
)
target_include_directories(ramses-glad PUBLIC
    glad/include/
)
folderizeTarget(ramses-glad)

if(WIN32)
    target_include_directories(ramses-glad PUBLIC
        khronos # find wgl.h/wglext.h
    )
    target_link_libraries(ramses-glad PUBLIC opengl32)
endif()

# abseil library
function(add_abseil_scope)
    set(BUILD_TESTING OFF)
    cmakePathToFolderName(folder_base)
    set(ABSL_IDE_FOLDER "${folder_base}/Abseil")
    set(ABSL_PROPAGATE_CXX_STD ON)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-anon-enum-enum-conversion")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-identifier")
        endif()
    endif()
    add_subdirectory(abseil)

    # Remove wrongly included O2 options from internal Abseil targets on Windows in Debug builds
    # The option conflicts with /RTC1 - added by VS by default
    # This is fixed in more recent Abseil versions, but not in LTS yet
    # Ramses doesn't even use these targets, but there is no way to disable them...
    # Why remove O2/Ob2 instead of RTC1? Because RTC1 can't be easily disabled without affecting other targets
    # TODO (Violin) remove this hack after an Abseil upgrade with fix included
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        get_target_property(randen_hwaes_CXX_FLAGS       absl_random_internal_randen_hwaes       COMPILE_OPTIONS)
        get_target_property(randen_hwaes_impl_CXX_FLAGS  absl_random_internal_randen_hwaes_impl  COMPILE_OPTIONS)
        list(REMOVE_ITEM randen_hwaes_CXX_FLAGS "/O2")
        list(REMOVE_ITEM randen_hwaes_CXX_FLAGS "/Ob2")
        list(REMOVE_ITEM randen_hwaes_impl_CXX_FLAGS "/O2")
        list(REMOVE_ITEM randen_hwaes_impl_CXX_FLAGS "/Ob2")
        set_target_properties(absl_random_internal_randen_hwaes PROPERTIES COMPILE_OPTIONS "${randen_hwaes_CXX_FLAGS}")
        set_target_properties(absl_random_internal_randen_hwaes_impl PROPERTIES COMPILE_OPTIONS "${randen_hwaes_impl_CXX_FLAGS}")
    endif()

    message(STATUS "+ abseil")
endfunction()
add_abseil_scope()

# create interface target for easier usage
add_library(ramses-abseil INTERFACE)
foreach(lib
        base
        algorithm
        memory
        meta
        span optional variant
        utility
        strings_internal int128 strings)
    target_link_libraries(ramses-abseil INTERFACE absl::${lib})
endforeach()

# glslang
macro(configureGlslangGeneratedBuildHeader)
    # This block is originally taken and adapted from glslang's CMakeLists.txt
    # and licensed as follows:
    # Copyright (C) 2020-2023 The Khronos Group Inc.
    #
    # All rights reserved.
    #
    # Redistribution and use in source and binary forms, with or without
    # modification, are permitted provided that the following conditions
    # are met:
    #
    #    Redistributions of source code must retain the above copyright
    #    notice, this list of conditions and the following disclaimer.
    #
    #    Redistributions in binary form must reproduce the above
    #    copyright notice, this list of conditions and the following
    #    disclaimer in the documentation and/or other materials provided
    #    with the distribution.
    #
    #    Neither the name of The Khronos Group Inc. nor the names of its
    #    contributors may be used to endorse or promote products derived
    #    from this software without specific prior written permission.
    #
    # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    # "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    # LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    # FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    # COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    # INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    # BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    # CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    # LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    # ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    # POSSIBILITY OF SUCH DAMAGE.

    set(GLSLANG_CHANGES_FILE      "${CMAKE_CURRENT_SOURCE_DIR}/glslang/CHANGES.md")
    set(GLSLANG_BUILD_INFO_H_TMPL "${CMAKE_CURRENT_SOURCE_DIR}/glslang/build_info.h.tmpl")
    set(GLSLANG_GENERATED_INCLUDEDIR "${CMAKE_BINARY_DIR}/include/glslang")
    set(GLSLANG_BUILD_INFO_H      "${GLSLANG_GENERATED_INCLUDEDIR}/glslang/build_info.h")

    include(glslang/parse_version.cmake)
    parse_version(${GLSLANG_CHANGES_FILE} GLSLANG)

    function(configurate_version)
        set(major ${GLSLANG_VERSION_MAJOR})
        set(minor ${GLSLANG_VERSION_MINOR})
        set(patch ${GLSLANG_VERSION_PATCH})
        set(flavor ${GLSLANG_VERSION_FLAVOR})
        configure_file(${GLSLANG_BUILD_INFO_H_TMPL} ${GLSLANG_BUILD_INFO_H} @ONLY)
    endfunction()

    configurate_version()
endmacro()

configureGlslangGeneratedBuildHeader()
createModule(
    NAME                    ramses-glslang
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           glslang/glslang
                            glslang/glslang/MachineIndependent
                            glslang
                            ${GLSLANG_GENERATED_INCLUDEDIR}
    SRC_FILES               glslang/glslang/Include/*.h
                            glslang/glslang/Public/*.h
                            glslang/glslang/OSDependent/*.h
                            glslang/glslang/MachineIndependent/*.h
                            glslang/OGLCompilersDLL/*.h
                            glslang/SPIRV/*.h
                            glslang/SPIRV*.hpp
                            glslang/glslang/MachineIndependent/*.cpp
                            glslang/glslang/MachineIndependent/preprocessor/*.cpp
                            glslang/glslang/GenericCodeGen/*.cpp
                            glslang/OGLCompilersDLL/*.cpp
                            glslang/SPIRV/*.cpp
                            )

if(WIN32)
    target_sources(ramses-glslang PRIVATE glslang/glslang/OSDependent/Windows/ossource.cpp)
else()
    target_sources(ramses-glslang PRIVATE glslang/glslang/OSDependent/Unix/ossource.cpp)
endif()

createModule(
    NAME                    lodepng
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           lodepng
    SRC_FILES               lodepng/*.h
                            lodepng/lodepng.cpp
                            )

createModule(
    NAME                    cityhash
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           cityhash/src
    SRC_FILES               cityhash/src/*.h
                            cityhash/src/city.cc
                            )

if(ramses-sdk_TEXT_SUPPORT)
    # find freetype with harfbuzz support
    if (ramses-sdk_USE_PLATFORM_FREETYPE)
        find_package(FreetypeAndHarfbuzz REQUIRED)
        if (NOT freetype_FOUND)
            message(FATAL_ERROR "Freetype package is not found on the system. Please make sure it is available or use bundled package instead.")
        endif()
    else()
        set(SKIP_INSTALL_ALL ON)

        # no 'd' suffix on debug libs
        set(DISABLE_FORCE_DEBUG_POSTFIX TRUE)

        # freetype's FindHarfbuzz.cmake won't find our internal harfbuzz - force harfbuzz usage
        set(HARFBUZZ_FOUND TRUE)

        add_subdirectory(freetype)
        target_include_directories(freetype PRIVATE "harfbuzz/src")
        message(STATUS "+ freetype")

        set(HB_BUILD_SUBSET OFF CACHE BOOL "don't build harfbuzz subset" FORCE)
        add_subdirectory(harfbuzz)
        message(STATUS "+ harfbuzz")

        folderizeTarget(freetype)
        folderizeTarget(harfbuzz)

        # Verify that harfbuzz is actually used
        file(READ "${CMAKE_CURRENT_BINARY_DIR}/freetype/include/freetype/config/ftoption.h" FTOPTION_H)
        string(REGEX MATCH "\n#define FT_CONFIG_OPTION_USE_HARFBUZZ" FREETYPE_USES_HARFBUZZ "${FTOPTION_H}")
        if (NOT FREETYPE_USES_HARFBUZZ)
            message(FATAL_ERROR "Failed to enable harfbuzz support for internal freetype.")
        endif()
    endif()
else()
    if (ramses-sdk_USE_PLATFORM_FREETYPE)
        message(WARNING "ramses-sdk_USE_PLATFORM_FREETYPE is set to ${ramses-sdk_USE_PLATFORM_FREETYPE} but ramses-sdk_TEXT_SUPPORT is ${ramses-sdk_TEXT_SUPPORT}. ramses-sdk_USE_PLATFORM_FREETYPE will not take effect. Please set it to OFF or check the configuration.")
        set(ramses-sdk_USE_PLATFORM_FREETYPE OFF)
        message(VERBOSE "ramses-sdk_USE_PLATFORM_FREETYPE is now ${ramses-sdk_USE_PLATFORM_FREETYPE}.")
    endif()
    message(STATUS "- harfbuzz/freetype (text support disabled)")
endif()

# try to find automotive-dlt (if not already available)
IF (ramses-sdk_ENABLE_DLT AND NOT automotive-dlt_FOUND)
    FIND_PACKAGE(automotive-dlt QUIET)
ENDIF()
IF (ramses-sdk_ENABLE_DLT AND automotive-dlt_FOUND)
    SET(ramses-sdk_HAS_DLT TRUE CACHE BOOL "dlt found" FORCE)
    message(STATUS "+ automotive-dlt (platform)")
ELSEIF()
    SET(ramses-sdk_HAS_DLT FALSE CACHE BOOL "dlt found" FORCE)
    message(STATUS "- automotive-dlt")
ENDIF()

# try find wayland libs for following targets
FIND_PACKAGE(wayland-client QUIET)
FIND_PACKAGE(wayland-server QUIET)

# wayland ivi extension library
FIND_PACKAGE(wayland-ivi-extension QUIET)
IF (wayland-ivi-extension_FOUND)
    message(STATUS "+ wayland-ivi-extension (system)")
ELSEIF (wayland-client_FOUND AND wayland-server_FOUND)
    createModule(
        NAME                    wayland-ivi-extension
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        INCLUDE_PATHS           wayland-ivi-extension/ivi-extension-protocol
        SRC_FILES               wayland-ivi-extension/ivi-extension-protocol/*.h
                                wayland-ivi-extension/ivi-extension-protocol/*.c
        )
ENDIF()

find_package(OpenGL QUIET)

importDependenciesAndCheckMissing(WAYLAND_IVI_EXT_MISSING wayland-ivi-extension)
if (ramses-sdk_BUILD_TOOLS AND (NOT WAYLAND_IVI_EXT_MISSING))
    # wayland ivi example/test application
    createModule(
        NAME                    ivi-gears
        TYPE                    BINARY
        ENABLE_INSTALL          ON
        SRC_FILES               wayland-ivi-example-client/gears.c
        DEPENDENCIES            EGL
                                OpenGL::GLES3
                                wayland-client
                                wayland-egl
                                wayland-ivi-extension
    )

    # TODO convert this module either to fully C, or fully CXX
    set_property(TARGET ivi-gears PROPERTY LINKER_LANGUAGE CXX)
endif()

# wayland zwp-linux-dmabuf-v1 extension protocol
IF (wayland-client_FOUND AND wayland-server_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wayland-zwp-linux-dmabuf-v1-extension")
    createModule(
        NAME                    wayland-zwp-linux-dmabuf-v1-extension
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        INCLUDE_PATHS           wayland-zwp-linux-dmabuf-v1-extension
        SRC_FILES               wayland-zwp-linux-dmabuf-v1-extension/*.h
                                wayland-zwp-linux-dmabuf-v1-extension/*.c
        )
ENDIF()


importDependenciesAndCheckMissing(MISSING_DEPENDENCY wayland-ivi-extension gbm libdrm)
if (ramses-sdk_BUILD_TOOLS AND (NOT MISSING_DEPENDENCY))
    # wayland ivi example/test application 2
    createModule(
        NAME                    ivi-simple-dmabuf-egl
        TYPE                    BINARY
        ENABLE_INSTALL          ON
        SRC_FILES            wayland-ivi-example-client/simple-dmabuf-egl.c
        DEPENDENCIES            EGL
                                OpenGL::GLES3
                                wayland-client
                                wayland-egl
                                wayland-ivi-extension
                                wayland-zwp-linux-dmabuf-v1-extension
                                libdrm
                                gbm
    )
endif()

if (ramses-sdk_ENABLE_TCP_SUPPORT)
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE asio/asio/include)
    # ASIO_USE_TS_EXECUTOR_AS_DEFAULT avoids an internal compiler error for gcc lto build
    # It restores the any_io_executor type alias to the one used in asio 1.16.1 an earlier
    target_compile_definitions(asio INTERFACE "-DASIO_USE_TS_EXECUTOR_AS_DEFAULT")
    message(STATUS "+ asio")
else()
    message(STATUS "- asio")
endif()


if(ramses-sdk_BUILD_TOOLS)
    createModule(
        NAME                    imgui
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        INCLUDE_PATHS           imgui
        SRC_FILES               imgui/*.h
                                imgui/misc/cpp/*.h
                                imgui/*.cpp
                                imgui/misc/cpp/*.cpp
        )

    add_executable(imgui_binary_to_compressed EXCLUDE_FROM_ALL
        imgui/misc/fonts/binary_to_compressed_c.cpp
        )
    folderizeTarget(imgui_binary_to_compressed)
endif()


# TODO (MacOS) find out why linking against the platform LZ4 doesn't work on Darwin
IF((${CMAKE_SYSTEM_NAME} MATCHES "iOS") OR (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
    SET(ramses-sdk_ALLOW_PLATFORM_LZ4 OFF)
    message(WARNING "Force ramses-sdk_ALLOW_PLATFORM_LZ4 set to OFF")
ENDIF()

IF (ramses-sdk_ALLOW_PLATFORM_LZ4)
    pkg_check_modules(lz4 liblz4)
ENDIF()

IF (lz4_FOUND)
    message(STATUS "+ lz4 (system)")
ELSE()
  SET(LZ4_DIR lz4)
  createModule(
    NAME                    lz4
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           ${LZ4_DIR}/lib
    SRC_FILES               ${LZ4_DIR}/lib/*.h
                            ${LZ4_DIR}/lib/*.c
    )
ENDIF()

# Ramses logic specific dependencies
################################################
################   Google Benchmark  ###########
################################################

#project specific setup for google benchmark
if(ramses-sdk_BUILD_TESTS AND NOT TARGET benchmark::benchmark)
	set(BENCHMARK_ENABLE_TESTING OFF CACHE INTERNAL "")
	set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE INTERNAL "")
	set(BENCHMARK_ENABLE_INSTALL OFF CACHE INTERNAL "")

	ensureSubmoduleExists(google-benchmark)

	add_subdirectory(google-benchmark)

	folderizeTarget(benchmark)
	folderizeTarget(benchmark_main)
endif()

add_library(ramses-google-benchmark-main INTERFACE)
target_link_libraries(ramses-google-benchmark-main INTERFACE benchmark_main)
add_library(ramses::google-benchmark-main ALIAS ramses-google-benchmark-main)

################################################
################     Lua      ##################
################################################

if(NOT TARGET lua::lua)
	message(STATUS "+ lua (internal) (STATIC_LIBRARY)")
	ensureSubmoduleExists(lua)

	# Collect all source and header files
	file(GLOB LUA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/lua/*.c)
	file(GLOB LUA_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/lua/*.h)

	# Remove luac from the list, because it contains a main function
	list(REMOVE_ITEM LUA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/lua/lua.c)

	set_source_files_properties(${LUA_SOURCE} PROPERTIES LANGUAGE CXX)

	# Create library for lua
	add_library(lua STATIC ${LUA_SOURCE} ${LUA_HEADER})

	# This is required for Lua on Android to suppress a false-positive fortification trigger when
	# the garbage collector is invoked
	if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
		target_compile_options(lua PRIVATE -fno-stack-protector -U_FORTIFY_SOURCE -Wno-deprecated-declarations)
	endif()

	if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
		# We compile lua as c++ even though it has .c files. convince clang to just do it without complaining
		target_compile_options(lua PRIVATE -x c++)
	endif()

	target_include_directories(lua
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lua>
	)

	folderizeTarget(lua)
	add_library(lua::lua ALIAS lua)
endif()

################################################
################     Sol      ##################
################################################

if(NOT TARGET sol2)
	set(SOL2_LUA_VERSION "5.1.1" CACHE STRING "" FORCE)
	set(INSTALL_SOL2 OFF CACHE INTERNAL "")

	ensureSubmoduleExists(sol)

	# TODO Violin remove EXCLUDE_FROM_ALL when upgrading to the next official sol version
	# Currently its's needed because otherwise sol is installed automatically and also
	# lands in the packaged version of ramses
	add_subdirectory(sol EXCLUDE_FROM_ALL)

	# Ensure sol is expecting c++ compiled lua
	target_compile_definitions(sol2 INTERFACE
		# catch and redirect exception to user handler func instead of
		# prapagating them directly through lua
		SOL_EXCEPTIONS_ALWAYS_UNSAFE=1
		# check if types are numbers before using them as numbers
		SOL_SAFE_NUMERICS=1
		# ensure sol calls luaL_checkstack to avoid stack overflows
		SOL_SAFE_STACK_CHECK=1
		# Make sure Debug flags are equivalent to Release flags in terms of behavior
		SOL_IN_DEBUG_DETECTED=0
	)

	if(NOT ramses-sdk_USE_PLATFORM_LUAJIT)
		target_compile_definitions(sol2 INTERFACE
			# we compile lua with c++, make sol not use extern C etc
			SOL_USING_CXX_LUA=1
			)
	endif()
endif()

################################################
################ Flatbuffers  ##################
################################################

if(NOT TARGET flatbuffers)
	# Don't build flatbuf targets we don't need
	set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(FLATBUFFERS_INSTALL OFF CACHE BOOL "" FORCE)
	set(FLATBUFFERS_BUILD_FLATHASH OFF CACHE BOOL "" FORCE)

	# Only add flatc target if flatbuffers header generation needed
	if(ramses-sdk_ENABLE_FLATBUFFERS_GENERATION)
		set(FLATBUFFERS_BUILD_FLATC ON CACHE BOOL "Build flatbuffers compiler" FORCE)
	else()
		set(FLATBUFFERS_BUILD_FLATC OFF CACHE BOOL "Build flatbuffers compiler" FORCE)
	endif()

	ensureSubmoduleExists(flatbuffers)

	add_subdirectory(flatbuffers)
	folderizeTarget(flatbuffers)
endif()

add_library(ramses::flatbuffers ALIAS flatbuffers)

if(ramses-sdk_ENABLE_FLATBUFFERS_GENERATION)
	folderizeTarget(flatc)
endif()


# ANGLE - Build for iOS support only
if((${CMAKE_SYSTEM_NAME} MATCHES "iOS"))
    # Define File Copy Function that WebKit ANGLE build expects to be there
    function(WEBKIT_COPY_FILES target_name)
        # This CMake macro is from the WebKit Repository and licensed as follows:
        #   BSD License
        #   
        #   Copyright (C) 2009 Apple Inc. All rights reserved.
        #   
        #   Redistribution and use in source and binary forms, with or without
        #   modification, are permitted provided that the following conditions are met: 
        #   
        #   1. Redistributions of source code must retain the above copyright notice, this
        #   list of conditions and the following disclaimer.
        #   
        #   2. Redistributions in binary form must reproduce the above copyright notice, this
        #   list of conditions and the following disclaimer in the documentation and/or other
        #   materials provided with the distribution.
        #   
        #   THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS “AS IS” AND ANY
        #   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
        #   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
        #   EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
        #   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
        #   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        #   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
        #   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
        #   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        set(options FLATTENED)
        set(oneValueArgs DESTINATION)
        set(multiValueArgs FILES)
        cmake_parse_arguments(opt "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
        set(files ${opt_FILES})
        set(dst_files)
        foreach (file IN LISTS files)
            if (IS_ABSOLUTE ${file})
                set(src_file ${file})
            else ()
                set(src_file ${CMAKE_CURRENT_SOURCE_DIR}/${file})
            endif ()
            if (opt_FLATTENED)
                get_filename_component(filename ${file} NAME)
                set(dst_file ${opt_DESTINATION}/${filename})
            else ()
                get_filename_component(file_dir ${file} DIRECTORY)
                file(MAKE_DIRECTORY ${opt_DESTINATION}/${file_dir})
                set(dst_file ${opt_DESTINATION}/${file})
            endif ()
            add_custom_command(OUTPUT ${dst_file}
                COMMAND ${CMAKE_COMMAND} -E copy ${src_file} ${dst_file}
                MAIN_DEPENDENCY ${file}
                VERBATIM
            )
            list(APPEND dst_files ${dst_file})
        endforeach ()
        add_custom_target(${target_name} ALL DEPENDS ${dst_files})
    endfunction()

    # Define Config Function that WebKit ANGLE build expects to be there. For iOS we need to define our own configuration, no config file exists.
    macro(WEBKIT_INCLUDE_CONFIG_FILES_IF_EXISTS)
        find_library(COREGRAPHICS_LIBRARY CoreGraphics)
        find_library(FOUNDATION_LIBRARY Foundation)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(IOSURFACE_LIBRARY IOSurface)
        find_library(METAL_LIBRARY Metal)
        find_package(ZLIB REQUIRED)
        
        list(APPEND ANGLE_SOURCES
            ${metal_backend_sources}
        
            ${angle_translator_lib_metal_sources}
            ${angle_translator_apple_sources}
            ${angle_translator_glsl_apple_sources}
        
            ${libangle_gpu_info_util_ios_sources}
            ${libangle_gpu_info_util_sources}
            ${libangle_ios_sources}
        )
        
        list(APPEND ANGLE_DEFINITIONS
            ANGLE_ENABLE_METAL
            ANGLE_PLATFORM_IOS
            ANGLE_ENABLE_APPLE_WORKAROUNDS
        )
        
        list(APPEND ANGLEGLESv2_LIBRARIES
            ${COREGRAPHICS_LIBRARY}
            ${FOUNDATION_LIBRARY}
            ${IOKIT_LIBRARY}
            ${IOSURFACE_LIBRARY}
            ${METAL_LIBRARY}
        )    
    endmacro()

    # Configure the ANGLE build
    set(USE_ANGLE_EGL ON)
    set(ENABLE_WEBGL OFF)
    set(ANGLE_FRAMEWORK_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/ANGLE/FRAMEWORK_HEADERS_DESTINATION)
    set(PORT Mac)
    set(is_apple ON)
    set(is_ios ON)
    set(is_mac OFF)

    # Add Angle CMake build
    add_subdirectory(ANGLE)

    target_include_directories(GLESv2  INTERFACE ANGLE/include)
    target_include_directories(EGL INTERFACE ANGLE/include)

    # Alias ANGLE as OpenGL, so it is a drop in replacement.
    target_link_libraries(ramses-glad PUBLIC GLESv2)
endif()


if(ramses-sdk_ENABLE_DEVICE_TYPE_VULKAN)
    message(STATUS "Environment VULKAN_SDK: $ENV{VULKAN_SDK}")

    find_package(Vulkan REQUIRED)
    message(STATUS "+ Vulkan ${Vulkan_VERSION}")
    add_library(ramses-vulkan INTERFACE)
    target_link_libraries(ramses-vulkan INTERFACE ${Vulkan_LIBRARIES})
    target_include_directories(ramses-vulkan INTERFACE ${Vulkan_INCLUDE_DIR})

    # add headers as sources to enable IntelliSense and other code completion features
    file(GLOB VULKAN_HEADERS ${Vulkan_INCLUDE_DIR}/vulkan/*.h)
    target_sources(ramses-vulkan INTERFACE ${VULKAN_HEADERS})
endif()
