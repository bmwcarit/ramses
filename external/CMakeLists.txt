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


createModule(
    NAME                    ramses-glslang
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           glslang/glslang
                            glslang/glslang/MachineIndependent
                            glslang
    SRC_FILES               glslang/glslang/Include/*.h
                            glslang/glslang/Public/*.h
                            glslang/glslang/OSDependent/*.h
                            glslang/glslang/MachineIndependent/*.h
                            glslang/OGLCompilersDLL/*.h
                            glslang/glslang/MachineIndependent/*.cpp
                            glslang/glslang/MachineIndependent/preprocessor/*.cpp
                            glslang/glslang/GenericCodeGen/*.cpp
                            glslang/OGLCompilersDLL/*.cpp
                            glslang-os-dep/GenericSingleThreaded/ossource.cpp
                            )


if(ramses-sdk_TEXT_SUPPORT)
    # find freetype with harfbuzz support
    if (ramses-sdk_ALLOW_PLATFORM_FREETYPE)
        find_package(FreetypeAndHarfbuzz QUIET)
    endif()
    if (ramses-sdk_ALLOW_CUSTOM_FREETYPE AND NOT freetype_FOUND)
        set(SKIP_INSTALL_ALL ON)

        # no 'd' suffix on debug libs
        set(DISABLE_FORCE_DEBUG_POSTFIX TRUE)

        add_subdirectory(freetype)
        message(STATUS "+ freetype")

        set(HB_BUILD_SUBSET OFF CACHE BOOL "don't build harfbuzz subset" FORCE)
        add_subdirectory(harfbuzz)
        message(STATUS "+ harfbuzz")

        folderizeTarget(freetype)
        folderizeTarget(harfbuzz)

    endif()
else()
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
IF (NOT ramses-sdk_DISABLE_WAYLAND_IVI_EXTENSION)
    FIND_PACKAGE(wayland-ivi-extension QUIET)
    IF (wayland-ivi-extension_FOUND)
        message(STATUS "+ wayland-ivi-extension (system)")
    ELSEIF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wayland-ivi-extension")
        IF (wayland-client_FOUND AND wayland-server_FOUND)
            createModule(
                NAME                    wayland-ivi-extension
                TYPE                    STATIC_LIBRARY
                ENABLE_INSTALL          OFF

                INCLUDE_PATHS           wayland-ivi-extension/ivi-extension-protocol
                SRC_FILES               wayland-ivi-extension/ivi-extension-protocol/*.h
                                        wayland-ivi-extension/ivi-extension-protocol/*.c
                )
        ENDIF()
    ENDIF()
ENDIF()

option(ramses-sdk_BUILD_IVI_TEST_APPS "Enable build of ivi test applications" ON)
if (ramses-sdk_BUILD_IVI_TEST_APPS AND (wayland-ivi-extension_FOUND OR TARGET wayland-ivi-extension) AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wayland-ivi-example-client")
    # wayland ivi example/test application
    createModule(
        NAME                    ivi-gears
        TYPE                    BINARY
        ENABLE_INSTALL          ON
        SRC_FILES            wayland-ivi-example-client/gears.c
        DEPENDENCIES            EGL
                                OpenGL
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

if (wayland-ivi-extension_FOUND OR TARGET wayland-ivi-extension)
    # wayland ivi example/test application 2
    createModule(
        NAME                    ivi-simple-dmabuf-egl
        TYPE                    BINARY
        ENABLE_INSTALL          ON
        SRC_FILES            wayland-ivi-example-client/simple-dmabuf-egl.c
        DEPENDENCIES            EGL
                                OpenGL
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
endif()

# LZ4 de-/compression library
OPTION(ramses-sdk_ALLOW_PLATFORM_LZ4 "Enable to search for platform provided lz4" ON)

# TODO (MacOS) find out why linking against the platform LZ4 doesn't work on Darwin
IF((${CMAKE_SYSTEM_NAME} MATCHES "iOS") OR (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
    SET(ramses-sdk_ALLOW_PLATFORM_LZ4 OFF)
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
if(NOT ramses-sdk_DISABLE_LOGIC)
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

    if(NOT TARGET lua)
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
    endif()

    add_library(lua::lua ALIAS lua)

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
            # we compile lua with c++, make sol not use extern C etc
            SOL_USING_CXX_LUA=1
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
endif()
