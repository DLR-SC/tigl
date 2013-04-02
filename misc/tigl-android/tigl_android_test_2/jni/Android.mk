#
# ANDROID Makefile for TIGL
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKPrim
LOCAL_SRC_FILES := libTKPrim.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKMath
LOCAL_SRC_FILES := libTKMath.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKernel 
LOCAL_SRC_FILES := libTKernel.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKG2d
LOCAL_SRC_FILES := libTKG2d.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKBRep
LOCAL_SRC_FILES := libTKBRep.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKGeomBase
LOCAL_SRC_FILES := libTKGeomBase.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKG3d
LOCAL_SRC_FILES := libTKG3d.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKTopAlgo
LOCAL_SRC_FILES := libTKTopAlgo.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKGeomAlgo
LOCAL_SRC_FILES := libTKGeomAlgo.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libTKBool
LOCAL_SRC_FILES := libTKBool.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS) 
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := tigl_bonary_wrapper
LOCAL_CPPFLAGS += -DHAVE_IOSTREAM -DHAVE_FSTREAM -DHAVE_LIMITS_H -DHAVE_IOMANIP
LOCAL_SRC_FILES := tigl_binary_wrapper.cpp
LOCAL_CFLAGS := -I${HOME}/tigl_android_test_2/jni -I${HOME}/oce_android_binaries/ros/inc -I${ANDROID_ARM_4}/include -I${ANDROID_NDK}/sources/cxx-stl/stlport/stlport
LOCAL_SHARED_LIBRARIES += libTKernel libTKPrim libTKG2d libTKMath libTKBRep libTKGeomBase libTKG3d libTKTopAlgo libTKGeomAlgo libTKBool
include $(BUILD_SHARED_LIBRARY)
