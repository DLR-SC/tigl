/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-06-18 Mahmoud Aly <Mahmoud.Aly@dlr.de>
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

#include "JavaCallbacks.h"
#include <CTiglLogging.h>

JavaCallbacks::JavaCallbacks(JNIEnv* mEnv, jobject hf)
{
    mJavaVM = NULL;

    if (mEnv->GetJavaVM(&mJavaVM) != JNI_OK) {
        LOG(ERROR) << "Cannot get JavaVM";
        return;
    }

    jclass ClassHaptic = mEnv->GetObjectClass(hf);
    if (!ClassHaptic) {
        LOG(ERROR) << "Cannot find object class";
        return;
    }

    methodHapticFeedback = mEnv->GetMethodID(ClassHaptic, "hapticFeedback", "()V");
    if (methodHapticFeedback == 0) {
        LOG(ERROR) << "Cannot find method id";
        return;
    }
    javaCallbackObj   = mEnv->NewGlobalRef(hf);
}
void JavaCallbacks::hapticFeedback()
{
    if (!mJavaVM) {
        return;
    }

    // attach
    JavaVMAttachArgs lJavaVMAttachArgs;
    lJavaVMAttachArgs.version = JNI_VERSION_1_6;
    lJavaVMAttachArgs.name = "NativeThread";
    lJavaVMAttachArgs.group = NULL;
    JNIEnv* lEnv;
    if (mJavaVM->AttachCurrentThread(&lEnv, &lJavaVMAttachArgs) != JNI_OK) {
        LOG(ERROR) << "Cannot attach thread to javavm. Haptic feedback disabled.";
        return;
    }

    if ( methodHapticFeedback == 0 || !javaCallbackObj || !lEnv) {
        LOG(ERROR) << "Cannot call ";
        return;
    }

    // call method
    lEnv->CallVoidMethod(javaCallbackObj, methodHapticFeedback);

}
