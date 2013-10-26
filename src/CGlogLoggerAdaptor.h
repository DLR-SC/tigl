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

#ifndef CGLOGLOGGERADAPTOR_H
#define CGLOGLOGGERADAPTOR_H

#include "tigl_config.h"

#ifdef GLOG_FOUND

#include <glog/logging.h>

namespace tigl {

class ITiglLogger;
class CMutex;

/**
 * @brief The CGlogLoggerAdaptor class acts as an adaptor to plug
 * a TiglLogger into the glog framework
 */
class CGlogLoggerAdaptor : public google::base::Logger
{
public:
    CGlogLoggerAdaptor(ITiglLogger*);
    
    virtual ~CGlogLoggerAdaptor();
  
    virtual void Write(bool force_flush,
                       time_t timestamp,
                       const char* message,
                       int message_len) = 0;
  
    // Flush any buffered messages
    virtual void Flush();

    virtual google::uint32 LogSize();

private:
    ITiglLogger* _mylogger;
    CMutex* _mutex;
};

} // namespace tigl

#endif // GLOG_FOUND

#endif // CGLOGLOGGERADAPTOR_H
