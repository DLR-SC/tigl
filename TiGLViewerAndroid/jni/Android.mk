LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := osgNativeLib
### Main Install dir
OSG_ANDROID_DIR	:= /Users/martin/Documents/Code/OpenSceneGraph-3.2.0/build-android/install
LIBDIR 			:= -L $(OSG_ANDROID_DIR)/obj/local/armeabi

TIGL_LIBS := -L /Users/martin/Documents/Code/tigl/misc/tigl-android/lib -lTIGL_static \
	-lTKIGES -lTKSTL -lTKSTEP \
	-lTKSTEPAttr -lTKOffset -lTKMesh \
	-lTKSTEP209 -lTKFillet -lTKSTEPBase \
	-lTKBool -lTKXSBase -lTKShHealing \
	-lTKBO -lTKPrim -lTKTopAlgo \
	-lTKGeomAlgo -lTKBRep -lTKGeomBase \
	-lTKG3d -lTKG2d -lTKMath -lTKernel \
	-lTIXI_static -lcurl -lxslt -lxml2
	
TIGL_INCLUDES := ../../src /Users/martin/Documents/Code/oce-install/include/oce/


ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	#LOCAL_ARM_NEON 	:= true
	LIBDIR 			:= -L $(OSG_ANDROID_DIR)/obj/local/armeabi-v7a 
endif

### Add all source file names to be included in lib separated by a whitespace 
LOCAL_C_INCLUDES:= $(OSG_ANDROID_DIR)/include $(TIGL_INCLUDES)
LOCAL_CFLAGS    := -Werror -fno-short-enums
LOCAL_CPPFLAGS  := -DOSG_LIBRARY_STATIC 

LOCAL_LDLIBS    := -llog -lGLESv1_CM -ldl -lz $(TIGL_LIBS)
LOCAL_SRC_FILES := osgNativeLib.cpp OsgMainApp.cpp OsgAndroidNotifyHandler.cpp AuxiliaryViewUpdater.cpp CrossNode.cpp crossnodegeode.cpp  VisObject.cpp  MaterialTemplate.cpp VirtualVisObject.cpp GeometricVisObject.cpp 
LOCAL_LDFLAGS   :=   $(LIBDIR) \
-losgdb_osg \
-losgdb_deprecated_osgviewer \
-losgdb_deprecated_osg \
-losgdb_serializers_osg \
-losgdb_deprecated_osgtext \
-losgViewer \
-losgGA \
-losgDB \
-losgText \
-losgUtil \
-losg \
-lOpenThreads

include $(BUILD_SHARED_LIBRARY)
