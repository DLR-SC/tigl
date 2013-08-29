#include <string.h>
#include <jni.h>
#include <android/log.h>

#include <iostream>

#include "OsgMainApp.hpp"

OsgMainApp mainApp;

extern "C" {
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_init(JNIEnv * env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_addObjectFromVTK(JNIEnv * env, jobject obj, jstring filepath);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_addObjectFromCPACS(JNIEnv * env, jobject obj, jstring filepath);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_removeObjects(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_changeCamera(JNIEnv * env, jobject obj, jint view);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_mouseButtonPressEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button , jint view);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_mouseButtonReleaseEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button , jint view);
    JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_mouseMoveEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint view);
};


JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_init(JNIEnv * env, jobject obj, jint width, jint height){
    mainApp.initOsgWindow(0,0,width,height);
}
JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_step(JNIEnv * env, jobject obj){
	mainApp.draw();
}
JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_changeCamera(JNIEnv * env, jobject obj, jint view)
{
	mainApp.changeCamera(view);
}
JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_addObjectFromVTK(JNIEnv * env, jobject obj, jstring filepath)
{
	 const char *nativeAddress = env->GetStringUTFChars(filepath, NULL);
	 mainApp.addObjectFromVTK(std::string(nativeAddress));
	 env->ReleaseStringUTFChars(filepath, nativeAddress);
}
JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_addObjectFromCPACS(JNIEnv * env, jobject obj, jstring filepath)
{
	const char *nativeAddress = env->GetStringUTFChars(filepath, NULL);
	mainApp.addObjectFromCPACS(std::string(nativeAddress));
	env->ReleaseStringUTFChars(filepath, nativeAddress);
	}
JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_removeObjects(JNIEnv * env, jobject obj)
{
	mainApp.removeObjects();
	}

JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_mouseButtonPressEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button, jint view){
    mainApp.mouseButtonPressEvent(x,y,button, view);
}
JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_mouseButtonReleaseEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button , jint view){
    mainApp.mouseButtonReleaseEvent(x,y,button,view);
}
JNIEXPORT void JNICALL Java_dlr_tiglviewer_osgNativeLib_mouseMoveEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y , jint view){
    mainApp.mouseMoveEvent(x,y,view);
}

