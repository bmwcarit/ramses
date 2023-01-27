#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(${CMAKE_SYSTEM_NAME} MATCHES "iOS") 
    enable_language(OBJC OBJCXX Swift)

    set(MACOSX_BUNDLE_EXECUTABLE_NAME "ramses-renderer-ios-app")
    set(MACOSX_BUNDLE_INFO_STRING "com.bmw.ramses-renderer-ios-app")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.bmw.ramses-renderer-ios-app")
    set(MACOSX_BUNDLE_BUNDLE_NAME "com.bmw.ramses-renderer-ios-app")
    set(MACOSX_BUNDLE_ICON_FILE "")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2022 BMW AG")
    set(MACOSX_DEPLOYMENT_TARGET 14.0)

    SET(CMAKE_XCODE_GENERATE_SCHEME ON)
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY, "./out")
    
    set(APP_HEADER_FILES
        ./include/RendererBundle.h
        ./include/ramses-renderer-ios-app-Bridging-Header.h
    )

    set(APP_SOURCE_FILES
        ./src/AppDelegate.swift
        ./src/RendererBundle.mm
        ./src/SceneDelegate.swift
        ./src/ViewController.swift
    )

    set(RESOURCES
        ./res/Main.storyboard
        ./res/LaunchScreen.storyboard
    )

    include_directories(PUBLIC ./include)
    
    add_executable(
        ramses-renderer-ios-app
        MACOSX_BUNDLE
        ${APP_HEADER_FILES}
        ${APP_SOURCE_FILES}
        ${RESOURCES}
    )

    SET(EMBED_FRAMEWORKS ramses-shared-lib-ios-egl-es-3-0 "${CMAKE_BINARY_DIR}/MetalANGLE.framework")
    set_target_properties(ramses-renderer-ios-app PROPERTIES
        MACOSX_FRAMEWORK_IDENTIFIER com.bmw.ramses-renderer-ios-app
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Frameworks"
        XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION
        XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/ramses-renderer-ios-app-Bridging-Header.h"
        XCODE_EMBED_FRAMEWORKS "${EMBED_FRAMEWORKS}"
        XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        RESOURCE "${RESOURCES}"
    )

    target_link_libraries(ramses-renderer-ios-app PUBLIC ramses-shared-lib-ios-egl-es-3-0)
endif()