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


#ifndef _HAPTICLISTENER_H_
#define _HAPTICLISTENER_H_

#include <jni.h>

class NativeCodeCallbacks
{
public:
    NativeCodeCallbacks(JNIEnv* mEnv, jobject hf);
    void startListening();
    // Cached JNI references.
    JavaVM* mJavaVM;
    jobject hapticFront;
    // Classes.
    jclass ClassHaptic;
    // Methods.
    jmethodID MethodOnAlert;
};

#endif
