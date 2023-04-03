//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "RendererBundle.h"
#include "SceneViewerBundle.h"
#include "UniformInputWrapper.h"
#include "ramses-client.h"


extern "C"
JNIEXPORT jlong JNICALL
Java_com_bmwgroup_ramses_RamsesRenderer_createRendererNative(JNIEnv *env, jobject /*instance*/,
                                                       jobject javaSurface, jint width, jint height,
                                                       jstring interfaceSelectionIP_, jstring daemonIP_) {
    const char *interfaceSelectionIP = env->GetStringUTFChars(interfaceSelectionIP_, 0);
    const char *daemonIP = env->GetStringUTFChars(daemonIP_, 0);
    ANativeWindow* nativeWindowHandle = ANativeWindow_fromSurface(env, javaSurface);

    RendererBundle* bundle = new RendererBundle(nativeWindowHandle, width, height,
                                                interfaceSelectionIP, daemonIP);
    bundle->connect();
    bundle->run();

    env->ReleaseStringUTFChars(interfaceSelectionIP_, interfaceSelectionIP);
    env->ReleaseStringUTFChars(daemonIP_, daemonIP);

    return reinterpret_cast<jlong>(bundle);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_RamsesRenderer_disposeRamsesRendererNative(JNIEnv* /*env*/, jobject /*instance*/,
                                                              jlong handle) {
    RendererBundle* bundle = reinterpret_cast<RendererBundle*>(handle);
    ANativeWindow* nativeWindowHandle = bundle->getNativeWindow();
    delete bundle;
    ANativeWindow_release(nativeWindowHandle);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_bmwgroup_ramses_RamsesSceneViewer_createSceneViewerNative(JNIEnv *env, jobject /*instance*/,
                                                             jobject surface, jint width,
                                                             jint height, jstring sceneFile_) {
    const char *sceneFile = env->GetStringUTFChars(sceneFile_, 0);
    ANativeWindow* nativeWindowHandle = ANativeWindow_fromSurface(env, surface);

    SceneViewerBundle* bundle = new SceneViewerBundle(nativeWindowHandle, width, height, sceneFile);

    env->ReleaseStringUTFChars(sceneFile_, sceneFile);

    bundle->connect();
    bundle->run();
    return reinterpret_cast<jlong>(bundle);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_RamsesSceneViewer_disposeSceneViewerNative(JNIEnv* /*env*/, jobject /*instance*/,
                                                              jlong handle) {
    SceneViewerBundle* bundle = reinterpret_cast<SceneViewerBundle*>(handle);
    ANativeWindow* nativeWindowHandle = bundle->getNativeWindow();
    delete bundle;
    ANativeWindow_release(nativeWindowHandle);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_bmwgroup_ramses_RamsesSceneViewer_findNodeByNameNative(JNIEnv* env, jobject /*instance*/,
                                                               jlong nativeHandle, jstring name_) {
    const char *name = env->GetStringUTFChars(name_, 0);
    ramses::Node* node = reinterpret_cast<SceneViewerBundle*>(nativeHandle)->findNodeByName(name);
    env->ReleaseStringUTFChars(name_, name);

    return reinterpret_cast<jlong>(node);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_RamsesSceneViewer_flushSceneNative(JNIEnv* /*env*/, jobject /*instance*/, jlong handle) {
    reinterpret_cast<SceneViewerBundle*>(handle)->flushScene();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_bmwgroup_ramses_RamsesSceneViewer_findUniformInputNative(JNIEnv* env, jobject /*instance*/,
                                                            jlong handle, jstring appearanceName_,
                                                            jstring inputName_) {
    const char *appearanceName = env->GetStringUTFChars(appearanceName_, 0);
    const char *inputName = env->GetStringUTFChars(inputName_, 0);

    UniformInputWrapper* inputWrapper = reinterpret_cast<SceneViewerBundle*>(handle)->findUniformInput
            (appearanceName, inputName);

    env->ReleaseStringUTFChars(appearanceName_, appearanceName);
    env->ReleaseStringUTFChars(inputName_, inputName);

    return reinterpret_cast<jlong>(inputWrapper);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_RamsesNode_setTranslationNative(JNIEnv* /*env*/, jobject /*instance*/, jlong handle,
                                                        jfloat x, jfloat y, jfloat z) {
    reinterpret_cast<ramses::Node*>(handle)->setTranslation(x, y, z);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_RamsesNode_translateNative(JNIEnv* /*env*/, jobject /*instance*/, jlong handle,
                                                   jfloat x, jfloat y, jfloat z) {
    reinterpret_cast<ramses::Node*>(handle)->translate(x, y, z);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_RamsesNode_setRotationNative(JNIEnv* /*env*/, jobject /*instance*/, jlong handle,
                                                     jfloat x, jfloat y, jfloat z) {
    reinterpret_cast<ramses::Node*>(handle)->setRotation(x, y, z, ramses::ERotationConvention::Euler_XYZ);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_UniformInput_setValueFloatNative(JNIEnv* /*env*/, jobject /*instance*/, jlong handle,
                                                    jfloat x) {
    reinterpret_cast<UniformInputWrapper*>(handle)->setInputValueFloat(x);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bmwgroup_ramses_UniformInput_disposeNative(JNIEnv* /*env*/, jobject /*instance*/, jlong handle) {
    delete reinterpret_cast<UniformInputWrapper*>(handle);
}
