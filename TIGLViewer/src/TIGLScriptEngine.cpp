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
#include <QMetaMethod>

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

QScriptValue isQObject(QScriptContext *context, QScriptEngine *)
{
    if (context->argumentCount() == 1) {
        return context->argument(0).isQObject();
    }
    else {
        return context->throwError("Invalid number of arguments");
    }
}

QScriptValue isQMetaObject(QScriptContext *context, QScriptEngine *)
{
    if (context->argumentCount() == 1) {
        return context->argument(0).isQMetaObject();
    }
    else {
        return context->throwError("Invalid number of arguments");
    }
}

QScriptValue QMetaObjectGetObjectDescription(QScriptContext *context, QScriptEngine *)
{
    if (context->argumentCount() != 1 || !context->argument(0).isQMetaObject()) {
        return context->throwError("Argument must be a qmetaobject");
    }

    const QMetaObject* meta = context->argument(0).toQMetaObject();
    
    int index=meta->indexOfClassInfo("Description");
    if (index >= 0) {
        QString desc = meta->classInfo(index).value();
        return desc;
    }
    else {
        // return object name
        return meta->className();
    }
}

QScriptValue QObjGetMetaObj(QScriptContext *context, QScriptEngine * engine)
{
    if (context->argumentCount() != 1 || !context->argument(0).isQObject()) {
        return context->throwError("Invalid argument");
    }
    
    QObject* obj = context->argument(0).toQObject();
    const QMetaObject* meta = obj->metaObject();
    
    return engine->newQMetaObject(meta);
}


QScriptValue QMetaObjectGetMemberFunctions(QScriptContext *context, QScriptEngine * engine)
{
    if (context->argumentCount() != 1 || !context->argument(0).isQMetaObject()) {
        return context->throwError("Argument must be a qmetaobject");
    }

    const QMetaObject* meta = context->argument(0).toQMetaObject();
    
    QStringList retval;
    for (int imeth = meta->methodOffset(); imeth < meta->methodCount(); ++imeth) {
        QMetaMethod method = meta->method(imeth);
        
        if (method.access() != QMetaMethod::Public) {
            continue;
        }
        
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QString name =  method.methodSignature();
#else
        QString name =  method.signature();
#endif
        int idx = name.indexOf("(");
        if (idx >= 0) {
            name = name.left(idx);
        }
        
        if (method.parameterTypes().size() > 0) {
            name += "(";
            for (int i = 0; i < method.parameterTypes().size(); ++i) {
                QString partype = method.parameterTypes().at(i);
                QString parname = method.parameterNames().at(i);
                if (parname.isEmpty()) {
                    name += partype + ", ";
                }
                else {
                    name += parname + ", ";
                }
            }

            name = name.left(name.size()-2);
            name += ")";
        }
        else {
            name += "()";
            //name = method.signature();
        }
        retval << name;
    }
    
    return qScriptValueFromSequence (engine, retval);
}

QScriptValue QMetaObjectGetProperties(QScriptContext *context, QScriptEngine * engine)
{
    if (context->argumentCount() != 1 || !context->argument(0).isQMetaObject()) {
        return context->throwError("Argument must be a qmetaobject");
    }

    const QMetaObject* meta = context->argument(0).toQMetaObject();
    QStringList retval;
    for (int iprop = meta->propertyOffset(); iprop < meta->propertyCount(); ++iprop) {
        QMetaProperty prop = meta->property(iprop);
        if (!prop.isReadable()) {
            continue;
        }
        QString name =  prop.name();
        retval << name;
    }
    
    return qScriptValueFromSequence (engine, retval);
}

QScriptValue QMetaObjectGetSuperclass(QScriptContext *context, QScriptEngine * engine)
{
    if (context->argumentCount() != 1 || !context->argument(0).isQMetaObject()) {
        return context->throwError("Argument must be a qmetaobject");
    }

    const QMetaObject* meta = context->argument(0).toQMetaObject();
    
    return engine->newQMetaObject(meta->superClass());
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
    qScriptRegisterMetaType(&engine, qobjectToScriptValue<TIGLViewerDocument>, objectFromScriptValue<TIGLViewerDocument>);

    // register/overwrite print function
    QScriptValue printFun = engine.newFunction(myPrintFunction);
    printFun.setData(engine.newQObject(this));
    engine.globalObject().setProperty("print", printFun);
    
    QScriptValue isQObjFun = engine.newFunction(isQObject,1);
    engine.globalObject().setProperty("_isQObject", isQObjFun);
    
    QScriptValue isQMetaObjFun = engine.newFunction(isQMetaObject,1);
    engine.globalObject().setProperty("_isQMetaObject", isQMetaObjFun);
    
    QScriptValue qobjGetMetaObj = engine.newFunction(QObjGetMetaObj,1);
    engine.globalObject().setProperty("_qobjGetMetaObj", qobjGetMetaObj);
    
    QScriptValue qobjGetMembers = engine.newFunction(QMetaObjectGetMemberFunctions, 1);
    engine.globalObject().setProperty("_qMetaObjMembers", qobjGetMembers);
    
    QScriptValue qobjGetProps = engine.newFunction(QMetaObjectGetProperties, 1);
    engine.globalObject().setProperty("_qMetaObjProperties", qobjGetProps);
    
    QScriptValue qobjDescription = engine.newFunction(QMetaObjectGetObjectDescription, 1);
    engine.globalObject().setProperty("_qMetaObjDescription", qobjDescription);
    
    QScriptValue qobjSuperclasss = engine.newFunction(QMetaObjectGetSuperclass, 1);
    engine.globalObject().setProperty("_qMetaObjGetSuperclass", qobjSuperclasss);
    
    // evaluate resource file
    openFile(":/scripts/globaldefs.js");
    
    engine.globalObject().setProperty("ans", 0);
    
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
        engine.globalObject().setProperty("ans", val);
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

    helpString =  "====== TiGL Viewer scripting help ======<br/><br/>";

    helpString += "Some examples how to use TiGL: <br/>";
    helpString += "Compute and draw point on wing:<br/>";
    helpString += "    p = tigl.wingGetUpperPoint(1, 1, 0.5, 0.5);<br/>";
    helpString += "    drawPoint(p);<br/><br/>";

    helpString += "Draw first wing:<br/>";
    helpString += "    uid = tigl.wingGetUID(1);<br/>";
    helpString += "    drawShape(tigl.getShape(uid));<br/><br/>";
    
    helpString += "Show top view and fit screen:<br/>";
    helpString += "    app.viewer.viewTop();<br/>";
    helpString += "    app.viewer.fitAll();<br/><br/>";
    
    helpString += "Make a screenshot:<br/>";
    helpString += "    app.viewer.makeScreenshot('image.png');//PNG image, white background<br/>";
    helpString += "    app.viewer.makeScreenshot('image.jpg', false);//JPEG image, current background<br/><br/>";

    helpString += "Type 'help(tigl)' to get a list of available TiGL fuctions.<br/>";
    helpString += "Use the help function on any object to display its public methods e.g. 'help(app.viewer)'.";
    emit scriptResult(helpString);
    emit evalDone();
}

void TIGLScriptEngine::printText(QString text)
{
    emit scriptResult(text);
}



