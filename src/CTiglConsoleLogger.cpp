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

#include "TiglLoggerDefinitions.h"
#include "CTiglConsoleLogger.h"

#include <iostream>

tigl::CTiglConsoleLogger::CTiglConsoleLogger() : verbosity(TILOG_ERROR)
{                                            
}
void tigl::CTiglConsoleLogger::LogMessage(TiglLogLevel level, const char *message)
{
    if (level<=verbosity)
    {
        if(level == TILOG_ERROR || level == TILOG_WARNING) 
        {
            fprintf(stderr, "%s\n", message);
        }
        else 
        {
            fprintf(stdout, "%s\n", message);
        }
    }
}

void tigl::CTiglConsoleLogger::SetVerbosity(TiglLogLevel vlevel)
{
    verbosity=vlevel;
}
