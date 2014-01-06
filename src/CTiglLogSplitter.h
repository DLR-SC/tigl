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

#ifndef CTIGLLOGSPLITTER_H
#define CTIGLLOGSPLITTER_H

#include "ITiglLogger.h"
#include <list>

namespace tigl {

class CTiglLogSplitter : public ITiglLogger
{
public:
    CTiglLogSplitter();
    virtual ~CTiglLogSplitter();

    // class specific functions

    // Adds a logger to the splitter. Only messages with
    // a log level < maxlevel are transferred to the logger
    // The logger becomes property of this class
    // Therefore the logger should not be deleted
    // manually.
    void AddLogger(ITiglLogger*, TiglLogLevel maxlevel);

    // override from ITiglLogger
    virtual void LogMessage(TiglLogLevel, const char * message);

private:
    class LoggerEntry {
    public:
        LoggerEntry(ITiglLogger* log, TiglLogLevel lev)
            : logger(log), level(lev) {}

        ITiglLogger* logger;
        TiglLogLevel level;
    };

    typedef std::list<LoggerEntry> Loggerlist;
    Loggerlist _loggers;
};

} // namespace tigl

#endif // CTIGLLOGSPLITTER_H
