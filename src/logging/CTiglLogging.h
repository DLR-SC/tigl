/* 
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2013-02-13 Markus Litz <Markus.Litz@dlr.de>

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
 * @brief  Implementation of the TIGL Logger.
 */

#ifndef CTIGLLOGGING_H
#define CTIGLLOGGING_H

#include "tigl_config.h"
#include "tigl.h"
#include "tigl_internal.h"
#include "CSharedPtr.h"

#include <string>
#include <stdio.h>
#ifdef GLOG_FOUND
#pragma warning( disable : 4251 4355 )
#include <glog/logging.h>
#pragma warning(disable: 4275)
#else
// dummy logger implementation
#include <iostream>
#include <sstream>
#include <cstring>
#endif

namespace tigl 
{

#ifndef GLOG_FOUND
    //dummy implementation if glog is not available
    #ifndef LOG_MAX_LEVEL
    #define LOG_MAX_LEVEL TILOG_DEBUG4
    #endif
    #ifndef LOG_MIN_LEVEL
    #define LOG_MIN_LEVEL TILOG_SILENT
    #endif

    /**
     * This macros can be used like streams e.g.: LOG(ERROR) << "that is an error";
     * Following log levels are supported: ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3, DEBUG4
     */
    #define LOG(level) \
        if (TILOG_ ## level > LOG_MAX_LEVEL || TILOG_ ## level <= LOG_MIN_LEVEL) ;\
    else tigl::DummyLogger_().AppendToStream(TILOG_ ## level, __FILE__, __LINE__)

    #define DLOG(level) \
        if (TILOG_ ## level > LOG_MAX_LEVEL) ;\
    else tigl::DebugStream_().AppendToStream(TILOG_ ## level, __FILE__, __LINE__)

    class DummyLogger_
    {
    public:
        TIGL_EXPORT DummyLogger_();
        TIGL_EXPORT virtual ~DummyLogger_();
        TIGL_EXPORT std::ostringstream& AppendToStream(TiglLogLevel level, const char *file, int line);
    protected:
        std::ostringstream stream;
    private:
        DummyLogger_(const DummyLogger_&);
        DummyLogger_& operator =(const DummyLogger_&);
        
        TiglLogLevel _lastLevel;
    };

    class DebugStream_
    {
    public:
        TIGL_EXPORT DebugStream_();
        TIGL_EXPORT virtual ~DebugStream_();
        TIGL_EXPORT std::ostringstream& AppendToStream(TiglLogLevel level, const char *file, int line);
    protected:
        std::ostringstream stream;
    private:
        DebugStream_(const DebugStream_&);
        DebugStream_& operator =(const DebugStream_&);

        TiglLogLevel _lastLevel;
    };
#endif // not GLOG_FOUND

// define smart pointer to logger
class ITiglLogger;
typedef class CSharedPtr<ITiglLogger> PTiglLogger;

class CTiglLogging
{
public:
    // Returns a reference to the only instance of this class
    TIGL_EXPORT static CTiglLogging& Instance();
    
    // convenience functions that insert the appropriate loggers
    TIGL_EXPORT void LogToFile(const char* filePrefix);
    TIGL_EXPORT void LogToStream(FILE * fp);
    TIGL_EXPORT void SetLogFileEnding(const char* ending);
    TIGL_EXPORT void SetTimeIdInFilenameEnabled(bool enabled);
    TIGL_EXPORT void LogToConsole();
    TIGL_EXPORT void SetConsoleVerbosity(TiglLogLevel vlevel);
    
    // allows installing a custom log sink/receiver
    // The logger becomes property of this class
    // Therefore the logger should not be deleted
    // manually.
    TIGL_EXPORT void SetLogger(PTiglLogger);
    TIGL_EXPORT PTiglLogger GetLogger();

    // Destructor
    TIGL_EXPORT ~CTiglLogging();

private:
    // Constructor
    CTiglLogging();

    // Logger Initialize with defaults
    void initLogger();

    // Copy constructor
    CTiglLogging(const CTiglLogging& )                { /* Do nothing */ }

    // Assignment operator
    void operator=(const CTiglLogging& )             { /* Do nothing */ }
    
    PTiglLogger      _myLogger;
    std::string        _fileEnding;
    bool               _timeIdInFilename;
    TiglLogLevel       _consoleVerbosity;
    PTiglLogger      _consoleLogger;

};

// get log level string (for testing purposes)
TIGL_EXPORT std::string getLogLevelString(TiglLogLevel level);

} // end namespace tigl

#endif // CTIGLLOGGING_H
