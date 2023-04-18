#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set(RendererTests-resources_MIXIN
    RESOURCE_FOLDERS        ../../TestContent/res
                            res

)

expandModuleByPlatform(
    NAME            RenderingTests
    TYPE            BINARY
    LINKED_TYPE     STATIC
    ENABLE_INSTALL  ON
    INCLUDE_PATHS           RendererTestFramework
                            RenderingTests
    SRC_FILES               RenderingTests/*.h
                            RendererTestFramework/*.h
                            RenderingTests/*.cpp
                            RendererTestFramework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            TestContent FrameworkTestUtils RendererTestUtils ramses-cli
)

expandModuleByPlatform(
    NAME            RendererLifecycleTests
    TYPE            BINARY
    LINKED_TYPE     STATIC
    ENABLE_INSTALL  ON

    INCLUDE_PATHS           RendererLifecycleTests
                            RendererTestFramework

    SRC_FILES               RendererLifecycleTests/*.h
                            RendererTestFramework/*.h

                            RendererLifecycleTests/main.cpp
                            RendererLifecycleTests/RendererLifecycleTests.cpp
                            RendererTestFramework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            TestContent FrameworkTestUtils RendererTestUtils
                            ramses-gmock-main
                            ramses-cli
)

# TODO Fix this...
if(TARGET RendererLifecycleTests-windows-wgl-4-2-core)
    target_sources(RendererLifecycleTests-windows-wgl-4-2-core PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    folderizeTarget(RendererLifecycleTests-windows-wgl-4-2-core)
endif()

if(TARGET RendererLifecycleTests-windows-wgl-4-5)
    target_sources(RendererLifecycleTests-windows-wgl-4-5 PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    folderizeTarget(RendererLifecycleTests-windows-wgl-4-5)
endif()

if(TARGET RendererLifecycleTests-windows-wgl-es-3-0)
    target_sources(RendererLifecycleTests-windows-wgl-es-3-0 PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    folderizeTarget(RendererLifecycleTests-windows-wgl-es-3-0)
endif()

if(TARGET RendererLifecycleTests-x11-egl-es-3-0)
    target_sources(RendererLifecycleTests-x11-egl-es-3-0 PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    folderizeTarget(RendererLifecycleTests-x11-egl-es-3-0)
endif()

if(TARGET platform-wayland-ivi-egl-es-3-0)
    createModule(
        NAME                    EmbeddedCompositingTests-wayland-ivi-egl-es-3-0
        TYPE                    BINARY
        ENABLE_INSTALL          ON

        INCLUDE_PATHS           RendererTestFramework
                                EmbeddedCompositingTests
                                EmbeddedCompositingTests/EmbeddedCompositingTestFramework
                                EmbeddedCompositingTests/TestWaylandApplication
                                EmbeddedCompositingTests/TestCases

        SRC_FILES               EmbeddedCompositingTests/EmbeddedCompositingTestFramework/*.h
                                EmbeddedCompositingTests/TestWaylandApplication/*.h
                                EmbeddedCompositingTests/TestCases/*.h

                                RendererTestFramework/*.cpp
                                EmbeddedCompositingTests/EmbeddedCompositingTestFramework/*.cpp
                                EmbeddedCompositingTests/TestWaylandApplication/*.cpp
                                EmbeddedCompositingTests/TestCases/*.cpp

        ${RendererTests-resources_MIXIN}

        DEPENDENCIES            ramses-renderer-lib
                                platform-wayland-ivi-egl-es-3-0
                                TestContent
                                FrameworkTestUtils
                                RendererTestUtils
                                WaylandUtilities
                                ramses-cli
    )

    # TODO(tobias/mohamed) Disable test broken by newer ubuntu 18.04 packages
    makeTestFromTarget(TARGET EmbeddedCompositingTests-wayland-ivi-egl-es-3-0
            SUFFIX RNDSANDWICHTEST_SWRAST
            EXTRA_ARGS --ivi-layer 3 --ivi-control --filter-out ShowStreamTextureAfterChangingSurfaceSize
            SKIPPABLE
    )
endif()

expandModuleByPlatform(
    NAME            DmaOffscreenBufferRenderingTests
    TYPE            BINARY
    LINKED_TYPE     STATIC
    ENABLE_INSTALL  ON

    INCLUDE_PATHS           RendererTestFramework
                            DmaOffscreenBufferRenderingTests

    SRC_FILES               DmaOffscreenBufferRenderingTests/*.h
                            RendererTestFramework/*.h

                            DmaOffscreenBufferRenderingTests/*.cpp
                            RendererTestFramework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            TestContent
                            FrameworkTestUtils
                            RendererTestUtils
                            Device_EGL_Extension
                            ramses-cli
)

# Tests which are supposed to be run with valgrind
SET(VALGRIND_PLATFORMS
    "wayland-ivi-egl-es-3-0"
)

# Tests which are supposed to be run in the gate build job
SET(GATE_PLATFORMS
    "x11-egl-es-3-0"
    "wayland-ivi-egl-es-3-0"
)

if (ramses-sdk_BUILD_TESTS)
    foreach(TEST_PLATFORM ${PLATFORM_LIST})
        if(TARGET platform-${TEST_PLATFORM})
            makeTestFromTarget(
                TARGET RenderingTests-${TEST_PLATFORM}
                SUFFIX RNDSANDWICHTEST
                EXTRA_ARGS --ivi-control --ivi-layer 3
            )

            makeTestFromTarget(
                TARGET RendererLifecycleTests-${TEST_PLATFORM}
                SUFFIX RNDSANDWICHTEST_SWRAST
                EXTRA_ARGS --ivi-control --ivi-layer 3
            )
            # These tests don't work generally on emulated ES drivers
            #    MSAA and anisotropic filters are not strictly specified by OpenGL standard -> emulated driver don't support it
            #    ETC2 and ASTC compressed formats don't work with glTexSubImage()/glTexStorage() in the software emulated MESA drivers (reason: unknown, probably bug)
            #    16 bit float texture formats cause crash in docker environment
            SET(SW_EMULATION_FILTER "AntiAliasingTest_MSAA4:TextureTest_Texture2D_AnisotropicFilter:TextureTest_TextureCube_AnisotropicFilter:RenderBuffer_Msaa:TextureTest_Texture2D_Sampling_MinLinearMagNearest:TextureTest_Texture2D_Sampling_MinNearestMagLinear:TextureTest_Texture2D_Format_ETC2RGB:TextureTest_Texture2D_Format_ETC2RGBA:TextureTest_Texture2D_CompressedMipMap:TextureTest_Texture2D_Format_ASTC_RGBA_4x4:TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4:RenderTarget_Format_R16F:RenderTarget_Format_RG16F:RenderTarget_Format_RGB16F:RenderTarget_Format_RGBA16F:RenderTarget_Format_RGB32F:TextureTest_Texture2D_Format_R16F:TextureTest_Texture2D_Format_RG16F:TextureTest_Texture2D_Format_RGB16F:TextureTest_Texture2D_Format_RGBA16F:GeometryShader:OffscreenBufferLinkTest_MSAA:OffscreenBufferLinkTest_MSAAUnlinked")

            LIST(FIND VALGRIND_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                # A limited set of rendering tests executed in the gate (executing all tests takes too long)
                makeTestFromTarget(
                    TARGET RenderingTests-${TEST_PLATFORM}
                    SUFFIX RNDSANDWICHTEST_VALGRINDGATE
                    EXTRA_ARGS --ivi-control --ivi-layer 3 --filter-in TextTest_AccurateText:TextureTest_Texture2D_AddressMode
                )
                # A limited set of renderer lifecycle tests executed in the gate (executing all tests takes too long)
                makeTestFromTarget(
                    TARGET RendererLifecycleTests-${TEST_PLATFORM}
                    SUFFIX RNDSANDWICHTEST_VALGRINDGATE
                    EXTRA_ARGS --ivi-control --ivi-layer 3 --gtest_filter=*DestroyRenderer_ChangeScene_ThenRecreateRenderer*:*RecreateSceneWithSameId*
                )
                # A limited set of EC tests executed in the gate (executing all tests takes too long)
                makeTestFromTarget(
                    TARGET EmbeddedCompositingTests-${TEST_PLATFORM}
                    SUFFIX RNDSANDWICHTEST_VALGRINDGATE
                    EXTRA_ARGS --ivi-control --ivi-layer 3 --filter-in ShowStreamTextureAfterRecreatingScene:Recreated:ClientCanBindMultipleTimes
                )
            ENDIF()

            LIST(FIND GATE_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                makeTestFromTarget(
                    TARGET RenderingTests-${TEST_PLATFORM}
                    SUFFIX RNDSANDWICHTEST_SWRAST
                    EXTRA_ARGS --ivi-control --ivi-layer 3 --filter-out ${SW_EMULATION_FILTER}
                )
            ENDIF()
        ENDIF()
    ENDFOREACH()

    if (TARGET RenderingTests-wayland-ivi-egl-es-3-0)
        # Disable LSAN leak detection inside ASAN because it segfaults on destruction for this test only.
        # Crash seems to be caused when traversing DTLS info for a potentially unmapped thread stack.
        message(STATUS "Disable leak detection for RenderingTests-wayland-ivi-egl-es-3-0_RNDSANDWICHTEST_SWRAST")

        get_test_property(RenderingTests-wayland-ivi-egl-es-3-0_RNDSANDWICHTEST_SWRAST ENVIRONMENT CURRENT_ENV)
        set_tests_properties(RenderingTests-wayland-ivi-egl-es-3-0_RNDSANDWICHTEST_SWRAST
            PROPERTIES ENVIRONMENT "ASAN_OPTIONS=$ENV{ASAN_OPTIONS}:detect_leaks=0;${CURRENT_ENV}")
    endif()

ENDIF()
