#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set(RendererTests-resources_MIXIN
    RESOURCE_FOLDER         ../../TestContent/res
                            res

)

RENDERER_MODULE_PER_CONFIG_STATIC(RenderingTests
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    INCLUDE_BASE            RendererTestFramework
                            RenderingTests

    FILES_PRIVATE_HEADER    RenderingTests/*.h
                            RendererTestFramework/*.h

    FILES_SOURCE            RenderingTests/*.cpp
                            RendererTestFramework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            TestContent FrameworkTestUtils RendererTestUtils
)

RENDERER_MODULE_PER_CONFIG_STATIC(RendererLifecycleTests
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    INCLUDE_BASE            RendererLifecycleTests
                            RendererTestFramework

    FILES_PRIVATE_HEADER    RendererLifecycleTests/*.h
                            RendererTestFramework/*.h

    FILES_SOURCE            RendererLifecycleTests/main.cpp
                            RendererLifecycleTests/RendererLifecycleTests.cpp
                            RendererTestFramework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            TestContent FrameworkTestUtils RendererTestUtils
                            ramses-gmock-main
)

if(TARGET RendererLifecycleTests-windows-wgl-4-2-core)
    target_sources(RendererLifecycleTests-windows-wgl-4-2-core PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    ACME_FOLDERIZE_TARGET(RendererLifecycleTests-windows-wgl-4-2-core)
endif()

if(TARGET RendererLifecycleTests-windows-wgl-4-5)
    target_sources(RendererLifecycleTests-windows-wgl-4-5 PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    ACME_FOLDERIZE_TARGET(RendererLifecycleTests-windows-wgl-4-5)
endif()

if(TARGET RendererLifecycleTests-windows-wgl-es-3-0)
    target_sources(RendererLifecycleTests-windows-wgl-es-3-0 PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    ACME_FOLDERIZE_TARGET(RendererLifecycleTests-windows-wgl-es-3-0)
endif()

if(TARGET RendererLifecycleTests-x11-egl-es-3-0)
    target_sources(RendererLifecycleTests-x11-egl-es-3-0 PUBLIC "RendererLifecycleTests/ExternalWindowTests.cpp")
    ACME_FOLDERIZE_TARGET(RendererLifecycleTests-x11-egl-es-3-0)
endif()

# TODO Violin/Alex add support for embedded compositing on a renderer with x11-backend
ACME_MODULE(
    NAME                    EmbeddedCompositingTests-wayland-ivi-egl-es-3-0
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    INCLUDE_BASE            RendererTestFramework
                            EmbeddedCompositingTests
                            EmbeddedCompositingTests/EmbeddedCompositingTestFramework
                            EmbeddedCompositingTests/TestWaylandApplication
                            EmbeddedCompositingTests/TestCases

    FILES_PRIVATE_HEADER    EmbeddedCompositingTests/EmbeddedCompositingTestFramework/*.h
                            EmbeddedCompositingTests/TestWaylandApplication/*.h
                            EmbeddedCompositingTests/TestCases/*.h

    FILES_SOURCE            RendererTestFramework/*.cpp
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
)

# TODO(tobias/mohamed) Disable test broken by newer ubuntu 18.04 packages
if(TARGET EmbeddedCompositingTests-wayland-ivi-egl-es-3-0)
    ACME_ADD_TEST(EmbeddedCompositingTests-wayland-ivi-egl-es-3-0
        RNDSANDWICHTEST_SWRAST
         --ivi-layer 3 --ivi-control --filterOut ShowStreamTextureAfterChangingSurfaceSize
        )
endif()

RENDERER_MODULE_PER_CONFIG_STATIC(DmaOffscreenBufferRenderingTests
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    INCLUDE_BASE            RendererTestFramework
                            DmaOffscreenBufferRenderingTests

    FILES_PRIVATE_HEADER    DmaOffscreenBufferRenderingTests/*.h
                            RendererTestFramework/*.h

    FILES_SOURCE            DmaOffscreenBufferRenderingTests/*.cpp
                            RendererTestFramework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            TestContent
                            FrameworkTestUtils
                            RendererTestUtils
                            Device_EGL_Extension
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

IF (ramses-sdk_BUILD_TESTS)
    FOREACH(TEST_PLATFORM ${RENDERER_CONFIG_LIST})
        IF(TARGET platform-${TEST_PLATFORM})
            ACME_ADD_TEST(RenderingTests-${TEST_PLATFORM}
                RNDSANDWICHTEST
                --ivi-control --ivi-layer 3
            )
            ACME_ADD_TEST(RendererLifecycleTests-${TEST_PLATFORM}
                RNDSANDWICHTEST_SWRAST
                --ivi-control --ivi-layer 3
            )
            # These tests don't work generally on emulated ES drivers
            #    MSAA and anisotropic filters are not strictly specified by OpenGL standard -> emulated driver don't support it
            #    ETC2 and ASTC compressed formats don't work with glTexSubImage()/glTexStorage() in the software emulated MESA drivers (reason: unknown, probably bug)
            #    16 bit float texture formats cause crash in docker environment
            SET(SW_EMULATION_FILTER "AntiAliasingTest_MSAA4:TextureTest_Texture2D_AnisotropicFilter:TextureTest_TextureCube_AnisotropicFilter:RenderBuffer_Msaa:TextureTest_Texture2D_Sampling_MinLinearMagNearest:TextureTest_Texture2D_Sampling_MinNearestMagLinear:TextureTest_Texture2D_Format_ETC2RGB:TextureTest_Texture2D_Format_ETC2RGBA:TextureTest_Texture2D_CompressedMipMap:TextureTest_Texture2D_Format_ASTC_RGBA_4x4:TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4:RenderTarget_Format_R16F:RenderTarget_Format_RG16F:RenderTarget_Format_RGB16F:RenderTarget_Format_RGBA16F:RenderTarget_Format_RGB32F:TextureTest_Texture2D_Format_R16F:TextureTest_Texture2D_Format_RG16F:TextureTest_Texture2D_Format_RGB16F:TextureTest_Texture2D_Format_RGBA16F:GeometryShader:OffscreenBufferLinkTest_MSAA:OffscreenBufferLinkTest_MSAAUnlinked")

            LIST(FIND VALGRIND_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                # A limited set of rendering tests executed in the gate (executing all tests takes too long)
                ACME_ADD_TEST(RenderingTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_VALGRINDGATE
                    --ivi-control --ivi-layer 3 --filterIn TextTest_AccurateText:TextureTest_Texture2D_AddressMode
                )
                # A limited set of renderer lifecycle tests executed in the gate (executing all tests takes too long)
                ACME_ADD_TEST(RendererLifecycleTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_VALGRINDGATE
                     --ivi-control --ivi-layer 3 --gtest_filter=*DestroyRenderer_ChangeScene_ThenRecreateRenderer*:*RecreateSceneWithSameId*
                )
                # A limited set of EC tests executed in the gate (executing all tests takes too long)
                ACME_ADD_TEST(EmbeddedCompositingTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_VALGRINDGATE
                     --ivi-control --ivi-layer 3 --filterIn ShowStreamTextureAfterRecreatingScene:Recreated:ClientCanBindMultipleTimes
                )
            ENDIF()

            LIST(FIND GATE_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                ACME_ADD_TEST(RenderingTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_SWRAST
                    --ivi-control --ivi-layer 3 --filterOut ${SW_EMULATION_FILTER}
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
