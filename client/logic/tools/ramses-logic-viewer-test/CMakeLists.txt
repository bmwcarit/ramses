#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

find_package(ImageMagick COMPONENTS compare)

if(ramses-sdk_USE_IMAGEMAGICK AND NOT ImageMagick_FOUND)
    message(FATAL_ERROR "ImageMagick compare not found, but required (ramses-sdk_USE_IMAGEMAGICK=ON)")
endif()

add_executable(ramses-logic-viewer-unittests
    LogicViewerTestBase.h
    LogicViewerTestBase.cpp
    LogicViewerTest.cpp
    LogicViewerLuaTest.cpp
)

target_link_libraries(ramses-logic-viewer-unittests
    PRIVATE
        ramses-logic-viewer-lib
        ramses-client
        ramses-gmock
        ramses-gmock-main
    )

target_include_directories(ramses-logic-viewer-unittests
    PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/client/logic/tools/ramses-logic-viewer>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/client/logic/unittests/shared>
)

MakeTestFromTarget(
    TARGET ramses-logic-viewer-unittests
    SUFFIX UNITTEST)

folderizeTarget(ramses-logic-viewer-unittests)

if(ramses-sdk_USE_IMAGEMAGICK AND ImageMagick_FOUND)
    expandModuleByPlatform(
        NAME            ramses-logic-viewer-swrast-tests
        TYPE            BINARY
        LINKED_TYPE     DYNAMIC
        ANY_PLATFORM
        ENABLE_INSTALL  ${ramses-sdk_ENABLE_INSTALL}
        INCLUDE_PATHS   ${PROJECT_SOURCE_DIR}/client/logic/tools/ramses-logic-viewer
                        ${PROJECT_SOURCE_DIR}/client/logic/unittests/shared
        SRC_FILES       LogicViewerAppTest.cpp
        DEPENDENCIES    ramses-logic-viewer-gui-lib
                        ramses-gmock
                        ramses-gmock-main
    )

    if(TARGET ramses-logic-viewer-swrast-tests)
        target_compile_definitions(ramses-logic-viewer-swrast-tests
            PRIVATE MAGICK_COMPARE="${ImageMagick_compare_EXECUTABLE}"
            )

        MakeTestFromTarget(
            TARGET ramses-logic-viewer-swrast-tests
            SUFFIX RNDSANDWICHTEST_SWRAST)

        add_custom_command(TARGET ramses-logic-viewer-swrast-tests PRE_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/unittests)
    endif()
endif()
