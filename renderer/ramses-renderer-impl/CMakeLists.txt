#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB
    RAMSES_RENDERER_FILES_SOURCE
    src/*.cpp)

createModule(
    NAME                    ramses-renderer-impl
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    INCLUDE_PATHS           include
    SRC_FILES               include/*.h
                            ${RAMSES_RENDERER_FILES_SOURCE}

    DEPENDENCIES            PlatformFactory
)

set(ramses-shared-lib-renderer-MIXIN
    ${ramses-shared-lib-renderer-MIXIN}
    SRC_FILES ${RAMSES_RENDERER_FILES_SOURCE}
    DEPENDENCIES  ramses-renderer-lib
    CACHE INTERNAL "")


IF (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    ramses-renderer-impl-test
        TYPE                    BINARY
        SRC_FILES               test/*.h
                                test/*.cpp
        DEPENDENCIES            ramses-client
                                ramses-renderer-impl
                                ramses-gmock-main
                                RendererTestCommon
    )

    makeTestFromTarget(
        TARGET ramses-renderer-impl-test
        SUFFIX UNITTEST)
endif()
