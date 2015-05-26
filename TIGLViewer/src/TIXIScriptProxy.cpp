/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIXIScriptProxy.cpp 64 2012-10-23 23:54:09Z markus.litz $
*
* Version: $Revision: 64 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* Â  Â  http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "TIXIScriptProxy.h"

#include <QWidget>

TIXIScriptProxy::TIXIScriptProxy()
{
    tixiHandle = 1;
}

// Returns the tixi handle
TixiDocumentHandle TIXIScriptProxy::GetConfiguration(void) const
{
    return tixiHandle;
}

char* TIXIScriptProxy::qString2char(QString str)
{
    return (str.toLatin1().data());
}

void TIXIScriptProxy::registerClass(QScriptEngine* engine)
{
    if (!engine) {
        return ;
    }
    
    QScriptValue ctor = engine->newFunction(TIXIScriptProxyConstructor);
    QScriptValue metaObject = engine->newQMetaObject(&QObject::staticMetaObject, ctor);
    engine->globalObject().setProperty("TIXI", metaObject);
}

QScriptValue TIXIScriptProxyConstructor(QScriptContext * context, QScriptEngine * engine)
{
    QString fileName = context->argument(0).toString();
    TIXIScriptProxy *TIXI = new TIXIScriptProxy();//, parent);
    return engine->newQObject(TIXI, QScriptEngine::ScriptOwnership);
}

QStringList TIXIScriptProxy::getMemberFunctions()
{
    return QStringList()  << "tixiGetTextElement(elementPath)" << "tixiGetVersion ";
}

QString TIXIScriptProxy::tixiGetVersion()
{
    return(::tixiGetVersion());
}

QString TIXIScriptProxy::tixiGetTextElement(QString elementPath)
{
    char *text;
    ::tixiGetTextElement (GetConfiguration(), qString2char(elementPath), &text);
    printf("\n%s\n", text);
    return(QString(text));
}


