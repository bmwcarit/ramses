//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses.public_tests

import android.content.Context
import android.graphics.SurfaceTexture
import android.view.Surface
import com.bmwgroup.ramses.RamsesBundle
import kotlin.test.*
import androidx.test.platform.app.InstrumentationRegistry.getInstrumentation as getInstrumentation

class RamsesBundleTest {
    @Test
    fun ramsesBundle_DisposingWorks() {
        val ramsesBundle = RamsesBundle(null)

        assertFalse(ramsesBundle.nativeObjectDisposed())
        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_CanLoadSceneWithValidSceneAndLogic() {
        val ramsesBundle = RamsesBundle(null)

        val logicFD =
            getInstrumentation().context.resources.assets.openFd("testLogic.rlogic")
        val sceneFD =
            getInstrumentation().context.resources.assets.openFd("testScene.ramses")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        assertEquals(1, ramsesBundle.featureLevel)

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_CanLoadSceneWithValidSceneAndLogicWithFeatureLevel02() {
        val ramsesBundle = RamsesBundle(null)

        val logicFD =
            getInstrumentation().context.resources.assets.openFd("testLogic_02.rlogic")
        val sceneFD =
            getInstrumentation().context.resources.assets.openFd("testScene.ramses")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        assertEquals(2, ramsesBundle.featureLevel)

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_CanLoadSceneWithValidSceneAndNoLogic() {
        val ramsesBundle = RamsesBundle(null)

        val sceneFD =
            getInstrumentation().context.resources.assets.openFd("testScene.ramses")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                null,
                0
            )
        )

        assertEquals(0, ramsesBundle.featureLevel)
        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_LoadingSceneFailsWithValidLogicAndNoScene() {
        val ramsesBundle = RamsesBundle(null)

        val logicFD =
            getInstrumentation().context.resources.assets.openFd("testLogic.rlogic")
        assertFailsWith<IllegalArgumentException> {
            ramsesBundle.loadScene(
                null,
                0,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        }
    }

    @Test
    fun ramsesBundle_LoadingSceneFailsWithValidSceneAndInvalidLogic() {
        val ramsesBundle = RamsesBundle(null)
        val logicFD = getInstrumentation().context.resources.assets
            .openFd("testLogic_invalid.rlogic")
        val sceneFD = getInstrumentation().context.resources.assets
            .openFd("testScene.ramses")

        assertFalse(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_LoadingSceneFailsWithInvalidSceneAndValidLogic() {
        val ramsesBundle = RamsesBundle(null)

        val sceneFD = getInstrumentation().context.resources.assets
            .openFd("testScene_invalid.ramses")
        val logicFD = getInstrumentation().context.resources.assets
            .openFd("testLogic.rlogic")

        assertFalse(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_canGetInterfaceProperty() {
        val ramsesBundle = RamsesBundle(null)

        val sceneFD = getInstrumentation().context.resources.assets
            .openFd("testScene.ramses")
        val logicFD = getInstrumentation().context.resources.assets
            .openFd("testLogic.rlogic")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        assertNotNull(ramsesBundle.getInterface("intf"))
        assertNotNull(ramsesBundle.getInterface("intf").getChild("struct"))
        assertNotNull(ramsesBundle.getInterface("intf").getChild("struct").getChild("floatInput"))

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_getInterfacePropertyReturnsNullIfInterfaceNotFound() {
        val ramsesBundle = RamsesBundle(null)

        val sceneFD = getInstrumentation().context.resources.assets
            .openFd("testScene.ramses")
        val logicFD = getInstrumentation().context.resources.assets
            .openFd("testLogic.rlogic")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        assertNotNull(ramsesBundle.getInterface("intf"))
        assertNull(ramsesBundle.getInterface("wrongInterfaceName"))

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_canGetLogicNodeRootInputAndOutput() {
        val ramsesBundle = RamsesBundle(null)

        val sceneFD = getInstrumentation().context.resources.assets
            .openFd("testScene.ramses")
        val logicFD = getInstrumentation().context.resources.assets
            .openFd("testLogic.rlogic")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        assertNotNull(ramsesBundle.getLogicNodeRootInput("script1"))
        assertNotNull(ramsesBundle.getLogicNodeRootOutput("script1"))

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_gettingLogicNodeRootInputAndOutputFailsWithWrongInputName() {
        val ramsesBundle = RamsesBundle(null)

        val sceneFD = getInstrumentation().context.resources.assets
            .openFd("testScene.ramses")
        val logicFD = getInstrumentation().context.resources.assets
            .openFd("testLogic.rlogic")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        assertNull(ramsesBundle.getLogicNodeRootInput("InvalidName"))
        assertNull(ramsesBundle.getLogicNodeRootOutput("InvalidName"))

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_cannotGetLogicNodeRootInputAndOutput_BeforeSceneLoaded() {
        val ramsesBundle = RamsesBundle(null)
        assertFailsWith<IllegalStateException> { ramsesBundle.getLogicNodeRootInput("script1") }
        assertFailsWith<IllegalStateException> { ramsesBundle.getLogicNodeRootOutput("script1") }
    }

    @Test
    fun ramsesBundle_cannotShowScene_BeforeSceneLoaded() {
        val ramsesBundle = RamsesBundle(null)
        assertFailsWith<IllegalStateException> { ramsesBundle.showScene() }
    }

    @Test
    fun ramsesBundle_cannotUpdateScene_BeforeSceneLoaded() {
        val ramsesBundle = RamsesBundle(null)
        assertFailsWith<IllegalStateException> { ramsesBundle.updateLogic() }
        assertFailsWith<IllegalStateException> { ramsesBundle.flushRamsesScene() }
    }

    @Test
    fun ramsesBundle_cannotUpdateLogic_WhenNoLogicLoaded() {
        val ramsesBundle = RamsesBundle(null)

        val sceneFD = getInstrumentation().context.resources.assets
            .openFd("testScene.ramses")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                null,
                0
            )
        )

        assertFalse(ramsesBundle.updateLogic())
    }

    /* ------------------------ Testing protected members ----------------------- */
    class RamsesBundleForProtectedMethods(context: Context?) : RamsesBundle(context) {
        fun getDisplaySizePublic(): IntArray? {
            return super.getDisplaySize()
        }
    }

    @Test
    fun ramsesBundle_canGetDisplaySize() {
        val ramsesBundle = RamsesBundleForProtectedMethods(null)
        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        ramsesBundle.createDisplay(
            surface, null
        )
        assertNotEquals(null, ramsesBundle.getDisplaySizePublic())

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_canDispatchRendererEvents() {
        val ramsesBundle = RamsesBundleForProtectedMethods(null)
        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        ramsesBundle.createDisplay(
            surface, null
        )
        assertTrue(ramsesBundle.dispatchRendererEvents())
        // Test also that it works with an empty event queue (second dispatch)
        assertTrue(ramsesBundle.dispatchRendererEvents())
    }

    @Test
    fun ramsesBundle_getDisplaySizeReturnsNullIfNoDisplayCreated() {
        val ramsesBundle = RamsesBundleForProtectedMethods(null)

        assertEquals(null, ramsesBundle.getDisplaySizePublic())

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }

    @Test
    fun ramsesBundle_getDisplaySizeReturnsNullAfterDisplayDestruction() {
        val ramsesBundle = RamsesBundleForProtectedMethods(null)
        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        ramsesBundle.createDisplay(
            surface, null
        )
        assertNotEquals(null, ramsesBundle.getDisplaySizePublic())
        ramsesBundle.destroyDisplay()
        assertEquals(null, ramsesBundle.getDisplaySizePublic())

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())
    }
    // TODO add tests for linking/unlinking properties
    // TODO Daniel Add tests which verify that surface creation and corresponding states work as expected (e.g. clear color, viewport size, etc)
    // TODO Asko Add tests for createDisplay and setMaximumFramerate (and all the other missing stuff).
}
