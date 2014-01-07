# Locate AndroidSDK
# This module defines
# ANDROID_SDK
# ANDROID_FOUND, if false, do not try to use AndroidSDK
#

FIND_PATH(ANDROID_SDK tools/android
    ${ANDROID_SDK}
    NO_DEFAULT_PATH
)

IF(NOT ANDROID_SDK)
    FIND_PATH(ANDROID_SDK tools/android
        $ENV{ANDROID_SDK}
        $ENV{ANDROID_HOME}
        NO_DEFAULT_PATH
    )
ENDIF()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (AndroidSDK  DEFAULT_MSG   ANDROID_SDK)

