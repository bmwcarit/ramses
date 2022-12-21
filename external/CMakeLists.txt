if(POLICY CMP0054)
   # skip policies not known to earlier cmake versions
   cmake_policy(SET CMP0054 NEW)
endif(POLICY CMP0054)

find_package(PkgConfig)

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

    ACME_FOLDERIZE_TARGETS(gmock gmock_main gtest gtest_main)
    ACME_INFO("+ googletest (custom)")
ELSE()
    ACME_INFO("+ googletest (external)")
ENDIF()

add_library(ramses-gmock INTERFACE)
add_library(ramses-gmock-main INTERFACE)
target_link_libraries(ramses-gmock INTERFACE gmock gtest)
target_link_libraries(ramses-gmock-main INTERFACE gmock_main gtest)


# fmt string formatting library
if (TARGET fmt::fmt)
    ACME_INFO("+ fmt (existing target)")
else()
    add_subdirectory("fmt")
    target_compile_definitions(fmt PUBLIC "-DFMT_EXCEPTIONS=0")
    ACME_FOLDERIZE_TARGET(fmt)
    ACME_INFO("+ fmt")
endif()


# CLI11 command line parsing library
if (TARGET CLI11::CLI11)
    ACME_INFO("+ CLI11 (existing target)")
else()
    add_subdirectory(cli11)

    # show CLI11 INTERFACE library in VS
    file(GLOB CLI11_HEADER
        ${CMAKE_CURRENT_SOURCE_DIR}/cli11/include/CLI/*.hpp
        )
    add_custom_target(cli11 SOURCES ${CLI11_HEADER})
    ACME_FOLDERIZE_TARGET(cli11)
    ACME_INFO("+ CLI11")
endif()


# abseil library
function(add_abseil_scope)
    set(BUILD_TESTING OFF)
    ACME_CURRENT_FOLDER_PATH(folder_base)
    set(ABSL_IDE_FOLDER "${folder_base}/Abseil")
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

    ACME_INFO("+ abseil")
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


#==========================================================================
# glslang
#==========================================================================
ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-glslang
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            glslang/glslang
                            glslang/glslang/MachineIndependent
                            glslang
    FILES_PRIVATE_HEADER    glslang/glslang/Include/*.h
                            glslang/glslang/Public/*.h
                            glslang/glslang/OSDependent/*.h
                            glslang/glslang/MachineIndependent/*.h
                            glslang/OGLCompilersDLL/*.h
    FILES_SOURCE            glslang/glslang/MachineIndependent/*.cpp
                            glslang/glslang/MachineIndependent/preprocessor/*.cpp
                            glslang/glslang/GenericCodeGen/*.cpp
                            glslang/OGLCompilersDLL/*.cpp
                            glslang-os-dep/GenericSingleThreaded/ossource.cpp
                            )
IF("${TARGET_OS}" STREQUAL "Integrity")
    # prevent error because __inline not allowed in c++ code
    TARGET_COMPILE_DEFINITIONS(ramses-glslang PRIVATE "-D__inline=inline")
ENDIF()

# find freetype with harfbuzz support
IF (ramses-sdk_ALLOW_PLATFORM_FREETYPE)
    find_package(FreetypeAndHarfbuzz QUIET)
ENDIF()
IF (ramses-sdk_ALLOW_CUSTOM_FREETYPE AND NOT freetype_FOUND)

    # Don't install anything from Freetype and Harfbuzz, manual install later
    # Reason: the libraries are not meant to be build as submodule together
    set(SKIP_INSTALL_ALL ON)

    # no 'd' suffix on debug libs
    set(DISABLE_FORCE_DEBUG_POSTFIX TRUE)

    # newer harfbuzz versions use include (FindFreetype): provide an adhoc find script
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/FindFreetype.cmake"
        "set(FREETYPE_INCLUDE_DIRS \"${CMAKE_CURRENT_SOURCE_DIR}/freetype\" \"${CMAKE_CURRENT_SOURCE_DIR}/freetype/include\")\n"
        "set(FREETYPE_LIBRARIES freetype)\n"
        "set(FREETYPE_FOUND TRUE)\n")

    # enable freetype in harfbuzz
    set(HB_HAVE_FREETYPE ON CACHE BOOL "Enable freetype interop helpers" FORCE)

    # save and restore module search path
    set(OLD_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_MODULE_PATH})

    # Checking the availability of the functions below by harfbuzz does not work, because
    # check_function_exists() checks against an existing platform freetype library.
    # The freetype version from external does not support those functions, so set them to off manually:
    set(HAVE_FT_GET_VAR_BLEND_COORDINATES OFF CACHE BOOL "" FORCE)
    set(HAVE_FT_SET_VAR_BLEND_COORDINATES OFF CACHE BOOL "" FORCE)
    set(HAVE_FT_DONE_MM_VAR OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_TESTS OFF CACHE BOOL "" FORCE)

    ADD_SUBDIRECTORY(harfbuzz)
    ACME_INFO("+ harfbuzz")

    IF("${TARGET_OS}" STREQUAL "Integrity")
        # define __restrict to nothing because not known to integrity compiler
        TARGET_COMPILE_DEFINITIONS(harfbuzz PRIVATE "-D__restrict=")
    ENDIF()

    # remove find script again
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/FindFreetype.cmake")

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/FindHarfbuzz.cmake"
        "set(HARFBUZZ_INCLUDE_DIRS \"${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz\" \"${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/include\")\n"
        "set(HARFBUZZ_LIBRARIES harfbuzz)\n"
        "set(HARFBUZZ_FOUND TRUE)\n")

    # config and include freetype
    foreach (inc "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src" "${CMAKE_CURRENT_BINARY_DIR}/harfbuzz" "${CMAKE_CURRENT_BINARY_DIR}/harfbuzz/src")
        if (EXISTS "${inc}")
            target_include_directories(harfbuzz INTERFACE ${inc})
        endif()
    endforeach()
    ADD_SUBDIRECTORY(freetype)
    set(CMAKE_MODULE_PATH ${OLD_CMAKE_MODULE_PATH})

    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/FindHarfbuzz.cmake")

    target_include_directories(freetype INTERFACE "freetype/include")

    ACME_FOLDERIZE_TARGETS(freetype harfbuzz)
    foreach (tgt dist harfbuzz-subset hb-ot-tag main test-buffer-serialize test-size-params test-size-params test-test test-unicode-ranges test-would-substitute)
        if (TARGET ${tgt})
            ACME_FOLDERIZE_TARGETS(${tgt})
            set_target_properties(${tgt} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
        endif()
    endforeach()

    ACME_INFO("+ freetype")
ENDIF()

# try to find automotive-dlt (if not already available)
IF (ramses-sdk_ENABLE_DLT AND NOT automotive-dlt_FOUND)
    FIND_PACKAGE(automotive-dlt QUIET)
ENDIF()
IF (ramses-sdk_ENABLE_DLT AND automotive-dlt_FOUND)
    SET(ramses-sdk_HAS_DLT TRUE CACHE BOOL "dlt found" FORCE)
    ACME_INFO("+ automotive-dlt (platform)")
ELSEIF()
    SET(ramses-sdk_HAS_DLT FALSE CACHE BOOL "dlt found" FORCE)
    ACME_INFO("- automotive-dlt")
ENDIF()

# try find wayland libs for following targets
FIND_PACKAGE(wayland-client QUIET)
FIND_PACKAGE(wayland-server QUIET)

# wayland ivi extension library
IF (NOT ramses-sdk_DISABLE_WAYLAND_IVI_EXTENSION)
    FIND_PACKAGE(wayland-ivi-extension QUIET)
    IF (wayland-ivi-extension_FOUND)
        ACME_INFO("+ wayland-ivi-extension (system)")
    ELSEIF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wayland-ivi-extension")
        IF (wayland-client_FOUND AND wayland-server_FOUND)
            ACME_MODULE(
                NAME                    wayland-ivi-extension
                TYPE                    STATIC_LIBRARY
                ENABLE_INSTALL          OFF

                INCLUDE_BASE            wayland-ivi-extension/ivi-extension-protocol
                FILES_PRIVATE_HEADER    wayland-ivi-extension/ivi-extension-protocol/*.h
                FILES_SOURCE            wayland-ivi-extension/ivi-extension-protocol/*.c
                )
        ENDIF()
    ENDIF()
ENDIF()

option(ramses-sdk_BUILD_IVI_TEST_APPS "Enable build of ivi test applications" ON)
if (ramses-sdk_BUILD_IVI_TEST_APPS AND (wayland-ivi-extension_FOUND OR TARGET wayland-ivi-extension) AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wayland-ivi-example-client")
    # wayland ivi example/test application
    ACME_MODULE(
        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    ivi-gears
        TYPE                    BINARY
        ENABLE_INSTALL          ON

        #==========================================================================
        # files of this module
        #==========================================================================
        FILES_SOURCE            wayland-ivi-example-client/gears.c

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            EGL
                                OpenGL
                                wayland-client
                                wayland-egl
                                wayland-ivi-extension
    )
endif()

# wayland zwp-linux-dmabuf-v1 extension protocol
IF (wayland-client_FOUND AND wayland-server_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wayland-zwp-linux-dmabuf-v1-extension")
    ACME_MODULE(
        NAME                    wayland-zwp-linux-dmabuf-v1-extension
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

        INCLUDE_BASE            wayland-zwp-linux-dmabuf-v1-extension
        FILES_PRIVATE_HEADER    wayland-zwp-linux-dmabuf-v1-extension/*.h
        FILES_SOURCE            wayland-zwp-linux-dmabuf-v1-extension/*.c
        )
ENDIF()

# LZ4 de-/compression library
OPTION(ramses-sdk_ALLOW_PLATFORM_LZ4 "Enable to search for platform provided lz4" ON)

IF((${CMAKE_SYSTEM_NAME} MATCHES "iOS") OR (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")) 
    SET(ramses-sdk_ALLOW_PLATFORM_LZ4 OFF)
ENDIF()

IF (ramses-sdk_ALLOW_PLATFORM_LZ4)
    pkg_check_modules(lz4 liblz4)
ENDIF()

IF (lz4_FOUND)
    ACME_INFO("+ lz4 (system)")
ELSE()
  SET(LZ4_DIR lz4)
  ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    lz4
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            ${LZ4_DIR}/lib
    FILES_PRIVATE_HEADER    ${LZ4_DIR}/lib/*.h
    FILES_SOURCE            ${LZ4_DIR}/lib/*.c
    )
ENDIF()

if (ramses-sdk_ENABLE_TCP_SUPPORT)
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE asio/asio/include)
    ACME_INFO("+ asio")
else()
    ACME_INFO("- asio")
endif()


if(ramses-sdk_BUILD_TOOLS)
    ACME_MODULE(
        NAME                    imgui
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        INCLUDE_BASE            imgui
        FILES_PRIVATE_HEADER    imgui/*.h
                                imgui/misc/cpp/*.h
        FILES_SOURCE            imgui/*.cpp
                                imgui/misc/cpp/*.cpp
        )
endif()
