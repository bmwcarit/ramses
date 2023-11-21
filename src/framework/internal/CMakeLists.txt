#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if (ramses-sdk_ENABLE_TCP_SUPPORT)
    list(APPEND FRAMEWORK_INTERNAL_SOURCES  Communication/TransportTCP/*.h
                                            Communication/TransportTCP/*.cpp)
    list(APPEND FRAMEWORK_INTERNAL_LIBS     asio)
endif()

if(ramses-sdk_HAS_DLT)
    list(APPEND FRAMEWORK_INTERNAL_SOURCES  DltLogAppender/DltAdapterImpl/*.h
                                            DltLogAppender/DltAdapterImpl/*.cpp)
    list(APPEND FRAMEWORK_INTERNAL_LIBS     automotive-dlt)
endif()

find_package(AndroidSDK)
if(AndroidSDK_FOUND)
    list(APPEND FRAMEWORK_INTERNAL_SOURCES  Core/Utils/AndroidLogger/*.h
                                            Core/Utils/AndroidLogger/*.cpp)
    list(APPEND FRAMEWORK_INTERNAL_LIBS     AndroidSDK)
endif()

createModule(
    NAME                    ramses-framework-internal
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    INCLUDE_PATHS           ${PROJECT_SOURCE_DIR}/src/framework
    # internal
    SRC_FILES               ${FRAMEWORK_INTERNAL_SOURCES}
    SRC_FILES               Watchdog/*.h
                            Watchdog/*.cpp
    SRC_FILES               PlatformAbstraction/*.h
                            PlatformAbstraction/Collections/*.h
                            PlatformAbstraction/*.h
                            PlatformAbstraction/*.cpp
    SRC_FILES               Core/Math3d/*.h
                            Core/Math3d/*.cpp
    SRC_FILES               Core/Utils/*.h
                            Core/Utils/*.cpp
    SRC_FILES               Core/Common/*.h
                            Core/Common/*.cpp
    SRC_FILES               Core/TaskFramework/*.h
                            Core/TaskFramework/*.cpp
    SRC_FILES               SceneGraph/SceneAPI/*.h
                            SceneGraph/SceneAPI/*.cpp
    SRC_FILES               SceneGraph/Scene/*.h
                            SceneGraph/Scene/*.cpp
    SRC_FILES               SceneGraph/Resource/*.h
                            SceneGraph/Resource/*.cpp
    SRC_FILES               SceneGraph/SceneUtils/*.h
                            SceneGraph/SceneUtils/*.cpp
    SRC_FILES               Communication/TransportCommon/*.h
                            Communication/TransportCommon/*.cpp
    SRC_FILES               Ramsh/*.h
                            Ramsh/*.cpp
    SRC_FILES               Components/*.h
                            Components/*.cpp
    SRC_FILES               DltLogAppender/*.h
                            DltLogAppender/*.cpp
                            DltLogAppender/src/*.cpp
    SRC_FILES               SceneReferencing/*.h
                            SceneReferencing/*.cpp

    DEPENDENCIES            ramses-api
                            ramses-common-base
                            lz4
                            fmt::fmt
                            ramses-abseil
                            lodepng
                            cityhash
                            ${FRAMEWORK_INTERNAL_LIBS}
    )

if (ramses-sdk_ENABLE_TCP_SUPPORT)
  message(STATUS "+ TCP communication system support enabled")
  target_compile_definitions(ramses-framework-internal PUBLIC "-DHAS_TCP_COMM=1")
else()
  message(STATUS "- TCP communication system support disabled")
endif()

if (ramses-sdk_HAS_DLT)
    target_compile_definitions(ramses-framework-internal PUBLIC "-DDLT_ENABLED")

    if (automotive-dlt_HAS_FILETRANSFER)
        target_compile_definitions(ramses-framework-internal PUBLIC "-DDLT_HAS_FILETRANSFER")
    endif()
endif()

if(ramses-sdk_USE_LINUX_DEV_PTP)
    target_compile_definitions(ramses-framework-internal PUBLIC "-DRAMSES_LINUX_USE_DEV_PTP=1")
endif()

# Thread priority and binding for worker threads
if (DEFINED ramses-sdk_WORKER_THREAD_PRIORITY AND NOT ramses-sdk_WORKER_THREAD_PRIORITY STREQUAL "")
    target_compile_definitions(ramses-framework-internal PRIVATE "-DRAMSES_WORKER_THREAD_PRIORITY=${ramses-sdk_WORKER_THREAD_PRIORITY}")
endif()

if (DEFINED ramses-sdk_WORKER_THREAD_CORE_BINDING AND NOT ramses-sdk_WORKER_THREAD_CORE_BINDING STREQUAL "")
    target_compile_definitions(ramses-framework-internal PRIVATE "-DRAMSES_WORKER_THREAD_CORE_BINDING=${ramses-sdk_WORKER_THREAD_CORE_BINDING}")
endif()

if (DEFINED ramses-sdk_CONN_KEEPALIVE_THREAD_CORE_BINDING AND NOT ramses-sdk_CONN_KEEPALIVE_THREAD_CORE_BINDING STREQUAL "")
    target_compile_definitions(ramses-framework-internal PRIVATE "-DRAMSES_CONN_KEEPALIVE_THREAD_CORE_BINDING=${ramses-sdk_CONN_KEEPALIVE_THREAD_CORE_BINDING}")
endif()


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-framework-internal PRIVATE "-Wsuggest-override")
endif()
