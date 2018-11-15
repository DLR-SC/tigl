/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerDocument.cpp 226 2012-10-23 19:18:29Z martinsiggel $
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

#include "TIGLViewerDocument.h"
#include "TIGLViewerWindow.h"
#include "TIGLViewerScopedCommand.h"

// QT Stuff
#include <qnamespace.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>
#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>

#include <string.h>

// OpenCascade Stuff
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopoDS_Shell.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gce_MakeLin.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBndLib.hxx>

// TIGLViewer includes
#include "UniquePtr.h"
#include "CPACSProfileGeometry.h"
#include "TIGLViewerInternal.h"
#include "TIGLViewerVTKExportDialog.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSFarField.h"
#include "CCPACSExternalObject.h"
#include "TIGLViewerInputoutput.h"
#include "ISession_Point.h"
#include "ISession_Text.h"
#include "CTiglPoint.h"
#include "CTiglError.h"
#include "TIGLViewerSettings.h"
#include "CTiglIntersectionCalculation.h"
#include "TIGLViewerWindow.h"
#include "TIGLViewerEtaXsiDialog.h"
#include "TIGLViewerDrawVectorDialog.h"
#include "TIGLViewerFuseDialog.h"
#include "TIGLViewerShapeIntersectionDialog.h"
#include "CTiglExportVtk.h"
#include "tiglcommonfunctions.h"
#include "CTiglFusePlane.h"
#include "CTiglWingProfilePointList.h"
#include "CCPACSWingSegment.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSGenericSystem.h"
#include "tiglcommonfunctions.h"
#include "CTiglPoint.h"
#include "CTiglExportCollada.h"
#include "CCPACSWingCSStructure.h"
#include "CCPACSWingSparSegment.h"
#include "CCPACSWingRibsDefinition.h"
#include "CTiglAttachedRotorBlade.h"
#include "TIGLGeometryChoserDialog.h"
#include "CCPACSEnginePylon.h"

#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef _MSC_VER
#define strdup(str) _strdup((str))
#endif

#define START_COMMAND() TIGLViewerScopedCommand command(app->getConsole());Q_UNUSED(command);

double getAbsDeflection (const TopoDS_Shape& theShape, double relDeflection)
{
    double aDeflection = relDeflection;
    Bnd_Box aBndBox;
    BRepBndLib::Add (theShape, aBndBox, Standard_False);
    if (!aBndBox.IsVoid()) {
        Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
        aBndBox.Get (aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
        aDeflection *= max(max(aXmax-aXmin, aYmax-aYmin), aZmax-aZmin);
    }
    
    return aDeflection;
}

TIGLViewerDocument::TIGLViewerDocument(TIGLViewerWindow *parentWidget)
    : QObject(parentWidget)
{
    app = parentWidget;
    m_cpacsHandle = -1;
}

TIGLViewerDocument::~TIGLViewerDocument( )
{
    closeCpacsConfiguration();
    m_cpacsHandle = -1;
}

void TIGLViewerDocument::writeToStatusBar(QString text)
{
    app->statusBar()->showMessage(text);
    qApp->processEvents();
}

void TIGLViewerDocument::displayError(QString text, QString header)
{
    app->displayErrorMessage(text, header);
}


char* TIGLViewerDocument::qstringToCstring(QString text)
{
    return strdup((const char*)text.toLatin1());
}

TiglReturnCode TIGLViewerDocument::openCpacsConfiguration(const QString fileName)
{
    START_COMMAND();
    QStringList configurations;

    app->getScene()->getContext()->SetDisplayMode(AIS_Shaded,Standard_False);

    TixiDocumentHandle tixiHandle = -1;

    char *cfileName = strdup((const char*)fileName.toLatin1());
    ReturnCode tixiRet = tixiOpenDocument(cfileName, &tixiHandle);
    if (tixiRet != SUCCESS) {
        displayError(QString("Error in function <u>tixiOpenDocument</u> when opening <br>file <i>"+fileName+"</i>. Error code: %1").arg(tixiRet), "TIXI Error");
        return TIGL_XML_ERROR;
    }

    // read configuration names
    int countAircrafts = 0;
    int countRotorcrafts = 0;
    tixiRet = tixiGetNamedChildrenCount( tixiHandle, CPACS_XPATH_AIRCRAFT, "model", &countAircrafts );
    tixiRet = tixiGetNamedChildrenCount( tixiHandle, CPACS_XPATH_ROTORCRAFT, "model", &countRotorcrafts );
    for (int i = 0; i < countAircrafts; i++) {
        char *text;
        std::stringstream xpath;
        xpath << CPACS_XPATH_AIRCRAFT_MODEL << "[" << i+1 << "]";
        tixiRet = tixiGetTextAttribute( tixiHandle, xpath.str().c_str(), "uID", &text);
        if (tixiRet == SUCCESS) {
            configurations << text;
        }
        else {
            displayError(QString("Error: missing uID for aircraft model %1!").arg(i), "TIXI Error");
            return TIGL_OPEN_FAILED;
        }
    }    
    for (int i = 0; i < countRotorcrafts; i++) {
        char *text;
        std::stringstream xpath;
        xpath << CPACS_XPATH_ROTORCRAFT_MODEL << "[" << i+1 << "]";
        tixiRet = tixiGetTextAttribute(tixiHandle, xpath.str().c_str(), "uID", &text);
        if (tixiRet == SUCCESS) {
            configurations << text;
        }
        else {
            displayError(QString("Error: missing uID for rotorcraft model %1!").arg(i), "TIXI Error");
            return TIGL_OPEN_FAILED;
        }

    }

    // Get configuration from user and open with TIGL
    TiglReturnCode tiglRet = TIGL_UNINITIALIZED;
    if (countRotorcrafts + countAircrafts == 0) {
        // no configuration present
        loadedConfigurationFileName = fileName;
        return TIGL_UNINITIALIZED;
    }
    else if (countRotorcrafts + countAircrafts == 1) {
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &m_cpacsHandle);
    }
    else {
        bool ok;
        QString item = QInputDialog::getItem(app, tr("Select CPACS Configuration"),
                                             tr("Available Configurations:"), configurations, 0, false, &ok);
        if (ok && !item.isEmpty()) {
            tiglRet = tiglOpenCPACSConfiguration(tixiHandle, strdup((const char*)item.toLatin1()), &m_cpacsHandle);
        }
    }

    if (tiglRet != TIGL_SUCCESS) {
        tixiCloseDocument(tixiHandle);
        m_cpacsHandle = -1;
        displayError(QString("<u>tiglOpenCPACSConfiguration</u> returned %1").arg(tiglGetErrorString(tiglRet)), "Error while reading in CPACS configuration");
        return tiglRet;
    }
    drawConfiguration();
    loadedConfigurationFileName = fileName;
    return TIGL_SUCCESS;
}

void TIGLViewerDocument::closeCpacsConfiguration()
{
    if (m_cpacsHandle < 1) {
        return;
    }

    TixiDocumentHandle tixiHandle = -1;
    tiglGetCPACSTixiHandle(m_cpacsHandle, &tixiHandle);

    tiglCloseCPACSConfiguration(m_cpacsHandle);
    m_cpacsHandle = -1;

    if (tixiHandle > 0) {
        tixiCloseDocument(tixiHandle);
    }

    emit documentUpdated(m_cpacsHandle);
}


/**
 * Re-reads the CPACS configuration.
 */
void TIGLViewerDocument::updateConfiguration()
{
    // Right now, we just close the tigl session and open a new one
    if (!loadedConfigurationFileName.isEmpty()) {
        START_COMMAND();
        tiglCloseCPACSConfiguration(m_cpacsHandle);
        m_cpacsHandle = -1;
        app->getScene()->deleteAllObjects();
        openCpacsConfiguration(loadedConfigurationFileName);
        emit documentUpdated(m_cpacsHandle);
    }
}


// Returns the CPACS configuration
tigl::CCPACSConfiguration& TIGLViewerDocument::GetConfiguration() const
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    return manager.GetConfiguration(m_cpacsHandle);
}


