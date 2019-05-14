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

IF (ramses-sdk_ENABLE_TCP_SUPPORT)
    SET(ramses-framework-TCP_MIXIN
        INCLUDE_BASE            Communication/TransportTCP/include
        FILES_PRIVATE_HEADER    Communication/TransportTCP/include/TransportTCP/*.h
        FILES_SOURCE            Communication/TransportTCP/src/*.cpp)

    SET(ramses-framework-test-TCP_MIXIN
        INCLUDE_BASE            Communication/TransportTCP/test
        FILES_PRIVATE_HEADER    Communication/TransportTCP/test/*.h
        FILES_SOURCE            Communication/TransportTCP/test/*.cpp)
ENDIF()

IF(ramses-sdk_BUILD_PERFORMANCE_PROFILER_SUPPORT MATCHES VTUNE)
    FIND_PACKAGE(VTune QUIET)
    IF (VTune_FOUND)
        SET(ramses-framework-VTune_MIXIN DEPENDENCIES VTune)
    ENDIF()
ENDIF()

IF(ramses-sdk_HAS_DLT)
    SET(ramses-framework-DLT_MIXIN
        DEPENDENCIES            automotive-dlt

        FILES_PRIVATE_HEADER    DltLogAppender/include/DltLogAppender/DltAdapterImpl/*.h
        FILES_SOURCE            DltLogAppender/src/DltAdapterImpl/*.cpp)
ENDIF()

FIND_PACKAGE(AndroidSDK)
IF (AndroidSDK_FOUND)
  SET(ramses-framework-AndroidLogger_MIXIN
    DEPENDENCIES            AndroidSDK

    INCLUDE_BASE            Core/Utils/AndroidLogger/include
    FILES_PRIVATE_HEADER    Core/Utils/AndroidLogger/include/*.h
    FILES_SOURCE            Core/Utils/AndroidLogger/src/*.cpp
    )
ENDIF()

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-framework
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================

    # public api
    FILES_PRIVATE_HEADER    ramses-framework-api/include/*.h
                            ramses-framework-api/include/ramses-framework-api/*.h
    DEPENDENCIES            ramses-framework-api

    # FrameworkAPI
    INCLUDE_BASE            Watchdog/include
    FILES_PRIVATE_HEADER    Watchdog/include/Watchdog/*.h
    FILES_SOURCE            Watchdog/src/*.cpp

    INCLUDE_BASE            PlatformAbstraction/include
    FILES_PRIVATE_HEADER    PlatformAbstraction/include/*.h
                            PlatformAbstraction/include/Collections/*.h
                            PlatformAbstraction/include/PlatformAbstraction/*.h
                            PlatformAbstraction/include/PlatformAbstraction/arch/*/*.h
    FILES_SOURCE            PlatformAbstraction/src/*.cpp
    DEPENDENCIES            ramses-capu
                            lz4
                            asio

    INCLUDE_BASE            Core/Math3d/include
    FILES_PRIVATE_HEADER    Core/Math3d/include/Math3d/*.h
    FILES_SOURCE            Core/Math3d/src/*.cpp

    INCLUDE_BASE            Core/Utils/include
    FILES_PRIVATE_HEADER    Core/Utils/include/Utils/*.h
    FILES_SOURCE            Core/Utils/src/*.cpp

    INCLUDE_BASE            Core/Transfer/include
    FILES_PRIVATE_HEADER    Core/Transfer/include/Transfer/*.h
    FILES_SOURCE            Core/Transfer/src/*.cpp

    INCLUDE_BASE            Core/Common/include
    FILES_PRIVATE_HEADER    Core/Common/include/Common/*.h
    FILES_SOURCE            Core/Common/src/*.cpp

    INCLUDE_BASE            Core/TaskFramework/include
    FILES_PRIVATE_HEADER    Core/TaskFramework/include/*.h
                            Core/TaskFramework/include/TaskFramework/*.h
    FILES_SOURCE            Core/TaskFramework/src/*.cpp

    # Scenegraph

    INCLUDE_BASE            SceneGraph/SceneAPI/include
    FILES_PRIVATE_HEADER    SceneGraph/SceneAPI/include/SceneAPI/*.h

    INCLUDE_BASE            SceneGraph/Scene/include
    FILES_PRIVATE_HEADER    SceneGraph/Scene/include/Scene/*.h
                            SceneGraph/Scene/include/Scene/*/*.h
    FILES_SOURCE            SceneGraph/Scene/src/*.cpp
                            SceneGraph/Scene/src/*/*.cpp

    INCLUDE_BASE            SceneGraph/Resource/include
    FILES_PRIVATE_HEADER    SceneGraph/Resource/include/Resource/*.h
    FILES_SOURCE            SceneGraph/Resource/src/*.cpp

    INCLUDE_BASE            SceneGraph/SceneUtils/include
    FILES_PRIVATE_HEADER    SceneGraph/SceneUtils/include/SceneUtils/*.h
    FILES_SOURCE            SceneGraph/SceneUtils/src/*.cpp

    INCLUDE_BASE            Collision/include
    FILES_PRIVATE_HEADER    Collision/include/Collision/*.h
    FILES_SOURCE            Collision/src/*.cpp

    INCLUDE_BASE            Animation/Animation/include
    FILES_PRIVATE_HEADER    Animation/Animation/include/Animation/*.h
    FILES_SOURCE            Animation/Animation/src/*.cpp

    INCLUDE_BASE            Animation/AnimationAPI/include
    FILES_PRIVATE_HEADER    Animation/AnimationAPI/include/AnimationAPI/*.h

    INCLUDE_BASE            Communication/TransportCommon/include
    FILES_PRIVATE_HEADER    Communication/TransportCommon/include/TransportCommon/*.h
    FILES_SOURCE            Communication/TransportCommon/src/*.cpp

    INCLUDE_BASE            Ramsh/include
    FILES_PRIVATE_HEADER    Ramsh/include/Ramsh/*.h
    FILES_SOURCE            Ramsh/src/*.cpp

    INCLUDE_BASE            Components/include
    FILES_PRIVATE_HEADER    Components/include/*.h
                            Components/include/Components/*.h
    FILES_SOURCE            Components/src/*.cpp

    INCLUDE_BASE            ramses-framework/include
    FILES_PRIVATE_HEADER    ramses-framework/include/*.h
                            ramses-framework/include/ramses-framework/*.h
    FILES_SOURCE            ${RAMSES_FRAMEWORK_FILES_SOURCE}

    INCLUDE_BASE            DltLogAppender/include
    FILES_PRIVATE_HEADER    DltLogAppender/include/DltLogAppender/*.h
                            DltLogAppender/include/DltLogAppender/DltAdapterDummy/*.h
    FILES_SOURCE            DltLogAppender/src/DltAdapterDummy/*.cpp

    FILES_SOURCE            DltLogAppender/src/*.cpp

    INCLUDE_BASE            Monitoring/include
    FILES_PRIVATE_HEADER    Monitoring/include/Monitoring/*.h
    FILES_SOURCE            Monitoring/src/*.cpp

    # cityhash from external
    INCLUDE_BASE            ../external/cityhash/src
    FILES_PRIVATE_HEADER    ../external/cityhash/src/*.h
    FILES_SOURCE            ../external/cityhash/src/city.cc

    # lodepng from external
    INCLUDE_BASE            ../external/lodepng
    FILES_PRIVATE_HEADER    ../external/lodepng/*.h
    FILES_SOURCE            ../external/lodepng/lodepng.cpp

    # conditional values
    ${ramses-framework-TCP_MIXIN}
    ${ramses-framework-VTune_MIXIN}
    ${ramses-framework-DLT_MIXIN}
    ${ramses-framework-AndroidLogger_MIXIN}
)

IF (ramses-sdk_ENABLE_TCP_SUPPORT)
  TARGET_COMPILE_DEFINITIONS(ramses-framework PUBLIC "-DHAS_TCP_COMM=1")
ENDIF()

IF(ramses-sdk_BUILD_PERFORMANCE_PROFILER_SUPPORT MATCHES LOG_OUTPUT)
  TARGET_COMPILE_DEFINITIONS(ramses-framework PUBLIC "-DPERFORMANCE_PROFILER_OUTPUT_TYPE_LOG=1")
ELSEIF(ramses-sdk_BUILD_PERFORMANCE_PROFILER_SUPPORT MATCHES VTUNE)
  TARGET_COMPILE_DEFINITIONS(ramses-framework PUBLIC "-DPERFORMANCE_PROFILER_OUTPUT_TYPE_VTUNE=1")
ENDIF()

IF (ramses-sdk_HAS_DLT)
    TARGET_COMPILE_DEFINITIONS(ramses-framework PUBLIC "-DDLT_ENABLED")

    IF (automotive-dlt_HAS_FILETRANSFER)
        TARGET_COMPILE_DEFINITIONS(ramses-framework PUBLIC "-DDLT_HAS_FILETRANSFER")
    ENDIF()
ENDIF()

IF(ramses-sdk_USE_LINUX_DEV_PTP)
    TARGET_COMPILE_DEFINITIONS(ramses-framework PUBLIC "-DRAMSES_LINUX_USE_DEV_PTP=1")
ENDIF()

if (ramses-sdk_CONSOLE_LOGLEVEL)
    target_compile_definitions(ramses-framework PRIVATE "-DRAMSES_CONSOLE_LOGLEVEL_DEFAULT=${ramses-sdk_CONSOLE_LOGLEVEL}")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ramses-framework PRIVATE "-Wsuggest-override")
endif()

INSTALL(DIRECTORY ramses-framework-api/include/ DESTINATION "${PROJECT_INSTALL_HEADER}" COMPONENT ramses-sdk-devel)

set(ramses-shared-lib-MIXIN
    ${ramses-shared-lib-MIXIN}
    INCLUDE_BASE ${RAMSES_FRAMEWORK_API_INCLUDE_BASE}
    FILES_SOURCE ${RAMSES_FRAMEWORK_FILES_SOURCE}
    DEPENDENCIES  ramses-framework
    CACHE INTERNAL "")

IF (${ramses-sdk_BUILD_TESTS})
    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    FrameworkTestUtils
        TYPE                    STATIC_LIBRARY

        #==========================================================================
        # files of this module
        #==========================================================================

        INCLUDE_BASE            FrameworkTestUtils/include
        FILES_PRIVATE_HEADER    FrameworkTestUtils/include/*.h
        FILES_SOURCE            FrameworkTestUtils/src/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            ramses-framework
                                ramses-gmock
    )
ENDIF()

# all tests in one binary
ACME_MODULE(
    NAME                    ramses-framework-test
    TYPE                    TEST

    INCLUDE_BASE            PlatformAbstraction/test
    FILES_PRIVATE_HEADER    PlatformAbstraction/test/*.h
    FILES_SOURCE            PlatformAbstraction/test/*.cpp

    INCLUDE_BASE            ramses-framework/test
    FILES_PRIVATE_HEADER    ramses-framework/test/*.h
    FILES_SOURCE            ramses-framework/test/*.cpp

    INCLUDE_BASE            Core/Math3d/test
    FILES_PRIVATE_HEADER    Core/Math3d/test/*.h
    FILES_SOURCE            Core/Math3d/test/*.cpp

    INCLUDE_BASE            Core/Utils/test
    FILES_PRIVATE_HEADER    Core/Utils/test/*.h
    FILES_SOURCE            Core/Utils/test/*.cpp
    FILES_RESOURCE          test/res/*

    INCLUDE_BASE            Core/Transfer/test
    FILES_PRIVATE_HEADER    Core/Transfer/test/*.h
    FILES_SOURCE            Core/Transfer/test/*.cpp

    INCLUDE_BASE            Core/Common/test/*.h
    FILES_PRIVATE_HEADER    Core/Common/test
    FILES_SOURCE            Core/Common/test/*.cpp

    INCLUDE_BASE            Core/TaskFramework/test
    FILES_PRIVATE_HEADER    Core/TaskFramework/test/*.h
    FILES_SOURCE            Core/TaskFramework/test/*.cpp

    INCLUDE_BASE            SceneGraph/Scene/test
    FILES_PRIVATE_HEADER    SceneGraph/Scene/test/*.h
    FILES_SOURCE            SceneGraph/Scene/test/*.cpp

    INCLUDE_BASE            SceneGraph/Resource/test
    FILES_PRIVATE_HEADER    SceneGraph/Resource/test/*.h
    FILES_SOURCE            SceneGraph/Resource/test/*.cpp

    INCLUDE_BASE            Effect/test
    FILES_PRIVATE_HEADER    Effect/test/*.h
    FILES_SOURCE            Effect/test/*.cpp

    INCLUDE_BASE            Animation/Animation/test
    FILES_PRIVATE_HEADER    Animation/Animation/test/*.h
    FILES_SOURCE            Animation/Animation/test/*.cpp

    INCLUDE_BASE            Communication/TransportCommon/test
    FILES_PRIVATE_HEADER    Communication/TransportCommon/test/*.h
    FILES_SOURCE            Communication/TransportCommon/test/*.cpp

    INCLUDE_BASE            Ramsh/test
    FILES_PRIVATE_HEADER    Ramsh/test/*.h
    FILES_SOURCE            Ramsh/test/*.cpp

    INCLUDE_BASE            Components/test
    FILES_PRIVATE_HEADER    Components/test/*.h
    FILES_SOURCE            Components/test/*.cpp

    INCLUDE_BASE            Monitoring/test
    FILES_PRIVATE_HEADER    Monitoring/test/*.h
    FILES_SOURCE            Monitoring/test/*.cpp

    INCLUDE_BASE            Collision/test
    FILES_PRIVATE_HEADER    Collision/test/*.h
    FILES_SOURCE            Collision/test/*.cpp

    INCLUDE_BASE            DltLogAppender/test
    FILES_PRIVATE_HEADER    DltLogAppender/test/*.h
    FILES_SOURCE            DltLogAppender/test/*.cpp

    INCLUDE_BASE            Watchdog/test
    FILES_PRIVATE_HEADER    Watchdog/test/*.h
    FILES_SOURCE            Watchdog/test/*.cpp

    ${ramses-framework-test-DLT_MIXIN}
    ${ramses-framework-test-TCP_MIXIN}

    FILES_SOURCE            test/main.cpp

    DEPENDENCIES            ramses-framework
                            FrameworkTestUtils
                            ramses-gmock-main
)
