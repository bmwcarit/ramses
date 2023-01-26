//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
package com.bmwgroup.ramses

import android.content.Intent
import android.os.SystemClock
import android.util.Log
import androidx.test.internal.runner.junit4.statement.UiThreadStatement
import java.io.File
import java.util.concurrent.CompletableFuture
import kotlin.test.*
import androidx.test.platform.app.InstrumentationRegistry.getInstrumentation as getInstrumentation
import com.bmwgroup.ramses.RamshCommandBroadcastReceiver.ACTION_DUMP_SCENE as ACTION_DUMP_SCENE
import com.bmwgroup.ramses.RamshCommandBroadcastReceiver.ACTION_RAMSH_COMMAND as ACTION_RAMSH_COMMAND
import com.bmwgroup.ramses.RamshCommandBroadcastReceiver.EXTRA_COMMAND as EXTRA_COMMAND
import com.bmwgroup.ramses.RamshCommandBroadcastReceiver.EXTRA_FILENAME as EXTRA_FILENAME

class RamshTest {
    companion object {
        lateinit var ramsesBundle: RamsesBundle
    }

    private fun cleanAppFilesDirectory() {
        val applicationFolder = getInstrumentation().targetContext.getExternalFilesDir(null)
        if (applicationFolder != null && applicationFolder.isDirectory) {
            for (child in applicationFolder.listFiles()) {
                child.delete()
            }
        }
    }

    @BeforeTest
    fun setup() {
        cleanAppFilesDirectory()
        ramsesBundle = RamsesBundle(getInstrumentation().targetContext)
    }

    @AfterTest
    fun disposeRamsesBundle() {
        ramsesBundle.dispose()
    }

