#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

INCLUDE(../../cmake/ramses/apiDirectories.cmake)
RAMSES_GET_PATHS_RELATIVE_TO_DIRECTORY("${RAMSES_RENDERER_API_HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR} RELATIVE_RAMSES_RENDERER_API_HEADERS)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-renderer-api
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            include
    FILES_PRIVATE_HEADER    ${RELATIVE_RAMSES_RENDERER_API_HEADERS}

    FILES_SOURCE            src/*.cpp
    DEPENDENCIES            ramses-framework-api
)

INSTALL(DIRECTORY include/ DESTINATION "${PROJECT_INSTALL_HEADER}" COMPONENT ramses-sdk-devel)
