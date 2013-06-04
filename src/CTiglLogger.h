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
 * @brief  Implementation of the TIGL Logger.
 */

#ifndef CTIGLLOGGER_H
#define CTIGLLOGGER_H

#include "tigl_config.h"

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
enum LogLevelDummy_ {ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3, DEBUG4};
#endif

namespace tigl {

#ifndef GLOG_FOUND
    //dummy implementation if glog is not available
    #ifndef LOG_MAX_LEVEL
    #define LOG_MAX_LEVEL DEBUG4
    #endif

    #define LOG(level) \
        if (level > LOG_MAX_LEVEL) ;\
        else tigl::DummyLogger_().AppendToStream(level)

    class DummyLogger_
    {
    public:
        DummyLogger_();
        virtual ~DummyLogger_();
        std::ostringstream& AppendToStream(LogLevelDummy_ level = INFO);
    protected:
        std::ostringstream stream;
    private:
        DummyLogger_(const DummyLogger_&);
        DummyLogger_& operator =(const DummyLogger_&);
    };
#endif // not GLOG_FOUND


class CTiglLogger {

    public:
        // Returns a reference to the only instance of this class
        static CTiglLogger& GetLogger(void);

        // Destructor
        ~CTiglLogger(void);

    private:
        // Constructor
        CTiglLogger(void);

        // Logger Initialize with defaults
        void initLogger(void);

        // Copy constructor
        CTiglLogger(const CTiglLogger& )                { /* Do nothing */ }

        // Assignment operator
        void operator=(const CTiglLogger& )             { /* Do nothing */ }

};

} // end namespace tigl

#endif // CTIGLLOGGER_H
