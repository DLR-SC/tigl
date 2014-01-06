LOCAL_PATH := $(call my-dir)

OCCLIBS := TKIGES TKSTL TKSTEP \
   TKSTEPAttr TKOffset TKMesh \
   TKSTEP209 TKFillet TKSTEPBase \
   TKBool TKXSBase TKShHealing \
   TKBO TKPrim TKTopAlgo \
   TKGeomAlgo TKBRep TKGeomBase \
   TKG3d TKG2d TKMath TKernel

#
# build TIGL_static library
#
include $(CLEAR_VARS)

LOCAL_MODULE    := TIGL_static
### Main Install dir
include $(LOCAL_PATH)/Directories.mk
	
TIGL_INCLUDES := $(LOCAL_PATH)/../../src
LOCAL_STATIC_LIBRARIES := $(OCCLIBS)


### Add all source file names to be included in lib separated by a whitespace 
LOCAL_C_INCLUDES := $(TIGL_INCLUDES)
LOCAL_EXPORT_C_INCLUDES := $(TIGL_INCLUDES)
FILE_LIST := $(wildcard $(LOCAL_PATH)/../../src/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../src/**/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_STATIC_LIBRARY)
# -------------------------------------------- #


#
# build native library for app communcation
#
include $(CLEAR_VARS)

LOCAL_MODULE    := tiglviewer-native

TIGL_LIBS := -L $(LOCAL_PATH)/../../misc/tigl-android/lib/static  \
	-lTIXI_static -lcurl -lxslt -lxml2

OSGLIBDIR 			:= $(OSG_ANDROID_DIR)/obj/local/armeabi
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	#LOCAL_ARM_NEON 	:= true
	OSGLIBDIR 			:= $(OSG_ANDROID_DIR)/obj/local/armeabi-v7a 
endif

OSG_LIBS := -L $(OSGLIBDIR) -losgdb_osg \
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
-lOpenThreads \
-losgdb_freetype \
-lft2


### Add all source file names to be included in lib separated by a whitespace 
LOCAL_C_INCLUDES:= $(OSG_ANDROID_DIR)/include
LOCAL_CFLAGS    := -Werror -fno-short-enums
LOCAL_CPPFLAGS  := -DOSG_LIBRARY_STATIC 

LOCAL_STATIC_LIBRARIES := TIGL_static $(OCCLIBS)

LOCAL_LDLIBS    := -llog -landroid -lGLESv1_CM -ldl -lz $(TIGL_LIBS) $(OSG_LIBS)
FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_SHARED_LIBRARY)

$(call import-module,oce)
