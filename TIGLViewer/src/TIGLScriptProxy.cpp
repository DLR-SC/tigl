/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLScriptProxy.h 64 2012-10-23 23:54:09Z markus.litz $
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

#include "TIGLScriptProxy.h"

#include <QtGui/QWidget>

TIGLScriptProxy::TIGLScriptProxy()
{
    m_cpacsHandle = 1;
}

// Returns the CPACS configuration
tigl::CCPACSConfiguration& TIGLScriptProxy::GetConfiguration(void) const
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    return manager.GetConfiguration(m_cpacsHandle);
}

void TIGLScriptProxy::registerClass(QScriptEngine* engine)
{
        if (!engine)
                return ;
        
        QScriptValue ctor = engine->newFunction(TIGLScriptProxyConstructor);
        QScriptValue metaObject = engine->newQMetaObject(&QObject::staticMetaObject, ctor);
        engine->globalObject().setProperty("TIGL", metaObject);
}

QStringList TIGLScriptProxy::getMemberFunctions()
{
}

int TIGLScriptProxy::tiglGetWingCount()
{
    tigl::CCPACSConfiguration& config = GetConfiguration();
    return(config.GetWingCount());
}


QScriptValue TIGLScriptProxyConstructor(QScriptContext * context, QScriptEngine * engine)
{
        QString fileName = context->argument(0).toString();
        //QObject *parent = context->argument(1).toQObject();
        TIGLScriptProxy *TIGL = new TIGLScriptProxy();//, parent);
        return engine->newQObject(TIGL, QScriptEngine::ScriptOwnership);
}

