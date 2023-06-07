#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB
    RAMSES_FRAMEWORK_FILES_SOURCE
    ramses-framework/src/*.cpp)
file(GLOB
    RAMSES_FRAMEWORK_API_INCLUDE_BASE
    ramses-framework-api/include)

add_library(ramses-framework-api INTERFACE)
target_include_directories(ramses-framework-api INTERFACE ${RAMSES_FRAMEWORK_API_INCLUDE_BASE})

if (ramses-sdk_ENABLE_TCP_SUPPORT)
    set(ramses-framework-TCP_MIXIN
        DEPENDENCIES            asio

        INCLUDE_PATHS           Communication/TransportTCP/include
        SRC_FILES               Communication/TransportTCP/include/TransportTCP/*.h
                                Communication/TransportTCP/src/*.cpp)

    set(ramses-framework-test-TCP_MIXIN
        INCLUDE_PATHS           Communication/TransportTCP/test
        SRC_FILES               Communication/TransportTCP/test/*.h
                                Communication/TransportTCP/test/*.cpp)
endif()

if(ramses-sdk_HAS_DLT)
    set(ramses-framework-DLT_MIXIN
        DEPENDENCIES            automotive-dlt

        SRC_FILES               DltLogAppender/include/DltLogAppender/DltAdapterImpl/*.h
                                DltLogAppender/src/DltAdapterImpl/*.cpp)
endif()

find_package(AndroidSDK)
if (AndroidSDK_FOUND)
  set(ramses-framework-AndroidLogger_MIXIN
    DEPENDENCIES            AndroidSDK

    INCLUDE_PATHS           Core/Utils/AndroidLogger/include
    SRC_FILES               Core/Utils/AndroidLogger/include/*.h
                            Core/Utils/AndroidLogger/src/*.cpp
    )
endif()

createModule(
    NAME                    ramses-framework
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    # public api
    SRC_FILES               ramses-framework-api/include/*.h
                            ramses-framework-api/include/ramses-framework-api/*.h

    # Internals
    INCLUDE_PATHS           ramses-framework/include
    SRC_FILES               ramses-framework/include/*.h
                            ramses-framework/include/ramses-framework/*.h
                            ${RAMSES_FRAMEWORK_FILES_SOURCE}
    INCLUDE_PATHS           Watchdog/include
    SRC_FILES               Watchdog/include/Watchdog/*.h
                            Watchdog/src/*.cpp
    INCLUDE_PATHS           PlatformAbstraction/include
    SRC_FILES               PlatformAbstraction/include/*.h
                            PlatformAbstraction/include/Collections/*.h
                            PlatformAbstraction/include/PlatformAbstraction/*.h
                            PlatformAbstraction/include/PlatformAbstraction/internal/*.h
                            PlatformAbstraction/src/*.cpp
    INCLUDE_PATHS           Core/Math3d/include
    SRC_FILES               Core/Math3d/include/Math3d/*.h
                            Core/Math3d/src/*.cpp
    INCLUDE_PATHS           Core/Utils/include
    SRC_FILES               Core/Utils/include/Utils/*.h
                            Core/Utils/src/*.cpp
    INCLUDE_PATHS           Core/Common/include
    SRC_FILES               Core/Common/include/Common/*.h
                            Core/Common/src/*.cpp
    INCLUDE_PATHS           Core/TaskFramework/include
    SRC_FILES               Core/TaskFramework/include/*.h
                            Core/TaskFramework/include/TaskFramework/*.h
                            Core/TaskFramework/src/*.cpp
    INCLUDE_PATHS           SceneGraph/SceneAPI/include
    SRC_FILES               SceneGraph/SceneAPI/include/SceneAPI/*.h
                            SceneGraph/SceneAPI/src/*.cpp
    INCLUDE_PATHS           SceneGraph/Scene/include
    SRC_FILES               SceneGraph/Scene/include/Scene/*.h
                            SceneGraph/Scene/include/Scene/*/*.h
                            SceneGraph/Scene/src/*.cpp
                            SceneGraph/Scene/src/*/*.cpp
    INCLUDE_PATHS           SceneGraph/Resource/include
    SRC_FILES               SceneGraph/Resource/include/Resource/*.h
                            SceneGraph/Resource/src/*.cpp
    INCLUDE_PATHS           SceneGraph/SceneUtils/include
    SRC_FILES               SceneGraph/SceneUtils/include/SceneUtils/*.h
                            SceneGraph/SceneUtils/src/*.cpp
    INCLUDE_PATHS           Communication/TransportCommon/include
    SRC_FILES               Communication/TransportCommon/include/TransportCommon/*.h
                            Communication/TransportCommon/src/*.cpp
    INCLUDE_PATHS           Ramsh/include
    SRC_FILES               Ramsh/include/Ramsh/*.h
                            Ramsh/src/*.cpp
    INCLUDE_PATHS           Components/include
    SRC_FILES               Components/include/*.h
                            Components/include/Components/*.h
                            Components/src/*.cpp
    INCLUDE_PATHS           DltLogAppender/include
    SRC_FILES               DltLogAppender/include/DltLogAppender/*.h
                            DltLogAppender/include/DltLogAppender/DltAdapterDummy/*.h
                            DltLogAppender/src/DltAdapterDummy/*.cpp
                            DltLogAppender/src/*.cpp
    INCLUDE_PATHS           SceneReferencing/include
    SRC_FILES               SceneReferencing/include/SceneReferencing/*.h
                            SceneReferencing/src/*.cpp

    # TODO create targets for these...
    # cityhash from external
    INCLUDE_PATHS           ../external/cityhash/src
    SRC_FILES               ../external/cityhash/src/*.h
                            ../external/cityhash/src/city.cc
    # lodepng from external
    INCLUDE_PATHS           ../external/lodepng
    SRC_FILES               ../external/lodepng/*.h
                            ../external/lodepng/lodepng.cpp

    DEPENDENCIES            ramses-framework-api
                            ramses-common-base
                            lz4
                            fmt::fmt
                            glm::glm
                            ramses-abseil
    # conditional values
    ${ramses-framework-TCP_MIXIN}
    ${ramses-framework-DLT_MIXIN}
    ${ramses-framework-AndroidLogger_MIXIN}
    )

if (ramses-sdk_ENABLE_TCP_SUPPORT)
  message(STATUS "+ TCP communication system support enabled")
  target_compile_definitions(ramses-framework PUBLIC "-DHAS_TCP_COMM=1")
else()
  message(STATUS "- TCP communication system support disabled")
endif()

if (ramses-sdk_HAS_DLT)
    target_compile_definitions(ramses-framework PUBLIC "-DDLT_ENABLED")

    if (automotive-dlt_HAS_FILETRANSFER)
        target_compile_definitions(ramses-framework PUBLIC "-DDLT_HAS_FILETRANSFER")
    endif()
endif()

if(ramses-sdk_USE_LINUX_DEV_PTP)
    target_compile_definitions(ramses-framework PUBLIC "-DRAMSES_LINUX_USE_DEV_PTP=1")
endif()

# Thread priority and binding for worker threads
if (DEFINED ramses-sdk_WORKER_THREAD_PRIORITY AND NOT ramses-sdk_WORKER_THREAD_PRIORITY STREQUAL "")
    target_compile_definitions(ramses-framework PRIVATE "-DRAMSES_WORKER_THREAD_PRIORITY=${ramses-sdk_WORKER_THREAD_PRIORITY}")
endif()

if (DEFINED ramses-sdk_WORKER_THREAD_CORE_BINDING AND NOT ramses-sdk_WORKER_THREAD_CORE_BINDING STREQUAL "")
    target_compile_definitions(ramses-framework PRIVATE "-DRAMSES_WORKER_THREAD_CORE_BINDING=${ramses-sdk_WORKER_THREAD_CORE_BINDING}")
endif()

if (DEFINED ramses-sdk_CONN_KEEPALIVE_THREAD_CORE_BINDING AND NOT ramses-sdk_CONN_KEEPALIVE_THREAD_CORE_BINDING STREQUAL "")
    target_compile_definitions(ramses-framework PRIVATE "-DRAMSES_CONN_KEEPALIVE_THREAD_CORE_BINDING=${ramses-sdk_CONN_KEEPALIVE_THREAD_CORE_BINDING}")
endif()


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-framework PRIVATE "-Wsuggest-override")
endif()

if(ramses-sdk_ENABLE_INSTALL)
    install(DIRECTORY ramses-framework-api/include/ DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}" COMPONENT ramses-sdk-devel)
endif()

set(ramses-shared-lib-MIXIN
    ${ramses-shared-lib-MIXIN}
    INCLUDE_PATHS ${RAMSES_FRAMEWORK_API_INCLUDE_BASE}
    SRC_FILES ${RAMSES_FRAMEWORK_FILES_SOURCE}
    DEPENDENCIES  ramses-framework
    CACHE INTERNAL "")

if (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    FrameworkTestUtils
        TYPE                    STATIC_LIBRARY
        INCLUDE_PATHS           FrameworkTestUtils/include
        SRC_FILES               FrameworkTestUtils/include/*.h
                                FrameworkTestUtils/src/*.cpp
        DEPENDENCIES            ramses-framework
                                ramses-gmock
    )

    # all tests in one binary
    createModule(
        NAME                    ramses-framework-test
        TYPE                    BINARY

        INCLUDE_PATHS           PlatformAbstraction/test
        SRC_FILES               PlatformAbstraction/test/*.h
                                PlatformAbstraction/test/*.cpp
        INCLUDE_PATHS           ramses-framework/test
        SRC_FILES               ramses-framework/test/*.h
                                ramses-framework/test/*.cpp
        INCLUDE_PATHS           Core/Math3d/test
        SRC_FILES               Core/Math3d/test/*.h
                                Core/Math3d/test/*.cpp
        INCLUDE_PATHS           Core/Utils/test
        SRC_FILES               Core/Utils/test/*.h
                                Core/Utils/test/*.cpp
        SRC_FILES               Core/Common/test/*.h
                                Core/Common/test/*.cpp
        INCLUDE_PATHS           Core/TaskFramework/test
        SRC_FILES               Core/TaskFramework/test/*.h
                                Core/TaskFramework/test/*.cpp
        INCLUDE_PATHS           SceneGraph/Scene/test
        SRC_FILES               SceneGraph/Scene/test/*.h
                                SceneGraph/Scene/test/*.cpp
        INCLUDE_PATHS           SceneGraph/Resource/test
        SRC_FILES               SceneGraph/Resource/test/*.h
                                SceneGraph/Resource/test/*.cpp
        INCLUDE_PATHS           SceneGraph/SceneAPI/test
        SRC_FILES               SceneGraph/SceneAPI/test/*.h
                                SceneGraph/SceneAPI/test/*.cpp
        INCLUDE_PATHS           Communication/TransportCommon/test
        SRC_FILES               Communication/TransportCommon/test/*.h
                                Communication/TransportCommon/test/*.cpp
        INCLUDE_PATHS           Ramsh/test
        SRC_FILES               Ramsh/test/*.h
                                Ramsh/test/*.cpp
        INCLUDE_PATHS           Components/test
        SRC_FILES               Components/test/*.h
                                Components/test/*.cpp
        INCLUDE_PATHS           DltLogAppender/test
        SRC_FILES               DltLogAppender/test/*.h
                                DltLogAppender/test/*.cpp
        INCLUDE_PATHS           Watchdog/test
        SRC_FILES               Watchdog/test/*.h
                                Watchdog/test/*.cpp
        INCLUDE_PATHS           SceneReferencing/test
        SRC_FILES               SceneReferencing/test/*.h
                                SceneReferencing/test/*.cpp

        ${ramses-framework-test-TCP_MIXIN}

        SRC_FILES               test/main.cpp
        RESOURCE_FOLDERS        test/res

        DEPENDENCIES            ramses-framework
                                FrameworkTestUtils
                                ramses-gmock-main
    )

    # disable maybe uninitialized
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
            target_compile_options(ramses-framework-test PRIVATE -Wno-maybe-uninitialized)
        endif()
    endif()

    makeTestFromTarget(TARGET ramses-framework-test SUFFIX UNITTEST)
endif()
