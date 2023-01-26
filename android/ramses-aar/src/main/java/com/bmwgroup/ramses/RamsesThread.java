//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.Surface;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.io.IOException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

/**
 * This class provides execution of all actions performed on the RamsesBundle in a separate Thread.
 * You can create your own version of it if needed, this is our suggested default implementation.
 * The class methods are designed to fit to certain Activity/Fragment lifecycle or SurfaceView/TextureView events
 * (e.g. it is recommended to call initRamsesThreadAndLoadScene in onCreate/onViewCreated) that are typically executed
 * in the recommended callbacks to make use of the RamsesBundle in a tested and safe way. Otherwise the
 * user has to make sure that calling the functions in other places works as expected.
 * Once the initRamsesThreadAndLoadScene method is finished the scene is loaded and the thread (HandlerThread) is started.
 * The functions that are designed to operate on RamsesThread
 * should be called after the thread is started (after initRamsesThreadAndLoadScene() is called).
 * Subsequently the display should be created and Render/Update loop started.
 * This loop has a certain rate that runs by default at 60 Hz. It can be changed with setRenderingFramerate.
 * Rendering/updating can be started and stopped with startRendering and stopRendering methods.
 * Rendering state should be tracked in the application and switched on or off accordingly.
 * On top the class offers multiple abstract callbacks that enable the user to interact with the content
 * at significant points in the application lifecycle such as onSceneLoaded or onUpdate. To interact
 * with logic nodes outside of the thread safe callbacks there is the function addRunnableToThreadQueue
 * provided that adds the given piece of work to the RamsesThread's work queue. If a logic node is initialized
 * outside of the callbacks and uses RamsesThread's getLogicNodeRootInput for
 * it, the access wouldn't be thread safe and thus an Exception will be thrown. To avoid this, the
 * access has to be made safe with the addRunnableToThreadQueue.
 */
public abstract class RamsesThread {
    /**
     * Constructor which changes thread name and optionally activates debugging functionality if valid context provided.
     *
     * @param threadName name ot the RamsesThread
     * @param context    Context used to enable debugging functionality
     */
    public RamsesThread(String threadName, @Nullable Context context) {
        m_threadName = "RamsesThread_" + threadName;
        m_context = context;
    }

    private void updateSceneAndLogic() {
        if (!m_sceneLoaded) {
            return;
        }
        ensureRamsesBundleCreated("updateSceneAndLogic");

        try {
            onUpdate();
        } catch (Exception e) {
            Log.e(m_threadName, "updateSceneAndLogic: The function onUpdate threw an " +
                    e.getCause() + "Look at stacktrace for further info: ", e);
        }

        /*
         * Updates the LogicEngine. This will execute previously created and linked scripts and will set properties like the rotation of
         * Ramses nodes bound to the output of the script.
         */
        m_ramsesBundle.updateLogic();

        try {
            onLogicUpdated();
        } catch (Exception e) {
            Log.e(m_threadName, "onLogicUpdated: The function onLogicUpdated threw an " +
                    e.getCause() + "Look at stacktrace for further info: ", e);
        }

        /*
         * Tells ramses to "flush" the scene state and finish the frame, which will contain the state produced
         * by the logic engine.
         */
        m_ramsesBundle.flushRamsesScene();

        /*
         * Dispatch renderer events. Currently, no events are reported to user, but this is still needed
         * to ensure the ramses event queue doesn't overflow.
         */
        m_ramsesBundle.dispatchRendererEvents();

    }

    private void initRamsesThread() {
        if (m_workerThread == null) {
            m_workerThread = new HandlerThread(m_threadName);
            m_workerThread.start();
        } else {
            throw new IllegalThreadStateException("Calling initRamsesThreadAndLoadScene() on an object that is already initialized.");
        }
        // getLooper blocks until looper is initialized
        m_ramsesThreadHandler = new Handler(m_workerThread.getLooper());
    }

