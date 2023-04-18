# RAMSES Renderer demo app

This is an example how a RAMSES renderer can be integrated into an Android app. It includes compiling RAMSES as native code for Android with gradle and cmake, a sample JNI interface that connects the native code to Java or Kotlin app code and a simple app.
The app contains a layout with a SurfaceView element. The Surface of the SurfaceView is passed via the JNI interface to the native code and is used to instantiate the RAMSES renderer. The renderer then uses the surface as window for rendering.

The RAMSES renderer app connects to pre-defined IPs set in ramses-renderer-android-app/app/src/main/java/com/bmwgroup/ramsesrenderer/MainActivity.java. The default IPs can be used to test the app with the Android emulator. For running the app on a hardware device the values need to be adapted accordingly. Furthermore the ABI of the device needs to be included in DemoRamsesAndroidModule/build.gradle in this case.

To show content inside the RAMSES renderer app with the Android emulator and a Linux host system follow these steps:

- Build the ramses-renderer-android-app project with Android Studio. This automatically also builds RAMSES as native code for Android
- Build RAMSES for the linux host system with the standard cmake build (use the same version of the RAMSES repository as used to build the app)
- Run the ramses-renderer-android-app on the Android emulator with Android Studio.
- Execute
```shell
sudo ifconfig lo:1 10.0.2.2 netmask 255.255.255.0 up
```
in a shell on the host system (This temporary assigns a second IP to the localhost interface used to communicate to the Android emulator. The step is only necessary with the emulator to simulate a normal network connection)
- Go to the bin directory of the RAMSES linux build and execute in two separate shells
```shell
./ramses-daemon --ip 10.0.2.2
```
and
```shell
./ramses-test-client --daemon-ip 10.0.2.2 --ip 10.0.2.2
```
to start a RAMSES daemon and a RAMSES client on the host system

The renderer will connect to the client on the host system and render the scene provided by the client. Instead of the ramses-test-client also one of the examples can be started with the same arguments.


# RAMSES client functionality

Using the client functionality (scene generation) of RAMSES on Android is supported as well and works in a similar fashion (see DemoJNIInterface/src/SceneViewerBundle.cpp for an example how to create a RAMSES client).