// creates triangulation of shape
// returns true, of mesh was done
bool meshShape(const TopoDS_Shape& loft, double rel_deflection) 
{
    // Now we build the triangulation of the loft. To determine a reasonable
    // value for the deflection (see OpenCascade documentation), we build the
    // bounding box around the loft. The greatest dimension of the bounding
    // box is then used as a measure for the deflection.
    Bnd_Box boundingBox;
    BRepBndLib::Add(loft, boundingBox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    Standard_Real xdist = xmax - xmin;
    Standard_Real ydist = ymax - ymin;
    Standard_Real zdist = zmax - zmin;

    double dist = max(max(xdist, ydist), zdist);

    double deflection = max(rel_deflection, dist * rel_deflection);
    if (!BRepTools::Triangulation (loft, deflection)) {
        BRepTools::Clean(loft);
        BRepMesh_IncrementalMesh(loft, deflection);
        return true;
    }
    else {
        return false;
    }
}


// Wing and Rotor Blade Selection Dialog
QString TIGLViewerDocument::dlgGetWingOrRotorBladeSelection()
{
    QStringList wings;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        std::string name = wing.GetUID();
        if (name == "") {
            name = "Unknown wing";
        }
        wings << name.c_str();
    }

    QString choice = QInputDialog::getItem(app, tr("Select Wing or Rotor Blade"), tr("Available Wings:"), wings, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Wing selection Dialog
QString TIGLViewerDocument::dlgGetWingSelection()
{
    QStringList wings;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        if (!wing.IsRotorBlade()) {
        std::string name = wing.GetUID();
        if (name == "") {
            name = "Unknown wing";
        }
        wings << name.c_str();
    }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Wing"), tr("Available Wings:"), wings, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Wing Component Segment selection Dialog
QString TIGLViewerDocument::dlgGetWingComponentSegmentSelection()
{
    QStringList compSegs;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        if (wing.IsRotorBlade()) {
            continue;
        }

        for (int j = 1; j <= wing.GetComponentSegmentCount(); ++j) {
            tigl::CCPACSWingComponentSegment& segment = wing.GetComponentSegment(j);
            compSegs << segment.GetUID().c_str();
        }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Component Segment"), tr("Available Component Segments:"), compSegs, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}


// Wing Segment selection Dialog
QString TIGLViewerDocument::dlgGetWingSegmentSelection()
{
    QStringList segs;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        if (wing.IsRotorBlade()) {
            continue;
        }
        for (int j = 1; j <= wing.GetSegmentCount(); ++j) {
            tigl::CCPACSWingSegment& segment = wing.GetSegment(j);
            std::string name = segment.GetUID();
            if (name == "") {
                name = "Unknown segment";
            }
            segs << name.c_str();
        }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Segment"), tr("Available Segments:"), segs, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Wing profile Dialog
QString TIGLViewerDocument::dlgGetWingProfileSelection()
{
    QStringList wingProfiles;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    if (config.GetWingProfiles()) {
        std::vector<tigl::unique_ptr<tigl::generated::CPACSProfileGeometry> >& airfoils = config.GetWingProfiles()->GetWingAirfoils();
        for (int i = 0; i < airfoils.size(); i++) {
            tigl::generated::CPACSProfileGeometry* profile = airfoils.at(i).get();

            std::string profileUID = profile->GetUID();
            wingProfiles << profileUID.c_str();
        }
    }
    QString choice = QInputDialog::getItem(app, tr("Select Wing Profile"), tr("Available Wing Profiles:"), wingProfiles, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}


// Rotor selection Dialog
QString TIGLViewerDocument::dlgGetRotorSelection()
{
    QStringList rotors;
    bool ok;

    // Initialize rotorBlade list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int rotorCount = config.GetRotorCount();
    for (int i = 1; i <= rotorCount; i++) {
        tigl::CCPACSRotor& rotor = config.GetRotor(i);
        std::string name = rotor.GetUID();
        if (name == "") {
            name = "Unknown rotor";
        }
        rotors << name.c_str();
    }

    QString choice = QInputDialog::getItem(app, tr("Select Rotor"), tr("Available Rotors:"), rotors, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Rotor Blade selection Dialog
QString TIGLViewerDocument::dlgGetRotorBladeSelection()
{
    QStringList wings;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        if (wing.IsRotorBlade()) {
            std::string name = wing.GetUID();
            if (name == "") {
                name = "Unknown rotor blade";
            }
            wings << name.c_str();
        }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Rotor Blade"), tr("Available Rotor Blades:"), wings, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Rotor Blade Component Segment Selection Dialog
QString TIGLViewerDocument::dlgGetRotorBladeComponentSegmentSelection()
{
    QStringList compSegs;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        if (wing.IsRotorBlade()) {
            for (int j = 1; j <= wing.GetComponentSegmentCount(); ++j) {
                tigl::CCPACSWingComponentSegment& segment = wing.GetComponentSegment(j);
                compSegs << segment.GetUID().c_str();
            }
        }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Component Segment"), tr("Available Component Segments:"), compSegs, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Rotor Blade Segment Selection Dialog
QString TIGLViewerDocument::dlgGetRotorBladeSegmentSelection()
{
    QStringList segs;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        if (wing.IsRotorBlade()) {
            for (int j = 1; j <= wing.GetSegmentCount(); ++j) {
                tigl::CCPACSWingSegment& segment = wing.GetSegment(j);
                std::string name = segment.GetUID();
                if (name == "") {
                    name = "Unknown segment";
                }
                segs << name.c_str();
            }
        }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Segment"), tr("Available Segments:"), segs, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Rotor Profile Dialog
QString TIGLViewerDocument::dlgGetRotorProfileSelection()
{
    QStringList wingProfiles;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    if (config.GetRotorProfiles()) {
        std::vector<tigl::unique_ptr<tigl::generated::CPACSProfileGeometry> >& airfoils = config.GetRotorProfiles()->GetRotorAirfoils();
        for (int i = 0; i < airfoils.size(); i++) {
            tigl::generated::CPACSProfileGeometry* profile = airfoils.at(i).get();
            wingProfiles << profile->GetUID().c_str();
        }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Rotor Profile"), tr("Available Rotor Profiles:"), wingProfiles, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}


// Fuselage selection Dialog
QString TIGLViewerDocument::dlgGetFuselageSelection()
{
    QStringList fuselages;
    bool ok;

    // Initialize wing list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int fuselageCount = config.GetFuselageCount();
    for (int i = 1; i <= fuselageCount; i++) {
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(i);
        std::string name = fuselage.GetUID();
        if (name == "") {
            name = "Unknown fuselage";
        }
        fuselages << name.c_str();
    }
    QString choice = QInputDialog::getItem(app, tr("Select Fuselage"), tr("Available Fuselages:"), fuselages, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Fuselage Segment selection Dialog
QString TIGLViewerDocument::dlgGetFuselageSegmentSelection()
{
    QStringList segs;
    bool ok;

    // Initialize fuselage list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int fuselageCount = config.GetFuselageCount();
    for (int i = 1; i <= fuselageCount; i++) {
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(i);
        for (int j = 1; j <= fuselage.GetSegmentCount(); ++j) {
            tigl::CCPACSFuselageSegment& segment = fuselage.GetSegment(j);
            std::string name = segment.GetUID();
            if (name == "") {
                name = "Unknown segment";
            }
            segs << name.c_str();
        }
    }

    QString choice = QInputDialog::getItem(app, tr("Select Segment"), tr("Available Segments:"), segs, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

// Fuselage profile Dialog
QString TIGLViewerDocument::dlgGetFuselageProfileSelection()
{
    QStringList fuselageProfiles;
    bool ok;

    // Initialize fuselage list
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int profileCount = config.GetFuselageProfileCount();
    for (int i = 1; i <= profileCount; i++) {
        tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(i);
        std::string profileUID = profile.GetUID();
        std::string name     = profile.GetName();
        fuselageProfiles << profileUID.c_str();
    }

    QString choice = QInputDialog::getItem(app, tr("Select Fuselage Profile"), tr("Available Fuselage Profiles:"), fuselageProfiles, 0, false, &ok);
    if (ok) {
        return choice;
    }
    else {
        return "";
    }
}

void TIGLViewerDocument::drawConfiguration( )
{
    try {
        START_COMMAND();
        // Draw all wings
        for (int w = 1; w <= GetConfiguration().GetWingCount(); w++) {

            try {
                tigl::CCPACSWing& wing = GetConfiguration().GetWing(w);

                if (wing.IsRotorBlade()) {
                    continue;
                }
                
                app->getScene()->displayShape(wing.GetLoft(), false);

                if ( !(wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY)) {
                    app->getScene()->displayShape(wing.GetMirroredLoft(), false, Quantity_NOC_MirrShapeCol);
                }

                app->getScene()->updateViewer();
            }
            catch(tigl::CTiglError& err) {
                displayError(err.what());
            }
            
        }
    
        // Draw all fuselages
        for (int f = 1; f <= GetConfiguration().GetFuselageCount(); f++) {
            try {
                tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(f);

                app->getScene()->displayShape(fuselage.GetLoft(), false);

                if ( !(fuselage.GetSymmetryAxis() == TIGL_NO_SYMMETRY) ) {
                    app->getScene()->displayShape(fuselage.GetMirroredLoft(), false, Quantity_NOC_MirrShapeCol);
                }
                app->getScene()->updateViewer();
            }
            catch(tigl::CTiglError& err) {
                displayError(err.what());
            }
        }
        
        // Draw all external objects
        for (int eo = 1; eo <= GetConfiguration().GetExternalObjectCount(); eo++) {
            try {
                tigl::CCPACSExternalObject& obj = GetConfiguration().GetExternalObject(eo);

                app->getScene()->displayShape(obj.GetLoft(), true);

                if (obj.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
                    continue;
                }

                app->getScene()->displayShape(obj.GetMirroredLoft()->Shape(), true, Quantity_NOC_MirrShapeCol);
            }
            catch(tigl::CTiglError& err) {
                displayError(err.what());
            }
        }
        
        // Draw rotors
        for (int i=1; i <= GetConfiguration().GetRotorCount(); ++i) {
            try {
                tigl::CCPACSRotor& rotor = GetConfiguration().GetRotor(i);
                // Draw rotor
                app->getScene()->displayShape(rotor.GetLoft(), false, Quantity_NOC_RotorCol);
                // Draw rotor disk
                TopoDS_Shape rotorDisk = rotor.GetRotorDisk()->Shape();
                app->getScene()->displayShape(rotorDisk, false, Quantity_NOC_RotorCol, 0.9);

                app->getScene()->updateViewer();

                if (rotor.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
                    continue;
                }

                // Draw mirrored rotor
                app->getScene()->displayShape(rotor.GetMirroredLoft()->Shape(), false, Quantity_NOC_MirrRotorCol);
                // Draw mirrored rotor disk
                gp_Ax2 mirrorPlane;
                if (rotor.GetSymmetryAxis() == TIGL_X_Z_PLANE) {
                    mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,1.,0.));
                }
                else if (rotor.GetSymmetryAxis() == TIGL_X_Y_PLANE) {
                    mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,0.,1.));
                }
                else if (rotor.GetSymmetryAxis() == TIGL_Y_Z_PLANE) {
                    mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(1.,0.,0.));
                }
                gp_Trsf theTransformation;
                theTransformation.SetMirror(mirrorPlane);
                BRepBuilderAPI_Transform myBRepTransformation(rotorDisk, theTransformation);
                const TopoDS_Shape& mirrRotorDisk = myBRepTransformation.Shape();
                app->getScene()->displayShape(mirrRotorDisk, true, Quantity_NOC_MirrRotorCol, 0.9);
            }
            catch(tigl::CTiglError& err) {
                displayError(err.what());
            }
        }

        // draw pylons
        boost::optional<tigl::CCPACSEnginePylons>& pylons = GetConfiguration().GetEnginePylons();
        int nPylons = pylons ? pylons->GetEnginePylons().size() : 0;
        for (int i=1; i <= nPylons; ++i) {
            try {
                tigl::CCPACSEnginePylon& obj = pylons->GetEnginePylon(i);
                app->getScene()->displayShape(obj.GetLoft(), true);

                if (obj.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
                    continue;
                }

                app->getScene()->displayShape(obj.GetMirroredLoft()->Shape(), true, Quantity_NOC_MirrShapeCol);

            }
            catch(tigl::CTiglError& err) {
                displayError(err.what());
            }
        }
    }
    catch(tigl::CTiglError& err) {
        displayError(err.what());
    }
}



void TIGLViewerDocument::drawWingProfiles()
{
    QString wingProfile = dlgGetWingProfileSelection();
    try {
        tigl::CCPACSWingProfile& profile = GetConfiguration().GetWingProfile(wingProfile.toStdString());
        drawAirfoil(profile);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}


void TIGLViewerDocument::drawWingOverlayProfilePoints()
{
    QString wingUid = dlgGetWingSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingOverlayProfilePoints(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}


void TIGLViewerDocument::drawWingGuideCurves()
{
    QString wingUid = dlgGetWingSelection();
    if (wingUid == "") {
        return;
    }

    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingGuideCurves(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what(), "Error");
    }
}


/*
 * Draw guide curves of the input wing
 */
void TIGLViewerDocument::drawWingGuideCurves(tigl::CCPACSWing& wing)
{
    START_COMMAND();

    TopoDS_Compound guideCurves = wing.GetGuideCurveWires();
    TopoDS_Iterator anIter(guideCurves);
    if (!anIter.More()) {
        displayError("There are no guide curves defined for this wing.", "Cannot compute guide curves.");
        return;
    }

    for (; anIter.More(); anIter.Next()) {
        TopoDS_Shape wire = anIter.Value();
        app->getScene()->displayShape(wire, Standard_False, Quantity_NOC_RED);
    }

    //display guide curve points (TODO: these points are not sorted and
    // there are probably duplicates)
    std::vector<gp_Pnt> points = wing.GetGuideCurvePoints();
    for (size_t i=0; i < points.size(); i++) {
        app->getScene()->displayPoint(points[i], "", Standard_False, 0, 0, 0, 1.);
    }

    app->getScene()->getContext()->UpdateCurrentViewer();
}


void TIGLViewerDocument::drawFuselageProfiles()
{
    QString fuselageProfile = dlgGetFuselageProfileSelection();
    if (fuselageProfile == "") {
        return;
    }

    START_COMMAND();
    app->getScene()->deleteAllObjects();

    tigl::CCPACSFuselageProfile& profile = GetConfiguration().GetFuselageProfile(fuselageProfile.toStdString());
    TopoDS_Wire wire        = profile.GetWire();
    app->getScene()->displayShape(wire, true,  Quantity_NOC_WHITE);

    if (profile.GetPointList_choice1()) {
        const std::vector<tigl::CTiglPoint>& points = profile.GetPointList_choice1()->AsVector();
        if (points.size() < 15) {
            for (unsigned int i = 0; i < points.size(); ++i) {
                const tigl::CTiglPoint& p = points.at(i);
                std::stringstream str;
                str << i << ": (" << p.x << ", " << p.y << ", " << p.z << ")";
                gp_Pnt pnt = p.Get_gp_Pnt();
                app->getScene()->displayPoint(pnt, str.str().c_str(), Standard_False, 0., 0., 0., 6.);
            }
        }
        else {
            for (double zeta = 0.0; zeta <= 1.0; zeta += 0.1) {
                try {
                  gp_Pnt wirePoint = profile.GetPoint(zeta);
                  std::ostringstream text;
                  text << "PT(" << zeta << ")";
                  app->getScene()->displayPoint(wirePoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
                  text.str("");
                }
                catch (tigl::CTiglError& ex) {
                  displayError(ex.what());
                }
             }
         }
    }
}

void TIGLViewerDocument::drawFuselageGuideCurves()
{
    QString fuselageUid = dlgGetFuselageSelection();
    if (fuselageUid == "") {
        return;
    }
    
    START_COMMAND();
    const tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());
    const TopoDS_Compound& guideCurves = fuselage.GetGuideCurveWires();

    TopoDS_Iterator anIter(guideCurves);
    if (!anIter.More()) {
        displayError("There are no guide curves defined for this fuselage.", "Cannot compute guide curves.");
        return;
    }

    for (; anIter.More(); anIter.Next()) {
        TopoDS_Shape wire = anIter.Value();
        app->getScene()->displayShape(wire, Standard_False, Quantity_NOC_RED);
    }

    //display guide curve points (TODO: these points are not sorted and
    // there are probably duplicates)
    const std::vector<gp_Pnt>& points = fuselage.GetGuideCurvePoints();
    for (size_t i=0; i < points.size(); i++) {
        app->getScene()->displayPoint(points[i], "", Standard_False, 0, 0, 0, 1.);
    }

    app->getScene()->getContext()->UpdateCurrentViewer();
}

void TIGLViewerDocument::drawWing()
{
    QString wingUid = dlgGetWingSelection();
    try {
    tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWing(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}



void TIGLViewerDocument::drawFuselage()
{
    QString fuselageUid = dlgGetFuselageSelection();
    if (fuselageUid == "") {
        return;
    }

    START_COMMAND();
    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

    app->getScene()->deleteAllObjects();

    for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
        // Draw segment loft
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
        app->getScene()->displayShape(segment.GetLoft(), true);
    }
}

void TIGLViewerDocument::drawWingTriangulation()
{
    QString wingUid = dlgGetWingSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingTriangulation(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}


void TIGLViewerDocument::drawFuselageTriangulation()
{
    QString fuselageUid = dlgGetFuselageSelection();
    if (fuselageUid == "") {
        return;
    }

    START_COMMAND();
    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

    //clear screen
    app->getScene()->deleteAllObjects();
    const TopoDS_Shape& fusedWing = fuselage.GetLoft()->Shape();

    TopoDS_Compound triangulation;
    createShapeTriangulation(fusedWing, triangulation);

    app->getScene()->displayShape(triangulation, true);
}


void TIGLViewerDocument::drawWingSamplePoints()
{
    QString wingUid = dlgGetWingSelection();
    try {
    tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingSamplePoints(wing);
        }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}


void TIGLViewerDocument::drawFuselageSamplePoints()
{
    QString fuselageUid = dlgGetFuselageSelection();
    if (fuselageUid == "") {
        return;
    }

    START_COMMAND();
    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

    for (int segmentIndex = 1; segmentIndex <= fuselage.GetSegmentCount(); segmentIndex++) {

        // Draw some points on the fuselage segment
        for (double eta = 0.0; eta <= 1.0; eta += 0.25) {
            for (double zeta = 0.0; zeta <= 1.0; zeta += 0.1) {
                double x, y, z;
                tiglFuselageGetPoint(m_cpacsHandle,
                                     1,    // TODO: we need to implement that function to use UID instead of index!
                                     segmentIndex,
                                     eta,
                                     zeta,
                                     &x,
                                     &y,
                                     &z);

                app->getScene()->displayPoint(gp_Pnt(x,y,z), "", false, 0, 0, 0, 1.0);
            }
        }
    }
}



void TIGLViewerDocument::drawFuselageSamplePointsAngle()
{
    // ask user defined angle
    bool ok = false;
    double angle = QInputDialog::getDouble(NULL, tr("Choose angle"), tr("Angle [degree]:"), 45., -360., 360., 1, &ok);
    if (!ok) {
        return;
    }

    START_COMMAND();
    int fuselageIndex = 1;
    double x, y, z;

    app->getScene()->deleteAllObjects();
    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageIndex);

    // Draw the fuselage
    for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);

        app->getScene()->displayShape(segment.GetLoft(), true);

        // Display the intersection point
        tiglFuselageGetPointAngle(m_cpacsHandle,
                        fuselageIndex, i,
                        0.5, angle,
                        &x, &y, &z);

        app->getScene()->displayPoint(gp_Pnt(x,y,z),"",Standard_False, 0., 0., 0., 1.);
    }

}


void TIGLViewerDocument::drawAllFuselagesAndWingsSurfacePoints()
{
    START_COMMAND();
    app->getScene()->deleteAllObjects();

    // Draw all wings
    for (int wingIndex = 1; wingIndex <= GetConfiguration().GetWingCount(); wingIndex++) {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingIndex);
        if (wing.IsRotorBlade()) {
            continue;
        }

        app->getScene()->displayShape(wing.GetLoft(), true);

        for (int segmentIndex = 1; segmentIndex <= wing.GetSegmentCount(); segmentIndex++) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

            for (double eta = 0.0; eta <= 1.0; eta += 0.1) {
                for (double xsi = 0.0; xsi <= 1.0; xsi += 0.1) {
                    double x, y, z;
                     tiglWingGetUpperPoint( m_cpacsHandle,
                                            wingIndex,
                                            segmentIndex,
                                            eta,
                                            xsi,
                                            &x,
                                            &y,
                                            &z);
                    
                    app->getScene()->displayPoint(gp_Pnt(x,y,z), "",Standard_False, 0,0,0,1.);

                    tiglWingGetLowerPoint( m_cpacsHandle,
                                           wingIndex,
                                           segmentIndex,
                                           eta,
                                           xsi,
                                           &x,
                                           &y,
                                           &z);

                    app->getScene()->displayPoint(gp_Pnt(x,y,z), "",Standard_False, 0,0,0,1.);
                }
            }
        }
    }
    app->getScene()->updateViewer();

    // Draw all fuselages
    for (int fuselageIndex = 1; fuselageIndex <= GetConfiguration().GetFuselageCount(); fuselageIndex++) {
        tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageIndex);

        app->getScene()->displayShape(fuselage.GetLoft(), true);

        for (int segmentIndex = 1; segmentIndex <= fuselage.GetSegmentCount(); segmentIndex++) {
            // Draw some points on the fuselage segment
            for (double eta = 0.0; eta <= 1.0; eta += 0.25) {
                for (double zeta = 0.0; zeta <= 1.0; zeta += 0.1) {
                    double x, y, z;
                    tiglFuselageGetPoint( m_cpacsHandle,
                                          fuselageIndex,
                                          segmentIndex,
                                          eta,
                                          zeta,
                                          &x,
                                          &y,
                                          &z);

                    app->getScene()->displayPoint(gp_Pnt(x,y,z), "",Standard_False, 0,0,0,1.);
                }
            }
        }
    }
    app->getScene()->updateViewer();
}


// -----------------------
// Export Functions
// -----------------------
void TIGLViewerDocument::exportAsIges()
{
    QString     fileName;
    QString        fileType;
    QFileInfo    fileInfo;

    TIGLViewerInputOutput writer;

    writeToStatusBar(tr("Saving as IGES file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export IGES(*.igs)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportIGES(m_cpacsHandle, qstringToCstring(fileName));
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportIGES</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}


void TIGLViewerDocument::exportFusedAsIges()
{
    QString     fileName;
    QString        fileType;
    QFileInfo    fileInfo;

    TIGLViewerInputOutput writer;

    writeToStatusBar(tr("Saving fused model as IGES file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export IGES(*.igs)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportFusedWingFuselageIGES(m_cpacsHandle, qstringToCstring(fileName));
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportFusedWingFuselageIGES</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}

void TIGLViewerDocument::exportAsStep()
{
    QString     fileName;
    QString     fileType;
    QFileInfo   fileInfo;

    TIGLViewerInputOutput writer;

    writeToStatusBar(tr("Saving as STEP file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export STEP(*.stp *.step)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportSTEP(m_cpacsHandle, qstringToCstring(fileName));
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportSTEP</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}

void TIGLViewerDocument::exportAsStepFused()
{
    QString     fileName;

    writeToStatusBar(tr("Saving as STEP file with TIGL. This can take a while. Please wait..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export STEP(*.stp *.step)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportFusedSTEP(m_cpacsHandle, qstringToCstring(fileName));
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportFusedSTEP</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}


void TIGLViewerDocument::exportMeshedWingSTL()
{
    QString     fileName;

    QString wingUid = dlgGetWingOrRotorBladeSelection();
    if (wingUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving meshed Wing as STL file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(qstringToCstring(wingUid));
        double deflection = wing.GetWingspan()/2. * TIGLViewerSettings::Instance().triangulationAccuracy();
        TiglReturnCode err = tiglExportMeshedWingSTLByUID(m_cpacsHandle, qstringToCstring(wingUid), qstringToCstring(fileName), deflection);
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportMeshedWingSTLByUID</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}



void TIGLViewerDocument::exportMeshedFuselageSTL()
{
    QString     fileName;

    QString fuselageUid = dlgGetFuselageSelection(); 
    if (fuselageUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving meshed Fuselage as STL file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportMeshedFuselageSTLByUID(m_cpacsHandle, qstringToCstring(fuselageUid), qstringToCstring(fileName), TIGLViewerSettings::Instance().triangulationAccuracy());
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportMeshedFuselageSTLByUID</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}

void TIGLViewerDocument::exportMeshedConfigSTL()
{
    QString     fileName;


    writeToStatusBar(tr("Saving meshed Configuration as STL file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportMeshedGeometrySTL(m_cpacsHandle, qstringToCstring(fileName), TIGLViewerSettings::Instance().triangulationAccuracy());
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportMeshedGeometrySTL</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}


void TIGLViewerDocument::exportMeshedWingVTK()
{
    QString     fileName;

    QString wingUid = dlgGetWingOrRotorBladeSelection();
    if (wingUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving meshed Wing as VTK file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

    if (fileName.isEmpty()) {
        return;
    }

    double deflection = 1.0;
    if (1) {
        START_COMMAND();
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(qstringToCstring(wingUid));
        deflection = wing.GetWingspan()/2. * TIGLViewerSettings::Instance().triangulationAccuracy();
    }

    TIGLViewerVTKExportDialog settings(app);
    settings.setDeflection(deflection);
    settings.exec();

    if (1) {
        START_COMMAND();
        TiglReturnCode err = tiglExportMeshedWingVTKByUID(m_cpacsHandle, wingUid.toStdString().c_str(), qstringToCstring(fileName), settings.getDeflection());
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportMeshedWingVTKByUID</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}


void TIGLViewerDocument::exportMeshedWingVTKsimple()
{
    QString     fileName;
    QString        fileType;
    QFileInfo    fileInfo;
    TIGLViewerInputOutput writer;

    QString wingUid = dlgGetWingOrRotorBladeSelection();
    if (wingUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving meshed Wing as simple VTK file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

    if (fileName.isEmpty()) {
        return;
    }

    double deflection = 1.0;
    if (1) {
        START_COMMAND();
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(qstringToCstring(wingUid));
        deflection = wing.GetWingspan()/2. * TIGLViewerSettings::Instance().triangulationAccuracy();
    }

    TIGLViewerVTKExportDialog settings(app);
    settings.setDeflection(deflection);
    settings.exec();

    if (1) {
        START_COMMAND();
        TiglReturnCode err = tiglExportMeshedWingVTKSimpleByUID(m_cpacsHandle, qstringToCstring(wingUid), qstringToCstring(fileName), deflection);
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportMeshedWingVTKSimpleByUID</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}


void TIGLViewerDocument::exportWingCollada()
{
    QString     fileName;

    QString wingUid = dlgGetWingOrRotorBladeSelection();
    if (wingUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving meshed Wing as Collada file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export Collada(*.dae)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(qstringToCstring(wingUid));
        double deflection = wing.GetWingspan()/2. * TIGLViewerSettings::Instance().triangulationAccuracy();

        TiglReturnCode err = tiglExportWingColladaByUID(m_cpacsHandle, wingUid.toStdString().c_str(), qstringToCstring(fileName), deflection);
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportWingColladaByUID</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}

void TIGLViewerDocument::exportFuselageCollada()
{
    QString     fileName;

    QString fuselageUid = dlgGetFuselageSelection();

    writeToStatusBar(tr("Saving meshed Fuselage as Collada file with TIGL..."));

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export Collada(*.dae)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        double deflection = GetConfiguration().GetAirplaneLenth() 
                * TIGLViewerSettings::Instance().triangulationAccuracy();
        TiglReturnCode err = tiglExportFuselageColladaByUID(m_cpacsHandle, qstringToCstring(fuselageUid), qstringToCstring(fileName), deflection);
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportFuselageColladaByUID</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}

void TIGLViewerDocument::exportConfigCollada()
{
    QString fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export Collada(*.dae)"));
    if (!fileName.isEmpty()) {
        START_COMMAND();
        TIGLViewerSettings& settings = TIGLViewerSettings::Instance();
        double relDeflect = settings.triangulationAccuracy();
        tigl::TriangulatedExportOptions options(relDeflect);

        tigl::CTiglExportCollada exporter;
        tigl::CCPACSConfiguration& config = GetConfiguration();
        exporter.AddConfiguration(config, options);

        writeToStatusBar(tr("Meshing and writing COLLADA file ") + fileName + ".");
        bool okay = exporter.Write(fileName.toStdString());
        writeToStatusBar("");
        if (!okay) {
            displayError(QString("Error while exporting to COLLADA."), "TIGL Error");
        }
    }
}


void TIGLViewerDocument::exportMeshedFuselageVTK()
{
    QString     fileName;

    QString wingUid = dlgGetFuselageSelection();
    if (wingUid == "") {
        return;
    }

    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

    if (fileName.isEmpty()) {
        return;
    }

    double deflection = 1.0;
    if (1) {
        START_COMMAND();
        deflection = GetConfiguration().GetAirplaneLenth()
                * TIGLViewerSettings::Instance().triangulationAccuracy();
    }

    TIGLViewerVTKExportDialog settings(app);
    settings.setDeflection(deflection);

    if (settings.exec()) {
        writeToStatusBar(tr("Saving meshed Fuselage as VTK file with TIGL..."));
        START_COMMAND();
        TiglReturnCode err = tiglExportMeshedFuselageVTKByUID(m_cpacsHandle, wingUid.toStdString().c_str(), qstringToCstring(fileName), settings.getDeflection());
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportMeshedFuselageVTKByUID</u>. Error code: %1").arg(err), "TIGL Error");
        }
    }
}


void TIGLViewerDocument::exportMeshedConfigVTK()
{
    QString     fileName;
    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

    if (fileName.isEmpty()) {
        return;
    }

    double deflection = 1.0;
    if (1) {
        START_COMMAND();
        deflection = GetConfiguration().GetAirplaneLenth()
                        * TIGLViewerSettings::Instance().triangulationAccuracy();
    }

    TIGLViewerVTKExportDialog settings(app);
    settings.setDeflection(deflection);

    if (settings.exec()) {
        START_COMMAND();
        writeToStatusBar("Calculating fused airplane, this can take a while");

        tiglExportMeshedGeometryVTK(m_cpacsHandle, fileName.toStdString().c_str(), settings.getDeflection());

        writeToStatusBar("");
    }
}

void TIGLViewerDocument::exportMeshedConfigVTKNoFuse()
{
    QString     fileName;
    fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

    if (fileName.isEmpty()) {
        return;
    }

    double deflection = 1.0;
    if (1) {
        START_COMMAND();
        deflection = GetConfiguration().GetAirplaneLenth()
                        * TIGLViewerSettings::Instance().triangulationAccuracy();
    }

    TIGLViewerVTKExportDialog settings(app);
    settings.setDeflection(deflection);

    if (settings.exec()) {
        START_COMMAND();
        writeToStatusBar("Writing meshed vtk file");

        tigl::CTiglExportVtk exporter;
        exporter.AddConfiguration(GetConfiguration(), tigl::TriangulatedExportOptions(settings.getDeflection()));

        exporter.Write(fileName.toStdString());
        writeToStatusBar("");
    }
}


void TIGLViewerDocument::exportWingBRep() 
{
    QString wingUid = dlgGetWingOrRotorBladeSelection();
    if (wingUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving Wing as BRep file..."));

    QString fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export BRep(*.brep)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportWingBREPByUID(m_cpacsHandle, qstringToCstring(wingUid), qstringToCstring(fileName));
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportWingBREPByUID</u>. Error code: %1").arg(err), "TIGL Error");
            return;
        }
    }
}

void TIGLViewerDocument::exportFuselageBRep() 
{
    QString fuselageUid = dlgGetFuselageSelection();
    if (fuselageUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving Fuselage as BRep file..."));

    QString fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export BRep(*.brep)"));

    if (!fileName.isEmpty()) {
        START_COMMAND();
        TiglReturnCode err = tiglExportFuselageBREPByUID(m_cpacsHandle, qstringToCstring(fuselageUid), qstringToCstring(fileName));
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>exportFuselageBRepByUID</u>. Error code: %1").arg(err), "TIGL Error");
            return;
        }
    }
}

void TIGLViewerDocument::exportFusedConfigBRep()
{
    QString fileName = QFileDialog::getSaveFileName(app, tr("Save as..."), myLastFolder, tr("Export BRep(*.brep)"));
    if (fileName.isEmpty()) {
        return;
    }

    START_COMMAND();
    try {
        TiglReturnCode err = tiglExportFusedBREP(m_cpacsHandle, qstringToCstring(fileName));
        if (err != TIGL_SUCCESS) {
            displayError(QString("Error in function <u>tiglExportBREP</u>. Error code: %1").arg(err), "TIGL Error");
            return;
        }
    }
    catch(tigl::CTiglError & error){
        displayError(error.what(), "Error in BRep export");
    }
    catch(...){
        displayError("Unknown Exception during computation of fused aircraft.", "Error in BRep export");
    }
}

void TIGLViewerDocument::exportWingCurvesBRep()
{
    QString wingUid = dlgGetWingOrRotorBladeSelection();
    if (wingUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving Wing Curves as BRep file..."));

    QString dirname = QFileDialog::getExistingDirectory(app, tr("Chose directory..."), myLastFolder,  
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dirname == "") {
        return;
    }
    
    START_COMMAND();
    tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
    QString baseName = QDir(std::string(dirname.toStdString() + "/" + wing.GetUID()).c_str()).absolutePath();
    
    TopoDS_Compound profiles;
    BRep_Builder builder;
    builder.MakeCompound(profiles);
    
    Handle(TopTools_HSequenceOfShape) allGuides = new TopTools_HSequenceOfShape;
    for (int isegment = 1; isegment <= wing.GetSegmentCount(); ++isegment) {
        // get profiles
        tigl::CCPACSWingSegment& segment = static_cast<tigl::CCPACSWingSegment&>(wing.GetSegment(isegment));
        builder.Add(profiles, segment.GetInnerWire());
        if (isegment == wing.GetSegmentCount()) {
            builder.Add(profiles, segment.GetOuterWire());
        }
    }
    
    // write profiles to brep
    std::string profFileName = baseName.toStdString() + "_profiles.brep";
    BRepTools::Write(profiles, profFileName.c_str());


    TopoDS_Compound guides = wing.GetGuideCurveWires();
    TopoDS_Iterator it(guides);
    if (it.More()) {
        // write to brep
        std::string guideFileName = baseName.toStdString() + "_guides.brep";
        BRepTools::Write(guides, guideFileName.c_str());
    }
}

void TIGLViewerDocument::exportFuselageCurvesBRep()
{
    QString fuselageUid = dlgGetFuselageSelection();
    if (fuselageUid == "") {
        return;
    }

    writeToStatusBar(tr("Saving Fuselage curves as BRep file..."));

    QString dirname = QFileDialog::getExistingDirectory(app, tr("Chose directory..."), myLastFolder,  
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dirname == "") {
        return;
    }
    
    START_COMMAND();
    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());
    QString baseName = QDir(std::string(dirname.toStdString() + "/" + fuselage.GetUID()).c_str()).absolutePath();
    
    TopoDS_Compound profiles;
    BRep_Builder builder;
    builder.MakeCompound(profiles);
    
    Handle(TopTools_HSequenceOfShape) allGuides = new TopTools_HSequenceOfShape;
    for (int isegment = 1; isegment <= fuselage.GetSegmentCount(); ++isegment) {
        // get profiles
        tigl::CCPACSFuselageSegment& segment = static_cast<tigl::CCPACSFuselageSegment&>(fuselage.GetSegment(isegment));
        builder.Add(profiles, segment.GetStartWire());
        if (isegment == fuselage.GetSegmentCount()) {
            builder.Add(profiles, segment.GetEndWire());
        }
    }
    
    // write profiles to brep
    std::string profFileName = baseName.toStdString() + "_profiles.brep";
    BRepTools::Write(profiles, profFileName.c_str());
    
    TopoDS_Compound guides = fuselage.GetGuideCurveWires();
    TopoDS_Iterator it(guides);
    if (it.More()) {
        // write to brep
        std::string guideFileName = baseName.toStdString() + "_guides.brep";
        BRepTools::Write(guides, guideFileName.c_str());
    }
}

void TIGLViewerDocument::drawFusedFuselage()
{
    QString fuselageUid = dlgGetFuselageSelection();
    if (fuselageUid == "") {
        return;
    }

    START_COMMAND();
    app->getScene()->deleteAllObjects();
    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());
    app->getScene()->displayShape(fuselage.GetLoft(), true);
}


void TIGLViewerDocument::drawFusedWing()
{
    QString wingUid = dlgGetWingSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawFusedWing(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}


void TIGLViewerDocument::drawFusedAircraft()
{
    FuseDialog dialog(app);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    tigl::TiglFuseResultMode mode = tigl::HALF_PLANE;
    // make option
    if (!dialog.TrimWithFarField()     && !dialog.UseSymmetries()) {
        mode = tigl::HALF_PLANE;
    }
    else if (!dialog.TrimWithFarField() && dialog.UseSymmetries()) {
        mode = tigl::FULL_PLANE;
    }
    else if (dialog.TrimWithFarField()  && !dialog.UseSymmetries()) {
        mode = tigl::HALF_PLANE_TRIMMED_FF;
    }
    else if (dialog.TrimWithFarField()  && dialog.UseSymmetries()) {
        mode = tigl::FULL_PLANE_TRIMMED_FF;
    }

    START_COMMAND();
    try {
        tigl::PTiglFusePlane fuser = GetConfiguration().AircraftFusingAlgo();
        fuser->SetResultMode(mode);
        PNamedShape airplane = fuser->FusedPlane();
        if (!airplane) {
            displayError("Error computing fused aircraft");
            return;
        }

        app->getScene()->deleteAllObjects();


        ListPNamedShape map = GroupFaces(airplane, tigl::NAMED_COMPOUNDS);
        ListPNamedShape::iterator it;
        int icol = 0;
        Quantity_NameOfColor colors[] = {Quantity_NOC_BLUE4,
                                         Quantity_NOC_RED,
                                         Quantity_NOC_GREEN,
                                         Quantity_NOC_MAGENTA1,
                                         Quantity_NOC_AZURE,
                                         Quantity_NOC_FIREBRICK};
        int ncolors = sizeof(colors)/sizeof(Quantity_NameOfColor);
        for (it = map.begin(); it != map.end(); ++it) {
            if (icol >= ncolors) {
                icol = 0;
            }
            PNamedShape shape = *it;
            if (shape) {
                app->getScene()->displayShape(shape, true, colors[icol++]);
            }
        }

        const ListPNamedShape& ints = fuser->Intersections();
        ListPNamedShape::const_iterator it2 = ints.begin();
        for (; it2 != ints.end(); ++it2) {
            if (*it2) {
                app->getScene()->displayShape((*it2), true, Quantity_NOC_WHITE);
            }
        }

        // get far field if available
        PNamedShape ff = fuser->FarField();
        if (ff) {
            Handle(AIS_Shape) shape = new AIS_Shape(ff->Shape());
            shape->SetMaterial(Graphic3d_NOM_PEWTER);
            shape->SetTransparency(0.6);
            app->getScene()->getContext()->Display(shape, Standard_True);
        }
    }
    catch(tigl::CTiglError & error){
        displayError(error.what());
    }
    catch(...){
        displayError("Unknown Exception");
    }
}

void TIGLViewerDocument::drawFusedAircraftTriangulation()
{
    START_COMMAND();
    TopoDS_Shape airplane = GetConfiguration().AircraftFusingAlgo()->FusedPlane()->Shape();
    app->getScene()->deleteAllObjects();
    TopoDS_Compound triangulation;
    createShapeTriangulation(airplane, triangulation);

    app->getScene()->displayShape(triangulation, true);
}


void TIGLViewerDocument::drawIntersectionLine()
{
    tigl::CCPACSConfiguration& config = GetConfiguration();
    tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

    TIGLViewerShapeIntersectionDialog dialog(uidManager, app);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    START_COMMAND();
    int mode = dialog.GetMode();
    tigl::CTiglIntersectionCalculation* Intersector = NULL;
    if (mode == 0) {
        // shape - shape
        std::string uid1 = dialog.GetShape1UID().toStdString();
        std::string uid2 = dialog.GetShape2UID().toStdString();
        writeToStatusBar(QString(tr("Calculating %1 ...")).arg(uid1.c_str()));
        const TopoDS_Shape& compoundOne = uidManager.GetGeometricComponent(uid1).GetLoft()->Shape();
        writeToStatusBar(QString(tr("Calculating %1 ...")).arg(uid2.c_str()));
        const TopoDS_Shape& compoundTwo = uidManager.GetGeometricComponent(uid2).GetLoft()->Shape();

        writeToStatusBar(tr("Calculating intersection... This may take a while!"));
        Intersector = new tigl::CTiglIntersectionCalculation(&config.GetShapeCache(),uid1, uid2, compoundOne, compoundTwo);
    }
    else if (mode == 1) {
        // shape - plane
        std::string uid = dialog.GetShapeUID().toStdString();
        writeToStatusBar(QString(tr("Calculating %1 ...")).arg(uid.c_str()));
        const TopoDS_Shape& compoundOne = uidManager.GetGeometricComponent(uid).GetLoft()->Shape();

        gp_Pnt p = dialog.GetPoint().Get_gp_Pnt();
        tigl::CTiglPoint normal = dialog.GetNormal();
        if (normal.norm2() < 1e-7) {
            displayError("The plane normal vector must not be zero.");
            return;
        }
        gp_Dir n = normal.Get_gp_Pnt().XYZ();

        writeToStatusBar(tr("Calculating intersection... This may take a while!"));
        Intersector = new tigl::CTiglIntersectionCalculation(&config.GetShapeCache(), uid, compoundOne, p, n);
    }
    else if (mode == 2) {
        // shape - plane segment
        std::string uid = dialog.GetShapeSUID().toStdString();
        writeToStatusBar(QString(tr("Calculating %1 ...")).arg(uid.c_str()));
        const TopoDS_Shape& compound = uidManager.GetGeometricComponent(uid).GetLoft()->Shape();

        gp_Pnt p1 = dialog.GetPoint1().Get_gp_Pnt();
        gp_Pnt p2 = dialog.GetPoint2().Get_gp_Pnt();
        tigl::CTiglPoint normal = dialog.GetW();
        if (normal.norm2() < 1e-7) {
            displayError("The directional vector must not be zero.");
            return;
        }
        gp_Dir n = normal.Get_gp_Pnt().XYZ();
        writeToStatusBar(tr("Calculating intersection... This may take a while!"));
        Intersector = new tigl::CTiglIntersectionCalculation(&config.GetShapeCache(), uid, compound, p1, p2, n,false);
    }
    else {
        return;
    }

    if (Intersector->GetCountIntersectionLines() <= 0) {
        displayError(tr("Could not find any intersection between shapes"), "TIGL Error");
        writeToStatusBar("");
        return;
    }
    // load first wire
    for (int wireID = 1; wireID <= Intersector->GetCountIntersectionLines(); ++wireID) {
        app->getScene()->displayShape(Intersector->GetWire(wireID), true);
        
        /* now calculate intersection and display single points */
        for (double eta = 0.0; eta <= 1.0; eta += 0.025) {
            gp_Pnt point = Intersector->GetPoint(eta, wireID);
            app->getScene()->displayPoint(point, "", Standard_False, 0,0,0,1);
        }
    }
    app->getScene()->updateViewer();
    writeToStatusBar("");

    delete Intersector;
}


void TIGLViewerDocument::drawWingComponentSegment()
{
    QString csUid = dlgGetWingComponentSegmentSelection();
    if (csUid == "") {
        return;
    }

    for (int i = 1; i <= GetConfiguration().GetWingCount();++i) {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(i);
        for (int j = 1; j <= wing.GetComponentSegmentCount();++j) {
            tigl::CCPACSWingComponentSegment& segment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(j));
            if (segment.GetUID() == csUid.toStdString()) {
                drawWingComponentSegment(segment);
                break;
            }
        }
    }
}

void TIGLViewerDocument::drawWingComponentSegmentPoints()
{
    QString csUid = dlgGetWingComponentSegmentSelection();
    if (csUid == "") {
        return;
    }
    
    START_COMMAND();
    double eta = 0.5, xsi = 0.5;

    while (EtaXsiDialog::getEtaXsi(app, eta, xsi) == QDialog::Accepted) {
        drawWingComponentSegmentPoint(csUid.toStdString(), eta, xsi);
    }
}

void TIGLViewerDocument::drawWingShells()
{
    QString wingUid = dlgGetWingSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingShells(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}

void TIGLViewerDocument::drawWingStructure()
{
    QString csUid = dlgGetWingComponentSegmentSelection();
    if (csUid == "") {
        return;
    }

    // find component segment first
    tigl::CCPACSWingComponentSegment* cs = NULL;
    for (int i = 1; i <= GetConfiguration().GetWingCount();++i) {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(i);
        for (int j = 1; j <= wing.GetComponentSegmentCount();++j) {
            tigl::CCPACSWingComponentSegment& segment = wing.GetComponentSegment(j);
            if (segment.GetUID() == csUid.toStdString()) {
                cs = static_cast<tigl::CCPACSWingComponentSegment*>(&segment);
                break;
            }
        }
    }

    if (!cs || !cs->GetStructure()) {
        displayError("This wing has no structure defined.", "Information");
        return;
    }

    try {

        START_COMMAND();

        tigl::CCPACSWingComponentSegment& cs = GetConfiguration().GetUIDManager()
                .ResolveObject<tigl::CCPACSWingComponentSegment>(csUid.toStdString());

        if (!cs.GetStructure()) {
            displayError("This wing has no structure defined.", "Information");
            return;
        }

        app->getScene()->deleteAllObjects();

        // display component segment shape with transparency
        app->getScene()->displayShape(cs.GetLoft(), true, Quantity_NOC_ShapeCol, 0.5);

        const tigl::CCPACSWingCSStructure& structure = *cs.GetStructure();

        // draw spars
        for (int ispar = 1; ispar <= structure.GetSparSegmentCount(); ++ispar) {
            const tigl::CCPACSWingSparSegment& spar = structure.GetSparSegment(ispar);
            TopoDS_Shape sparGeom = spar.GetSparGeometry();
            app->getScene()->displayShape(sparGeom, true, Quantity_NOC_RED);
        }

        // draw ribs
        for (int irib = 1; irib <=structure.GetRibsDefinitionCount(); ++irib) {
            const tigl::CCPACSWingRibsDefinition& rib = structure.GetRibsDefinition(irib);
            TopoDS_Shape ribGeom = rib.GetRibsGeometry();
            app->getScene()->displayShape(ribGeom, true, Quantity_NOC_RED);
        }

    }
    catch(tigl::CTiglError& err) {
        displayError(QString("Error while computing the wing structure: \"%1\"").arg(err.what()));
    }
    catch(Standard_ConstructionError& err) {
        displayError(QString("Error while computing the wing structure: \"%1\"").arg(err.GetMessageString()));
    }
    catch(...) {
        displayError("Error while computing the wing structure.");
    }

}

void TIGLViewerDocument::drawFarField()
{
    START_COMMAND();
    tigl::CCPACSFarField& farField = GetConfiguration().GetFarField();
    if (farField.GetType() != tigl::NONE) {
        Handle(AIS_Shape) shape = new AIS_Shape(farField.GetLoft()->Shape());
        shape->SetMaterial(Graphic3d_NOM_PEWTER);
        shape->SetTransparency(0.6);
        app->getScene()->getContext()->Display(shape, Standard_True);
        app->getViewer()->fitAll();
    }
}


void TIGLViewerDocument::drawSystems()
{
    START_COMMAND();
    // Draw all generic systems
    for (int gs = 1; gs <= GetConfiguration().GetGenericSystemCount(); gs++) {
        tigl::CCPACSGenericSystem& genericSystem = GetConfiguration().GetGenericSystem(gs);
        app->getScene()->displayShape(genericSystem.GetLoft(), true );

        if (genericSystem.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
            continue;
        }

        app->getScene()->displayShape(genericSystem.GetMirroredLoft()->Shape(), true, Quantity_NOC_MirrShapeCol);
    }
}

void TIGLViewerDocument::drawComponent()
{
    tigl::CCPACSConfiguration& config = GetConfiguration();
    tigl::CTiglUIDManager& uidManager = config.GetUIDManager();

    TIGLGeometryChoserDialog dialog(GetConfiguration().GetUIDManager(), app);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    foreach (QString componentUID, dialog.GetSelectedUids()) {
        try {
            START_COMMAND();
            tigl::ITiglGeometricComponent& component = uidManager.GetGeometricComponent(componentUID.toStdString());
            app->getScene()->displayShape(component.GetLoft(), true);
        }
        catch(tigl::CTiglError& err) {
            displayError(err.what());
        }
        catch(...) {
            displayError("An unknown error occured. Sorry!");
        }
    }
}

void TIGLViewerDocument::drawRotorProfiles()
{
    QString wingProfile = dlgGetRotorProfileSelection();
    try {
        tigl::CCPACSWingProfile& profile = GetConfiguration().GetWingProfile(wingProfile.toStdString());
        drawAirfoil(profile);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}

void TIGLViewerDocument::drawRotorBladeOverlayProfilePoints()
{
    QString wingUid = dlgGetRotorBladeSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingOverlayProfilePoints(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}

void TIGLViewerDocument::drawRotorBladeGuideCurves()
{
    // loop over all wings
    tigl::CCPACSConfiguration& config = GetConfiguration();
    int wingCount = config.GetWingCount();
    for (int i = 1; i <= wingCount; i++) {
        tigl::CCPACSWing& wing = config.GetWing(i);
        if (wing.IsRotorBlade()) {
            drawWingGuideCurves(wing);
        }
    }
}

void TIGLViewerDocument::drawRotorBlade()
{
    QString wingUid = dlgGetRotorBladeSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWing(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}

void TIGLViewerDocument::drawRotorBladeTriangulation()
{
    QString wingUid = dlgGetRotorBladeSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingTriangulation(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}

void TIGLViewerDocument::drawRotorBladeSamplePoints()
{
    QString wingUid = dlgGetRotorBladeSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingSamplePoints(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}

void TIGLViewerDocument::drawFusedRotorBlade()
{
    QString wingUid = dlgGetRotorBladeSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawFusedWing(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}

void TIGLViewerDocument::drawRotorBladeComponentSegment()
{
    QString csUid = dlgGetRotorBladeComponentSegmentSelection();
    if (csUid == "") {
        return;
    }

    for (int i = 1; i <= GetConfiguration().GetWingCount();++i) {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(i);
        for (int j = 1; j <= wing.GetComponentSegmentCount();++j) {
            tigl::CCPACSWingComponentSegment& segment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(j));
            if (segment.GetUID() == csUid.toStdString()) {
                drawWingComponentSegment(segment);
                break;
            }
        }
    }
}

void TIGLViewerDocument::drawRotorBladeComponentSegmentPoints()
{
    QString csUid = dlgGetRotorBladeComponentSegmentSelection();
    if (csUid == "") {
        return;
    }
    
    double eta = 0.5, xsi = 0.5;

    while (EtaXsiDialog::getEtaXsi(app, eta, xsi) == QDialog::Accepted) {
        drawWingComponentSegmentPoint(csUid.toStdString(), eta, xsi);
    }
}

void TIGLViewerDocument::drawRotorBladeShells()
{
    QString wingUid = dlgGetRotorBladeSelection();
    try {
        tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
        drawWingShells(wing);
    }
    catch (tigl::CTiglError& ex) {
        displayError(ex.what());
    }
}


void TIGLViewerDocument::drawRotor()
{
    QString rotorUid = dlgGetRotorSelection();
    if (rotorUid=="") {
        return;
    }

    tigl::CCPACSRotor& rotor = GetConfiguration().GetRotor(rotorUid.toStdString());

    START_COMMAND();

    //clear screen
    app->getScene()->deleteAllObjects();

    // Draw segment loft
    app->getScene()->displayShape(rotor.GetLoft(), true, Quantity_NOC_RotorCol);
}

void TIGLViewerDocument::drawRotorDisk()
{
    QString rotorUid = dlgGetRotorSelection();
    if (rotorUid=="") {
        return;
    }

    tigl::CCPACSRotor& rotor = GetConfiguration().GetRotor(rotorUid.toStdString());

    START_COMMAND();

    //clear screen
    app->getScene()->deleteAllObjects();

    // Draw rotor disk
    TopoDS_Shape rotorDisk = rotor.GetRotorDisk()->Shape();
    app->getScene()->displayShape(rotorDisk, true, Quantity_NOC_RotorCol, 0.9);
}

void TIGLViewerDocument::showRotorProperties()
{
    QString rotorUid = dlgGetRotorSelection();
    if (rotorUid=="") {
        return;
    }

    tigl::CCPACSRotor& rotor = GetConfiguration().GetRotor(rotorUid.toStdString());

    QString propertiesText;
    QString valueText;
    tigl::CTiglPoint tmpPoint;

    {
        START_COMMAND();

#define ADD_PROPERTY_TEXT(propertyName, text) \
        try { \
           valueText = (text); \
        } \
        catch(...) { \
           valueText = "ERROR"; \
        } \
        propertiesText += QString("<b>%1:</b> %2<br/>").arg(propertyName).arg(valueText);

        ADD_PROPERTY_TEXT("UID", QString::fromStdString(rotor.GetUID()));
        ADD_PROPERTY_TEXT("Type", QString::number(rotor.GetDefaultedType()));
        ADD_PROPERTY_TEXT("Name", QString::fromStdString(rotor.GetName()));
        ADD_PROPERTY_TEXT("Description", QString::fromStdString(rotor.GetDescription().get_value_or("")));

        tmpPoint = rotor.GetTranslation();
        ADD_PROPERTY_TEXT("Translation", "(" + QString::number(tmpPoint.x) + "; " + QString::number(tmpPoint.y) + "; " + QString::number(tmpPoint.z) + ")")
        ADD_PROPERTY_TEXT("RPM", QString::number(rotor.GetNominalRotationsPerMinute().get_value_or(0)));
        ADD_PROPERTY_TEXT("Tip Speed", QString::number(rotor.GetTipSpeed()));
        ADD_PROPERTY_TEXT("RotorBladeAttachmentCount", QString::number(rotor.GetRotorBladeAttachmentCount()));
        ADD_PROPERTY_TEXT("RotorBladeCount", QString::number(rotor.GetRotorBladeCount()));
        ADD_PROPERTY_TEXT("Radius", QString::number(rotor.GetRadius()));
        ADD_PROPERTY_TEXT("Total blade planform area", QString::number(rotor.GetTotalBladePlanformArea()));
        ADD_PROPERTY_TEXT("Reference area", QString::number(rotor.GetReferenceArea()));
        ADD_PROPERTY_TEXT("Solidity", QString::number(rotor.GetSolidity()));
        ADD_PROPERTY_TEXT("Surface area", QString::number(rotor.GetSurfaceArea()));
        ADD_PROPERTY_TEXT("Volume", QString::number(rotor.GetVolume()));

        if (rotor.GetRotorBladeCount() > 0) {
            propertiesText += "<br/>";
            tigl::CTiglAttachedRotorBlade& blade = rotor.GetRotorBlade(1);

            ADD_PROPERTY_TEXT("Blade 1 Root Radius", QString::number(blade.GetLocalRadius(1, 0.)));
            ADD_PROPERTY_TEXT("Blade 1 Tip Radius",
                              QString::number(blade.GetLocalRadius(blade.GetUnattachedRotorBlade().GetSegmentCount(), 1.)));
            ADD_PROPERTY_TEXT("Blade 1 Root Chord", QString::number(blade.GetLocalChord(1, 0.)));
            ADD_PROPERTY_TEXT("Blade 1 Tip Chord",
                              QString::number(blade.GetLocalChord(blade.GetUnattachedRotorBlade().GetSegmentCount(), 1.)));

            ADD_PROPERTY_TEXT("Blade 1 Root Twist", QString::number(blade.GetLocalTwistAngle(1,0.)));
            ADD_PROPERTY_TEXT("Blade 1 Tip Twist",
                              QString::number(blade.GetLocalTwistAngle(blade.GetUnattachedRotorBlade().GetSegmentCount(),1.)));
        }
    }

    QMessageBox msgBox;
    msgBox.setText("Properties of rotor '" + rotorUid + "':");
    msgBox.setInformativeText(propertiesText);

    msgBox.exec();
}


/*
 * Draw the input airfoil and sample some points on it
 */
void TIGLViewerDocument::drawAirfoil(tigl::CCPACSWingProfile& profile)
{
    app->getScene()->deleteAllObjects();

    TopoDS_Wire wire        = profile.GetWire();
    app->getScene()->displayShape(wire, true, Quantity_NOC_WHITE);

    // Leading/trailing edges
    gp_Pnt lePoint = profile.GetLEPoint();
    gp_Pnt tePoint = profile.GetTEPoint();

    std::ostringstream text;
    text << "LE(" << lePoint.X() << ", " << lePoint.Y() << ", " << lePoint.Z() << ")";
    app->getScene()->displayPoint(lePoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
    text.str("");
    text << "TE(" << tePoint.X() << ", " << tePoint.Y() << ", " << tePoint.Z() << ")";
    app->getScene()->displayPoint(tePoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
    text.str("");

    gp_Lin gpline = gce_MakeLin(lePoint, tePoint);

    // Lets make a limited line for display reasons
    Standard_Real length = lePoint.Distance(tePoint);
    if (length > 0.0) {
        Handle(Geom_TrimmedCurve) trimmedLine = GC_MakeSegment(gpline, -length * 0.2, length * 1.2);
        TopoDS_Edge le_te_edge = BRepBuilderAPI_MakeEdge(trimmedLine);
        app->getScene()->displayShape(le_te_edge, true, Quantity_NOC_GOLD);
    }

    // display points in case of a few sample points
    const std::vector<tigl::CTiglPoint>& fewPointList = profile.GetProfileAlgo()->GetSamplePoints();
    if (fewPointList.size() < 15 && fewPointList.size() > 0) {
        for (unsigned int i = 0; i<fewPointList.size(); ++i) {
            const tigl::CTiglPoint& p = fewPointList.at(i);
            std::stringstream str;
            str << i << ": (" << p.x << ", " << p.y << ", " << p.z << ")";
            gp_Pnt pnt = p.Get_gp_Pnt();
            app->getScene()->displayPoint(pnt, str.str().c_str(), Standard_False, 0., 0., 0., 6.);
        }
    }
    else {
        // Draw some points on the wing profile
        for (double xsi = 0.1; xsi <= 0.9; xsi = xsi + 0.2) {
            try {
                gp_Pnt chordPoint = profile.GetChordPoint(xsi);
                text << "CPT(" << xsi << ")";
                text << "(" << chordPoint.X() << ", " << chordPoint.Y() << ", " << chordPoint.Z() << ")";
                app->getScene()->displayPoint(chordPoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
                text.str("");
            }
            catch (tigl::CTiglError& ex) {
                displayError(ex.what());
            }

            try {
                gp_Pnt upperPoint = profile.GetUpperPoint(xsi);
                text << "UPT(" << xsi << ")";
                text << "(" << upperPoint.X() << ", " << upperPoint.Y() << ", " << upperPoint.Z() << ")";
                app->getScene()->displayPoint(upperPoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
                text.str("");
            }
            catch (tigl::CTiglError& ex) {
                displayError(ex.what());
            }

            try {
                gp_Pnt lowerPoint = profile.GetLowerPoint(xsi);
                text << "LPT(" << xsi << ")";
                text << "(" << lowerPoint.X() << ", " << lowerPoint.Y() << ", " << lowerPoint.Z() << ")";
                app->getScene()->displayPoint(lowerPoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
                text.str("");
            }
            catch (tigl::CTiglError& ex) {
                displayError(ex.what());
            }
        }
    }

    app->getViewer()->viewLeft();
    app->getViewer()->fitAll();
}

/*
 * Draw the input wing together with profile points
 */
void TIGLViewerDocument::drawWingOverlayProfilePoints(tigl::CCPACSWing& wing)
{
    START_COMMAND();

    for (int i = 1; i <= wing.GetSegmentCount(); i++) {
        // Get segment
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
        // Get inner profile point list
        tigl::CTiglWingConnection& innerConnection = segment.GetInnerConnection();
        tigl::CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
        const std::vector<tigl::CTiglPoint>& innerProfilePointList = innerProfile.GetProfileAlgo()->GetSamplePoints();
        // get points and transform them
        tigl::CTiglTransformation innerT = innerConnection.GetSectionElementTransformation();
        innerT.PreMultiply(innerConnection.GetSectionTransformation());
        innerT.PreMultiply(innerConnection.GetPositioningTransformation());
        innerT.PreMultiply(wing.GetTransformationMatrix());
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < innerProfilePointList.size(); i++) {

            gp_Pnt pnt = innerProfilePointList[i].Get_gp_Pnt();
            pnt = innerT.Transform(pnt);

            app->getScene()->displayPoint(pnt, "", Standard_False, 0.0, 0.0, 0.0, 2.0);
        }

        // Get outer profile point list
        tigl::CTiglWingConnection& outerConnection = segment.GetOuterConnection();
        tigl::CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
        const std::vector<tigl::CTiglPoint>& outerProfilePointList = outerProfile.GetProfileAlgo()->GetSamplePoints();

        // get points and transform them
        tigl::CTiglTransformation outerT = outerConnection.GetSectionElementTransformation();
        outerT.PreMultiply(outerConnection.GetSectionTransformation());
        outerT.PreMultiply(outerConnection.GetPositioningTransformation());
        outerT.PreMultiply(wing.GetTransformationMatrix());
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < outerProfilePointList.size(); i++) {

            gp_Pnt pnt = outerProfilePointList[i].Get_gp_Pnt();
            pnt = outerT.Transform(pnt);

            app->getScene()->displayPoint(pnt, "", Standard_False, 0.0, 0.0, 0.0, 2.0);
        }

    }
}

/*
 * Draw the input wing
 */
void TIGLViewerDocument::drawWing(tigl::CCPACSWing& wing)
{
    START_COMMAND();

    app->getScene()->deleteAllObjects();

    for (int i = 1; i <= wing.GetSegmentCount(); i++) {
        // Draw segment loft
        app->getScene()->displayShape(wing.GetSegment(i).GetLoft(), true);
    }
}

/*
 * Draw the triangulation of the input wing
 */
void TIGLViewerDocument::drawWingTriangulation(tigl::CCPACSWing& wing)
{
    START_COMMAND();

    //clear screen
    app->getScene()->deleteAllObjects();

    //we do not fuse segments anymore but build it from scratch with the profiles
    const TopoDS_Shape& fusedWing = wing.GetLoft()->Shape();

    TopoDS_Compound compound;
    createShapeTriangulation(fusedWing, compound);
    app->getScene()->displayShape(compound, true);
}

/*
 * Draw the input wing together with sampled points
 */
void TIGLViewerDocument::drawWingSamplePoints(tigl::CCPACSWing& wing)
{
    START_COMMAND();

    int wingIndex = 0;
    for (int i = 1; i <= GetConfiguration().GetWingCount(); ++i) {
        tigl::CCPACSWing& curWing = GetConfiguration().GetWing(i);
        if (wing.GetUID() == curWing.GetUID()) {
            wingIndex = i;
            break;
        }
    }

    if (wingIndex <= 0) {
        return;
    }

    app->getScene()->deleteAllObjects();

    for (int segmentIndex = 1; segmentIndex <= wing.GetSegmentCount(); segmentIndex++) {
        // Draw segment loft
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);

        app->getScene()->displayShape(segment.GetLoft(), true);

        // Draw some points on the wing segment
        for (double eta = 0.0; eta <= 1.0; eta += 0.1) {
            for (double xsi = 0.0; xsi <= 1.0; xsi += 0.1) {
                double x, y, z;
                tiglWingGetUpperPoint(m_cpacsHandle,
                                      wingIndex,
                                      segmentIndex,
                                      eta,
                                      xsi,
                                      &x,
                                      &y,
                                      &z);

                app->getScene()->displayPoint(gp_Pnt(x,y,z),"",Standard_False, 0., 0., 0., 1.);

                tiglWingGetLowerPoint(m_cpacsHandle,
                                      wingIndex,
                                      segmentIndex,
                                      eta,
                                      xsi,
                                      &x,
                                      &y,
                                      &z);

                app->getScene()->displayPoint(gp_Pnt(x,y,z),"",Standard_False, 0., 0., 0., 1.);
            }
        }
    }
}

/*
 * Draw the input wing loft
 */
void TIGLViewerDocument::drawFusedWing(tigl::CCPACSWing& wing)
{
    START_COMMAND();
    app->getScene()->deleteAllObjects();
    app->getScene()->displayShape( wing.GetLoft(), true);
}

/*
 * Draw the input wing component segment
 */
void TIGLViewerDocument::drawWingComponentSegment(tigl::CCPACSWingComponentSegment& segment)
{
    START_COMMAND();
    app->getScene()->deleteAllObjects();

    app->getScene()->displayShape(segment.GetLoft(), true);
}

/*
 * Draw a point on the input wing component segment
 */
void TIGLViewerDocument::drawWingComponentSegmentPoint(const std::string& csUID, const double& eta, const double& xsi)
{
        // here are two alternative methods to determine the 3d point of the CS
#if 0
        // A more indirect method, good to debug errors in CCPACSWingComponentSegment::findSegment
        char * wingUID, * segmentUID;
        double alpha, beta;
        TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(
                    getCpacsHandle(),
                             csUID.c_str(),
                    eta, xsi,
                    &wingUID,
                    &segmentUID,
                    &alpha, &beta);

        if (ret == TIGL_SUCCESS){
            tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUID);
            tigl::CCPACSWingSegment& segment = dynamic_cast<tigl::CCPACSWingSegment&>(wing.GetSegment(segmentUID));
            gp_Pnt point = segment.GetChordPoint(alpha, beta);
            app->getContext().DisplayPoint(point, "", Standard_True, 0,0,0,1.0);
        }
        else {
            displayError(QString("Error in <b>tiglWingComponentSegmentPointGetSegmentEtaXsi</b>. ReturnCode: %1").arg(ret), "Error");
        }
#else
        double x,y,z;
        TiglReturnCode ret = tiglWingComponentSegmentGetPoint(
                    getCpacsHandle(),
                             csUID.c_str(),
                    eta, xsi,
                    &x, &y, &z);
        if (ret == TIGL_SUCCESS){
            gp_Pnt point(x,y,z);
            app->getScene()->displayPoint(point, "", Standard_True, 0,0,0,1.0);
        }
        else {
            displayError(QString("Error in <b>tiglWingComponentSegmentPointGetPoint</b>. ReturnCode: %1").arg(ret), "Error");
        }
#endif
}

/*
 * Draw upper and lower surface of the input wing
 */
void TIGLViewerDocument::drawWingShells(tigl::CCPACSWing& wing)
{
    START_COMMAND();
    app->getScene()->deleteAllObjects();

    app->getScene()->displayShape(wing.GetUpperShape(), true, Quantity_NOC_GREEN);
    app->getScene()->displayShape(wing.GetLowerShape(), true, Quantity_NOC_RED);
}

/*
 * Reads traingles from Mesh of shape and creates vertices and triangular faces
 */
void TIGLViewerDocument::createShapeTriangulation(const TopoDS_Shape& shape, TopoDS_Compound& compound)
{
    meshShape(shape, TIGLViewerSettings::Instance().triangulationAccuracy());
    BRep_Builder builder;
    builder.MakeCompound(compound);
    builder.Add(compound, shape);
    
    TopExp_Explorer faceExplorer;

    for (faceExplorer.Init(shape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
        if (triangulation.IsNull()) {
            continue;
        }

        gp_Trsf nodeTransformation = location;
        const TColgp_Array1OfPnt& nodes = triangulation->Nodes();

        int index1, index2, index3;
        const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
        for (int j = triangles.Lower(); j <= triangles.Upper(); j++) {
            const Poly_Triangle& triangle = triangles(j);
            triangle.Get(index1, index2, index3);
            gp_Pnt point1 = nodes(index1).Transformed(nodeTransformation);
            gp_Pnt point2 = nodes(index2).Transformed(nodeTransformation);
            gp_Pnt point3 = nodes(index3).Transformed(nodeTransformation);

            BRepBuilderAPI_MakeEdge edge1(point1, point2);
            BRepBuilderAPI_MakeEdge edge2(point2, point3);
            BRepBuilderAPI_MakeEdge edge3(point3, point1);
            if (edge1.IsDone()) {
                builder.Add(compound, edge1);
            }
            if (edge2.IsDone()) {
                builder.Add(compound, edge2);
            }
            if (edge3.IsDone()) {
                builder.Add(compound, edge3);
            }
        }
    }
}

TiglCPACSConfigurationHandle TIGLViewerDocument::getCpacsHandle() const 
{
    return this->m_cpacsHandle;
}


