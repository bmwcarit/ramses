//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses.public_tests

import android.graphics.SurfaceTexture
import android.view.Surface
import com.bmwgroup.ramses.ClearColor
import com.bmwgroup.ramses.Property
import com.bmwgroup.ramses.RamsesThread
import java.util.concurrent.CompletableFuture
import kotlin.test.*
import androidx.test.platform.app.InstrumentationRegistry.getInstrumentation as getInstrumentation

class RamsesThreadTest {

    companion object {
        private lateinit var testThread: TestRamsesThread
    }

    @BeforeTest
    fun setupTestRamsesThread() {
        testThread = TestRamsesThread("TestRamsesThread")
    }

    @AfterTest
    fun cleanUpRamsesThread() {
        if (testThread.isAlive) {
            testThread.destroyRamsesBundleAndQuitThread()
            assertFalse(testThread.isAlive)
        }
    }

    enum class SceneState {
        LoadedSuccessfully,
        LoadingFailed
    }

    inner class TestRamsesThread(threadName: String) : RamsesThread(threadName, null) {

        override fun onUpdate() {
            m_futureOnUpdate.complete(true)
        }

        override fun onDisplayResize(width: Int, height: Int) {
            m_width = width
            m_height = height
        }

        override fun onSceneLoaded() {
            addRunnableToThreadQueue {
                m_futureAfterSceneLoad.complete(
                    RamsesThreadStateAfterSceneLoad(
                        testThread.isSceneLoaded,
                        testThread.isRendering,
                        SceneState.LoadedSuccessfully
                    )
                )
            }
        }

        override fun onSceneLoadFailed() {
            addRunnableToThreadQueue {
                m_futureAfterSceneLoad.complete(
                    RamsesThreadStateAfterSceneLoad(
                        testThread.isSceneLoaded,
                        testThread.isRendering,
                        SceneState.LoadingFailed

                    )
                )
            }
        }

        override fun onLogicUpdated() {
            m_futureOnLogicUpdated.complete(true)
        }

        fun getRenderingFramerateThreadSafe(): Float {
            val future = CompletableFuture<Float>()
            addRunnableToThreadQueue {
                future.complete(testThread.renderingFramerate)
            }
            return future.get()
        }

        fun getInterfaceThreadSafe(name: String): Property {
            val future = CompletableFuture<Property>()
            addRunnableToThreadQueue {
                future.complete(testThread.getInterface(name))
            }
            return future.get()
        }

        fun getLogicNodeRootInputThreadSafe(logicNodeName: String): Property {
            val future = CompletableFuture<Property>()
            addRunnableToThreadQueue {
                future.complete(testThread.getLogicNodeRootInput(logicNodeName))
            }
            return future.get()
        }

        fun getLogicNodeRootOutputThreadSafe(logicNodeName: String): Property {
            val future = CompletableFuture<Property>()
            addRunnableToThreadQueue {
                future.complete(testThread.getLogicNodeRootOutput(logicNodeName))
            }
            return future.get()
        }

        fun getWindowSizeThreadSafe(): Pair<Int, Int> {
            val future = CompletableFuture<Pair<Int, Int>>()
            addRunnableToThreadQueue {
                future.complete(Pair(m_width, m_height))
            }
            return future.get()
        }

        fun isDisplayCreatedThreadSafe(): Boolean {
            val future = CompletableFuture<Boolean>()
            addRunnableToThreadQueue {
                future.complete(testThread.isDisplayCreated())
            }
            return future.get()
        }

        fun isRenderingThreadSafe(): Boolean {
            val future = CompletableFuture<Boolean>()
            addRunnableToThreadQueue {
                future.complete(testThread.isRendering())
            }
            return future.get()
        }

        fun getCurrentFeatureLevel(): Long {
            val future = CompletableFuture<Long>()
            addRunnableToThreadQueue {
                future.complete(testThread.featureLevel)
            }
            return future.get()
        }

        private var m_width = 0
        private var m_height = 0
        val m_futureAfterSceneLoad = CompletableFuture<RamsesThreadStateAfterSceneLoad>()
        val m_futureOnUpdate = CompletableFuture<Boolean>()
        val m_futureOnLogicUpdated = CompletableFuture<Boolean>()

        inner class RamsesThreadStateAfterSceneLoad(
            val isSceneLoaded: Boolean,
            val isUpdateLoopRunning: Boolean,
            val onSceneLoadedState: SceneState,
        )
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadSceneSuccessfullyLoadsScene() {
        assertFalse(testThread.isAlive)

        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )

        val state = testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)
        assertTrue(state.isSceneLoaded)
        assertFalse(state.isUpdateLoopRunning)
        assertEquals(state.onSceneLoadedState, SceneState.LoadedSuccessfully)
        assertEquals(1, testThread.getCurrentFeatureLevel())
    }

    @Test
    fun ramsesThread_canLoadSceneWithValidSceneAndLogicWithFeatureLevel02() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic_02.rlogic"
        )
        assertEquals(2, testThread.getCurrentFeatureLevel())
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadSceneSuccessfullyLoadsSceneFromFileDescriptors() {
        assertFalse(testThread.isAlive)

        val assetManager = getInstrumentation().context.resources.assets
        val fdRamses = assetManager.openFd("testScene.ramses")
        val fdRlogic = assetManager.openFd("testLogic.rlogic")

        testThread.initRamsesThreadAndLoadScene(
            fdRamses.parcelFileDescriptor,
            fdRamses.startOffset,
            fdRlogic.parcelFileDescriptor,
            fdRlogic.startOffset
        )

        val state = testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)
        assertTrue(state.isSceneLoaded)
        assertFalse(state.isUpdateLoopRunning)
        assertEquals(state.onSceneLoadedState, SceneState.LoadedSuccessfully)
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadSceneFailsLoadingScene() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "invalidScene.bin",
            "testLogic.rlogic"
        )

        val state = testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)
        assertFalse(state.isSceneLoaded)
        assertFalse(state.isUpdateLoopRunning)
        assertEquals(state.onSceneLoadedState, SceneState.LoadingFailed)
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadSceneFailsLoadingSceneFromFileDescriptor() {
        val assetManager = getInstrumentation().context.resources.assets
        val fdRamses = assetManager.openFd("testScene_invalid.ramses")
        val fdRlogic = assetManager.openFd("testLogic.rlogic")

        testThread.initRamsesThreadAndLoadScene(
            fdRamses.parcelFileDescriptor,
            fdRamses.startOffset,
            fdRlogic.parcelFileDescriptor,
            fdRlogic.startOffset
        )

        val state = testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)
        assertFalse(state.isSceneLoaded)
        assertFalse(state.isUpdateLoopRunning)
        assertEquals(state.onSceneLoadedState, SceneState.LoadingFailed)
    }

    @Test
    fun ramsesThread_createsDisplayWithMSAAEnabled() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture1 = SurfaceTexture(1)
        val surface1 = Surface(surfaceTexture1)
        // MSAA is very device dependent unfortunately, so can't test with a non-trivial set of
        // samples... Still, testing that using the overload with MSAA=1 works
        testThread.createDisplayAndShowScene(
            surface1, ClearColor(0F, 0F, 0F, 1F), 1
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())
    }

    @Test
    fun ramsesThread_throwsExceptionWhenDisplayCreatedWithInvalidMSAASize() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture1 = SurfaceTexture(1)
        val surface1 = Surface(surfaceTexture1)
        assertFailsWith<IllegalArgumentException> {
            testThread.createDisplayAndShowScene(
                surface1, ClearColor(0F, 0F, 0F, 1F), 13
            )
        }
        assertFalse(testThread.isDisplayCreatedThreadSafe())
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadScene_succeedsAfterCalling_destroyRamsesBundleAndQuitThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)

        testThread.destroyRamsesBundleAndQuitThread()
        assertFalse(testThread.isAlive)

        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadScene_succeedsAfterCalling_destroyRamsesBundleAndQuitThread_onFailed_initRamsesThreadAndLoadScene() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "invalidScene.bin",
            "invalidLogic.bin"
        )
        assertFalse(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertTrue(testThread.isAlive)
        assertFalse(testThread.isRenderingThreadSafe())

        testThread.destroyRamsesBundleAndQuitThread()
        assertFalse(testThread.isAlive)

        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadScene_failsOnInitializedObject() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)

        assertFailsWith<IllegalThreadStateException> {
            testThread.initRamsesThreadAndLoadScene(
                getInstrumentation().context.resources.assets,
                "testScene.ramses",
                "testLogic.rlogic"
            )
        }
    }

    @Test
    fun ramsesThread_initRamsesThreadAndLoadScene_failsAfterFailedInitialization() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "invalidScene.bin",
            "invalidLogic.bin"
        )
        testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)

        assertFailsWith<IllegalThreadStateException> {
            testThread.initRamsesThreadAndLoadScene(
                getInstrumentation().context.resources.assets,
                "testScene.ramses",
                "testLogic.rlogic"
            )
        }
    }

    @Test
    fun ramsesThread_onUpdateCalled() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )

        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.addRunnableToThreadQueue { testThread.startRendering() }

        assertTrue(testThread.isRenderingThreadSafe())

        assertTrue(testThread.m_futureOnUpdate.get())
    }

    @Test
    fun ramsesThread_onLogicUpdatedCalled() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.addRunnableToThreadQueue { testThread.startRendering() }
        assertTrue(testThread.isRenderingThreadSafe())

        assertTrue(testThread.m_futureOnUpdate.get())
    }

    @Test
    fun ramsesThread_setRenderingFramerateWorks() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )

        assertTrue(testThread.isDisplayCreatedThreadSafe())

        assertEquals(60f, testThread.getRenderingFramerateThreadSafe())
        testThread.addRunnableToThreadQueue {
            testThread.startRendering()
        }

        testThread.addRunnableToThreadQueue {
            testThread.renderingFramerate = 20f
        }
        assertEquals(20f, testThread.getRenderingFramerateThreadSafe())

        // Rates below accepted range throw an exception
        assertFailsWith<IllegalArgumentException> {
            testThread.renderingFramerate = 0f
        }
        assertEquals(20f, testThread.getRenderingFramerateThreadSafe())
        assertTrue(testThread.isRenderingThreadSafe())

        // Rates above accepted range throw an exception
        assertFailsWith<IllegalArgumentException> {
            testThread.renderingFramerate = 240.5f
        }
        assertEquals(20f, testThread.getRenderingFramerateThreadSafe())
        assertTrue(testThread.isRenderingThreadSafe())
    }

    @Test
    fun ramsesThread_setRenderingFramerateBeforeCreatingDisplayFails() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalStateException> {
                testThread.renderingFramerate = 70f
            }
        }
    }

    @Test
    fun ramsesThread_setRenderingFramerateAfterDestroyingDisplayFails() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.destroyDisplay()
        assertFalse(testThread.isDisplayCreatedThreadSafe())

        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalStateException> {
                testThread.renderingFramerate = 70f
            }
        }
    }

    @Test
    fun ramsesThread_stoppingAndStartingRenderingWorks() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertEquals(60f, testThread.getRenderingFramerateThreadSafe())
        assertFalse(testThread.isRenderingThreadSafe())

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )

        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.addRunnableToThreadQueue {
            testThread.startRendering()
        }
        assertTrue(testThread.isRenderingThreadSafe())

        testThread.addRunnableToThreadQueue {
            testThread.stopRendering()
        }
        assertFalse(testThread.isRenderingThreadSafe())

        testThread.addRunnableToThreadQueue {
            testThread.startRendering()
        }
        assertTrue(testThread.isRenderingThreadSafe())
    }

    @Test
    fun ramsesThread_destroyRamsesBundleAndQuitThreadStopsThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertFalse(testThread.isRenderingThreadSafe())
        assertEquals(60f, testThread.getRenderingFramerateThreadSafe())

        testThread.destroyRamsesBundleAndQuitThread()
        assertFalse(testThread.isAlive)
    }

    @Test
    fun ramsesThread_destroyRamsesBundleAndQuitThreadStopsThread_failsWhenCalledFromRamsesThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        testThread.m_futureAfterSceneLoad.get()
        assertTrue(testThread.isAlive)

        val future = CompletableFuture<Unit>()
        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalThreadStateException> {
                testThread.destroyRamsesBundleAndQuitThread()
            }
        }

        // RamsesThread is still alive after calling destroyRamsesBundleAndQuitThread
        assertTrue(testThread.isAlive)
    }

    @Test
    fun ramsesThread_resizeDisplayTriggersCallback() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        testThread.resizeDisplay(244, 562)
        assertEquals(Pair(244, 562), testThread.getWindowSizeThreadSafe())
    }

    @Test
    fun ramsesThread_getInterfaceWorks() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        assertTrue(testThread.getInterfaceThreadSafe("intf").hasChild("struct"))
    }

    @Test
    fun ramsesThread_getLogicNodeRootInputAndOutputWorks() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        assertTrue(testThread.getLogicNodeRootInputThreadSafe("script1").hasChild("structInput"))
        assertTrue(testThread.getLogicNodeRootOutputThreadSafe("script1").hasChild("floatOutput"))
    }

    @Test
    fun ramsesThread_getLogicNodeRootInputAndOutputThrowException_WhenCalledFromDifferentThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        assertFailsWith<IllegalThreadStateException> {
            testThread.getLogicNodeRootInput("script1")
        }
        assertFailsWith<IllegalThreadStateException> {
            testThread.getLogicNodeRootOutput("script1")
        }
    }

    @Test
    fun ramsesThread_isRenderingThrowsException_WhenCalledFromDifferentThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        assertFailsWith<IllegalThreadStateException> { testThread.isRendering }
    }

    @Test
    fun ramsesThread_isSceneLoadedThrowsException_WhenCalledFromDifferentThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertFailsWith<IllegalThreadStateException> { testThread.renderingFramerate }
    }

    @Test
    fun ramsesThread_getUpdateAndRenderingRateThrowsException_WhenCalledFromDifferentThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertFailsWith<IllegalThreadStateException> { testThread.renderingFramerate }
    }

    @Test
    fun ramsesThread_destroyRamsesBundleAndQuitThread_ThrowsException_WhenCalledOnAlreadyDestroyedObject() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertFalse(testThread.isRenderingThreadSafe())

        testThread.destroyRamsesBundleAndQuitThread()
        assertFalse(testThread.isAlive)

        assertFailsWith<IllegalThreadStateException> { testThread.destroyRamsesBundleAndQuitThread() }
    }

    @Test
    fun ramsesThread_invokingFunctionsOnNotInitializedRamsesThreadFails() {
        assertFalse(testThread.isAlive)
        assertFailsWith<IllegalThreadStateException> { testThread.renderingFramerate = 100f }
        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        assertFailsWith<IllegalThreadStateException> {
            testThread.createDisplayAndShowScene(
                surface, ClearColor(0F, 0F, 0F, 0F)
            )
        }
        assertFailsWith<IllegalThreadStateException> { testThread.destroyDisplay() }
        assertFailsWith<IllegalThreadStateException> { testThread.startRendering() }
        assertFailsWith<IllegalThreadStateException> { testThread.stopRendering() }
        assertFailsWith<IllegalThreadStateException> { testThread.resizeDisplay(0, 0) }
        assertFailsWith<IllegalThreadStateException> { testThread.destroyRamsesBundleAndQuitThread() }
        assertFailsWith<IllegalThreadStateException> {
            testThread.addRunnableToThreadQueue { testThread.isSceneLoaded }
        }
        assertFailsWith<IllegalThreadStateException> {
            testThread.addRunnableToThreadQueue { testThread.getLogicNodeRootInput("name") }
        }
        assertFailsWith<IllegalThreadStateException> {
            testThread.addRunnableToThreadQueue { testThread.getLogicNodeRootOutput("name") }
        }
    }

    @Test
    fun ramsesThread_invokingFunctionsOnDestroyedRamsesThreadFails() {
        assertFalse(testThread.isAlive)
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertTrue(testThread.isAlive)
        testThread.destroyRamsesBundleAndQuitThread()
        assertFalse(testThread.isAlive)

        assertFailsWith<IllegalThreadStateException> {
            testThread.addRunnableToThreadQueue { testThread.renderingFramerate = 100f }
        }
        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        assertFailsWith<IllegalThreadStateException> {
            testThread.createDisplayAndShowScene(
                surface, ClearColor(0F, 0F, 0F, 0F)
            )
        }
        assertFailsWith<IllegalThreadStateException> { testThread.destroyDisplay() }
        assertFailsWith<IllegalThreadStateException> { testThread.startRendering() }
        assertFailsWith<IllegalThreadStateException> { testThread.stopRendering() }
        assertFailsWith<IllegalThreadStateException> { testThread.resizeDisplay(0, 0) }
        assertFailsWith<IllegalThreadStateException> { testThread.destroyRamsesBundleAndQuitThread() }
        assertFailsWith<IllegalThreadStateException> {
            testThread.addRunnableToThreadQueue { testThread.isSceneLoaded }
        }
        assertFailsWith<IllegalThreadStateException> {
            testThread.addRunnableToThreadQueue { testThread.getLogicNodeRootInput("name") }
        }
        assertFailsWith<IllegalThreadStateException> {
            testThread.addRunnableToThreadQueue { testThread.getLogicNodeRootOutput("name") }
        }
    }

    @Test
    fun ramsesThread_startAndStopRenderingWork() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )

        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.addRunnableToThreadQueue { testThread.startRendering() }
        assertTrue(testThread.isRenderingThreadSafe())

        testThread.addRunnableToThreadQueue { testThread.stopRendering() }
        assertFalse(testThread.isRenderingThreadSafe())
    }

    @Test
    fun ramsesThread_startRenderingFailsIfDisplayWasntCreated() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalStateException> {
                testThread.startRendering()
            }
        }
    }

    @Test
    fun ramsesThread_startRenderingFailsIfAlreadyRendering() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )

        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.addRunnableToThreadQueue { testThread.startRendering() }
        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalStateException> {
                testThread.startRendering()
            }
        }
    }

    @Test
    fun ramsesThread_stopRenderingFailsIfRendererNotRunningAlready() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalStateException> {
                testThread.stopRendering()
            }
        }
    }

    @Test
    fun ramsesThread_createDisplayAndShowSceneCanBeCalledAgainAfterDestroyingDisplay() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture1 = SurfaceTexture(1)
        val surface1 = Surface(surfaceTexture1)
        testThread.createDisplayAndShowScene(
            surface1, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.destroyDisplay()
        assertFalse(testThread.isDisplayCreatedThreadSafe())

        val surfaceTexture2 = SurfaceTexture(2)
        val surface2 = Surface(surfaceTexture2)
        testThread.createDisplayAndShowScene(
            surface2, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())
    }

    @Test
    fun ramsesThread_destroyDisplayFails_whenCalledBeforeCreatingDisplay() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)
        assertFalse(testThread.isDisplayCreatedThreadSafe())

        assertFailsWith<IllegalStateException> {
            testThread.destroyDisplay()
        }
    }

    @Test
    fun ramsesThread_destroyDisplayFails_whenCalledTwice() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())

        testThread.destroyDisplay()
        assertFalse(testThread.isDisplayCreatedThreadSafe())

        assertFailsWith<IllegalStateException> {
            testThread.destroyDisplay()
        }
    }

    @Test
    fun ramsesThread_createDisplayAndShowSceneFails_WhenCalledTwiceWithoutDestroyingDisplay() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture1 = SurfaceTexture(1)
        val surface1 = Surface(surfaceTexture1)
        testThread.createDisplayAndShowScene(
            surface1, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())

        val surfaceTexture2 = SurfaceTexture(2)
        val surface2 = Surface(surfaceTexture2)
        assertFailsWith<IllegalStateException> {
            testThread.createDisplayAndShowScene(
                surface2, ClearColor(0F, 0F, 0F, 0F)
            )
        }
    }

    @Test
    fun ramsesThread_methodsPostingOnRamsesThreadAndWaitingForResultFailWhenCalledInRamsesThread() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture1 = SurfaceTexture(1)
        val surface1 = Surface(surfaceTexture1)
        testThread.createDisplayAndShowScene(
            surface1, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())

        val surfaceTexture2 = SurfaceTexture(2)
        val surface2 = Surface(surfaceTexture2)
        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalThreadStateException> {
                testThread.createDisplayAndShowScene(
                    surface2,
                    ClearColor(0F, 0F, 0F, 0F)
                )
            }
        }

        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalThreadStateException> {
                testThread.destroyDisplay()
            }
        }

        testThread.addRunnableToThreadQueue {
            assertFailsWith<IllegalThreadStateException> {
                testThread.destroyRamsesBundleAndQuitThread()
            }
        }
    }

    @Test
    fun ramsesThread_createDisplayAndShowSceneCallsOnDisplayResize() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(testThread.m_futureAfterSceneLoad.get().isSceneLoaded)

        val surfaceTexture = SurfaceTexture(1).apply { setDefaultBufferSize(35, 47) }
        val surface = Surface(surfaceTexture)
        testThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )
        assertTrue(testThread.isDisplayCreatedThreadSafe())
        assertEquals(Pair<Int, Int>(35, 47), testThread.getWindowSizeThreadSafe())
    }

    @Test
    fun ramsesThread_onLogicUpdatedHasChangesFromOnUpdate() {
        val futureAfterSceneLoad = CompletableFuture<Boolean>()
        val futureOnLogicUpdated = CompletableFuture<Boolean>()
        val outputsTestThread = object : RamsesThread("outputsTestThread", null) {
            lateinit var floatInput: Property
            lateinit var floatOutput: Property
            override fun onUpdate() {
                floatInput.set(14.1f)
            }

            override fun onDisplayResize(width: Int, height: Int) {
            }

            override fun onSceneLoaded() {
                futureAfterSceneLoad.complete(
                    isSceneLoaded
                )
                // setting floatInput should result in floatOuput being read with the same value
                floatInput = getLogicNodeRootInput("script1").getChild("floatInput")
                floatOutput = getLogicNodeRootOutput("script1").getChild("nodeTranslation")
            }

            override fun onSceneLoadFailed() {
            }

            override fun onLogicUpdated() {
                futureOnLogicUpdated.complete(floatOutput.vec3f[0] == floatInput.float)
            }
        }
        outputsTestThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertTrue(futureAfterSceneLoad.get())

        val surfaceTexture = SurfaceTexture(1)
        val surface = Surface(surfaceTexture)
        outputsTestThread.createDisplayAndShowScene(
            surface, ClearColor(0F, 0F, 0F, 0F)
        )
        outputsTestThread.addRunnableToThreadQueue { outputsTestThread.startRendering() }
        assertTrue(futureOnLogicUpdated.get())
    }

    @Test
    fun ramsesThread_runnablesQueuedAfterDestroyRamsesBundleAndQuitThreadAreNotExecuted() {
        testThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )
        assertEquals(testThread.m_futureAfterSceneLoad.get().isSceneLoaded, true)

        // This runnable will repeatedly queue itself to the RamsesThread and check isRendering which
        // would throw an Exception if it was executed right after the native RamsesBundle was disposed (happens
        // in the synchronized part in destroyRamsesBundleAndQuitThread) and before the Thread is quit (happens
        // right after the synchronzized part in destroyRamsesBundleAndQuitThread
        lateinit var selfSpawningRunnable: Runnable
        selfSpawningRunnable = Runnable {
            testThread.isRendering
            testThread.addRunnableToThreadQueue(selfSpawningRunnable)
        }
        testThread.addRunnableToThreadQueue(selfSpawningRunnable)

        testThread.destroyRamsesBundleAndQuitThread()
        assertFalse(testThread.isAlive)
    }

    // TODO add tests for linking/unlinking properties in specific callbacks
}
