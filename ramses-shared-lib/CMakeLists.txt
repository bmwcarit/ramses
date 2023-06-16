#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

ADD_DEFINITIONS(-DRAMSES_LINK_SHARED_EXPORT)

if (ramses-sdk_BUILD_FULL_SHARED_LIB)
    createModuleWithRenderer(
        NAME            ramses-shared-lib
        TYPE            SHARED_LIBRARY
        ENABLE_INSTALL  ON
        ${ramses-shared-lib-MIXIN}
        ${ramses-shared-lib-renderer-MIXIN}
    )

    target_link_libraries(ramses-shared-lib PUBLIC glm::glm)
endif()

if (ramses-sdk_BUILD_HEADLESS_SHARED_LIB)

    # build shared library without renderer
    createModule(
        NAME                    ramses-shared-lib-headless
        TYPE                    SHARED_LIBRARY
        ENABLE_INSTALL          ON

        ${ramses-shared-lib-MIXIN}
        )

    target_link_libraries(ramses-shared-lib-headless PUBLIC glm::glm)
endif()
