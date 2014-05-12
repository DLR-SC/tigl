/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2014-01-25 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef TIGLTOANDROIDLOGADAPTER_H_
#define TIGLTOANDROIDLOGADAPTER_H_

#include "ITiglLogger.h"

#include <string>

class TiglAndroidLogger : public tigl::ITiglLogger
{
public:
    virtual void LogMessage(TiglLogLevel, const char * message) ;
    virtual void SetVerbosity(TiglLogLevel) ;

    void SetTag(const char* tag);

private:
    std::string tag;
};

#endif /* TIGLTOANDROIDLOGADAPTER_H_ */
