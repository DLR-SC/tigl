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

#include <QtCore/QObject>
#include "TIGLViewer.h"
#include "TIGLViewerColors.h"
#include "CCPACSConfiguration.h"

#include <Handle_AIS_Shape.hxx>
#include <Quantity_Color.hxx>

class QOCC_DECLSPEC TIGLViewerDocument : public QObject
{
    Q_OBJECT

    #define CPACS_XPATH_AIRCRAFT             "/cpacs/vehicles/aircraft"
    #define CPACS_XPATH_ROTORCRAFT             "/cpacs/vehicles/rotorcraft"
    #define CPACS_XPATH_AIRCRAFT_MODEL         "/cpacs/vehicles/aircraft/model"
    #define CPACS_XPATH_ROTORCRAFT_MODEL     "/cpacs/vehicles/rotorcraft/model"

public:

    TIGLViewerDocument( QWidget *parentWidget, const Handle_AIS_InteractiveContext& ic );
    ~TIGLViewerDocument( );

    TiglReturnCode openCpacsConfiguration(const QString fileName);
    void closeCpacsConfiguration();
    TiglCPACSConfigurationHandle getCpacsHandle(void) const;

    // a small helper when we just want to display a shape
    Handle(AIS_Shape) displayShape(const TopoDS_Shape& shape, Quantity_Color col = Quantity_NOC_ShapeCol);

    // Draws a point
    void DisplayPoint(gp_Pnt& aPoint,
                      const char* aText,
                      Standard_Boolean UpdateViewer,
                      Standard_Real anXoffset,
                      Standard_Real anYoffset,
                      Standard_Real aZoffset,
                      Standard_Real TextScale);

    // Returns the CPACS configuration
    tigl::CCPACSConfiguration& GetConfiguration(void) const;

    void drawWingFlapsForInteractiveUse(std::string selectedWing, std::map<std::string,double> flapStatus);
    void updateControlSurfacesInteractiveObjects(std::string selectedWing, std::map<std::string,double> flapStatus, std::string controlUID);

signals:
    void documentUpdated(TiglCPACSConfigurationHandle);

public slots:
    // Aircraft slots
    void drawAllFuselagesAndWings();
    void drawAllFuselagesAndWingsSurfacePoints();
    void drawFusedAircraft();
    void drawFusedAircraftTriangulation();
    void drawIntersectionLine();
    void drawFarField();

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
    void drawWingFlaps();

    // Fuselage slots
    void drawFuselageProfiles();
    void drawFuselage();
    void drawFuselageTriangulation();
    void drawFuselageSamplePoints();
    void drawFuselageSamplePointsAngle();
    void drawFusedFuselage();
    void drawFuselageGuideCurves();
    
    // Misc slots
    void drawPoint();
    void drawVector();

    // TIGL slots
    void exportAsIges();
    void exportFusedAsIges();
    void exportAsStep();
    void exportAsStepFused();
    void exportMeshedWingSTL();
    void exportMeshedFuselageSTL();
    void exportMeshedWingVTK();
    void exportMeshedWingVTKsimple();
    void exportMeshedFuselageVTK();
    void exportMeshedFuselageVTKsimple();
    void exportMeshedConfigVTK();
    void exportMeshedConfigVTKNoFuse();
    void exportWingCollada();
    void exportFuselageCollada();
    void exportWingBRep();
    void exportFuselageBRep();
    void exportWingCurvesBRep();
    void exportFuselageCurvesBRep();
    void exportFusedConfigBRep();


    // General slots
    void updateConfiguration();

private slots:

    // Wing selection dialogs
    QString dlgGetWingSelection();
    QString dlgGetWingComponentSegmentSelection();
    QString dlgGetWingSegmentSelection();
    QString dlgGetWingProfileSelection();


    // Fuselage selection dialogs
    QString dlgGetFuselageSelection();
    QString dlgGetFuselageSegmentSelection();
    QString dlgGetFuselageProfileSelection();


private:
    TiglCPACSConfigurationHandle                    m_cpacsHandle;
    QWidget*                                        parent;
    Handle_AIS_InteractiveContext                   myAISContext;
    class TIGLViewerWidget*                         myOCC;
    QString                                         loadedConfigurationFileName;
    std::map<std::string,Handle(AIS_Shape)>         flapsForInteractiveUse;

    void writeToStatusBar(QString text);
    void displayError(QString text, QString header="");
    QString myLastFolder; // TODO: synchronize with TIGLViewerWindow
    char* qstringToCstring(QString text);
    void createShapeTriangulation(const class TopoDS_Shape& shape, class TopoDS_Compound& compound);

};

#endif // TIGLVIEWERDOCUMENT_H
