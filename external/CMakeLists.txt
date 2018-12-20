if(POLICY CMP0054)
   # skip policies not known to earlier cmake versions
   cmake_policy(SET CMP0054 NEW)
endif(POLICY CMP0054)

find_package(PkgConfig)

# #### externals that are build with normal compiler flags ####

#project specific setup for gtest/gmock
set(gtest_force_shared_crt true CACHE BOOL "Must be enabled for linking our unit tests" FORCE)
IF(ramses-sdk_BUILD_TESTS AND NOT TARGET gmock)
    pkg_check_modules(gmock_main gmock)
    pkg_check_modules(gmock gmock)

    IF (gmock_FOUND AND gmock_main_FOUND)
        ACME_INFO("+ googletest (platform)")
    ELSE()
        set(INSTALL_GTEST OFF CACHE INTERNAL "")
        set(INSTALL_GMOCK OFF CACHE INTERNAL "")

        ADD_SUBDIRECTORY(googletest)
        target_include_directories(gmock INTERFACE "googletest/googletest/include")
        target_include_directories(gmock_main INTERFACE "googletest/googletest/include")
        target_compile_options(gtest PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>)
        target_compile_options(gmock PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>)

        ACME_FOLDERIZE_TARGETS(gmock gmock_main gtest gtest_main)
        ACME_INFO("+ googletest (custom)")
    ENDIF()
ELSE()
    ACME_INFO("+ googletest (external)")
ENDIF()

add_library(ramses-gmock INTERFACE)
add_library(ramses-gmock-main INTERFACE)
target_link_libraries(ramses-gmock INTERFACE gmock gtest)
target_link_libraries(ramses-gmock-main INTERFACE gmock_main gtest)

ADD_SUBDIRECTORY(capu)

include(../cmake/ramses/adaptCompilerFlagsForExternals.cmake)

# these directories will be built with
# modified compiler flags

#==========================================================================
# glslang
#==========================================================================
ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-glslang
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

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

    # save c/cxx flags and remove -flto (freetype/harfbuzz should not be built with LTO)
    set(OLD_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(OLD_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS}" "-flto" CMAKE_CXX_FLAGS)
    REMOVE_FROM_FLAGS("${CMAKE_C_FLAGS}" "-flto" CMAKE_C_FLAGS)

    # save and restore module search path
    set(OLD_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_MODULE_PATH})

    # Checking the availability of the functions below by harfbuzz does not work, because
    # check_function_exists() checks against an existing platform freetype library.
    # The freetype version from external does not support those functions, so set them to off manually:
    set(HAVE_FT_GET_VAR_BLEND_COORDINATES OFF CACHE BOOL "" FORCE)
    set(HAVE_FT_SET_VAR_BLEND_COORDINATES OFF CACHE BOOL "" FORCE)
    set(HAVE_FT_DONE_MM_VAR OFF CACHE BOOL "" FORCE)

    ADD_SUBDIRECTORY(harfbuzz)
    ACME_INFO("+ harfbuzz")

    IF("${TARGET_OS}" STREQUAL "Integrity")
        # define __restrict to nothing because not known to integrity compiler
        TARGET_COMPILE_DEFINITIONS(harfbuzz PRIVATE "-D__restrict=")
    ENDIF()

    # remove find script again
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/FindFreetype.cmake")
    set(CMAKE_MODULE_PATH ${OLD_CMAKE_MODULE_PATH})

    # config and include freetype
    target_include_directories(harfbuzz INTERFACE "harfbuzz/src" "${CMAKE_CURRENT_BINARY_DIR}/harfbuzz")
    ADD_SUBDIRECTORY(freetype)

    target_include_directories(freetype INTERFACE "freetype/include")

    # Manually install only static libs for static RAMSES builds
    # Not possible to use CMake targets because of complicated CMake scripts of each lib
    # TODO(tobias/violin): try with newer versions of FT/HB to use CMake install targets
    if(${ramses-sdk_INSTALL_STATIC_LIBS})
        if (${CMAKE_VERSION} VERSION_LESS "2.9")  # aka not 2.8.x
            install(FILES
                ${CMAKE_CURRENT_BINARY_DIR}/freetype/libfreetype.a
                ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz/libharfbuzz.a
                DESTINATION ${PROJECT_INSTALL_STATIC_LIB})
        else()
            install(FILES
                $<TARGET_FILE:freetype>
                $<TARGET_FILE:harfbuzz>
                DESTINATION ${PROJECT_INSTALL_STATIC_LIB})
         endif()
    endif()

    # restore c/cxx flags
    set(CMAKE_CXX_FLAGS "${OLD_CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${OLD_CMAKE_C_FLAGS}")

    ACME_FOLDERIZE_TARGETS(freetype harfbuzz)
    foreach (tgt dist harfbuzz-subset hb-ot-tag main test-buffer-serialize test-size-params test-size-params test-test test-unicode-ranges test-would-substitute)
        if (TARGET ${tgt})
            ACME_FOLDERIZE_TARGETS(${tgt})
            set_target_properties(${tgt} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
        endif()
    endforeach()

    ACME_INFO("+ freetype")
ENDIF()


IF (ramses-sdk_ENABLE_DLT)
    FIND_PACKAGE(automotive-dlt QUIET)
ENDIF()
IF (automotive-dlt_FOUND)
    SET(ramses-sdk_HAS_DLT TRUE CACHE BOOL "dlt found" FORCE)
    ACME_INFO("+ automotive-dlt (platform)")
ELSEIF()
    SET(ramses-sdk_HAS_DLT FALSE CACHE BOOL "dlt found" FORCE)
    ACME_INFO("- automotive-dlt")
ENDIF()


# wayland ivi extension library
IF (NOT ramses-sdk_DISABLE_WAYLAND_IVI_EXTENSION)
    FIND_PACKAGE(wayland-ivi-extension QUIET)
    IF (wayland-ivi-extension_FOUND)
        ACME_INFO("+ wayland-ivi-extension (system)")
    ELSE()
        FIND_PACKAGE(wayland-client QUIET)
        FIND_PACKAGE(wayland-server QUIET)
        IF (wayland-client_FOUND AND wayland-server_FOUND)
            ACME_MODULE(
                NAME                    wayland-ivi-extension
                TYPE                    STATIC_LIBRARY
                ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

                INCLUDE_BASE            wayland-ivi-extension/ivi-extension-protocol
                FILES_PRIVATE_HEADER    wayland-ivi-extension/ivi-extension-protocol/*.h
                FILES_SOURCE            wayland-ivi-extension/ivi-extension-protocol/*.c
                )
        ENDIF()
    ENDIF()
ENDIF()

if (wayland-ivi-extension_FOUND OR TARGET wayland-ivi-extension)
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

# LZ4 de-/compression library
OPTION(ramses-sdk_ALLOW_PLATFORM_LZ4 "Enable to search for platform provided lz4" ON)

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
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            ${LZ4_DIR}/lib
    FILES_PRIVATE_HEADER    ${LZ4_DIR}/lib/*.h
    FILES_SOURCE            ${LZ4_DIR}/lib/*.c
    )
ENDIF()


add_library(asio INTERFACE)
target_include_directories(asio INTERFACE asio/asio/include)
ACME_INFO("+ asio")
