#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


# RAMSES compiles the .cpp for its API for framework, client and renderer into static libs.
#
# For shared libs, the .cpp for API need to be re-compiled with API EXPORT enabled,
# which changes the API headers effectively, by declaring API functions and classes
# to be exported (visible on linux systems, or available in DLL) into the shared libs.
#
# To accomplish that, this modules 1st collects all the relevant .cpp files declared
# in impl folders in framework, client and renderer (depending on configured cmake options)
# and compiles them in the modules of the shared libs.

set(LIB_TYPE "SHARED_LIBRARY")

# Sanitizers can not handles shared libs properly, so in case of building for a sanitizer (for testing/debugging)
# force override lib type to be a static lib
if (ramses-sdk_ENABLE_SANITIZER STREQUAL "asan" OR
    ramses-sdk_ENABLE_SANITIZER STREQUAL "ubsan" OR
    ramses-sdk_ENABLE_SANITIZER STREQUAL "asan+ubsan")
    message(WARNING "Static library type not supported for ramses-shared-lib in general, switching to static lib for address sanitizer to work correctly.")
    set(LIB_TYPE "STATIC_LIBRARY")
endif()

# Collect impl.cpp for framework and client depending on configured cmake options
list(APPEND HEADLESS_SOURCES        ${PROJECT_SOURCE_DIR}/src/framework/impl/*.cpp)
list(APPEND HEADLESS_SOURCES        ${PROJECT_SOURCE_DIR}/src/client/impl/*.cpp)
if(ramses-sdk_TEXT_SUPPORT)
    list(APPEND HEADLESS_SOURCES    ${PROJECT_SOURCE_DIR}/src/client/impl/text/*.cpp)
endif()

if(ramses-sdk_ENABLE_LOGIC)
    list(APPEND HEADLESS_SOURCES    ${PROJECT_SOURCE_DIR}/src/client/impl/logic/*.cpp)
endif()

set(RENDERER_SOURCES                ${PROJECT_SOURCE_DIR}/src/renderer/impl/*.cpp)

# build shared library without renderer
createModule(
    NAME                    ramses-shared-lib-headless
    TYPE                    ${LIB_TYPE}
    ENABLE_INSTALL          ON
    SRC_FILES               ${HEADLESS_SOURCES}
    )

target_link_libraries(ramses-shared-lib-headless PUBLIC glm::glm)
target_link_libraries(ramses-shared-lib-headless PRIVATE ramses-client-internal)
target_link_libraries(ramses-shared-lib-headless INTERFACE ramses-api)
if(${LIB_TYPE} STREQUAL "SHARED_LIBRARY")
    target_compile_definitions(ramses-shared-lib-headless PUBLIC RAMSES_LINK_SHARED_EXPORT)
endif()

if (ramses-sdk_BUILD_FULL_SHARED_LIB)
    createModule(
        NAME                    ramses-shared-lib
        TYPE                    ${LIB_TYPE}
        ENABLE_INSTALL          ON
        SRC_FILES               ${RENDERER_SOURCES}
        DEPENDENCIES            Platform
    )

    target_link_libraries(ramses-shared-lib PUBLIC ramses-shared-lib-headless)
endif()
