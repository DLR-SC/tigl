/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLScriptProxy.cpp 64 2012-10-23 23:54:09Z markus.litz $
*
* Version: $Revision: 64 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
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

TiglCPACSConfigurationHandle TIGLScriptProxy::getTiglHandle(void)
{
    return m_cpacsHandle;
}

char* TIGLScriptProxy::qString2char(QString str)
{
    QByteArray ba = str.toLocal8Bit();
    return (ba.data());
}

void TIGLScriptProxy::registerClass(QScriptEngine* engine)
{
        if (!engine) {
            return;
        }
        
        QScriptValue ctor = engine->newFunction(TIGLScriptProxyConstructor);
        QScriptValue metaObject = engine->newQMetaObject(&QObject::staticMetaObject, ctor);
        engine->globalObject().setProperty("TIGL", metaObject);
}

QScriptValue TIGLScriptProxyConstructor(QScriptContext * context, QScriptEngine * engine)
{
        QString fileName = context->argument(0).toString();
        //QObject *parent = context->argument(1).toQObject();
        TIGLScriptProxy *TIGL = new TIGLScriptProxy();//, parent);
        return engine->newQObject(TIGL, QScriptEngine::ScriptOwnership);
}

QStringList TIGLScriptProxy::getMemberFunctions()
{
    return QStringList()  << "tiglGetWingCount" << "tiglGetVersion" << "tiglComponentGetHashCode(componentUID)" << "tiglComponentIntersectionLineCount(uid1, uid2)"
                          << "tiglExportFusedWingFuselageIGES(filename)" << "tiglExportIGES(filename)" << "tiglExportMeshedFuselageSTL(fuselageIndex, filename, deflection)"
                          << "tiglExportMeshedFuselageVTKByIndex(fuselageIndex, filename, deflection)" << "tiglExportMeshedFuselageVTKByUID(fuselageUID, filename, deflection)"
                          << "tiglFuselageGetCircumference(fuselageIndex, segmentIndex, eta)" << "tiglFuselageGetPoint(fuselageIndex, segmentIndex, eta, zeta)"
                          << "tiglFuselageGetSegmentUID(fuselageIndex, segmentIndex)" << "tiglFuselageGetSegmentVolume(fuselageIndex, segmentIndex)"
                          << "tiglGetFuselageCount";
}

int TIGLScriptProxy::tiglGetWingCount()
{
    tigl::CCPACSConfiguration& config = GetConfiguration();
    return(config.GetWingCount());
}

QString TIGLScriptProxy::tiglGetVersion()
{
    return(::tiglGetVersion());
}


int TIGLScriptProxy::tiglComponentGetHashCode (QString componentUID)
{
    int hashCode;
    ::tiglComponentGetHashCode (getTiglHandle(), qString2char(componentUID), &hashCode);
    return(hashCode);
}

int TIGLScriptProxy::tiglComponentIntersectionLineCount (QString componentUidOne, QString componentUidTwo)
{
    int numWires;
    ::tiglComponentIntersectionLineCount (getTiglHandle(), qString2char(componentUidOne), qString2char(componentUidTwo), &numWires);
    return(numWires);
}

void TIGLScriptProxy::tiglExportFusedWingFuselageIGES (QString filenamePtr)
{
    ::tiglExportFusedWingFuselageIGES(getTiglHandle(), qString2char(filenamePtr));
}

void TIGLScriptProxy::tiglExportIGES (QString filenamePtr)
{
    ::tiglExportIGES(getTiglHandle(), qString2char(filenamePtr));
}

void TIGLScriptProxy::tiglExportMeshedFuselageSTL (int fuselageIndex, QString filenamePtr, double deflection)
{
    ::tiglExportMeshedFuselageSTL(getTiglHandle(), fuselageIndex, qString2char(filenamePtr), deflection);
}

void TIGLScriptProxy::tiglExportMeshedFuselageVTKByIndex (int fuselageIndex, QString filenamePtr, double deflection)
{
    ::tiglExportMeshedFuselageVTKByIndex(getTiglHandle(), fuselageIndex, qString2char(filenamePtr), deflection);
}

void TIGLScriptProxy::tiglExportMeshedFuselageVTKByUID (QString fuselageUID, QString filenamePtr, double deflection)
{
    ::tiglExportMeshedFuselageVTKByUID(getTiglHandle(), qString2char(fuselageUID), qString2char(filenamePtr), deflection);
}

double TIGLScriptProxy::tiglFuselageGetCircumference (int fuselageIndex, int segmentIndex, double eta)
{
    double circumference;
    ::tiglFuselageGetCircumference (getTiglHandle(), fuselageIndex, segmentIndex, eta, &circumference);
    return circumference;
}

QString TIGLScriptProxy::tiglFuselageGetPoint (int fuselageIndex, int segmentIndex, double eta, double zeta)
{
    double x,y,z;
    QString strX, strY, strZ, pnt;
    ::tiglFuselageGetPoint (getTiglHandle(), fuselageIndex, segmentIndex, eta, zeta, &x, &y, &z);
    strX.setNum(x);
    strY.setNum(y);
    strZ.setNum(z);
    pnt = strX + ", " + strY + ", " + strZ;
    return(pnt);
}

QString TIGLScriptProxy::tiglFuselageGetSegmentUID (int fuselageIndex, int segmentIndex)
{
    char *uidName;
    ::tiglFuselageGetSegmentUID (getTiglHandle(), fuselageIndex, segmentIndex, &uidName);
    return(QString(uidName));
}

double TIGLScriptProxy::tiglFuselageGetSegmentVolume (int fuselageIndex, int segmentIndex)
{
    double volume;
    ::tiglFuselageGetSegmentVolume (getTiglHandle(), fuselageIndex, segmentIndex, &volume);
    return volume;
}

int TIGLScriptProxy::tiglGetFuselageCount()
{
    tigl::CCPACSConfiguration& config = GetConfiguration();
    return(config.GetFuselageCount());
}

