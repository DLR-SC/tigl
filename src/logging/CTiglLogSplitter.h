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

#include "tigl_internal.h"
#include "ITiglLogger.h"
#include "CTiglLogging.h"
#include <vector>

namespace tigl 
{

class CTiglLogSplitter : public ITiglLogger
{
public:
    TIGL_EXPORT CTiglLogSplitter();
    TIGL_EXPORT ~CTiglLogSplitter() OVERRIDE;

    // Adds a logger to the splitter
    TIGL_EXPORT void AddLogger(PTiglLogger);

    // override from ITiglLogger
    TIGL_EXPORT void LogMessage(TiglLogLevel, const char * message) OVERRIDE;
    TIGL_EXPORT void SetVerbosity(TiglLogLevel) OVERRIDE;

private:
    std::vector<PTiglLogger> _loggers;
    TiglLogLevel verbosity;
};

} // namespace tigl

#endif // CTIGLLOGSPLITTER_H
