# How to build MetalANGLE.xcframework

Checkout MetalANGLE from https://github.com/kakashidinho/metalangle.

Open a terminal and navigate to `MetalANGLE/iOS/xcode`. Then execute the following:
```
xcodebuild archive \
-project OpenGLES.xcodeproj \
-scheme MetalANGLE \
-configuration Release \
-destination 'generic/platform=iOS Simulator' \
-archivePath './build/MetalANGLE.framework-iphonesimulator.xcarchive' \
SKIP_INSTALL=NO \
BUILD_LIBRARIES_FOR_DISTRIBUTION=YES

xcodebuild archive \
-project OpenGLES.xcodeproj \
-scheme MetalANGLE \
-configuration Release \
-destination 'generic/platform=iOS' \
-archivePath './build/MetalANGLE.framework-iphoneos.xcarchive' \
SKIP_INSTALL=NO \
BUILD_LIBRARIES_FOR_DISTRIBUTION=YES

xcodebuild archive \
-project OpenGLES.xcodeproj \
-scheme MetalANGLE_mac \
-configuration Release \
-destination 'generic/platform=OS X' \
-archivePath './build/MetalANGLE.framework-macos.xcarchive' \
SKIP_INSTALL=NO \
BUILD_LIBRARIES_FOR_DISTRIBUTION=YES

xcodebuild archive \
-project OpenGLES.xcodeproj \
-scheme MetalANGLE \
-configuration Release \
-destination 'generic/platform=OS X' \
-archivePath './build/MetalANGLE.framework-maccatalyst.xcarchive' \
SKIP_INSTALL=NO \
BUILD_LIBRARIES_FOR_DISTRIBUTION=YES

xcodebuild -create-xcframework \
-framework './build/MetalANGLE.framework-iphonesimulator.xcarchive/Products/Library/Frameworks/MetalANGLE.framework' \
-framework './build/MetalANGLE.framework-iphoneos.xcarchive/Products/Library/Frameworks/MetalANGLE.framework' \
-framework './build/MetalANGLE.framework-maccatalyst.xcarchive/Products/Library/Frameworks/MetalANGLE.framework' \
-framework './build/MetalANGLE.framework-macos.xcarchive/Products/Library/Frameworks/MetalANGLE.framework' \
-output './build/MetalAngle.xcframework'
```

Finally, copy the generated file `MetalANGLE/ios/xcode/build/build/MetalAngle.xcframework` to the ramses project.
