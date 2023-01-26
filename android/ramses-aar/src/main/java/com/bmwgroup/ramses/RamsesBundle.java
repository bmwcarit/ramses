//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

import android.content.Context;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.Surface;

import org.jetbrains.annotations.Nullable;

import java.io.IOException;
import java.util.Arrays;

/**
 * This class bundles ramses scene and ramses logic engine. This enables the user to have a single
 * object for rendering and interacting with a scene and its logic.
 * The render thread renders with a default framerate of 60fps. This can be changed with setMaximumFramerate(). To make it easy on the user this
 * object manages all underlying c++ objects that are needed to keep the scene in a rendered state and apply per-frame updates. Creating
 * a RamsesBundle and disposing it when not needed anymore ensures that there won't be any memory
 * leaks caused by hidden c++ objects of the ramses and ramses logic world.
 * The RamsesBundle is not thread safe. This means if the user wants to use it in a multi threaded scenario
 * the responsibility of thread safety lies in the hand of the user.
 */
public class RamsesBundle
{
    static {
        System.loadLibrary("RamsesJNI");
    }

    /**
     * Creates a RamsesBundle with potential debugging functionality
     * <p>
     * Passing a non-null context to the constructor activates debugging functionality by enabling shell access to ramses through broadcasting an intent with adb.
     *
     * @param context Context used to register BroadcastReceiver
     */
    public RamsesBundle(@Nullable Context context) {
        m_nativeHandle = createRamsesBundle();
        if (context != null) {
            // this will make it possible to trigger ramsh commands in the running application
            // by sending a broadcast to it (adb, or other application)
            context.registerReceiver(new RamshCommandBroadcastReceiver(this), RamshCommandBroadcastReceiver.getRamshIntentFilter());
        }
    }

    /**
     * Cleans up all underlying c++ objects of the RamsesBundle.
     * <p>
     * After dispose all calls on this RamsesBundle and of every Property referencing it will throw exceptions and cannot be used anymore from this point on.
     */
    public void dispose() {
        if (!m_nativeObjectDisposed) {
            dispose(m_nativeHandle);
            m_nativeObjectDisposed = true;
        }
    }

    /**
     * Last resort warning showing the user that he hasn't disposed the underlying c++ objects before the java object has been garbage collected.
     */
    @Override
    protected void finalize() {
        if (!m_nativeObjectDisposed) {
            dispose();
            m_nativeObjectDisposed = true;
            Log.e("RamsesBundle", "RamsesBundle.finalize() is executed and disposing" +
                    "RamsesBundle. Disposing should be done by the user before the object gets garbage collected!");
        }
    }

    /**
     * Creates a ramses display
     * @param surface the Android surface used to create a ramses display
     * @param clearColor the color with which the display will be cleared (default is 0, 0, 0, 0)
     * @return if the display creation was successful
     */
    public boolean createDisplay(Surface surface, @Nullable ClearColor clearColor) {
        ClearColor color = (clearColor != null) ? clearColor : new ClearColor(0.f, 0.f, 0.f, 0.f);
        return createDisplay(m_nativeHandle, surface, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha(), 1);
    }

    /**
     * Creates a ramses display
     * @param surface the Android surface used to create a ramses display
     * @param clearColor the color with which the display will be cleared (default is 0, 0, 0, 0)
     * @param msaaSamples the number of MSAA samples to use (can be 1, 2, 4, 8) where 1 is a special value which disables MSAA
     * @return if the display creation was successful
     */
    public boolean createDisplay(Surface surface, @Nullable ClearColor clearColor, int msaaSamples) {
        ClearColor color = (clearColor != null) ? clearColor : new ClearColor(0.f, 0.f, 0.f, 0.f);
        ensureMsaaSampleCountValid(msaaSamples, "RamsesBundle.createDisplay()");
        return createDisplay(m_nativeHandle, surface, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha(), msaaSamples);
    }

    /**
     * Destroys the ramses display. If a scene is currently shown, hides the scene before destroying the display.
     * @return if destroying the display was successful
     */
    public boolean destroyDisplay() {
        return destroyDisplay(m_nativeHandle);
    }

    /**
     * Tells Ramses that the native Android window it uses for rendering was resized to resize ramses display accordingly.
     *
     * This is currently not implemented and thus has no effect.
     * @param width the new width of the resized display
     * @param height the new height of the resized display
     */
    public void resizeDisplay(int width, int height) {
        resizeDisplay(m_nativeHandle, width, height);
    }

