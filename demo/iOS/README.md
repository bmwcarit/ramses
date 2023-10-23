# RAMSES Renderer demo app

This is an example how a RAMSES renderer can be integrated into an iOS app. It includes compiling RAMSES as native code for iOS with cmake, and invokes it using ObjectiveC++ from a Swift App.
The app contains an UIView with a CAMetalLayer which is used to instantiate the RAMSES renderer. The renderer then uses the surface as window for rendering.

The RAMSES renderer app connects to pre-defined IPs set in iOS/src/ViewController.swift. The default IPs can be used to test the app with the iOS simulator. For running the app on a hardware device the values need to be adapted accordingly.

To show content inside the RAMSES renderer app with the iOS simulator and a macOS host system follow these steps:

- Generate an Xcode project for iOS using cmake with the ramses repository root folder as input. You can add the ID of your development team using `-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=...` at the end of this command, otherwise Xcode might ask you to set a development team ID manually.
```
cmake .. -DCMAKE_SYSTEM_NAME=iOS -GXcode -DCMAKE_OSX_SYSROOT=iphonesimulator
```
- Open the resulting Xcode project to build and run the ramses-renderer-ios-app on the iOS simulator.
- Build RAMSES for the macOS host system with a C++ cmake build (not an iOS build). Use the same version of RAMSES repository used for building the iOS app.
- Go to the bin directory of the RAMSES macOS build and execute in two separate shells:
```shell
./ramses-daemon
```
and
```shell
./ramses-example-basic-geometry
```
to start a RAMSES daemon and a RAMSES client example on the host system.

The renderer will connect to the client on the host system and render the scene provided by the client. Any of the non-local ramses examples can be started in the same manner.

# RAMSES Client creation

Ramse client can be created, in order to create scene and render them on a Ramses renderer in a similar fashion to any of the other supported platforms.
