/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2014-01-25 Martin Siggel <martin.siggel@dlr.de>
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

#include "TiglAndroidLogger.h"

#include <string>

#include <android/log.h>

void TiglAndroidLogger::SetTag(const char* theTag)
{
    tag = theTag;
}

void TiglAndroidLogger::LogMessage(TiglLogLevel level, const char * message)
{
    switch ( level ) {
    case TILOG_DEBUG1:
    case TILOG_DEBUG2:
    case TILOG_DEBUG3:
    case TILOG_DEBUG4:
        __android_log_write(ANDROID_LOG_VERBOSE, tag.c_str(),message);
        break;
    case TILOG_DEBUG:
        __android_log_write(ANDROID_LOG_DEBUG, tag.c_str(),message);
        break;
    case TILOG_INFO:
        __android_log_write(ANDROID_LOG_INFO, tag.c_str(),message);
        break;
    case TILOG_WARNING:
        __android_log_write(ANDROID_LOG_WARN, tag.c_str(),message);
        break;
    case TILOG_ERROR:
        __android_log_write(ANDROID_LOG_ERROR, tag.c_str(),message);
        break;
    default:
        __android_log_write(ANDROID_LOG_DEBUG, tag.c_str(),message);
        break;
    }
}

void TiglAndroidLogger::SetVerbosity(TiglLogLevel) {}
