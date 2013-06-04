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

#include "CTiglLogger.h"

namespace tigl {


CTiglLogger::CTiglLogger(void)
{
    initLogger();
}

CTiglLogger::~CTiglLogger(void)
{
#ifdef GLOG_FOUND
    google::ShutdownGoogleLogging();
#endif
}

CTiglLogger& CTiglLogger::GetLogger(void)
{
    static CTiglLogger instance;
    return instance;
}

void CTiglLogger::initLogger(void)
{
#ifdef GLOG_FOUND
    // Initialize Google's logging library.
    google::InitGoogleLogging("TIGL-log");

    for (int severity = google::INFO; severity < google::NUM_SEVERITIES; severity++) {
      google::SetLogDestination(severity, "TIGL-log-");
    }
#endif
}


#ifndef GLOG_FOUND

DummyLogger_::DummyLogger_(){}
DummyLogger_::~DummyLogger_(){
    stream << std::endl;
    std::cout << stream.str();
}

std::string getLogLevelString(LogLevelDummy_ level){
    static const char* const buffer[] = {"ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4"};
    return buffer[level];
}

std::ostringstream& DummyLogger_::AppendToStream(LogLevelDummy_ level){
    stream << " " <<  getLogLevelString(level) << ": ";
    stream << std::string(level > DEBUG ? level - DEBUG : 0, '\t');
    return stream;
}

#endif

} // end namespace tigl
