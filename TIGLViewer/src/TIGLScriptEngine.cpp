/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLScriptEngine.h 64 2012-10-23 23:54:09Z markus.litz $
*
* Version: $Revision: 64 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "TIGLScriptEngine.h"
#include "TIGLScriptProxy.h"

#include <QtGui/QWidget>

TIGLScriptEngine::TIGLScriptEngine()
{
    scriptProxy = new TIGLScriptProxy();
    
    QScriptEngine engine;
    TIGLScriptProxy::registerClass(&engine);
    
}


QString TIGLScriptEngine::textChanged(QString line)
{
    script = line;
    return line;
}

void TIGLScriptEngine::eval()
{
    emit printResults("Return Pressed");
}





