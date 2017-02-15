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
* �  �  http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef TIGLSCRIPTENGIE_H
#define TIGLSCRIPTENGIE_H

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QLineEdit>
#include <QtScript>

class TIGLViewerWindow;
class TIGLScriptProxy;
class TIXIScriptProxy;

class TIGLScriptEngine :public QObject
{
    Q_OBJECT
    
public:
    TIGLScriptEngine(TIGLViewerWindow* window);
    //~TIGLScriptEngine();
    void openFile(QString fileName);
    
public slots:
    void textChanged(QString line);
    void eval(QString commandLine);
    void printText(QString);
    
signals:
    void scriptResult(QString text);
    void scriptError(QString text);
    void evalDone();
    
private:
    QScriptEngine engine;
    TIGLScriptProxy *tiglScriptProxy;
    TIXIScriptProxy *tixiScriptProxy;
    QString script;
    QString prefixString;
    void displayHelp();
};


#endif // TIGLSCRIPTENGIE_H

