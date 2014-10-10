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
#include "TIGLViewerDocument.h"

#include <QWidget>

TIGLScriptProxy::TIGLScriptProxy(TIGLViewerWindow* app)
    : QObject(app)
{
    _app = app;
}

// Returns the CPACS configuration
tigl::CCPACSConfiguration& TIGLScriptProxy::GetConfiguration(void)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    return manager.GetConfiguration(getTiglHandle());
}

TiglCPACSConfigurationHandle TIGLScriptProxy::getTiglHandle(void)
{
    return _app->getDocument()->getCpacsHandle();
}

char* TIGLScriptProxy::qString2char(QString str)
{
    QByteArray ba = str.toLocal8Bit();
    return (ba.data());
}

QStringList TIGLScriptProxy::getMemberFunctions()
{
    QStringList retval;
    const QMetaObject* meta = this->metaObject();
    for (int imeth = meta->methodOffset(); imeth < meta->methodCount(); ++imeth) {
        QMetaMethod method = meta->method(imeth);
        QString name =  method.signature();
        int idx = name.indexOf("(");
        if (idx >= 0) {
            name = name.left(idx);
        }
        
        if (method.parameterNames().size() > 0) {
            name += "(";
            foreach(QString parName, method.parameterNames()) {
                name += parName + ", ";
            }
            name = name.left(name.size()-2);
            name += ")";
        }
        retval << name;
    }
    return retval;
}

QScriptValue TIGLScriptProxy::getWingCount()
{
    int count = 0;
    TiglReturnCode ret = ::tiglGetWingCount(getTiglHandle(), &count);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        return count;
    }
}

QString TIGLScriptProxy::getVersion()
{
    return(::tiglGetVersion());
}


int TIGLScriptProxy::componentGetHashCode (QString componentUID)
{
    int hashCode;
    ::tiglComponentGetHashCode (getTiglHandle(), qString2char(componentUID), &hashCode);
    return(hashCode);
}

int TIGLScriptProxy::componentIntersectionLineCount (QString componentUidOne, QString componentUidTwo)
{
    int numWires;
    ::tiglComponentIntersectionLineCount (getTiglHandle(), qString2char(componentUidOne), qString2char(componentUidTwo), &numWires);
    return(numWires);
}

void TIGLScriptProxy::exportFusedWingFuselageIGES (QString filenamePtr)
{
    ::tiglExportFusedWingFuselageIGES(getTiglHandle(), qString2char(filenamePtr));
}

void TIGLScriptProxy::exportIGES (QString filenamePtr)
{
    ::tiglExportIGES(getTiglHandle(), qString2char(filenamePtr));
}

void TIGLScriptProxy::exportMeshedFuselageSTL (int fuselageIndex, QString filenamePtr, double deflection)
{
    ::tiglExportMeshedFuselageSTL(getTiglHandle(), fuselageIndex, qString2char(filenamePtr), deflection);
}

void TIGLScriptProxy::exportMeshedFuselageVTKByIndex (int fuselageIndex, QString filenamePtr, double deflection)
{
    ::tiglExportMeshedFuselageVTKByIndex(getTiglHandle(), fuselageIndex, qString2char(filenamePtr), deflection);
}

void TIGLScriptProxy::exportMeshedFuselageVTKByUID (QString fuselageUID, QString filenamePtr, double deflection)
{
    ::tiglExportMeshedFuselageVTKByUID(getTiglHandle(), qString2char(fuselageUID), qString2char(filenamePtr), deflection);
}

double TIGLScriptProxy::fuselageGetCircumference (int fuselageIndex, int segmentIndex, double eta)
{
    double circumference;
    ::tiglFuselageGetCircumference (getTiglHandle(), fuselageIndex, segmentIndex, eta, &circumference);
    return circumference;
}

QScriptValue TIGLScriptProxy::fuselageGetPoint (int fuselageIndex, int segmentIndex, double eta, double zeta)
{
    double x,y,z;
    TiglReturnCode ret = ::tiglFuselageGetPoint (getTiglHandle(), fuselageIndex, segmentIndex, eta, zeta, &x, &y, &z);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        QScriptValue Point3dCtor = engine()->globalObject().property("Point3d");
        return Point3dCtor.construct(QScriptValueList() << x << y << z);
    }
}

