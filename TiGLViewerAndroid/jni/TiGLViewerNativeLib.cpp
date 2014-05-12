/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <osg/Version>
#include <Standard_Version.hxx>

#include "tigl.h"

#include <iostream>

#include "OsgMainApp.hpp"

extern "C"
{
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_init(JNIEnv * env, jobject obj, jint width, jint height);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_createScene(JNIEnv * env, jobject obj);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_setAssetMgr(JNIEnv * env, jobject obj, jobject mgr);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_step(JNIEnv * env, jobject obj);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_openFile(JNIEnv * env, jobject obj, jstring filepath);
JNIEXPORT jboolean JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_isFiletypeSupported(JNIEnv * env, jobject obj, jstring filepath);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_removeObjects(JNIEnv * env, jobject obj);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_changeCamera(JNIEnv * env, jobject obj, jint view);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_fitScreen(JNIEnv * env, jobject obj);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseButtonPressEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button, jint view);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseButtonReleaseEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button, jint view);
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseMoveEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint view);
JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_tiglGetVersion(JNIEnv *, jobject);
JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_osgGetVersion(JNIEnv *, jobject);
JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_occtGetVersion(JNIEnv *, jobject);
}
;

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_setAssetMgr(JNIEnv * env, jobject obj, jobject mgr)
{
    AAssetManager* manager = AAssetManager_fromJava(env, mgr);
    OsgMainApp::Instance().setAssetManager(manager);
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_init(JNIEnv * env, jobject obj, jint width, jint height)
{
    OsgMainApp::Instance().initOsgWindow(0, 0, width, height);
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_step(JNIEnv * env, jobject obj)
{
    OsgMainApp::Instance().draw();
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_createScene(JNIEnv * env, jobject obj)
{
    OsgMainApp::Instance().createScene();
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_changeCamera(JNIEnv * env, jobject obj, jint view)
{
    OsgMainApp::Instance().changeCamera(view);
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_fitScreen(JNIEnv * env, jobject obj)
{
    OsgMainApp::Instance().fitScreen();
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_openFile(JNIEnv * env, jobject obj, jstring filepath)
{
    const char *nativeAddress = env->GetStringUTFChars(filepath, NULL);
    OsgMainApp::Instance().openFile(std::string(nativeAddress));
    env->ReleaseStringUTFChars(filepath, nativeAddress);
}

JNIEXPORT jboolean JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_isFiletypeSupported(JNIEnv * env, jobject obj, jstring filepath)
{
    const char *nativeAddress = env->GetStringUTFChars(filepath, NULL);
    jboolean result = OsgMainApp::Instance().isFileSupported(std::string(nativeAddress));
    env->ReleaseStringUTFChars(filepath, nativeAddress);
    return result;
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_removeObjects(JNIEnv * env, jobject obj)
{
    OsgMainApp::Instance().removeObjects();
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseButtonPressEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button, jint view)
{
    OsgMainApp::Instance().mouseButtonPressEvent(x, y, button, view);
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseButtonReleaseEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button, jint view)
{
    OsgMainApp::Instance().mouseButtonReleaseEvent(x, y, button, view);
}

JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseMoveEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint view)
{
    OsgMainApp::Instance().mouseMoveEvent(x, y, view);
}


JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_tiglGetVersion(JNIEnv * env, jobject)
{
    return env->NewStringUTF(tiglGetVersion());
}

JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_osgGetVersion(JNIEnv * env, jobject)
{
    return env->NewStringUTF(osgGetVersion());
}

JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_occtGetVersion(JNIEnv * env, jobject)
{
    return env->NewStringUTF(OCC_VERSION_COMPLETE);
}

