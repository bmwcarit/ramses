#!/bin/bash

set -o errexit

{
if [ ! -d ./MetalAngle.xcframework ]; 
then
    echo "MetalAngle.xcframework not found. Building from source... (this can over an hour)"
else
    echo "MetalAngle.xcframework found. Skipping Build!"
    exit 0
fi
}

rm -rf ./metalangle
rm -rf ./build
rm -rf ./include

git clone https://github.com/kakashidinho/metalangle.git
./metalangle/ios/xcode/fetchDependencies.sh

xcodebuild archive \
-project metalangle/iOS/xcode/OpenGLES.xcodeproj \
-scheme MetalANGLE \
-configuration Release \
-destination 'generic/platform=iOS Simulator' \
-archivePath './build/MetalANGLE.framework-iphonesimulator.xcarchive' \
SKIP_INSTALL=NO \
BUILD_LIBRARIES_FOR_DISTRIBUTION=YES

xcodebuild archive \
-project metalangle/iOS/xcode/OpenGLES.xcodeproj \
-scheme MetalANGLE \
-configuration Release \
-destination 'generic/platform=iOS' \
-archivePath './build/MetalANGLE.framework-iphoneos.xcarchive' \
SKIP_INSTALL=NO \
BUILD_LIBRARIES_FOR_DISTRIBUTION=YES

xcodebuild archive \
-project metalangle/iOS/xcode/OpenGLES.xcodeproj \
-scheme MetalANGLE_mac \
-configuration Release \
-destination 'generic/platform=OS X' \
-archivePath './build/MetalANGLE.framework-macos.xcarchive' \
SKIP_INSTALL=NO \
BUILD_LIBRARIES_FOR_DISTRIBUTION=YES

xcodebuild archive \
-project metalangle/iOS/xcode/OpenGLES.xcodeproj \
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
-output './MetalAngle.xcframework'

ditto ./MetalAngle.xcframework/ios-arm64/MetalANGLE.framework/Headers ./include