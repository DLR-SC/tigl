/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerDocument.h 226 2012-10-23 19:18:29Z martinsiggel $
*
* Version: $Revision: 226 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef TIGLVIEWERDOCUMENT_H
#define TIGLVIEWERDOCUMENT_H

#include <QObject>
#include <QMetaType>
#include "TIGLViewer.h"
#include "CCPACSConfiguration.h"

#include <Quantity_Color.hxx>

class TIGLViewerWindow;

class TIGLViewerDocument : public QObject
{
    Q_OBJECT

    #define CPACS_XPATH_AIRCRAFT             "/cpacs/vehicles/aircraft"
    #define CPACS_XPATH_ROTORCRAFT           "/cpacs/vehicles/rotorcraft"
    #define CPACS_XPATH_AIRCRAFT_MODEL       "/cpacs/vehicles/aircraft/model"
    #define CPACS_XPATH_ROTORCRAFT_MODEL     "/cpacs/vehicles/rotorcraft/model"

public:

    TIGLViewerDocument(TIGLViewerWindow *parentWidget);
    ~TIGLViewerDocument( ) OVERRIDE;

    TiglReturnCode openCpacsConfiguration(const QString fileName);
    void closeCpacsConfiguration();
    TiglCPACSConfigurationHandle getCpacsHandle() const;

    // Returns the CPACS configuration
    tigl::CCPACSConfiguration& GetConfiguration() const;

signals:
    void documentUpdated(TiglCPACSConfigurationHandle);

public slots:
    // Aircraft slots
    void drawConfiguration();
    void drawAllFuselagesAndWingsSurfacePoints();
    void drawFusedAircraft();
    void drawFusedAircraftTriangulation();
    void drawIntersectionLine();
    void drawFarField();
    void drawSystems();
    void drawComponent();
    void drawComponentByUID(const QString& uid);

    // Wing slots
    void drawWingProfiles();
    void drawWing();
    void drawWingOverlayProfilePoints();
    void drawWingGuideCurves();
    void drawWingTriangulation();
    void drawWingSamplePoints();
    void drawFusedWing();
    void drawWingComponentSegment();
    void drawWingComponentSegmentPoints();
    void drawWingShells();
    void drawWingStructure();

    // Fuselage slots
    void drawFuselageProfiles();
    void drawFuselage();
    void drawFuselageTriangulation();
    void drawFuselageSamplePoints();
    void drawFuselageSamplePointsAngle();
    void drawFusedFuselage();
    void drawFuselageGuideCurves();

    // Rotor blade slots
    void drawRotorProfiles();
    void drawRotorBlade();
    void drawRotorBladeOverlayProfilePoints();
    void drawRotorBladeGuideCurves();
    void drawRotorBladeTriangulation();
    void drawRotorBladeSamplePoints();
    void drawFusedRotorBlade();
    void drawRotorBladeComponentSegment();
    void drawRotorBladeComponentSegmentPoints();
    void drawRotorBladeShells();

    // Rotorcraft slots
    void drawRotorByUID(const QString& uid);
    void drawRotor();
    void drawRotorDisk();
    void showRotorProperties();

    // TIGL slots
    void exportAsIges();
    void exportFusedAsIges();
    void exportAsStep();
    void exportAsStepFused();
    void exportMeshedWingSTL();
    void exportMeshedFuselageSTL();
    void exportMeshedConfigSTL();
    void exportMeshedWingVTK();
    void exportMeshedWingVTKsimple();
    void exportMeshedFuselageVTK();
    void exportMeshedConfigVTK();
    void exportMeshedConfigVTKNoFuse();
    void exportWingCollada();
    void exportFuselageCollada();
    void exportConfigCollada();
    void exportWingBRep();
    void exportFuselageBRep();
    void exportWingCurvesBRep();
    void exportFuselageCurvesBRep();
    void exportFusedConfigBRep();


    // General slots
    void updateConfiguration();


private slots:

    // Wing selection dialogs
    QString dlgGetWingOrRotorBladeSelection();
    QString dlgGetWingSelection();
    QString dlgGetWingComponentSegmentSelection();
    QString dlgGetWingSegmentSelection();
    QString dlgGetWingProfileSelection();

    // Rotor selection dialogs
    QString dlgGetRotorSelection();

    // Rotor Blade selection dialogs
    QString dlgGetRotorBladeSelection();
    QString dlgGetRotorBladeComponentSegmentSelection();
    QString dlgGetRotorBladeSegmentSelection();
    QString dlgGetRotorProfileSelection();

    // Fuselage selection dialogs
    QString dlgGetFuselageSelection();
    QString dlgGetFuselageSegmentSelection();
    QString dlgGetFuselageProfileSelection();

private: 
    TiglCPACSConfigurationHandle            m_cpacsHandle;
    TIGLViewerWindow*                       app;
    QString                                 loadedConfigurationFileName;

    void writeToStatusBar(QString text);
    void displayError(QString text, QString header="");
    QString myLastFolder; // TODO: synchronize with TIGLViewerWindow
    char* qstringToCstring(QString text);

    void drawAirfoil(tigl::CCPACSWingProfile& profile);
    void drawWingOverlayProfilePoints(tigl::CCPACSWing& wing);
    void drawWingGuideCurves(tigl::CCPACSWing& wing);
    void drawWing(tigl::CCPACSWing& wing);
    void drawWingTriangulation(tigl::CCPACSWing& wing);
    void drawWingSamplePoints(tigl::CCPACSWing& wing);
    void drawFusedWing(tigl::CCPACSWing& wing);
    void drawWingComponentSegment(tigl::CCPACSWingComponentSegment& segment);
    void drawWingComponentSegmentPoint(const std::string& csUID, const double& eta, const double& xsi);
    void drawWingShells(tigl::CCPACSWing& wing);

    void createShapeTriangulation(const class TopoDS_Shape& shape, class TopoDS_Compound& compound);
    
};

Q_DECLARE_METATYPE(TIGLViewerDocument*)

#endif // TIGLVIEWERDOCUMENT_H
