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
	
TIGL_INCLUDES := \
  $(LOCAL_PATH)/../../src \
  $(LOCAL_PATH)/../../src/api \
  $(LOCAL_PATH)/../../src/boolean_operations \
  $(LOCAL_PATH)/../../src/common \
  $(LOCAL_PATH)/../../src/configuration \
  $(LOCAL_PATH)/../../src/cpacs_other \
  $(LOCAL_PATH)/../../src/exports \
  $(LOCAL_PATH)/../../src/fuselage \
  $(LOCAL_PATH)/../../src/geometry \
  $(LOCAL_PATH)/../../src/guide_curves \
  $(LOCAL_PATH)/../../src/logging \
  $(LOCAL_PATH)/../../src/math \
  $(LOCAL_PATH)/../../src/system \
  $(LOCAL_PATH)/../../src/wing \
  $(LOCAL_PATH)/../../thirdparty/boost_1_55_0

LOCAL_STATIC_LIBRARIES := $(OCCLIBS) TIXI_static


### Add all source file names to be included in lib separated by a whitespace 
LOCAL_C_INCLUDES := $(TIGL_INCLUDES)
LOCAL_EXPORT_C_INCLUDES := $(TIGL_INCLUDES)
FILE_LIST := $(wildcard $(LOCAL_PATH)/../../src/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../src/**/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../src/*.cxx)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../src/**/*.cxx)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_STATIC_LIBRARY)
# -------------------------------------------- #


#
# build native library for app communcation
#
include $(CLEAR_VARS)

LOCAL_MODULE    := tiglviewer-native

OSG_LIBS := osgdb_osg \
    osgdb_deprecated_osgviewer \
    osgdb_deprecated_osg \
    osgdb_serializers_osg \
    osgdb_deprecated_osgtext \
    osgViewer \
    osgGA \
    osgDB \
    osgText \
    osgUtil \
    osg \
    OpenThreads \
    osgdb_freetype \
    ft2


### Add all source file names to be included in lib separated by a whitespace 
LOCAL_C_INCLUDES:= $(OSG_ANDROID_DIR)/include
LOCAL_CFLAGS    := -Werror -fno-short-enums
LOCAL_CPPFLAGS  := -DOSG_LIBRARY_STATIC 

LOCAL_STATIC_LIBRARIES := TIGL_static TIXI_static $(OCCLIBS) $(OSG_LIBS)

LOCAL_LDLIBS    := -llog -landroid -lGLESv1_CM -ldl -lz 
FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_SHARED_LIBRARY)

$(call import-module,oce)
$(call import-module,tixi)
$(call import-module,osg)
