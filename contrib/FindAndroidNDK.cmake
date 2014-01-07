# Locate AndroidNDK
# This module defines
# ANDROID_NDK
# ANDROID_FOUND, if false, do not try to use AndroidNDK
#

FIND_PATH(ANDROID_NDK ndk-build
    ${ANDROID_NDK}
    NO_DEFAULT_PATH
)

IF(NOT ANDROID_NDK)
    FIND_PATH(ANDROID_NDK ndk-build
        $ENV{ANDROID_NDK}
        $ENV{ANDROID_ROOT}
        NO_DEFAULT_PATH
    )
ENDIF()

IF(NOT ANDROID_NDK)
    FIND_PATH(ANDROID_NDK ndk-build
    # search for r5c
        ~/android-ndk-r5c
        ~/android_develop/android-ndk-r5c
        ~/ndk-r5c
        ~/android_develop/ndk-r5c
        # search for r5b
        ~/android-ndk-r5b
        ~/android_develop/android-ndk-r5b
        ~/ndk-r5b
        ~/android_develop/ndk-r5b
        # search for r5
        ~/android-ndk-r5
        ~/android_develop/android-ndk-r5
        ~/ndk-r5
        ~/android_develop/ndk-r5
        # search for r4-crystax
        ~/android-ndk-r4-crystax
        ~/android_develop/android-ndk-r4-crystax
        ~/ndk-r4
        ~/android_develop/ndk-r4
    )
ENDIF()
 
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (AndroidNDK  DEFAULT_MSG   ANDROID_NDK)
