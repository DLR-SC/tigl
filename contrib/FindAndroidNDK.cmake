# Locate AndroidNDK
# This module defines
# ANDROID_NDK
# ANDROID_FOUND, if false, do not try to use AndroidNDK
#

FIND_PATH(ANDROID_NDK ndk-build
    ${ANDROID_NDK}
    $ENV{ANDROID_NDK_HOME}
    NO_DEFAULT_PATH
)
 
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (AndroidNDK  "Cannot find Android NDK, please set the appropriate ANDROID_NDK_HOME environment variable"  ANDROID_NDK)