    /**
     * Sets the maximum processing rate per second for the update/render loop.
     * <p>
     * The parameter is of type float in order to specify any desired frame time (e.g. below 1 FPS).
     * @param maximumFramerate The maximum frame rate per second to set for the render loop.
     *
     * @throws IllegalStateException if ramses display has not been created yet.
     */
    public void setMaximumFramerate(float maximumFramerate) {
        if (!setMaximumFramerate(m_nativeHandle, maximumFramerate)) {
            throw new IllegalStateException("RamsesBundle.setMaximumFramerate failed! Please make sure the ramses display has been created " +
                    "and ramses is not already rendering");
        }
    }

    /**
     * Starts the ramses rendering thread.
     * <p>
     * If the framerate was not set earlier with setMaxFramerate, rendering will start with the default 60fps.
     * <p>
     * Make sure you are also updating and flashing the scene with updateLogic and flushRamsesScene, otherwise the same image will be rendered.
     * @throws IllegalStateException if ramses display is not created or if the render thread is already running.
     */
    public void startRendering() throws IllegalStateException {
        if (!startRendering(m_nativeHandle)) {
            throw new IllegalStateException("RamsesBundle.startRendering failed! Please make sure the ramses display has been created " +
                    "and ramses is not already rendering");
        }
    }

    /**
     * Stops the ramses rendering thread and pauses scene updates.
     * <p>
     * Unless the display was also destroyed or a scene unloaded, this will cause the rendering to freeze until startRendering is called again.
     * @throws IllegalStateException if the render thread is not running.
     */
    public void stopRendering() throws IllegalStateException{
        if (!stopRendering(m_nativeHandle)) {
            throw new IllegalStateException("RamsesBundle.stopRendering failed! Please make sure ramses is rendering before calling this function");
        }
    }

    /**
     * Checks whether the ramses rendering thread is running.
     * @return whether ramses rendering thread is running
     */
    public boolean isRendering() {
        return isRendering(m_nativeHandle);
    }

    /**
     * Checks if the ramses display and renderer are created and initialized.
     * @return whether ramses display and renderer are created and initialized
     */
    public boolean isDisplayCreated() {
        return isDisplayCreated(m_nativeHandle);
    }

    /**
     * Loads a ramses scene and ramses logic from file descriptors.
     * <p>
     * The scene is in a hidden state after loading and must be shown with showScene() and a renderer must be started (startRenderer()) in order for the scene to be visible on a screen.
     * <p>
     * The user has ownership of the ParcelFileDescriptors that are passed into this function.
     * @param fdRamses file descriptor of ramses scene
     * @param fdRamsesOffset offset of ramses scene file descriptor
     * @param fdRlogic file descriptor of ramses logic engine
     * @param fdRlogicOffset offset of ramses logic engine file descriptor
     * @return success of loading the ramses scene
     * @throws IOException if something with the native file descriptors goes wrong
     * @throws IllegalArgumentException if no valid ramses scene was given as argument
     */
    public boolean loadScene(ParcelFileDescriptor fdRamses, long fdRamsesOffset,
                          ParcelFileDescriptor fdRlogic, long fdRlogicOffset) throws IOException {
        boolean success = false;
        if (fdRlogic != null && fdRamses != null) {
            success = loadScene(m_nativeHandle, fdRamses.dup().detachFd(), fdRamsesOffset, fdRamses.getStatSize(),
                            fdRlogic.dup().detachFd(), fdRlogicOffset, fdRlogic.getStatSize());
        }
        else if(fdRlogic == null && fdRamses != null) {
            success = loadScene(m_nativeHandle, fdRamses.dup().detachFd(), fdRamsesOffset, fdRamses.getStatSize(),
                            0, 0, 0);
        }
        else {
            throw new IllegalArgumentException("RamsesBundle.loadScene() is executed with invalid scene");
        }

        m_sceneLoaded = success;
        return success;
    }

    /**
     * Get feature level of currently loaded logic engine
     * @return feature level of logic engine, or 0 in case of an error
     */
    public long getFeatureLevel() {
        ensureSceneLoaded("getFeatureLevel");
        return getFeatureLevel(m_nativeHandle);
    }

