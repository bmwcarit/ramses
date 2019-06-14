#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set(RendererTests-resources_MIXIN
    FILES_RESOURCE          ../../TestContent/res/*
                            res/*
)

RENDERER_MODULE_PER_CONFIG_STATIC(RenderingTests
    TYPE                    TEST
    TEST_SUFFIX             RNDSANDWICHTEST

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
    TYPE                    TEST
    TEST_SUFFIX             RNDSANDWICHTEST_SWRAST

    INCLUDE_BASE            RendererLifecycleTests
                            RendererTestFramework

    FILES_PRIVATE_HEADER    RendererLifecycleTests/*.h
                            RendererTestFramework/*.h

    FILES_SOURCE            RendererLifecycleTests/*.cpp
                            RendererTestFramework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            TestContent FrameworkTestUtils RendererTestUtils
)

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
        --filterOut ShowStreamTextureAfterChangingSurfaceSize
        )
endif()


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
            # These tests don't work generally on emulated ES drivers
            #    MSAA and anisotropic filters are not strictly specified by OpenGL standard -> emulated driver don't support it
            #    ETC2 and ASTC compressed formats don't work with glTexSubImage()/glTexStorage() in the software emulated MESA drivers (reason: unknown, probably bug)
            #    16 bit float texture formats cause crash in docker environment
            SET(SW_EMULATION_FILTER "AntiAliasingTest_MSAA4:TextureTest_Texture2D_AnisotropicFilter:TextureTest_TextureCube_AnisotropicFilter:RenderBuffer_Msaa:TextureTest_Texture2D_Sampling_MinLinearMagNearest:TextureTest_Texture2D_Sampling_MinNearestMagLinear:TextureTest_Texture2D_Format_ETC2RGB:TextureTest_Texture2D_Format_ETC2RGBA:TextureTest_Texture2D_CompressedMipMap:TextureTest_Texture2D_Format_ASTC_RGBA_4x4:TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4:RenderTarget_Format_R16F:RenderTarget_Format_RG16F:RenderTarget_Format_RGB16F:RenderTarget_Format_RGBA16F:RenderTarget_Format_RGB32F:TextureTest_Texture2D_Format_R16F:TextureTest_Texture2D_Format_RG16F:TextureTest_Texture2D_Format_RGB16F:TextureTest_Texture2D_Format_RGBA16F")

            LIST(FIND VALGRIND_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                # A limited set of rendering tests executed in the gate (executing all tests takes too long)
                ACME_ADD_TEST(RenderingTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_VALGRINDGATE
                    --filterIn TextTest_AccurateText:TextureTest_Texture2D_AddressMode
                )
                # A limited set of renderer lifecycle tests executed in the gate (executing all tests takes too long)
                ACME_ADD_TEST(RendererLifecycleTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_VALGRINDGATE
                     --filterIn DestroyRenderer_ChangeScene_ThenRecreateRenderer:RecreateSceneWithSameId
                )
                # A limited set of EC tests executed in the gate (executing all tests takes too long)
                ACME_ADD_TEST(EmbeddedCompositingTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_VALGRINDGATE
                     --filterIn ShowStreamTextureAfterRecreatingScene:Recreated:ClientCanBindMultipleTimes
                )
            ENDIF()

            LIST(FIND GATE_PLATFORMS ${TEST_PLATFORM} _INDEX)
            IF(NOT ${_INDEX} LESS 0)
                ACME_ADD_TEST(RenderingTests-${TEST_PLATFORM}
                    RNDSANDWICHTEST_SWRAST
                    --filterOut ${SW_EMULATION_FILTER}
                )
            ENDIF()
        ENDIF()
    ENDFOREACH()
ENDIF()