QScriptValue TIGLScriptProxy::fuselageGetSegmentUID (int fuselageIndex, int segmentIndex)
{
    char *uidName;
    TiglReturnCode ret = ::tiglFuselageGetSegmentUID (getTiglHandle(), fuselageIndex, segmentIndex, &uidName);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        return uidName;
    }
}

double TIGLScriptProxy::fuselageGetSegmentVolume (int fuselageIndex, int segmentIndex)
{
    double volume;
    ::tiglFuselageGetSegmentVolume (getTiglHandle(), fuselageIndex, segmentIndex, &volume);
    return volume;
}

QScriptValue TIGLScriptProxy::getFuselageCount()
{
    int count = 0;
    TiglReturnCode ret = ::tiglGetFuselageCount(getTiglHandle(), &count);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        return count;
    }
}

QScriptValue TIGLScriptProxy::fuselageGetSegmentCount(int fuselageIndex)
{
    int count = 0;
    TiglReturnCode ret = ::tiglFuselageGetSegmentCount(getTiglHandle(), fuselageIndex, &count);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        return count;
    }
}

QScriptValue TIGLScriptProxy::wingGetUpperPoint(int wingIndex, int segmentIndex, double eta, double xsi)
{
    double px, py, pz;
    TiglReturnCode ret = ::tiglWingGetUpperPoint(getTiglHandle(), wingIndex, segmentIndex, eta, xsi, &px, &py, &pz);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        QScriptValue Point3dCtor = engine()->globalObject().property("Point3d");
        return Point3dCtor.construct(QScriptValueList() << px << py << pz);
    }
}

QScriptValue TIGLScriptProxy::wingGetLowerPoint(int wingIndex, int segmentIndex, double eta, double xsi)
{
    double px, py, pz;
    TiglReturnCode ret = ::tiglWingGetLowerPoint(getTiglHandle(), wingIndex, segmentIndex, eta, xsi, &px, &py, &pz);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        QScriptValue Point3dCtor = engine()->globalObject().property("Point3d");
        return Point3dCtor.construct(QScriptValueList() << px << py << pz);
    }
}

QScriptValue TIGLScriptProxy::wingGetChordPoint(int wingIndex, int segmentIndex, double eta, double xsi)
{
    double px, py, pz;
    TiglReturnCode ret = ::tiglWingGetChordPoint(getTiglHandle(), wingIndex, segmentIndex, eta, xsi, &px, &py, &pz);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        QScriptValue Point3dCtor = engine()->globalObject().property("Point3d");
        return Point3dCtor.construct(QScriptValueList() << px << py << pz);
    }
}

QScriptValue TIGLScriptProxy::wingGetChordNormal(int wingIndex, int segmentIndex, double eta, double xsi)
{
    double px, py, pz;
    TiglReturnCode ret = ::tiglWingGetChordNormal(getTiglHandle(), wingIndex, segmentIndex, eta, xsi, &px, &py, &pz);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        QScriptValue Point3dCtor = engine()->globalObject().property("Point3d");
        return Point3dCtor.construct(QScriptValueList() << px << py << pz);
    }
}

QScriptValue TIGLScriptProxy::wingGetUpperPointAtDirection(int wingIndex, int segmentIndex, double eta, double xsi, double dirx, double diry, double dirz)
{
    
    double px, py, pz;
    TiglReturnCode ret = ::tiglWingGetUpperPointAtDirection(getTiglHandle(), wingIndex, segmentIndex, eta, xsi, dirx, diry, dirz,&px, &py, &pz);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        QScriptValue Point3dCtor = engine()->globalObject().property("Point3d");
        return Point3dCtor.construct(QScriptValueList() << px << py << pz);
    }
}

QScriptValue TIGLScriptProxy::wingGetLowerPointAtDirection(int wingIndex, int segmentIndex, double eta, double xsi, double dirx, double diry, double dirz)
{
    
    double px, py, pz;
    TiglReturnCode ret = ::tiglWingGetLowerPointAtDirection(getTiglHandle(), wingIndex, segmentIndex, eta, xsi, dirx, diry, dirz,&px, &py, &pz);
    if (ret != TIGL_SUCCESS) {
        return context()->throwError(tiglGetErrorString(ret));
    }
    else {
        QScriptValue Point3dCtor = engine()->globalObject().property("Point3d");
        return Point3dCtor.construct(QScriptValueList() << px << py << pz);
    }
}
