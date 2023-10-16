#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    RendererTestUtils
        TYPE                    STATIC_LIBRARY

        INCLUDE_PATHS           .
        SRC_FILES               *.h
                                *.cpp
        DEPENDENCIES            ramses-renderer
                                ramses-client
                                renderer-test-common
    )
endif()
