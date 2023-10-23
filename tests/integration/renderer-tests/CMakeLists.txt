#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set(RendererTests-resources_MIXIN
    RESOURCE_FOLDERS        res
                            ../test-content/res
)

function(disable_leak_detection_for_test test_name)
    message(STATUS "Disable leak detection for ${test_name}")

    get_test_property(${test_name} ENVIRONMENT CURRENT_ENV)
    set_tests_properties(${test_name}
        PROPERTIES ENVIRONMENT "ASAN_OPTIONS=$ENV{ASAN_OPTIONS}:detect_leaks=0;${CURRENT_ENV}")
endfunction()

createModuleWithRenderer(
    NAME                    rendering-tests
    TYPE                    BINARY
    ENABLE_INSTALL          ON
    INCLUDE_PATHS           renderer-test-framework
                            rendering-tests
    SRC_FILES               rendering-tests/*.h
                            renderer-test-framework/*.h
                            rendering-tests/*.cpp
                            renderer-test-framework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            test-content framework-test-utils RendererTestUtils ramses-cli
)

createModuleWithRenderer(
    NAME                    renderer-lifecycle-tests
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    INCLUDE_PATHS           renderer-lifecycle-tests
                            renderer-test-framework

    SRC_FILES               renderer-lifecycle-tests/*.h
                            renderer-test-framework/*.h

                            renderer-lifecycle-tests/*.cpp
                            renderer-test-framework/*.cpp

    ${RendererTests-resources_MIXIN}

    DEPENDENCIES            test-content framework-test-utils RendererTestUtils
                            ramses-gmock-main
                            ramses-cli
)

if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
    createModule(
        NAME                    embedded-compositing-rendering-tests
        TYPE                    BINARY
        ENABLE_INSTALL          ON

        INCLUDE_PATHS           renderer-test-framework
                                embedded-compositing-rendering-tests
                                embedded-compositing-rendering-tests/EmbeddedCompositingTestFramework
                                embedded-compositing-rendering-tests/TestWaylandApplication
                                embedded-compositing-rendering-tests/TestCases

        SRC_FILES               embedded-compositing-rendering-tests/EmbeddedCompositingTestFramework/*.h
                                embedded-compositing-rendering-tests/TestWaylandApplication/*.h
                                embedded-compositing-rendering-tests/TestCases/*.h

                                renderer-test-framework/*.cpp
                                embedded-compositing-rendering-tests/EmbeddedCompositingTestFramework/*.cpp
                                embedded-compositing-rendering-tests/TestWaylandApplication/*.cpp
                                embedded-compositing-rendering-tests/TestCases/*.cpp

        ${RendererTests-resources_MIXIN}

        DEPENDENCIES            test-content
                                framework-test-utils
                                RendererTestUtils
                                ramses-cli
    )

    # TODO(tobias/mohamed) Disable test broken by newer ubuntu 18.04 packages
    makeTestFromTarget(TARGET embedded-compositing-rendering-tests
            SUFFIX RNDSANDWICHTEST_SWRAST
            EXTRA_ARGS --ivi-layer 3 --ivi-control --filter-out ShowStreamTextureAfterChangingSurfaceSize --window-type wayland-ivi
    )

    if(libdrm_FOUND AND gbm_FOUND)
        createModuleWithRenderer(
            NAME                    dma-offscreen-buffer-rendering-tests
            TYPE                    BINARY
            ENABLE_INSTALL          ON

            INCLUDE_PATHS           renderer-test-framework
                                    dma-offscreen-buffer-rendering-tests

            SRC_FILES               dma-offscreen-buffer-rendering-tests/*.h
                                    renderer-test-framework/*.h

                                    dma-offscreen-buffer-rendering-tests/*.cpp
                                    renderer-test-framework/*.cpp

            ${RendererTests-resources_MIXIN}

            DEPENDENCIES            test-content
                                    framework-test-utils
                                    RendererTestUtils
                                    ramses-cli
        )
    endif()
endif()

# Tests which are supposed to be run with valgrind
SET(VALGRIND_WINDOW_PLATFORMS
    "wayland-ivi"
)

# Tests which are supposed to be run in the gate build job
SET(GATE_WINDOW_PLATFORMS
    "x11"
    "wayland-ivi"
)

if (ramses-sdk_BUILD_TESTS)
    makeTestPerWindowTypeFromTarget(
        TARGET rendering-tests
        SUFFIX RNDSANDWICHTEST
        EXTRA_ARGS --ivi-control --ivi-layer 3
    )

    makeTestPerWindowTypeFromTarget(
        TARGET renderer-lifecycle-tests
        SUFFIX RNDSANDWICHTEST_SWRAST
        EXTRA_ARGS --ivi-control --ivi-layer 3
    )

    # These tests don't work generally on emulated ES drivers
    #    MSAA and anisotropic filters are not strictly specified by OpenGL standard -> emulated driver don't support it
    #    ETC2 and ASTC compressed formats don't work with glTexSubImage()/glTexStorage() in the software emulated MESA drivers (reason: unknown, probably bug)
    #    16 bit float texture formats cause crash in docker environment
    SET(SW_EMULATION_FILTER "AntiAliasingTest_MSAA4:TextureTest_Texture2D_AnisotropicFilter:TextureTest_TextureCube_AnisotropicFilter:RenderBuffer_Msaa:TextureTest_Texture2D_Sampling_MinLinearMagNearest:TextureTest_Texture2D_Sampling_MinNearestMagLinear:TextureTest_Texture2D_Format_ETC2RGB:TextureTest_Texture2D_Format_ETC2RGBA:TextureTest_Texture2D_CompressedMipMap:TextureTest_Texture2D_Format_ASTC_RGBA_4x4:TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4:RenderTarget_Format_R16F:RenderTarget_Format_RG16F:RenderTarget_Format_RGB16F:RenderTarget_Format_RGBA16F:RenderTarget_Format_RGB32F:TextureTest_Texture2D_Format_R16F:TextureTest_Texture2D_Format_RG16F:TextureTest_Texture2D_Format_RGB16F:TextureTest_Texture2D_Format_RGBA16F:GeometryShader:OffscreenBufferLinkTest_MSAA:OffscreenBufferLinkTest_MSAAUnlinked")

    # A limited set of rendering tests executed in the gate (executing all tests takes too long)
    makeTestPerWindowTypeFromTarget(
        TARGET rendering-tests
        SUFFIX RNDSANDWICHTEST_VALGRINDGATE
        WINDOW_TYPE_FILTER ${VALGRIND_WINDOW_PLATFORMS}
        EXTRA_ARGS --ivi-control --ivi-layer 3 --filter-in TextTest_AccurateText:TextureTest_Texture2D_AddressMode
    )
    # A limited set of renderer lifecycle tests executed in the gate (executing all tests takes too long)
    makeTestPerWindowTypeFromTarget(
        TARGET renderer-lifecycle-tests
        SUFFIX RNDSANDWICHTEST_VALGRINDGATE
        WINDOW_TYPE_FILTER ${VALGRIND_WINDOW_PLATFORMS}
        EXTRA_ARGS --ivi-control --ivi-layer 3 --gtest_filter=*DestroyRenderer_ChangeScene_ThenRecreateRenderer*:*RecreateSceneWithSameId*
    )
    # A limited set of EC tests executed in the gate (executing all tests takes too long)
    makeTestPerWindowTypeFromTarget(
        TARGET embedded-compositing-rendering-tests
        SUFFIX RNDSANDWICHTEST_VALGRINDGATE
        WINDOW_TYPE_FILTER ${VALGRIND_WINDOW_PLATFORMS}
        EXTRA_ARGS --ivi-control --ivi-layer 3 --filter-in ShowStreamTextureAfterRecreatingScene:Recreated:ClientCanBindMultipleTimes
    )

    makeTestPerWindowTypeFromTarget(
        TARGET rendering-tests
        SUFFIX RNDSANDWICHTEST_SWRAST
        WINDOW_TYPE_FILTER ${GATE_WINDOW_PLATFORMS}
        EXTRA_ARGS --ivi-control --ivi-layer 3 --filter-out ${SW_EMULATION_FILTER}
    )

    # Disable LSAN leak detection inside ASAN for rendering tests where lsan is unstable and sporadically crashes.
    if(ramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI)
        disable_leak_detection_for_test("rendering-tests_wayland-ivi-gles30_RNDSANDWICHTEST_SWRAST")
    endif()
    if(ramses-sdk_ENABLE_WINDOW_TYPE_X11)
        disable_leak_detection_for_test("rendering-tests_x11-gles30_RNDSANDWICHTEST_SWRAST")
        disable_leak_detection_for_test("renderer-lifecycle-tests_x11-gles30_RNDSANDWICHTEST_SWRAST")
    endif()

ENDIF()
