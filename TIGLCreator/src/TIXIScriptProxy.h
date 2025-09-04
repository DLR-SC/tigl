/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIXIScriptProxy.h 64 2012-10-23 23:54:09Z markus.litz $
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
#ifndef TIXISCRIPTPOXY_H
#define TIXISCRIPTPOXY_H

#include <QtCore/QString>
#include <QtCore/QObject>

#include <QtScript>
#include <QString>

#include "tixi.h"

class TIXIScriptProxy : public QObject
{
    Q_OBJECT
    
    // Q_PROPERTY( QString wingCount READ tiglGetWingCount() )
    
private:
    // Returns the TIXI handle
    TixiDocumentHandle GetConfiguration() const;
    char* qString2char(QString str);
    
public:
    TIXIScriptProxy();
    //~TIXIScriptProxy();
    
    static void registerClass(QScriptEngine *);
    
public slots:
    QStringList getMemberFunctions();
    
    // wrapped tixi functions
    QString tixiGetVersion();
    QString tixiGetTextElement (QString elementPath);
    
private:
    QStringList memberFunctions;
    int tixiHandle;

};

QScriptValue TIXIScriptProxyConstructor(QScriptContext *, QScriptEngine *);

#endif // TIXISCRIPTPOXY_H

