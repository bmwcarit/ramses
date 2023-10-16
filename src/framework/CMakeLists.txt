#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if (ramses-sdk_ENABLE_TCP_SUPPORT)
    list(APPEND FRAMEWORK_SOURCES   internal/Communication/TransportTCP/*.h
                                    internal/Communication/TransportTCP/*.cpp)
    list(APPEND FRAMEWORK_LIBS      asio)
endif()

if(ramses-sdk_HAS_DLT)
    list(APPEND FRAMEWORK_SOURCES   internal/DltLogAppender/DltAdapterImpl/*.h
                                    internal/DltLogAppender/DltAdapterImpl/*.cpp)
    list(APPEND FRAMEWORK_LIBS      automotive-dlt)
endif()

find_package(AndroidSDK)
if(AndroidSDK_FOUND)
    list(APPEND FRAMEWORK_SOURCES   internal/Core/Utils/AndroidLogger/*.h
                                    internal/Core/Utils/AndroidLogger/*.cpp)
    list(APPEND FRAMEWORK_LIBS      AndroidSDK)
endif()

createModule(
    NAME                    ramses-framework
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    INCLUDE_PATHS           .

    # impl
    SRC_FILES               impl/*.h
                            impl/*.cpp
    # internal
    SRC_FILES               ${FRAMEWORK_SOURCES}
    SRC_FILES               internal/Watchdog/*.h
                            internal/Watchdog/*.cpp
    SRC_FILES               internal/PlatformAbstraction/*.h
                            internal/PlatformAbstraction/Collections/*.h
                            internal/PlatformAbstraction/internal/*.h
                            internal/PlatformAbstraction/*.cpp
    SRC_FILES               internal/Core/Math3d/*.h
                            internal/Core/Math3d/*.cpp
    SRC_FILES               internal/Core/Utils/*.h
                            internal/Core/Utils/*.cpp
    SRC_FILES               internal/Core/Common/*.h
                            internal/Core/Common/*.cpp
    SRC_FILES               internal/Core/TaskFramework/*.h
                            internal/Core/TaskFramework/*.cpp
    SRC_FILES               internal/SceneGraph/SceneAPI/*.h
                            internal/SceneGraph/SceneAPI/*.cpp
    SRC_FILES               internal/SceneGraph/Scene/*.h
                            internal/SceneGraph/Scene/*.cpp
    SRC_FILES               internal/SceneGraph/Resource/*.h
                            internal/SceneGraph/Resource/*.cpp
    SRC_FILES               internal/SceneGraph/SceneUtils/*.h
                            internal/SceneGraph/SceneUtils/*.cpp
    SRC_FILES               internal/Communication/TransportCommon/*.h
                            internal/Communication/TransportCommon/*.cpp
    SRC_FILES               internal/Ramsh/*.h
                            internal/Ramsh/*.cpp
    SRC_FILES               internal/Components/*.h
                            internal/Components/*.cpp
    SRC_FILES               internal/DltLogAppender/*.h
                            internal/DltLogAppender/*.cpp
                            internal/DltLogAppender/src/*.cpp
    SRC_FILES               internal/SceneReferencing/*.h
                            internal/SceneReferencing/*.cpp

    DEPENDENCIES            ramses-api
                            ramses-common-base
                            lz4
                            fmt::fmt
                            ramses-abseil
                            lodepng
                            cityhash
                            ${FRAMEWORK_LIBS}
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
