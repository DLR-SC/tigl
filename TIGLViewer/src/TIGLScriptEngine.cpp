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
#include "TIXIScriptProxy.h"
#include "TIGLScriptProxy.h"

#include <QtGui/QWidget>

TIGLScriptEngine::TIGLScriptEngine(QLineEdit *le)
{
    tiglScriptProxy = new TIGLScriptProxy();
    tixiScriptProxy = new TIXIScriptProxy();
    TIGLScriptProxy::registerClass(&engine); 
    TIXIScriptProxy::registerClass(&engine);
    lineEdit = le;
    prefixString = "$";
}


void TIGLScriptEngine::textChanged(QString line)
{
    // do fancy stuff in future, like input-completion
}

void TIGLScriptEngine::eval()
{
    QScriptValue val;
    QString commandLine = lineEdit->text();

    // display help
    if (commandLine == "help" || commandLine == "hilfe" || commandLine == "damn" || commandLine == "?") {
        displayHelp();
        lineEdit->setText("");
        return;
    }

    // only for testing purpose
    //engine.evaluate("var tigl = new TIGL();");
    //engine.evaluate("var tixi = new TIXI();");
    //val = engine.evaluate("tigl.tiglGetFuselageCount();");
    //val = engine.evaluate("tixi.tixiGetTextElement('/cpacs/header/name');");

    val = engine.evaluate(lineEdit->text());
    QString result = val.toString();
    if(result == "undefined") {
        result = "done";
    }

    emit printResults("<i><font color=\"lime\">" + prefixString + lineEdit->text() + "</font></i>");
    emit printResults("<font color=\"olive\">" + prefixString + result + "</font>");
    lineEdit->setText("");
}

void TIGLScriptEngine::displayHelp()
{
    QString helpString;

    helpString =  "====== TIGLViewer scripting help ======\n\n";
    helpString += "Available TIGL functions: \n";
    helpString += tiglScriptProxy->getMemberFunctions().join("\n") + "\n\n";
    helpString += "Available TIXI functions: \n";
    helpString += tixiScriptProxy->getMemberFunctions().join("\n") + "\n\n";

    helpString += "Usage example TIGL: \n";
    helpString += "Initialize TIGL: \tvar tigl = new TIGL();\n";
    helpString += "Use TIGL: \ttigl.tiglGetFuselageCount();\n";
    helpString += "Usage example TIXI: \n";
    helpString += "Initialize TIXI: \tvar tixi = new TIXI();\n";
    helpString += "Use TIXI: \ttixi.tixiGetTextElement('/cpacs/header/name');\n";
    helpString += "Type 'help' to get a list of available TIXI/TIGL fuctions.";


    emit printResults( helpString );
}