    /**
     * Makes sure a scene is loaded to avoid calling functions that depend on it before the loaded state
     * @param callingFunctionName the name of the function calling ensureSceneLoaded
     * @throws IllegalStateException if scene isn't already loaded
     */
    private void ensureSceneLoaded(String callingFunctionName) {
        if (!m_sceneLoaded) {
            throw new IllegalStateException("Calling function RamsesBundle.'" + callingFunctionName +
                    "' before a scene is loaded is not allowed!");
        }
    }

    boolean sceneLoaded() {
        return m_sceneLoaded;
    }

    /**
     * Gets root input of an interface object with given name. This method will search exclusively
     * for interfaces and will ignore any other logic node which might have the same name. It's the
     * preferred way to obtain data from assets. Interfaces are guaranteed to have unique name
     * other than any other logic node type.
     * @param interfaceName the name of the interface of interest
     * @return root input of given interface or null if the interface doesn't exist
     */
    public Property getInterface(String interfaceName) {
        ensureSceneLoaded("getInterface");

        long nativeRlogicHandle = getInterface(m_nativeHandle, interfaceName);
        return (nativeRlogicHandle != 0) ? new Property(nativeRlogicHandle, this, false) : null;
    }

    /**
     * Gets root input of logic node with given name
     * @param logicNodeName the name of the logic node whose root input is of interest
     * @return root input of given logic node or null if the logic node doesn't exist
     */
    public Property getLogicNodeRootInput(String logicNodeName) {
        ensureSceneLoaded("getLogicNodeRootInput");

        long nativeRlogicHandle = getLogicNodeRootInput(m_nativeHandle, logicNodeName);
        return (nativeRlogicHandle != 0) ? new Property(nativeRlogicHandle, this, false) : null;
    }

    /**
     * Gets root output of logic node with given name.
     * <p>
     * The property will be read only so all setters on it will throw an IllegalStateException.
     * @param logicNodeName the name of the logic node whose root output is of interest
     * @return root output of given logic node or null if the logic node doesn't exist
     */
    public Property getLogicNodeRootOutput(String logicNodeName) {
        ensureSceneLoaded("getLogicNodeRootOutput");

        long nativeRlogicHandle = getLogicNodeRootOutput(m_nativeHandle, logicNodeName);
        return (nativeRlogicHandle != 0) ? new Property(nativeRlogicHandle, this, true) : null;
    }

    /**
     * Links sourceProperty to targetProperty.
     * <p>
     * Should call this in a thread-safe manner, preferably in the RamsesThread::onLogicUpdated() callback.
     * <p>
     * See the C++ method documentation for additional info:
     * https://ramses-logic.readthedocs.io/en/latest/classes/LogicEngine.html#_CPPv4N6rlogic11LogicEngine4linkERK8PropertyRK8Property
     * </p>
     * @param sourceProperty the source property (must be an output - see getLogicNodeRootOutput)
     * @param targetProperty the target property (must be an input - see getLogicNodeRootInput)
     * @return true if the properties were linked successfully, false otherwise
     */
    public boolean linkProperties(Property sourceProperty, Property targetProperty) {
        ensureSceneLoaded("linkProperties");

        if (sourceProperty == null || targetProperty == null) {
            throw new IllegalStateException("RamsesBundle.linkProperties failed! Please make sure that both properties aren't null!");
        }
        if (targetProperty.isLinked()){
            throw new IllegalStateException("RamsesBundle.linkProperties failed! TargetProperty: " + targetProperty.getName() + " is already linked!");
        }
        return linkProperties(m_nativeHandle, sourceProperty.getNativeHandle(), targetProperty.getNativeHandle());
    }

    /**
     * Unlinks sourceProperty from targetProperty.
     * <p>
     * Should call this in a thread-safe manner, preferably in the RamsesThread::onLogicUpdated() callback.
     * <p>
     * See the C++ method documentation for additional info:
     * https://ramses-logic.readthedocs.io/en/latest/classes/LogicEngine.html#_CPPv4N6rlogic11LogicEngine6unlinkERK8PropertyRK8Property
     * </p>
     * @param sourceProperty the source property (must be an output - see getLogicNodeRootOutput)
     * @param targetProperty the target property (must be an input - see getLogicNodeRootInput)
     * @return true if the properties were unlinked successfully, false otherwise
     */
    public boolean unlinkProperties(Property sourceProperty, Property targetProperty) {
        ensureSceneLoaded("unlinkProperties");
        if (sourceProperty == null || targetProperty == null) {
            throw new IllegalStateException("RamsesBundle.unlinkProperties failed! Please make sure that both properties aren't null!");
        }
        if (!targetProperty.isLinked()){
            throw new IllegalStateException("RamsesBundle.unlinkProperties failed! TargetProperty: " + targetProperty.getName() + " is not linked!");
        }
        return unlinkProperties(m_nativeHandle, sourceProperty.getNativeHandle(), targetProperty.getNativeHandle());
    }

