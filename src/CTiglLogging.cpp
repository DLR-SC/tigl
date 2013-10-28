/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file 
* @brief  Implementation of a simple TIGL Logger.
*/

#include "CTiglLogging.h"
#include "ITiglLogger.h"
#include <ctime>

namespace tigl {


CTiglLogging::CTiglLogging(void)
{
    initLogger();
    _myLogger = NULL;
}

CTiglLogging::~CTiglLogging(void)
{
#ifdef GLOG_FOUND
    google::ShutdownGoogleLogging();
#endif
}

ITiglLogger* CTiglLogging::GetLogger() {
    return _myLogger;
}

void CTiglLogging::SetLogger(ITiglLogger * logger) {
    _myLogger = logger;
}

CTiglLogging& CTiglLogging::Instance(void)
{
    static CTiglLogging instance;
    return instance;
}

void CTiglLogging::initLogger(void)
{
#ifdef GLOG_FOUND
    // Initialize Google's logging library.
    google::InitGoogleLogging("TIGL-log");

    // this is a workaround described in https://code.google.com/p/google-glog/issues/detail?id=41
    // to avoid different log files for each severity. The info log file already contains
    // all log informations also from the higher severity levels
    for (int severity = google::WARNING; severity < google::NUM_SEVERITIES; severity++) {
        google::SetLogDestination(severity, "");
    }
    google::SetLogDestination(google::INFO, "TIGL-log-");
#endif
}


#ifndef GLOG_FOUND

DummyLogger_::DummyLogger_(){}
DummyLogger_::~DummyLogger_(){
    tigl::ITiglLogger* logger = CTiglLogging::Instance().GetLogger();
    if(logger) {
        logger->LogMessage(_lastLevel, stream.str().c_str());
    }
    else {
        printf("%s\n", stream.str().c_str());
    }
}

std::string getLogLevelString(TiglLogLevel level){
    return LogLevelStrings[level];
}

std::ostringstream& DummyLogger_::AppendToStream(TiglLogLevel level, const char* file, int line){
    _lastLevel = level;
    
    stream <<  getLogLevelString(level) << " ";

    // timestamp
    time_t rawtime;
    time (&rawtime);
    struct tm *timeinfo = localtime (&rawtime);
    char buffer [80];
    strftime (buffer,80,"%m/%d %H:%M:%S",timeinfo);
    stream << buffer << " ";

    stream << file << ":" << line  << "] ";
    stream << std::string(level > TILOG_DEBUG ? level - TILOG_DEBUG : 0, '\t');
    return stream;
}

#endif

} // end namespace tigl