    private void loadSceneFromFileDescriptor(final ParcelFileDescriptor fdRamses, final long offsetRamses, final ParcelFileDescriptor fdRlogic, final long offsetRlogic,
                                             final String logText) {
        // Explicitly wait for resumed and focused state before rendering anything
        m_ramsesThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(m_threadName, "initRamsesThreadAndLoadScene: loading scene from " + logText);

                /*
                 * Creates a RamsesBundle instance which holds all low-level Ramses
                 * objects needed to render and manage a scene.
                 */
                m_ramsesBundle = new RamsesBundle(m_context);

                // start update loop to process scene loading callbacks
                startUpdateLoop();

                /*
                 * Only I/O exception gets caught here loading the scene from file.
                 * When using the scene object later it has to be checked that the scene is not null.
                 */
                try {
                    //Loads and publishes Ramses scene from file descriptors and loads logic content.
                    m_sceneLoaded = m_ramsesBundle.loadScene(fdRamses, offsetRamses, fdRlogic, offsetRlogic);
                } catch (IOException e) {
                    Log.e(m_threadName, "initRamsesThreadAndLoadScene: IOException occurred while loading scene from " + logText, e);
                }

                if (m_sceneLoaded) {
                    Log.i(m_threadName, "initRamsesThreadAndLoadScene: Scene has been loaded successfully from " + logText);
                    try {
                        onSceneLoaded();
                    } catch (Exception e) {
                        Log.e(m_threadName, "initRamsesThreadAndLoadScene: The function onSceneLoaded threw an " +
                                e.getCause() + "Look at stacktrace for further info: ", e);
                    }
                } else {
                    m_ramsesBundle.dispose();
                    m_ramsesBundle = null;

                    Log.e(m_threadName, "initRamsesThreadAndLoadScene: Scene loading from " + logText + " failed!");
                    try {
                        onSceneLoadFailed();
                    } catch (Exception e) {
                        Log.e(m_threadName, "initRamsesThreadAndLoadScene: The function onSceneLoadFailed threw an " +
                                e.getCause() + "Look at stacktrace for further info: ", e);
                    }
                }
                m_updateLoopRunning = false; // stop update loop until it's started with rendering
            }
        });
    }

    /**
     * This function sets up the RamsesThread.
     * <p>
     * That includes:
     * <ul>
     * <li>Creating a RamsesBundle</li>
     * <li>Loading ramses scene and logic from given File Descriptors</li>
     * <li>Calling the RamsesThreads onSceneLoaded or onSceneLoadFailed callbacks depending on the loading success</li>
     * </ul>
     * This method initializes the RamsesThread so don't queue it to RamsesThread with addRunnableToThreadQueue
     * <p>
     * It is typically called in the activities onCreate or fragments onViewCreated Callback to set up.
     * If initialization fails for some reason, C++ resources will be automatically cleaned up, but Android thread will keep running.
     * In order to reinitialize RamsesThread after failed or successful initialization,
     * the object first must be cleaned-up with destroyRamsesBundleAndQuitThread().
     * </p>
     *
     * @param fdRamses file descriptor of ramses scene; cannot be null
     * @param offsetRamses offset of ramses scene file descriptor
     * @param fdRlogic file descriptor of ramses logic engine
     * @param offsetRlogic offset of ramses logic engine file descriptor
     */
    public void initRamsesThreadAndLoadScene(@NotNull final ParcelFileDescriptor fdRamses, final long offsetRamses, final ParcelFileDescriptor fdRlogic, final long offsetRlogic) {
        initRamsesThread();
        loadSceneFromFileDescriptor(fdRamses, offsetRamses, fdRlogic, offsetRlogic, "File Descriptors");
    }

    /**
     * This function sets up the RamsesThread.
     * <p>
     * That includes:
     * <ul>
     * <li>Creating a RamsesBundle</li>
     * <li>Loading ramses scene and logic from the applications assets</li>
     * <li>Calling the RamsesThreads onSceneLoaded or onSceneLoadFailed callbacks depending on the loading success</li>
     * </ul>
     * This method initializes the RamsesThread so don't queue it to RamsesThread with addRunnableToThreadQueue
     * <p>
     * It is typically called in the activities onCreate or fragments onViewCreated Callback to set up.
     * If initialization fails for some reason, C++ resources will be automatically cleaned up, but Android thread will keep running.
     * In order to reinitialize RamsesThread after failed or successful initialization,
     * the object first must be cleaned-up with destroyRamsesBundleAndQuitThread().
     * </p>
     *
     * @param assetManager   the AssetManager used to load scene and logic from File Descriptor
     * @param ramsesFileName the file name of the ramses scene that should be loaded
     * @param logicFileName  the file name of the ramses logic that should be loaded
     */
    public void initRamsesThreadAndLoadScene(final AssetManager assetManager, final String ramsesFileName, final String logicFileName) {

        initRamsesThread();

        AssetFileDescriptor fdRamses = null;
        AssetFileDescriptor fdRlogic = null;

        try {
            fdRamses = assetManager.openFd(ramsesFileName);
            fdRlogic = assetManager.openFd(logicFileName);
        } catch (IOException e) {
            Log.e(m_threadName, "initRamsesThreadAndLoadScene: IOException occurred while opening assets " + ramsesFileName + ", " + logicFileName, e);
        }

        //Keep RamsesThread behavior intact, that onSceneLoadFailed is called when loading scene fails
        if (fdRamses == null || fdRlogic == null)
        {
            m_ramsesThreadHandler.post(new Runnable() {
                @Override
                public void run() {
                    try {
                        onSceneLoadFailed();
                    } catch (Exception e) {
                        Log.e(m_threadName, "initRamsesThreadAndLoadScene: The function onSceneLoadFailed threw an " +
                                e.getCause() + "Look at stacktrace for further info: ", e);
                    }
                }
            });
        } else {
            loadSceneFromFileDescriptor(fdRamses.getParcelFileDescriptor(), fdRamses.getStartOffset(), fdRlogic.getParcelFileDescriptor(), fdRlogic.getStartOffset(),
                    ramsesFileName + ", " + logicFileName);
        }
    }

    private void startUpdateLoop() {
        m_updateLoopRunning = true;
        Log.i(m_threadName, "startUpdateLoop: Update loop started running");

        m_ramsesThreadHandler.post(new Runnable() {
            public void run() {

                if (!m_updateLoopRunning) {
                    Log.i(m_threadName, "startUpdateLoop: Update loop stopped running");
                    return;
                }
                updateSceneAndLogic();
                m_ramsesThreadHandler.postDelayed(this, m_updateDelay);
            }
        });
    }

    /**
     * Sets the rates of rendering and logic update.
     * <p>
     * Only values in [1-240] range are accepted. The default value is 60 fps.
     * <p>
     * Though the same rate is applied to update and render logic, it is not guaranteed that they will update synchronously, since rendering and logic are processed in different threads.
     * <p>
     * This method must be called on RamsesThread since it changes renderer's internal state.
     *
     * @param framerate of rendering and logic updates (fps)
     * @throws IllegalThreadStateException if called outside of ramses thread
     * @throws IllegalStateException       if called before a display was created or after it was destroyed
     * @throws IllegalArgumentException    if provided framerate is outside of accepted range
     */
    public void setRenderingFramerate(final float framerate) {
        ensureRamsesThreadAlive("setRenderingFramerate");
        ensureRamsesBundleCreated("setRenderingFramerate");

        if (!m_ramsesBundle.isDisplayCreated()) {
            throw new IllegalStateException("setRenderingFramerate: called before creating a display!");
        }

        if (framerate < m_minFramerate || framerate > m_maxFramerate) {
            throw new IllegalArgumentException("setRenderingFramerate: "
                    + framerate + " not applied, should be in [" + m_minFramerate + ", " + m_maxFramerate + "] range.");
        } else {
            Log.i(m_threadName, "setRenderingFramerate: Framerate has been set to "
                    + framerate);
        }
        m_renderingFramerate = framerate;
        m_updateDelay = (long) (1000 / m_renderingFramerate);

        m_ramsesBundle.setMaximumFramerate(m_renderingFramerate);
    }

    /**
     * This function creates a ramses display and shows the ramses scene if it has been successfully loaded before.
     * <p>
     * OnDisplayResize will be triggered by this function, to be able to react accordingly by adapting viewport etc. Also the clear color of the created display can be set by providing a non-null input.
     * <p>
     * This function is executed within RamsesThread so don't queue it to RamsesThread with addRunnableToThreadQueue if not specifically intended
     * <p>
     * It is typically called in the SurfaceViews onSurfaceCreated or in the TextureViews onSurfaceTextureAvailable callback.
     *
     * @param surface       the surface that should get the ramses scene assigned
     * @param clearColor    Clear color of the created Ramses Display. The default value for the clear color is black without alpha.
     * @throws InterruptedException        if the caller thread gets interrupted while blocking on this method
     * @throws IllegalStateException       if called twice without destroying the display first with call to destroyDisplay
     * @throws IllegalThreadStateException if called on ramses thread
     */
    public void createDisplayAndShowScene(final Surface surface, @Nullable final ClearColor clearColor) throws InterruptedException, IllegalStateException {
        createDisplayAndShowSceneInternal(surface, clearColor, 1);
    }

    /**
     * This is an overload of createDisplayAndShowScene(surface, clearColor) which also accepts a MSAA sample count
     *
     * @param surface       the surface that should get the ramses scene assigned
     * @param clearColor    Clear color of the created Ramses Display. The default value for the clear color is black without alpha.
     * @param msaaSamples   MSAA sample count for the created Ramses Display. The default value is '1' i.e. disabled
     * @throws InterruptedException        if the caller thread gets interrupted while blocking on this method
     * @throws IllegalStateException       if called twice without destroying the display first with call to destroyDisplay
     * @throws IllegalThreadStateException if called on ramses thread
     */
    public void createDisplayAndShowScene(final Surface surface, @Nullable final ClearColor clearColor, int msaaSamples) throws InterruptedException, IllegalStateException {
        createDisplayAndShowSceneInternal(surface, clearColor, msaaSamples);
    }

    private void createDisplayAndShowSceneInternal(final Surface surface, @Nullable final ClearColor clearColor, int msaaSamples) throws InterruptedException, IllegalStateException {
        ensureRamsesThreadAlive("createDisplayAndShowScene");
        ensureNotRunningInRamsesThread("createDisplayAndShowScene");
        RamsesBundle.ensureMsaaSampleCountValid(msaaSamples, "RamsesThread.createDisplayAndShowSceneInternal()");

        final CompletableFuture<Boolean> futureWindowInitialized = new CompletableFuture<>();
        m_ramsesThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(m_threadName, "createDisplayAndShowScene: creating display");
                ensureRamsesBundleCreated("createDisplayAndShowScene");

                boolean success = m_ramsesBundle.createDisplay(surface, clearColor, msaaSamples);

                if (success) {
                    int[] displaySize = m_ramsesBundle.getDisplaySize();
                    if (displaySize == null) {
                        throw new AssertionError("This should never happen; after displayCreation there should always be a size");
                    }
                    try {
                        onDisplayResize(displaySize[0], displaySize[1]);
                    } catch (Exception e) {
                        Log.e(m_threadName, "resizeDisplay: The function resizeDisplay threw an " +
                                e.getCause() + "Look at stacktrace for further info: ", e);
                    }
                }
                // Synchronization needed to ensure createDisplay has exclusive access to surface handle
                futureWindowInitialized.complete(success);

                // Immediately shows the scene on the newly created display
                if (m_sceneLoaded) {
                    Log.i(m_threadName, "createDisplayAndShowScene: Show scene now");
                    m_ramsesBundle.showScene();
                }
            }
        });
        try {
            if (!futureWindowInitialized.get()) {
                throw new IllegalStateException("createDisplayAndShowScene: " +
                        "Display creation failed! Please look at the logs for further information");
            }
        } catch (ExecutionException e) {
            // This should never happen, as there is no "completeExceptionally" call on the future
            throw new AssertionError("An internal error occurred in " + m_threadName +
                    "createDisplayAndShowScene. For more information look at the stacktrace.", e);
        }
    }

    /**
     * This function destroys the display that got the ramses scene assigned to it.
     * <p>
     * It is typically called in the SurfaceViews onSurfaceDestroyed or the TextureViews onSurfaceTextureDestroyed.
     * <p>
     * This method is executed within RamsesThread so don't queue it to RamsesThread with addRunnableToThreadQueue
     *
     * @throws InterruptedException        if the caller thread gets interrupted while blocking on this method
     * @throws IllegalStateException       if called without having created a display or if display was already destroyed
     * @throws IllegalThreadStateException if called on ramses thread
     */
    public void destroyDisplay() throws InterruptedException {
        ensureRamsesThreadAlive("destroyDisplay");
        ensureNotRunningInRamsesThread("destroyDisplay");

        // as this will destroy the renderer it has to be synchronized, otherwise calls to
        // null object could happen and crash the app
        final CompletableFuture<Boolean> futureSurfaceDestroyed = new CompletableFuture<>();
        m_ramsesThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(m_threadName, "destroyDisplay: destroying display");
                // Destroy the ramses display (this implicitly also hides the scene)
                ensureRamsesBundleCreated("destroyDisplay");
                futureSurfaceDestroyed.complete(m_ramsesBundle.destroyDisplay());
            }
        });
        try {
            if (!futureSurfaceDestroyed.get()) {
                throw new IllegalStateException("destroyDisplay: trying to destroy display that wasn't created or was already destroyed!");
            }
        } catch (ExecutionException e) {
            // This should never happen, as there is no "completeExceptionally" call on the future
            throw new AssertionError("An internal error occurred in " + m_threadName +
                    "destroyDisplay. For more information look at the stacktrace.", e);
        }
    }

    /**
     * Causes Ramses to start the rendering thread, as well as the logic update loop and render the scene if already loaded.
     * <p>
     * Make sure to check that there is a created display(isDisplayCreated) and that the rendering thread is not already running(isRendering) before calling this method.
     * <p>
     * If no scene is loaded, the call will succeed but no content will be rendered afterwards.
     * <p>
     * Typically, this should be called when the activity/fragment is "resumed" and "focused", based on application requirements
     *
     * @throws IllegalThreadStateException if called outside of ramses thread.
     * @throws IllegalStateException       if ramses display has not been created yet or if the render thread is already running.
     * @throws IllegalStateException       if called before ramses bundle was created or after it was destoryed.
     */
    public void startRendering() throws IllegalStateException {
        ensureRunningInRamsesThread("startRendering");
        ensureRamsesBundleCreated("startRendering");

        if (!isSceneLoaded()) {
            Log.w(m_threadName, "startRendering: trying to start update and rendering before scene was loaded. No content will be shown!");
        }
        Log.i(m_threadName, "startRendering: starting to update and render!");

        // TODO Violin/Asko we currently always start the update loop, even if there is no renderer to "consume" the frames
        // This might (and as usual probably will!) lead to applications abusing the CPU with frame updates which are not shown anywhere because
        // e.g. the view/surface are not visible, paused etc.. There are various ways to improve this, I am listing two approaches here:
        // 1. Don't do "thread" but a on-dirty-update (renders single frame any time the surface is dirty, only do heavy stuff in thread (load scene))
        // 2. Perform a single update loop when scene loaded initially, and after that couple the update loop to isRendering==true

        startUpdateLoop();
        m_ramsesBundle.setMaximumFramerate(m_maxFramerate);
        m_ramsesBundle.startRendering();
    }

    /**
     * Causes Ramses to stop the rendering thread and update logic loop.
     * <p>
     * Make sure that the render thread is indeed running with the method isRendering before calling this function.
     * <p>
     * Typically this should be called when the activity/fragment is "paused" and "not focused", based on application requirements.
     *
     * @throws IllegalThreadStateException if called outside of ramses thread.
     * @throws IllegalStateException       if called before ramses bundle was created or after it was destroyed.
     * @throws IllegalStateException       if the render thread is not running.
     */
    public void stopRendering() throws IllegalStateException {
        ensureRunningInRamsesThread("stopRendering");
        ensureRamsesBundleCreated("stopRendering");

        Log.i(m_threadName, "stopRendering: stopping to update and render!");
        m_updateLoopRunning = false;
        m_ramsesBundle.stopRendering();
    }

    /**
     * Checks whether the ramses rendering thread is running.
     *
     * @return whether ramses rendering thread is running
     * @throws IllegalThreadStateException if called outside of ramses thread.
     * @throws IllegalStateException       if called before ramses bundle was created or after it was destroyed.
     */
    public boolean isRendering() {
        ensureRunningInRamsesThread("isRendering");

        return (m_ramsesBundle != null) && m_ramsesBundle.isRendering();
    }

    /**
     * Checks if RamsesThread is alive.
     * <p>
     * The function first checks if the thread is properly initialized, then queries the underlying HandlerThread's state.
     * HandlerThread is instantiated/started by initRamsesThreadAndLoadScene and stopped/destroyed by destroyRamsesBundleAndQuitThread.
     * </p>
     *
     * @return whether the RamsesThread is alive
     */
    public boolean isAlive() {
        return m_workerThread != null && m_workerThread.isAlive() && m_ramsesThreadHandler != null;
    }

    /**
     * Checks if the display was created.
     *
     * @return whether the display is created
     * @throws IllegalThreadStateException if called outside of ramses thread.
     * @throws IllegalStateException       if called before ramses bundle was created or after it was destroyed.
     */
    public boolean isDisplayCreated() {
        ensureRunningInRamsesThread("isDisplayCreated");
        ensureRamsesBundleCreated("isDisplayCreated");

        return m_ramsesBundle.isDisplayCreated();
    }

    /**
     * This function disposes the RamsesBundle and quits the RamsesThread.
     * <p>
     * It is typically called in the activities/fragments onDestroy.
     * <p>
     * This method should not be queued to RamsesThread with addRunnableToThreadQueue
     *
     * @throws InterruptedException        if the caller thread gets interrupted while blocking on this method
     * @throws IllegalStateException       if the thread is not alive when destroying (e.g. when the object was already destroyed or if called before invoking initRamsesThreadAndLoadScene())
     * @throws IllegalThreadStateException if called on ramses thread
     */
    public void destroyRamsesBundleAndQuitThread() throws InterruptedException, IllegalStateException {
        ensureRamsesThreadAlive("destroyRamsesBundleAndQuitThread");
        ensureNotRunningInRamsesThread("destroyRamsesBundleAndQuitThread");

        final CompletableFuture<Boolean> futureRamsesDisposed = new CompletableFuture<>();
        m_ramsesThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                // stop updating logic
                m_updateLoopRunning = false;

                // no more runnables will be executed after this runnable, even if they are already queued
                Looper.myLooper().quit();

                /*
                 * Cleans up all underlying c++ objects for the RamsesBundle. As the Properties just
                 * contain pointers to c++ objects of the RamsesBundle, they don't have to be disposed
                 * separately.
                 */
                if (m_ramsesBundle != null) {
                    Log.i(m_threadName, "destroyRamsesBundleAndQuitThread: disposing RamsesBundle");
                    m_ramsesBundle.dispose();
                    m_ramsesBundle = null;
                    m_sceneLoaded = false;
                } else {
                    Log.i(m_threadName, "destroyRamsesBundleAndQuitThread: not destroying RamsesBundle since none is instantiated.");
                }
                futureRamsesDisposed.complete(true);
            }
        });

        /*
         * disposing of RamsesBundle has to be finished together with the activity's/fragments onDestroy method
         * otherwise there can be crashes, because objects are deleted and accessed at the wrong time
         */
        try {
            futureRamsesDisposed.get();
        } catch (ExecutionException e) {
            // This should never happen, as there is no "completeExceptionally" call on the future
            throw new AssertionError("An internal error occurred in " + m_threadName +
                    "destroyRamsesBundleAndQuitThread. For more information look at the stacktrace.", e);
        }

        m_ramsesThreadHandler = null;

        m_workerThread.quitSafely();
        m_workerThread.join();
        m_workerThread = null;
    }

    /**
     * This function resizes the display that has the scene assigned to it.
     * <p>
     * It is typically called in the SurfaceViews onSurfaceChanged or TextureViews onSurfaceTextureSizeChanged.
     * <p>
     * This method queues work to the RamsesThread work queue so don't queue it to RamsesThread with addRunnableToThreadQueue on top if not specifically intended
     *
     * @param width  The width the display should be resized to
     * @param height The height the display should be resized to
     * @throws IllegalStateException if the thread is not alive when destroying (e.g. when the object was already destroyed or if called before invoking initRamsesThreadAndLoadScene())
     * @throws IllegalStateException if called before ramses bundle was created or after it was destroyed.
     */
    public void resizeDisplay(final int width, final int height) {
        ensureRamsesThreadAlive("resizeDisplay");

        m_ramsesThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                ensureRamsesBundleCreated("resizeDisplay");
                /*
                 * Tell Ramses that the display was resized, and adapt the scene's viewport to the
                 * new size and aspect ratio.
                 */
                m_ramsesBundle.resizeDisplay(width, height);

                if (m_sceneLoaded) {
                    Log.i(m_threadName, "resizeDisplay: Resizing display to "
                            + width + " x " + height);
                    try {
                        onDisplayResize(width, height);
                    } catch (Exception e) {
                        Log.e(m_threadName, "resizeDisplay: The function resizeDisplay threw an " +
                                e.getCause() + "Look at stacktrace for further info: ", e);
                    }
                }
            }
        });
    }

    /**
     * Adds a Runnable to the RamsesThreads work queue and thus makes sure there are no no thread related problems with executing the runnable code.
     * <p>
     * Only call this function after calling initRamsesThreadAndLoadScene, otherwise an IllegalThreadStateException will be thrown because of uninitialized Thread Handler.
     *
     * @param runnable the piece of code the should be executed in this RamsesThread
     * @throws IllegalThreadStateException if the Thread Handler wasn't initialized before calling this function
     */
    public void addRunnableToThreadQueue(Runnable runnable) {
        ensureRamsesThreadAlive(m_threadName + ".addRunnableToThreadQueue");
        m_ramsesThreadHandler.post(runnable);
    }

    /**
     * This is a wrapper function for RamsesBundle.getInterface to keep the RamsesBundle member variable private.
     * <p>
     * This will avoid problems resulting from multithreaded access to the RamsesBundle.
     *
     * @param interfaceName the name of the logic node whose root input is of interest
     * @return root input of given logic node name or null if the logic node doesn't exist
     * @throws IllegalThreadStateException if the function gets called from another thread
     * @throws IllegalStateException       if called before ramses bundle was created or after it was destroyed.
     */
    public Property getInterface(String interfaceName) {
        ensureRunningInRamsesThread("getInterface");
        ensureRamsesBundleCreated("getInterface");
        return m_ramsesBundle.getInterface(interfaceName);
    }

    /**
     * This is a wrapper function for RamsesBundle.getLogicNodeRootInput to keep the RamsesBundle member variable private.
     * <p>
     * This will avoid problems resulting from multithreaded access to the RamsesBundle.
     *
     * @param logicNodeName the name of the logic node whose root input is of interest
     * @return root input of given logic node name or null if the logic node doesn't exist
     * @throws IllegalThreadStateException if the function gets called from another thread
     * @throws IllegalStateException       if called before ramses bundle was created or after it was destroyed.
     */
    public Property getLogicNodeRootInput(String logicNodeName) {
        ensureRunningInRamsesThread("getLogicNodeRootInput");
        ensureRamsesBundleCreated("getLogicNodeRootInput");
        return m_ramsesBundle.getLogicNodeRootInput(logicNodeName);
    }

    /**
     * This is a wrapper function for RamsesBundle.linkProperties to keep the RamsesBundle member variable private.
     * <p>
     * This will avoid problems resulting from multithreaded access to the RamsesBundle.
     *
     * @param sourceProperty the source property (must be an output - see getLogicNodeRootOutput)
     * @param targetProperty the target property (must be an input - see getLogicNodeRootInput)
     * @return true if the properties were linked successfully, false otherwise
     */
    public boolean linkProperties(Property sourceProperty, Property targetProperty) {
        ensureRunningInRamsesThread("linkProperties");
        ensureRamsesBundleCreated("linkProperties");
        return m_ramsesBundle.linkProperties(sourceProperty, targetProperty);
    }

    /**
     * This is a wrapper function for RamsesBundle.unlinkProperties to keep the RamsesBundle member variable private.
     * <p>
     * This will avoid problems resulting from multithreaded access to the RamsesBundle.
     *
     * @param sourceProperty the source property (must be an output - see getLogicNodeRootOutput)
     * @param targetProperty the target property (must be an input - see getLogicNodeRootInput)
     * @return true if the properties were unlinked successfully, false otherwise
     */
    public boolean unlinkProperties(Property sourceProperty, Property targetProperty) {
        ensureRunningInRamsesThread("unlinkProperties");
        ensureRamsesBundleCreated("unlinkProperties");
        return m_ramsesBundle.unlinkProperties(sourceProperty, targetProperty);
    }

    /**
     * This is a wrapper function for RamsesBundle.getLogicNodeRootOutput to keep the RamsesBundle member variable private.
     * <p>
     * This will avoid problems resulting from multithreaded access to the RamsesBundle. Output properties can be only read, not written. See also onLogicUpdated().
     *
     * @param logicNodeName the name of the logic node whose root output is of interest
     * @return root output of given logic node name or null if no logic node with the given name was found in the logic engine
     * @throws IllegalThreadStateException if the function gets called from another thread
     */
    public Property getLogicNodeRootOutput(String logicNodeName) {
        ensureRunningInRamsesThread("getLogicNodeRootOutput");
        return m_ramsesBundle.getLogicNodeRootOutput(logicNodeName);
    }

    /**
     * Gets the status whether a scene has been loaded
     *
     * @return whether a secen has been loaded
     * @throws IllegalThreadStateException if the function gets called from another thread
     */
    public boolean isSceneLoaded() {
        ensureRunningInRamsesThread("isSceneLoaded");
        return m_sceneLoaded;
    }

    /**
     * Gets the rendering Framerate
     *
     * @return rendering Framerate
     * @throws IllegalThreadStateException if the function gets called from another thread
     */
    public float getRenderingFramerate() {
        ensureRunningInRamsesThread("getRenderingFramerate");
        return m_renderingFramerate;
    }

    /**
     * Get feature level of currently loaded logic engine
     * @return feature level of logic engine, or 0 in case of an error
     */
    public long getFeatureLevel() {
        ensureRunningInRamsesThread("getFeatureLevel");
        ensureRamsesBundleCreated("gertFeatureLevel");
        return m_ramsesBundle.getFeatureLevel();
    }

    private void ensureRunningInRamsesThread(String functionName) {
        if (m_workerThread == null) {
            String errorMessage = "Ramses Thread not initialized yet.";
            Log.e(m_threadName, errorMessage);
            throw new IllegalThreadStateException(errorMessage);
        }
        if (Thread.currentThread().getId() != m_workerThread.getId()) {
            String errorMessage = "Calling Function RamsesThread." + functionName +
                    " from outside the thread is not allowed";
            Log.e(m_threadName, errorMessage);
            throw new IllegalThreadStateException(errorMessage);
        }
    }

    private void ensureRamsesBundleCreated(String functionName) {
        if (m_ramsesBundle == null) {
            Log.e(m_threadName, "Calling function RamsesThread." + functionName +
                    " before Ramses Bundle was created or after it was destroyed.");
            throw new IllegalStateException("Calling Function RamsesThread." + functionName +
                    " before Ramses Bundle was created or after it was destroyed.");
        }
    }

    private void ensureNotRunningInRamsesThread(String functionName) {
        if (m_workerThread == null) {
            String errorMessage = "Ramses Thread not initialized yet.";
            Log.e(m_threadName, errorMessage);
            throw new IllegalThreadStateException(errorMessage);
        }
        if (Thread.currentThread().getId() == m_workerThread.getId()) {
            Log.e(m_threadName, "Calling function RamsesThread." + functionName +
                    " from inside ramses thread is not allowed");
            throw new IllegalThreadStateException("Calling Function RamsesThread." + functionName +
                    " from inside ramses thread is not allowed");
        }
    }

    private void ensureRamsesThreadAlive(String functionName) {
        if (!this.isAlive() || m_ramsesThreadHandler == null) {
            throw new IllegalThreadStateException(functionName + " called on RamsesThread that is not alive (either not initialized or already destroyed).");
        }
    }

    /**
     * This callback is called before every logic update. Inputs that change every update frame should be set here.
     * <p>
     * If setting inputs here, the resulting output values can be read out in onLogicUpdated.
     * <p>
     * It is executed within RamsesThread so if there is no reason for it, don't use addRunnableToThreadQueue in here
     */
    protected abstract void onUpdate();

    /**
     * This callback gets called when the display resizes. This is a good place to update viewport etc. to new size.
     * <p>
     * It is executed within RamsesThread so if there is no reason for it, don't use addRunnableToThreadQueue in here
     *
     * @param width  width of the display
     * @param height height of the display
     */
    protected abstract void onDisplayResize(int width, int height);

    /**
     * This callback is called if loading the scene was successful. This is a good place to initialize Properties.
     * <p>
     * It is executed within RamsesThread so if there is no reason for it, don't use addRunnableToThreadQueue in here
     */
    protected abstract void onSceneLoaded();

    /**
     * This callback is called if loading the scene failed and allows to react on this scenario
     * <p>
     * It is executed within RamsesThread so if there is no reason for it, don't use addRunnableToThreadQueue in here
     */
    protected abstract void onSceneLoadFailed();

    /**
     * This method is called after the scene logic was executed, but before the frame is submitted for rendering.
     * <p>
     * In this time frame, the logic engine outputs have well defined values and reading them will produce deterministic results.
     * <p>
     * Use this to read out (but not write!) scene state data, like LogicNode outputs (see getLogicNodeRootOutput() ).
     * <p>
     * This method executed within RamsesThread so if there is no reason for it, don't use addRunnableToThreadQueue in here
     */
    protected abstract void onLogicUpdated();

    private RamsesBundle m_ramsesBundle;
    private Handler m_ramsesThreadHandler;
    private HandlerThread m_workerThread;
    private Context m_context;

    private boolean m_sceneLoaded = false;
    private boolean m_updateLoopRunning = true;
    private float m_renderingFramerate = 60f;
    private long m_updateDelay = (long) (1000 / m_renderingFramerate); // inverse value of the update rate, used internally for logic updates
    private final String m_threadName;

    private final float m_minFramerate = 1f;
    private final float m_maxFramerate = 240f;
}
