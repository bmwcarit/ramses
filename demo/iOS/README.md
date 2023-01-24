# RAMSES Renderer demo app

This is an example how a RAMSES renderer can be integrated into an iOS app. It includes compiling RAMSES as native code for iOS with cmake, and invokes it using ObjectiveC++ from a Swift App.
The app contains an UIView with a CAMetalLayer which is used instantiate the RAMSES renderer. The renderer then uses the surface as window for rendering.

The RAMSES renderer app connects to pre-defined IPs set in iOS/src/ViewController.swift. The default IPs can be used to test the app with the iOS simulator. For running the app on a hardware device the values need to be adapted accordingly.

To show content inside the RAMSES renderer app with the iOS simulator and a macOS host system follow these steps:

- Generate an Xcode project for iOS using cmake with the ramses repository root folder as input. You can add the ID of your development team using `-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=...` at the end of this command, otherwise Xcode might ask you to set a development team ID manually.
```
cmake .. -DCMAKE_SYSTEM_NAME=iOS -GXcode -DCMAKE_OSX_SYSROOT=iphonesimulator
```
- Open the resulting Xcode project to build and run the ramses-renderer-ios-app on the iOS simulator.
- Build RAMSES for the macOS host system with the standard cmake build (use the same version of the RAMSES repository as used to build the app)
- Go to the bin directory of the RAMSES macOS build and execute in two separate shells
```shell
./ramses-daemon
```
and
```shell
./ramses-test-client
```
to start a RAMSES daemon and a RAMSES client on the host system.

The renderer will connect to the client on the host system and render the scene provided by the client. Instead of the ramses-test-client also one of the examples can be started with the same arguments.

# RAMSES client functionality

Using the client functionality (scene generation) of RAMSES on both iOS and Android is supported as well and works in a similar fashion (see DemoJNIInterface/src/SceneViewerBundle.cpp in the Android app for an example how to create a RAMSES client).
