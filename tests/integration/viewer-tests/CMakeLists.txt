#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

find_package(ImageMagick COMPONENTS compare)

if(NOT ImageMagick_FOUND)
    message(FATAL_ERROR "ImageMagick compare not found, but required (ramses-sdk_USE_IMAGEMAGICK=ON)")
endif()

createModule(
    NAME            ramses-viewer-gui-test
    TYPE            BINARY
    ENABLE_INSTALL  ${ramses-sdk_ENABLE_INSTALL}
    INCLUDE_PATHS   ${PROJECT_SOURCE_DIR}/tools/ramses-viewer
                    ${PROJECT_SOURCE_DIR}/tests/unittests/client/logic/shared
    SRC_FILES       LogicViewerAppTest.cpp
    RESOURCE_FOLDERS res

    PUBLIC_DEFINES  MAGICK_COMPARE="${ImageMagick_compare_EXECUTABLE}"

    DEPENDENCIES    ramses-viewer-gui-lib
                    ramses-renderer
                    ramses-gmock
                    ramses-gmock-main
)

MakeTestFromTarget(
    TARGET ramses-viewer-gui-test
    SUFFIX RNDSANDWICHTEST_SWRAST)
