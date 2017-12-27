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

#ifndef TIGLVIEWERLOGGERHTMLDECORATOR_H
#define TIGLVIEWERLOGGERHTMLDECORATOR_H

#include "tigl_internal.h"
#include "ITiglLogger.h"
#include "CSharedPtr.h"

class TIGLViewerLoggerHTMLDecorator : public tigl::ITiglLogger
{
public:
    TIGLViewerLoggerHTMLDecorator(CSharedPtr<ITiglLogger>);
    
    ~TIGLViewerLoggerHTMLDecorator() OVERRIDE;
    void LogMessage(TiglLogLevel, const char * message) OVERRIDE;
    void SetVerbosity(TiglLogLevel) OVERRIDE;
private:
    CSharedPtr<ITiglLogger> _mylogger;
};

#endif // TIGLVIEWERLOGGERHTMLDECORATOR_H
