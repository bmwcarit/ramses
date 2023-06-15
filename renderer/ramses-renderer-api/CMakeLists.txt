#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB
    RAMSES_RENDERER_API_INCLUDE_BASE
    include)

add_library(ramses-renderer-api INTERFACE)
target_include_directories(ramses-renderer-api INTERFACE ${RAMSES_RENDERER_API_INCLUDE_BASE})

if(ramses-sdk_ENABLE_INSTALL)
    install(DIRECTORY include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
endif()

set(ramses-shared-lib-renderer-MIXIN
    ${ramses-shared-lib-renderer-MIXIN}
    INCLUDE_PATHS ${RAMSES_RENDERER_API_INCLUDE_BASE}
    CACHE INTERNAL "")