    /**
     * Maps and shows a scene on the ramses renderer. This is a blocking operation until the scene reaches a "rendered" state in Ramses.
     * @return success of showing the scene
     */
    public boolean showScene() {
        ensureSceneLoaded("showScene");
        return showScene(m_nativeHandle);
    }

    /**
     * updates changes to ramses logic
     * @return success of updating the logic
     */
    public boolean updateLogic() {
        ensureSceneLoaded("updateLogic");
        return updateLogic(m_nativeHandle);
    }

    /**
     * flushes all changes to ramses scene
     * @return success of flushing the scene
     */
    public boolean flushRamsesScene() {
        ensureSceneLoaded("flushRamsesScene");
        return flushRamsesScene(m_nativeHandle);
    }

    /**
     * Empties the renderer event queue of ramses. This should be called regularly
     * to ensure the queue doesn't overflow.
     */
    public boolean dispatchRendererEvents() {
        return dispatchRendererEvents(m_nativeHandle);
    }

    static void ensureMsaaSampleCountValid(int sampleCount, String logText) {
        if (!Arrays.asList(1, 2, 4, 8).contains(sampleCount)) {
            throw new IllegalArgumentException(logText + " is executed with invalid msaa count " + sampleCount);
        }
    }

    /**
     * Provides info whether the java object can be garbage collected as all the underlying c++ objects have been disposed and thus any memory leaks are prevented.
     * @return whether native c++ objects have been disposed
     */
    public boolean nativeObjectDisposed() {
        return m_nativeObjectDisposed;
    }

    void executeRamshCommand(String command) {
        executeRamshCommand(m_nativeHandle, command);
    }

    void dumpSceneToFile(String filePath) {
        ensureSceneLoaded("dumpSceneToFile");
        dumpSceneToFile(m_nativeHandle, filePath);
    }

    void dumpLogicToFile(String filePath) {
        ensureSceneLoaded("dumpLogicToFile");
        dumpLogicToFile(m_nativeHandle, filePath);
    }

    /**
     * Gets the size of the created display
     * @return size of the created display or null if no display was created
     */
    protected int[] getDisplaySize() {
        return getDisplaySize(m_nativeHandle);
    }

    private final long m_nativeHandle;
    private boolean m_nativeObjectDisposed = false;
    private boolean m_sceneLoaded = false;

    private native long createRamsesBundle();
    private native boolean createDisplay(long handle, Surface surface, float red, float green, float blue, float alpha, int msaaSamples);
    private native boolean destroyDisplay(long handle);
    private native int[] getDisplaySize(long handle);
    private native void resizeDisplay(long handle, int width, int height);
    private native boolean setMaximumFramerate(long handle, float maximumFramerate);
    private native boolean startRendering(long handle);
    private native boolean stopRendering(long handle);
    private native boolean isRendering(long handle);
    private native boolean isDisplayCreated(long handle);
    private native void dispose(long handle);
    private native boolean loadScene(long handle, int fdRamses, long fdRamsesOffset, long fdRamsesLength,
                                  int fdRlogic, long lengthFdRlogic, long fdRlogicOffset);
    private native long getFeatureLevel(long handle);
    private native boolean showScene(long handle);
    private native boolean updateLogic(long handle);
    private native boolean flushRamsesScene(long handle);
    private native boolean dispatchRendererEvents(long handle);
    private native boolean linkProperties(long m_nativeHandle, long handleSource, long handleTarget);
    private native boolean unlinkProperties(long m_nativeHandle, long handleSource, long handleTarget);
    private native long getLogicNodeRootInput(long m_nativeHandle, String logicNodeName);
    private native long getLogicNodeRootOutput(long m_nativeHandle, String logicNodeName);
    private native long getInterface(long m_nativeHandle, String logicNodeName);
    private native void executeRamshCommand(long handle, String command);
    private native void dumpSceneToFile(long handle, String fileName);
    private native void dumpLogicToFile(long handle, String fileName);
}