    @Test
    fun ramsesBundle_dumpSceneToFileFailsWhenNoSceneLoaded() {
        val fileName = "invalidFile"
        val dumpSceneIntent = Intent()
        dumpSceneIntent.action = ACTION_DUMP_SCENE
        dumpSceneIntent.putExtra(EXTRA_FILENAME, fileName)

        getInstrumentation().targetContext.sendBroadcast(dumpSceneIntent)
        // have to sleep to make sure scene broadcast is received and processed
        SystemClock.sleep(1000)

        val savedRamsesFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".ramses"
        )
        assertTrue(savedRamsesFile.length() == 0L)
        val savedRlogicFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".rlogic"
        )
        assertTrue(savedRlogicFile.length() == 0L)

        UiThreadStatement.runOnUiThread {
            ramsesBundle.dispose()
            assertTrue(ramsesBundle.nativeObjectDisposed())
        }
    }

    @Test
    fun ramsesBundle_dumpSceneToFileFailsWhenBundleAlreadyDisposed() {
        val sceneFD = getInstrumentation().context.resources.assets.openFd("testScene.ramses")
        val logicFD = getInstrumentation().context.resources.assets.openFd("testLogic.rlogic")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        ramsesBundle.dispose()

        val fileName = "fileFromDumpSceneCommand"
        val dumpSceneIntent = Intent()
        dumpSceneIntent.action = ACTION_DUMP_SCENE
        dumpSceneIntent.putExtra(EXTRA_FILENAME, fileName)

        getInstrumentation().targetContext.sendBroadcast(dumpSceneIntent)
        // have to sleep to make sure broadcast is received and processed
        SystemClock.sleep(1000)

        val savedRamsesFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".ramses"
        )
        assertTrue(savedRamsesFile.length() == 0L)
        val savedRlogicFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".rlogic"
        )
        assertTrue(savedRlogicFile.length() == 0L)
    }

    @Test
    fun ramsesBundle_dumpSceneToFileWithDedicatedCommand() {
        val sceneFD = getInstrumentation().context.resources.assets.openFd("testScene.ramses")
        val logicFD = getInstrumentation().context.resources.assets.openFd("testLogic.rlogic")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        val fileName = "fileFromDumpSceneCommand"
        val dumpSceneIntent = Intent()
        dumpSceneIntent.action = ACTION_DUMP_SCENE
        dumpSceneIntent.putExtra(EXTRA_FILENAME, fileName)

        getInstrumentation().targetContext.sendBroadcast(dumpSceneIntent)
        // have to sleep to make sure scene is already dumped
        SystemClock.sleep(1000)

        val savedRamsesFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".ramses"
        )
        assertFalse(savedRamsesFile.length() == 0L)
        val savedRlogicFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".rlogic"
        )
        assertFalse(savedRlogicFile.length() == 0L)

        UiThreadStatement.runOnUiThread {
            ramsesBundle.dispose()
            assertTrue(ramsesBundle.nativeObjectDisposed())
        }
    }

    @Test
    fun ramsesThread_dumpSceneToFileWithDedicatedCommand() {
        val ramsesThread = object : RamsesThread("", getInstrumentation().targetContext) {
            override fun onUpdate() {
            }

            override fun onDisplayResize(width: Int, height: Int) {
            }

            override fun onSceneLoaded() {
            }

            override fun onSceneLoadFailed() {
            }

            override fun onLogicUpdated() {
            }
        }

        ramsesThread.initRamsesThreadAndLoadScene(
            getInstrumentation().context.resources.assets,
            "testScene.ramses",
            "testLogic.rlogic"
        )

        val future = CompletableFuture<Boolean>()
        ramsesThread.addRunnableToThreadQueue {
            future.complete(true)
        }
        future.get()

        val fileName = "fileFromDumpSceneCommand"
        val dumpSceneIntent = Intent()
        dumpSceneIntent.action = ACTION_DUMP_SCENE
        dumpSceneIntent.putExtra(EXTRA_FILENAME, fileName)

        getInstrumentation().targetContext.sendBroadcast(dumpSceneIntent)
        // have to sleep to make sure scene is already dumped
        SystemClock.sleep(1000)

        val savedRamsesFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".ramses"
        )
        assertFalse(savedRamsesFile.length() == 0L)
        val savedRlogicFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".rlogic"
        )
        assertFalse(savedRlogicFile.length() == 0L)

        UiThreadStatement.runOnUiThread {
            ramsesBundle.dispose()
            assertTrue(ramsesBundle.nativeObjectDisposed())

            try {
                ramsesThread.destroyRamsesBundleAndQuitThread()
                assertFalse(ramsesThread.isAlive)
            } catch (e: InterruptedException) {
                Log.e(
                    "ramsesThread_dumpSceneToFileWithDedicatedCommand",
                    "InterruptedException:",
                    e
                )
            }
        }
    }

    @Test
    fun ramsesBundle_dumpSceneToFileWithDedicatedCommandFailsWithNoFilePath() {
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

        val fileName = "fileFromDumpSceneCommand"
        val dumpSceneIntent = Intent()
        dumpSceneIntent.action = ACTION_DUMP_SCENE

        getInstrumentation().targetContext.sendBroadcast(dumpSceneIntent)
        // have to sleep to make sure scene is already dumped
        SystemClock.sleep(1000)

        val savedRamsesFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".ramses"
        )
        assertTrue(savedRamsesFile.length() == 0L)
        val savedRlogicFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".rlogic"
        )
        assertTrue(savedRlogicFile.length() == 0L)

        UiThreadStatement.runOnUiThread {
            ramsesBundle.dispose()
            assertTrue(ramsesBundle.nativeObjectDisposed())
        }
    }

    @Test
    fun ramsesBundle_dumpSceneToFileWithGenericCommand() {
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

        val fileName = "fileFromGenericCommand"
        val filePath = getInstrumentation().targetContext.getExternalFilesDir(null)
            .toString() + "/" + fileName
        val command = "dumpSceneToFile 123 $filePath"
        val dumpSceneIntent = Intent()
        dumpSceneIntent.action = ACTION_RAMSH_COMMAND
        dumpSceneIntent.putExtra(EXTRA_COMMAND, command)

        getInstrumentation().targetContext.sendBroadcast(dumpSceneIntent)
        // wait for the broadcast to take effect
        SystemClock.sleep(1000)
        // dumpSceneToFile is a scene command and requires a flush to be executed
        ramsesBundle.updateLogic()
        ramsesBundle.flushRamsesScene()

        val savedFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".ramses"
        )
        assertFalse(savedFile.length() == 0L)

        UiThreadStatement.runOnUiThread {
            ramsesBundle.dispose()
            assertTrue(ramsesBundle.nativeObjectDisposed())
        }
    }

    @Test
    fun ramsesBundle_dumpSceneToFileWithGenericCommandFailsWithNoCommand() {
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

        val fileName = "fileFromGenericCommand"
        val filePath = getInstrumentation().targetContext.getExternalFilesDir(null)
            .toString() + "/" + fileName
        val command = "dumpSceneToFile 123 $filePath"
        val dumpSceneIntent = Intent()
        dumpSceneIntent.action = ACTION_RAMSH_COMMAND

        getInstrumentation().targetContext.sendBroadcast(dumpSceneIntent)
        // wait for the broadcast to take effect
        SystemClock.sleep(1000)
        // dumpSceneToFile is a scene command and requires a flush to be executed
        ramsesBundle.updateLogic()
        ramsesBundle.flushRamsesScene()

        val savedFile = File(
            getInstrumentation().targetContext.getExternalFilesDir(null)
                .toString() + "/" + fileName + ".ramses"
        )
        assertTrue(savedFile.length() == 0L)

        UiThreadStatement.runOnUiThread {
            ramsesBundle.dispose()
            assertTrue(ramsesBundle.nativeObjectDisposed())
        }
    }

    @Test
    fun ramsesBundle_cannotDumpSceneToFile_BeforeSceneLoaded() {
        val ramsesBundle = RamsesBundle(null)
        assertFailsWith<IllegalStateException> { ramsesBundle.dumpSceneToFile("validFilePath") }
    }

    @Test
    fun ramsesBundle_cannotDumpLogicToFile_BeforeSceneLoaded() {
        val ramsesBundle = RamsesBundle(null)
        assertFailsWith<IllegalStateException> { ramsesBundle.dumpLogicToFile("validFilePath") }
    }
}
