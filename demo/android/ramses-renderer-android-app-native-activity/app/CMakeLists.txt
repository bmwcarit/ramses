#  -------------------------------------------------------------------------
#  Copyright (C) 2019 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if((TARGET DemoRamsesJNIInterface) AND ANDROID_NDK)
    add_library(exampleNativeLib SHARED src/main/cpp/exampleNative.cpp)

    include(../../../../cmake/ramses/adaptCompilerFlagsForExternals.cmake)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
    add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
    target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue)

    target_link_libraries(exampleNativeLib
        android
        native_app_glue
        log
        ramses-shared-lib-android-egl-es-3-0
        DemoRamsesJNIInterface
    )
endif()
