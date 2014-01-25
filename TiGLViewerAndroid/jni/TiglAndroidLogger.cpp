/*
 * TiglAndroidLogger.cpp
 *
 *  Created on: 25.01.2014
 *      Author: Martin Siggel <Martin.Siggel@dlr.de>
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
