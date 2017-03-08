/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-29 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglFileLogger.h"
#include "CTiglError.h"
#include "CMutex.h"
#include "CScopedLock.h"

#include <cstdio>
#include <ctime>
#include <iostream>

namespace tigl 
{

CTiglFileLogger::CTiglFileLogger(FILE * file) : logFileStream(file), mutex(new CMutex), verbosity(TILOG_DEBUG4)
{
    if (!logFileStream) {
        throw CTiglError("Null pointer for argument file in CTiglLogFile", TIGL_NULL_POINTER);
    }
    
    fileOpened = false;
}

CTiglFileLogger::CTiglFileLogger(const char* filename) :  mutex(new CMutex) , verbosity(TILOG_DEBUG4)
{
    logFileStream = fopen(filename,"w");
    if (!logFileStream) {
        throw CTiglError("Log file can not be created CTiglLogFile", TIGL_OPEN_FAILED);
    }
    fileOpened = true;

    // timestamp
    time_t rawtime;
    time (&rawtime);
    tm *timeinfo = localtime (&rawtime);
    char buffer [160];
    strftime (buffer,160,"TiGL log file created at: %y/%m/%d %H:%M:%S",timeinfo);
    
    fprintf(logFileStream, "%s\n", buffer);
}

CTiglFileLogger::~CTiglFileLogger()
{
    if (fileOpened) {
        fclose(logFileStream);
    }
}

void CTiglFileLogger::LogMessage(TiglLogLevel level, const char *message)
{
    if (level<=verbosity) {
        if (logFileStream) {
            CScopedLock lock(*mutex);
            fprintf(logFileStream, "%s\n", message);
        }
    }
}
void CTiglFileLogger::SetVerbosity(TiglLogLevel vlevel)
{
    verbosity=vlevel;
}

} // namespace tigl

