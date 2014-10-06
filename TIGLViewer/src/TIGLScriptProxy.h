/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
#ifndef TIGLSCRIPTPOXY_H
#define TIGLSCRIPTPOXY_H

#include <QtCore/QString>
#include <QtCore/QObject>

#include <QtScript>
#include <QString>

#include "tigl.h"
#include "tixi.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "TIGLViewerWindow.h"

class TIGLScriptProxy :public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QScriptValue wingCount READ getWingCount() )
    Q_PROPERTY(QScriptValue fuselageCount READ getFuselageCount() )
    Q_PROPERTY(QString version READ getVersion() )
    
private:
    // Returns the CPACS configuration
    tigl::CCPACSConfiguration& GetConfiguration(void);
    TiglCPACSConfigurationHandle getTiglHandle(void);
    char* qString2char(QString str);
    
public:
    TIGLScriptProxy(TIGLViewerWindow* app);
    //~TIGLScriptProxy();
    
public slots:
    QStringList getMemberFunctions();
    
    // wrapped tigl functions
    QScriptValue getWingCount();
    QString getVersion();
    int  componentGetHashCode (QString componentUID);
    int  componentIntersectionLineCount (QString componentUidOne, QString componentUidTwo);
    void exportFusedWingFuselageIGES (QString filenamePtr);
    void exportIGES (QString filenamePtr);
    void exportMeshedFuselageSTL(int fuselageIndex, QString filenamePtr, double deflection);
    void exportMeshedFuselageVTKByIndex (int fuselageIndex, QString filenamePtr, double deflection);
    void exportMeshedFuselageVTKByUID (QString fuselageUID, QString filenamePtr, double deflection);
    double  fuselageGetCircumference (int fuselageIndex, int segmentIndex, double eta);
    QScriptValue fuselageGetPoint (int fuselageIndex, int segmentIndex, double eta, double zeta);
    QString fuselageGetSegmentUID (int fuselageIndex, int segmentIndex);
    double  fuselageGetSegmentVolume (int fuselageIndex, int segmentIndex);
    QScriptValue getFuselageCount();
    
private:
    QString m_fileName;
    QStringList memberFunctions;
    TIGLViewerWindow* _app;

};

QScriptValue TIGLScriptProxyConstructor(QScriptContext *, QScriptEngine *);

#endif // TIGLSCRIPTPOXY_H

