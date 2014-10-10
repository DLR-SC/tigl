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

#include "TIGLScriptEngine.h"
#include "TIXIScriptProxy.h"
#include "TIGLScriptProxy.h"

#include <QWidget>

template <class T>
QScriptValue qobjectToScriptValue(QScriptEngine *engine, T* const &in)
{
    return engine->newQObject(in);
}

template <class T>
void objectFromScriptValue(const QScriptValue &object, T* &out)
{
    out = qobject_cast<T*>(object.toQObject());
}


QScriptValue myPrintFunction(QScriptContext *context, QScriptEngine *engine)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0) {
            result.append(" ");
        }
        result.append(context->argument(i).toString());
    }

    QScriptValue calleeData = context->callee().data();
    TIGLScriptEngine *scriptEngine = qobject_cast<TIGLScriptEngine*>(calleeData.toQObject());
    scriptEngine->printText(result);

    return engine->undefinedValue();
}


TIGLScriptEngine::TIGLScriptEngine(TIGLViewerWindow* app)
    : QObject(app)
{
    tiglScriptProxy = new TIGLScriptProxy(app);
    tixiScriptProxy = new TIXIScriptProxy();
    TIXIScriptProxy::registerClass(&engine);

    QScriptValue appValue = engine.newQObject(app);
    engine.globalObject().setProperty("app", appValue);

    QScriptValue tiglProxyValue =  engine.newQObject(tiglScriptProxy, QScriptEngine::ScriptOwnership);
    engine.globalObject().setProperty("tigl", tiglProxyValue);

    qScriptRegisterMetaType(&engine, qobjectToScriptValue<TIGLViewerWidget>, objectFromScriptValue<TIGLViewerWidget>);
    qScriptRegisterMetaType(&engine, qobjectToScriptValue<TIGLViewerContext>, objectFromScriptValue<TIGLViewerContext>);

    // register/overwrite print function
    QScriptValue printFun = engine.newFunction(myPrintFunction);
    printFun.setData(engine.newQObject(this));
    engine.globalObject().setProperty("print", printFun);
    
    // evaluate resource file
    openFile(":/scripts/globaldefs.js");
    
    prefixString = "";
}


void TIGLScriptEngine::textChanged(QString line)
{
    // do fancy stuff in future, like input-completion
}


void TIGLScriptEngine::openFile(QString fileName)
{
    QString script;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            script += file.readLine() + "\n";
        }
        eval(script);
    }
}

void TIGLScriptEngine::eval(QString commandLine)
{
    QScriptValue val;

    // display help
    if (commandLine == "help" || commandLine == "hilfe" || commandLine == "damn" || commandLine == "?") {
        displayHelp();
        return;
    }

    try {
        val = engine.evaluate(commandLine);
    }
    catch (tigl::CTiglError& err) {
        emit scriptError(err.getError());
        emit evalDone();
        return;
    }
    catch(...) {
        emit scriptError("Unknown exception!");
        emit evalDone();
        return;
    }

    QString result = val.toString();
    if (!val.isUndefined() && !val.isError() && !result.isEmpty()) {
        emit scriptResult( prefixString + result  );
    }
    else if (val.isError()) {
        emit scriptError(val.toString());
    }
    emit evalDone();
}

void TIGLScriptEngine::displayHelp()
{
    QString helpString;

    helpString =  "====== TIGLViewer scripting help ======<br/><br/>";
    helpString += "Available TIGL functions: <br/>";
    foreach(QString fun, tiglScriptProxy->getMemberFunctions()) {
        helpString += "    " + fun + "<br/>";
    }

    helpString += "<br/><br/>";
    helpString += "Usage example TIGL: <br/>";
    helpString += "Use TIGL: \ttigl.getFuselageCount();<br/>";

    helpString += "Type 'help' to get a list of available TIXI/TIGL fuctions.";


    emit scriptResult( helpString );
    emit evalDone();
}

void TIGLScriptEngine::printText(QString text)
{
    emit scriptResult(text);
}



