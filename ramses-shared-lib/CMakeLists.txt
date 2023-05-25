#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

ADD_DEFINITIONS(-DRAMSES_LINK_SHARED_EXPORT)

if (ramses-sdk_BUILD_FULL_SHARED_LIB)
    expandModuleByPlatform(
        NAME            ramses-shared-lib
        TYPE            SHARED_LIBRARY
        LINKED_TYPE     STATIC
        ENABLE_INSTALL  ON
        ${ramses-shared-lib-MIXIN}
        ${ramses-shared-lib-renderer-MIXIN}
    )

    # TODO Remove this, can be made part of the function call which creates the shared lib
    set_target_properties(ramses-shared-lib PROPERTIES RAMSES_VERSION "${ramses-sdk_VERSION}")
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

    set_target_properties(ramses-shared-lib-headless PROPERTIES RAMSES_VERSION "${ramses-sdk_VERSION}")
    target_link_libraries(ramses-shared-lib-headless PUBLIC glm::glm)

    SET_TARGET_PROPERTIES(ramses-shared-lib-headless PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    GET_TARGET_PROPERTY(MYMODULE_SOVERSION ramses-shared-lib-headless SOVERSION)
    message(STATUS "    setting shared library version to ${MYMODULE_SOVERSION}")

endif()
