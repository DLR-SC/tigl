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
* @brief  Implementation of a simple TIGL Logger.
*/

#include "CTiglLogging.h"
#include "ITiglLogger.h"
#include "CGlogLoggerAdaptor.h"
#include "CTiglFileLogger.h"
#include "CTiglLogSplitter.h"
#include "CTiglConsoleLogger.h"
#include <ctime>
#include <cstring>
#include <string>

//macro that extracts the filename of the current file
#if defined _WIN32 || defined __WIN32__
#define BASENAME(MYFILE) (strrchr((MYFILE), '\\') ? strrchr((MYFILE), '\\') + 1 : (MYFILE))
#else
#define BASENAME(MYFILE) (strrchr((MYFILE), '/') ? strrchr((MYFILE), '/') + 1 : (MYFILE))
#endif

static const char* const LogLevelStrings[] = {"SLT", "ERR", "WRN", "INF", "DBG", "DBG1", "DBG2", "DBG3", "DBG4"};

namespace tigl 
{


CTiglLogging::CTiglLogging()
{
    initLogger();
    _fileEnding = "log";
    _timeIdInFilename = true;
    _consoleVerbosity = TILOG_WARNING;
    // set logger to console logger
    PTiglLogger consoleLogger(new CTiglConsoleLogger);
    consoleLogger->SetVerbosity(_consoleVerbosity);
    SetLogger(consoleLogger);
    // set _consoleLogger variable
    _consoleLogger=_myLogger;
}

CTiglLogging::~CTiglLogging()
{
#ifdef GLOG_FOUND
    // TODO (bgruber): this is problematic, as the application linking to TIGL may also use glog after TIGL has been shutdown
    // TODO (bgruber): enabling this also causes an abort() at the end of the application if glog was linked statically by TIGL and the application
    google::ShutdownGoogleLogging();
#endif
}

PTiglLogger CTiglLogging::GetLogger() 
{
    return _myLogger;
}

void CTiglLogging::SetLogger(PTiglLogger logger) 
{

    _myLogger = logger;
    _consoleLogger.reset();
#ifdef GLOG_FOUND
    if (!_myLogger) {
        return;
    }

    CGlogLoggerAdaptor* adaptor = new CGlogLoggerAdaptor(logger);
    // pipe EVERYTHING from glog to the logger
    google::base::SetLogger(google::INFO, adaptor);
#endif
}

CTiglLogging& CTiglLogging::Instance()
{
    static CTiglLogging instance;
    return instance;
}

void CTiglLogging::initLogger()
{
#ifdef GLOG_FOUND
    // TODO (bgruber): this is problematic, as the application linking to TIGL may also use glog and initialize it itself
    // Initialize Google's logging library.
    google::InitGoogleLogging("TIGL-log");
#endif
}

void CTiglLogging::LogToFile(const char* prefix) 
{
    char buffer[80];
    if (_timeIdInFilename) {
        time_t rawtime;
        time(&rawtime);
        tm *timeinfo = localtime(&rawtime);
        strftime(buffer, 80, "%y%m%d-%H%M%S", timeinfo);
    } else {
        buffer[0] = 0;
    }
    std::string filename = std::string(prefix) + buffer + "." + _fileEnding;
    
    // add file and console logger to splitter
    CSharedPtr<CTiglLogSplitter> splitter (new CTiglLogSplitter);
    PTiglLogger fileLogger (new CTiglFileLogger(filename.c_str()));
    splitter->AddLogger(fileLogger);
    PTiglLogger consoleLogger (new CTiglConsoleLogger);
    consoleLogger->SetVerbosity(_consoleVerbosity);
    splitter->AddLogger(consoleLogger);
    SetLogger(splitter);
    // set _consoleLogger variable
    _consoleLogger=consoleLogger;
}

void CTiglLogging::LogToStream(FILE * fp) 
{

    // add file and console logger to splitter
    CSharedPtr<CTiglLogSplitter> splitter (new CTiglLogSplitter);
    PTiglLogger fileLogger (new CTiglFileLogger(fp));
    splitter->AddLogger(fileLogger);
    PTiglLogger consoleLogger (new CTiglConsoleLogger);
    consoleLogger->SetVerbosity(_consoleVerbosity);
    splitter->AddLogger(consoleLogger);
    SetLogger(splitter);
    // set _consoleLogger variable
    _consoleLogger=consoleLogger;
}

void CTiglLogging::SetLogFileEnding(const char* ending) 
{
    _fileEnding = ending;
}

void CTiglLogging::SetTimeIdInFilenameEnabled(bool enabled) 
{
    _timeIdInFilename = enabled;
}

void CTiglLogging::SetConsoleVerbosity(TiglLogLevel vlevel) 
{
    _consoleVerbosity=vlevel;
    if (_consoleLogger) {
        _consoleLogger->SetVerbosity(_consoleVerbosity);
    }
}

void CTiglLogging::LogToConsole() 
{
#ifdef GLOG_FOUND
    google::LogToStderr();
#else

    PTiglLogger consoleLogger (new CTiglConsoleLogger);
    consoleLogger->SetVerbosity(_consoleVerbosity);
    SetLogger(consoleLogger);
    // set _consoleLogger variable
    _consoleLogger=consoleLogger;
#endif
}

std::string getLogLevelString(TiglLogLevel level)
{
    return LogLevelStrings[level];
}

#ifndef GLOG_FOUND

DummyLogger_::DummyLogger_(){}
DummyLogger_::~DummyLogger_()
{
    tigl::PTiglLogger logger = CTiglLogging::Instance().GetLogger();
    std::string msg = stream.str();
    if (msg.size() > 0 && msg[msg.size()-1] == '\n') {
        msg.resize(msg.size() - 1);
    }
    if (logger) {
        logger->LogMessage(_lastLevel, msg.c_str());
    }
    else {
        if (_lastLevel == TILOG_WARNING || _lastLevel == TILOG_ERROR) {
            fprintf(stderr, "%s\n", msg.c_str());
        }
        else {
            fprintf(stdout, "%s\n", msg.c_str());
        }
    }
}


std::ostringstream& DummyLogger_::AppendToStream(TiglLogLevel level, const char* file, int line) 
{
    _lastLevel = level;
    
    stream <<  getLogLevelString(level) << " ";

    // timestamp
    time_t rawtime;
    time (&rawtime);
    struct tm *timeinfo = localtime (&rawtime);
    char buffer [80];
    strftime (buffer,80,"%m/%d %H:%M:%S",timeinfo);
    stream << buffer << " ";

    stream << BASENAME(file) << ":" << line  << "] ";
    stream << std::string(level > TILOG_DEBUG ? level - TILOG_DEBUG : 0, '\t');
    return stream;
}

DebugStream_::DebugStream_(){}
DebugStream_::~DebugStream_() 
{
#ifdef DEBUG
    tigl::PTiglLogger logger = CTiglLogging::Instance().GetLogger();
    std::string msg = stream.str();
    if (msg.size() > 0 && msg[msg.size()-1] == '\n') {
        msg.resize(msg.size() - 1);
    }
    if (logger) {
        logger->LogMessage(_lastLevel, msg.c_str());
    }
    else {
        if (_lastLevel == TILOG_WARNING || _lastLevel == TILOG_ERROR) {
            fprintf(stderr, "%s\n", msg.c_str());
        }
        else {
            fprintf(stdout, "%s\n", msg.c_str());
        }
    }
#endif
}

std::ostringstream& DebugStream_::AppendToStream(TiglLogLevel level, const char* file, int line) 
{
#ifdef DEBUG
    _lastLevel = level;

    stream <<  getLogLevelString(level) << "-DEBUG ";

    // timestamp
    time_t rawtime;
    time (&rawtime);
    struct tm *timeinfo = localtime (&rawtime);
    char buffer [80];
    strftime (buffer,80,"%m/%d %H:%M:%S",timeinfo);
    stream << buffer << " ";

    stream << BASENAME(file) << ":" << line  << "] ";
    stream << std::string(level > TILOG_DEBUG ? level - TILOG_DEBUG : 0, '\t');
#endif
    return stream;

}

#endif

} // end namespace tigl
