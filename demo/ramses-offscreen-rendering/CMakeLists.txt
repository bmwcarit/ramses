#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RENDERER_MODULE_PER_CONFIG_DYNAMIC(ramses-offscreen-rendering
    TYPE                BINARY
    ENABLE_INSTALL      ON
    FILES_SOURCE        src/*

    # lodepng from external for screenshot saveing
    INCLUDE_BASE            ${ramses-sdk_ROOT_CMAKE_PATH}/external/lodepng
    FILES_PRIVATE_HEADER    ${ramses-sdk_ROOT_CMAKE_PATH}/external/lodepng/*.h
    FILES_SOURCE            ${ramses-sdk_ROOT_CMAKE_PATH}/external/lodepng/lodepng.cpp
)
