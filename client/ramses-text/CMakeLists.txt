#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB
    RAMSES_TEXT_FILES_SOURCE
    impl/src/*.cpp
    ramses-text-api/*.cpp)

file(GLOB
    RAMSES_TEXT_API_INCLUDE_BASE
    ../ramses-text-api/include)

ACME_MODULE(
    NAME                    ramses-text
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    INCLUDE_BASE            impl/include

    FILES_PRIVATE_HEADER    impl/include/ramses-text/*.h

    FILES_SOURCE            ${RAMSES_TEXT_FILES_SOURCE}

    DEPENDENCIES            ramses-text-api
                            ramses-client
                            freetype
                            harfbuzz
)

ACME_MODULE(
    NAME                    ramses-text-test
    TYPE                    TEST
    ENABLE_INSTALL          ON

    INCLUDE_BASE            include

    FILES_PRIVATE_HEADER    test/*.h

    FILES_SOURCE            test/*.cpp

    FILES_RESOURCE          res/*

    DEPENDENCIES            ramses-text
                            gmock_main
)

set(ramses-shared-lib-MIXIN
    ${ramses-shared-lib-MIXIN}
    INCLUDE_BASE ${RAMSES_TEXT_API_INCLUDE_BASE}
    FILES_SOURCE ${RAMSES_TEXT_FILES_SOURCE}
    DEPENDENCIES  ramses-text ramses-text-api
    CACHE INTERNAL "")
