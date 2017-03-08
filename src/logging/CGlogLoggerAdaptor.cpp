/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-26 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CGlogLoggerAdaptor.h"
#include "ITiglLogger.h"
#include "CMutex.h"

#ifdef GLOG_FOUND

// uncomment to enable debugging messages
//#define LOGGER_DEBUG

namespace tigl 
{

CGlogLoggerAdaptor::CGlogLoggerAdaptor(PTiglLogger logger) : _mutex(new CMutex)
{
    _mylogger = logger;
}

CGlogLoggerAdaptor::~CGlogLoggerAdaptor() 
{
}

void CGlogLoggerAdaptor::Write(bool force_flush,
                               time_t /* timestamp */,
                               const char* message,
                               int message_len) 
{
#ifdef LOGGER_DEBUG
    printf("CGlogLoggerAdaptor::Write called. force_flush=%d\n", force_flush);
#endif
    
    if (_mylogger && message_len > 0) {
        if (_mutex) {
            _mutex->lock();
        }
        std::string msg(message, message_len);

        //TODO: determine log level
        TiglLogLevel level = TILOG_INFO;
        if (msg[0] == 'I') {
            level = TILOG_INFO;
        }
        else if (msg[0] == 'E') {
            level = TILOG_ERROR;
        }
        else if (msg[0] == 'W') {
            level = TILOG_WARNING;
        }

        _mylogger->LogMessage(level, msg.c_str());
        if (_mutex) {
            _mutex->unlock();
        }
    }
}

// Flush any buffered messages
void CGlogLoggerAdaptor::Flush() 
{
#ifdef LOGGER_DEBUG
    printf("CGlogLoggerAdaptor::Flush called.\n");
#endif
}

google::uint32 CGlogLoggerAdaptor::LogSize() 
{
#ifdef LOGGER_DEBUG
    printf("CGlogLoggerAdaptor::LogSize called.\n");
#endif
    return 0;
}

} // namespace tigl

#endif // GLOG_FOUND
