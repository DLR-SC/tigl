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
#include <Handle_AIS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepMesh.hxx>
#include <TopoDS_Shell.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <gce_MakeLin.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBndLib.hxx>

// TIGLViewer includes
#include "TIGLViewerInternal.h"
#include "CCPACSConfigurationManager.h"
#include "TIGLViewerInputoutput.h"
#include "ISession_Point.h"
#include "ISession_Text.h"
#include "CTiglPoint.h"
#include "CTiglError.h"
#include "TIGLViewerSettings.h"

#define max(a,b) ((a) > (b) ? (a) : (b))

TIGLViewerDocument::TIGLViewerDocument( QWidget *parentWidget, const Handle_AIS_InteractiveContext& ic, const TIGLViewerSettings& set )
: _settings(set)
{
	parent = parentWidget;
	myAISContext = ic;
	myOCC = ((TIGLViewerWindow*) parent)->getMyOCC();
	m_cpacsHandle = -1;
}

TIGLViewerDocument::~TIGLViewerDocument( )
{
	m_cpacsHandle = -1;
}

void TIGLViewerDocument::writeToStatusBar(QString text)
{
	((TIGLViewerWindow*) parent)->statusBar()->showMessage(text);
}

void TIGLViewerDocument::displayError(QString text, QString header = NULL)
{
	((TIGLViewerWindow*) parent)->displayErrorMessage(text, header);
}


char* TIGLViewerDocument::qstringToCstring(QString text)
{
	return strdup((const char*)text.toLatin1());
}

TiglReturnCode TIGLViewerDocument::openCpacsConfiguration(const QString fileName)
{
	QStringList configurations;

	myAISContext->SetDisplayMode(AIS_Shaded,Standard_False);

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
	for (int i = 0; i < countAircrafts; i++)
	{
		char *text;
		tixiRet = tixiGetTextAttribute( tixiHandle, CPACS_XPATH_AIRCRAFT_MODEL, "uID", &text);
		configurations << text;
	}	for (int i = 0; i < countRotorcrafts; i++) {
		char *text;
		tixiRet = tixiGetTextAttribute(tixiHandle, CPACS_XPATH_ROTORCRAFT_MODEL, "uID", &text);
		configurations << text;
	}

	// Get configuration from user and open with TIGL
    TiglReturnCode tiglRet = TIGL_UNINITIALIZED;
	if (countRotorcrafts + countAircrafts == 0)
	{
		// no configuration present
		loadedConfigurationFileName = fileName;
		return TIGL_UNINITIALIZED;
	}
	else if (countRotorcrafts + countAircrafts == 1)
	{
		tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &m_cpacsHandle);
	}
	else
	{
		bool ok;
		QString item = QInputDialog::getItem(parent, tr("Select CPACS Configuration"),
											 tr("Available Configurations:"), configurations, 0, false, &ok);
		if (ok && !item.isEmpty())
		{
			tiglRet = tiglOpenCPACSConfiguration(tixiHandle, strdup((const char*)item.toLatin1()), &m_cpacsHandle);
		}
	}

	if (tiglRet != TIGL_SUCCESS)
	{
		tixiCloseDocument(tixiHandle);
		m_cpacsHandle = -1;
		//displayError(QString("Error in function <u>tiglOpenCPACSConfiguration</u>. Error code: %1").arg(tiglRet), "TIGL Error");
		return tiglRet;
	}
	drawAllFuselagesAndWings();
	loadedConfigurationFileName = fileName;
	return TIGL_SUCCESS;
}

void TIGLViewerDocument::closeCpacsConfiguration(){
    if(m_cpacsHandle < 1)
        return;

    TixiDocumentHandle tixiHandle = -1;
    tiglGetCPACSTixiHandle(m_cpacsHandle, &tixiHandle);

    tiglCloseCPACSConfiguration(m_cpacsHandle);
    m_cpacsHandle = -1;

    if(tixiHandle > 0){
        tixiCloseDocument(tixiHandle);
    }

    myAISContext->EraseAll(Standard_False);
    emit documentUpdated(m_cpacsHandle);
}


/**
 * Re-reads the CPACS configuration.
 */
void TIGLViewerDocument::updateConfiguration()
{
	// Right now, we just close the tigl session and open a new one
	if (!loadedConfigurationFileName.isEmpty()) {
		tiglCloseCPACSConfiguration(m_cpacsHandle);
		m_cpacsHandle = -1;
		myAISContext->EraseAll(Standard_False);
		openCpacsConfiguration(loadedConfigurationFileName);
		emit documentUpdated(m_cpacsHandle);
	}
}


// Returns the CPACS configuration
tigl::CCPACSConfiguration& TIGLViewerDocument::GetConfiguration(void) const
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    return manager.GetConfiguration(m_cpacsHandle);
}


// creates triangulation of shape
// returns true, of mesh was done
bool meshShape(const TopoDS_Shape& loft, double rel_deflection){
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
	if(!BRepTools::Triangulation (loft, deflection)){
		BRepTools::Clean(loft);
		BRepMesh::Mesh(loft, deflection);
		return true;
	}
	else
		return false;
}

// a small helper when we just want to display a shape
Handle(AIS_Shape) TIGLViewerDocument::displayShape(const TopoDS_Shape& loft, Quantity_Color color)
{
	Handle(AIS_Shape) shape = new AIS_Shape(loft);
	shape->SetMaterial(Graphic3d_NOM_METALIZED);
	shape->SetColor(color);
	shape->SetOwnDeviationCoefficient(_settings.tesselationAccuracy());
	myAISContext->Display(shape, Standard_True);
	return shape;
}


// Displays a point on the screen
void TIGLViewerDocument::DisplayPoint(gp_Pnt& aPoint,
                                  const char* aText,
                                  Standard_Boolean UpdateViewer,
                                  Standard_Real anXoffset,
                                  Standard_Real anYoffset,
                                  Standard_Real aZoffset,
                                  Standard_Real TextScale)
{
    Handle(ISession_Point) aGraphicPoint = new ISession_Point(aPoint);
    myAISContext->Display(aGraphicPoint,UpdateViewer);
    Handle(ISession_Text) aGraphicText = new ISession_Text(aText, aPoint.X() + anXoffset,
    											 aPoint.Y() + anYoffset,
    											 aPoint.Z() + aZoffset);
    aGraphicText->SetScale(TextScale);
    myAISContext->Display(aGraphicText,UpdateViewer);
}


// Wing selection Dialog
QString TIGLViewerDocument::dlgGetWingSelection()
{
	QStringList wings;
	bool ok;

	// Initialize wing list
	tigl::CCPACSConfiguration& config = GetConfiguration();
	int wingCount = config.GetWingCount();
	for (int i = 1; i <= wingCount; i++)
	{
		tigl::CCPACSWing& wing = config.GetWing(i);
		std::string name = wing.GetUID();
		if (name == "") name = "Unknown wing";
		wings << name.c_str();
	}

	QString choice = QInputDialog::getItem(parent, tr("Select Wing"), tr("Available Wings:"), wings, 0, false, &ok);
    if(ok)
        return choice;
    else
        return "";
}

// Wing Component Segment selection Dialog
QString TIGLViewerDocument::dlgGetWingComponentSegmentSelection()
{
	QStringList compSegs;
	bool ok;

	// Initialize wing list
	tigl::CCPACSConfiguration& config = GetConfiguration();
	int wingCount = config.GetWingCount();
	for (int i = 1; i <= wingCount; i++)
	{
		tigl::CCPACSWing& wing = config.GetWing(i);
		for(int j = 1; j <= wing.GetComponentSegmentCount(); ++j){
			tigl::CTiglAbstractSegment& segment = wing.GetComponentSegment(j);
			std::string name = segment.GetUID();
			if (name == "") name = "Unknown component segment";
			compSegs << name.c_str();
		}
	}

	QString choice = QInputDialog::getItem(parent, tr("Select Component Segment"), tr("Available Component Segments:"), compSegs, 0, false, &ok);
    if(ok)
        return choice;
    else
        return "";
}


// Wing profile Dialog
QString TIGLViewerDocument::dlgGetWingProfileSelection()
{
	QStringList wingProfiles;
	bool ok;

	// Initialize wing list
	tigl::CCPACSConfiguration& config = GetConfiguration();
	int profileCount = config.GetWingProfileCount();
	for (int i = 1; i <= profileCount; i++)
	{
		tigl::CCPACSWingProfile& profile = config.GetWingProfile(i);
		std::string profileUID = profile.GetUID();
		std::string name     = profile.GetName();
		wingProfiles << profileUID.c_str();
	}

	QString choice = QInputDialog::getItem(parent, tr("Select Wing Profile"), tr("Available Wing Profiles:"), wingProfiles, 0, false, &ok);
    if(ok)
        return choice;
    else
        return "";
}


// Fuselage selection Dialog
QString TIGLViewerDocument::dlgGetFuselageSelection()
{
	QStringList fuselages;
	bool ok;

	// Initialize wing list
	tigl::CCPACSConfiguration& config = GetConfiguration();
	int fuselageCount = config.GetFuselageCount();
	for (int i = 1; i <= fuselageCount; i++)
	{
		tigl::CCPACSFuselage& fuselage = config.GetFuselage(i);
		std::string name = fuselage.GetUID();
		if (name == "") name = "Unknown fuselage";
		fuselages << name.c_str();
	}
	QString choice = QInputDialog::getItem(parent, tr("Select Fuselage"), tr("Available Fuselages:"), fuselages, 0, false, &ok);
    if(ok)
        return choice;
    else
        return "";
}

// Fuselage profile Dialog
QString TIGLViewerDocument::dlgGetFuselageProfileSelection()
{
	QStringList fuselageProfiles;
	bool ok;

	// Initialize fuselage list
	tigl::CCPACSConfiguration& config = GetConfiguration();
	int profileCount = config.GetFuselageProfileCount();
	for (int i = 1; i <= profileCount; i++)
	{
		tigl::CCPACSFuselageProfile& profile = config.GetFuselageProfile(i);
		std::string profileUID = profile.GetUID();
		std::string name     = profile.GetName();
		fuselageProfiles << profileUID.c_str();
	}

	QString choice = QInputDialog::getItem(parent, tr("Select Fuselage Profile"), tr("Available Fuselage Profiles:"), fuselageProfiles, 0, false, &ok);
    if(ok)
        return choice;
    else
        return "";
}

void TIGLViewerDocument::tiglInterpolateBsplineWire()
{
	tiglUseAlgorithm(TIGL_INTERPOLATE_BSPLINE_WIRE);
}

void TIGLViewerDocument::tiglInterpolateLinearWire()
{
    tiglUseAlgorithm(TIGL_INTERPOLATE_LINEAR_WIRE);
}

void TIGLViewerDocument::tiglApproximateBsplineWireAction()
{
    tiglUseAlgorithm(TIGL_APPROXIMATE_BSPLINE_WIRE);
}


void TIGLViewerDocument::drawAllFuselagesAndWings( )
{
	// Draw all wings
	for (int w = 1; w <= GetConfiguration().GetWingCount(); w++)
	{
		tigl::CCPACSWing& wing = GetConfiguration().GetWing(w);

		for (int i = 1; i <= wing.GetSegmentCount(); i++)
		{
			tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
			displayShape(segment.GetLoft());
		}

		if(wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY)
			continue;

		for (int i = 1; i <= wing.GetSegmentCount(); i++)
		{
			tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
			displayShape(segment.GetMirroredLoft(), Quantity_NOC_MirrShapeCol);
		}
	}

	// Draw all fuselages
	for (int f = 1; f <= GetConfiguration().GetFuselageCount(); f++)
	{
		tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(f);

		for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
		{
			tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
			TopoDS_Shape loft = segment.GetLoft();

			// Transform by fuselage transformation
			loft = fuselage.GetFuselageTransformation().Transform(loft);
			displayShape(loft);
		}

		if(fuselage.GetSymmetryAxis() == TIGL_NO_SYMMETRY)
			continue;

		for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
		{
			tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
			TopoDS_Shape loft = segment.GetMirroredLoft();

			// Transform by fuselage transformation
			loft = fuselage.GetFuselageTransformation().Transform(loft);
			displayShape(loft, Quantity_NOC_MirrShapeCol);
		}
	}
}



void TIGLViewerDocument::drawWingProfiles()
{
	QString wingProfile = dlgGetWingProfileSelection();
    if(wingProfile == "")
        return;

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSWingProfile& profile = GetConfiguration().GetWingProfile(wingProfile.toStdString());
    TopoDS_Wire wire        = profile.GetWire(true);
    Handle(AIS_Shape) shape = new AIS_Shape(wire);
    shape->SetColor(Quantity_NOC_WHITE);
    myAISContext->Display(shape, Standard_True);

    // Leading/trailing edges
    gp_Pnt lePoint = profile.GetLEPoint();
    gp_Pnt tePoint = profile.GetTEPoint();

    std::ostringstream text;
    text << "LE(" << lePoint.X() << ", " << lePoint.Y() << ", " << lePoint.Z() << ")";
    DisplayPoint(lePoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
    text.str("");
    text << "TE(" << tePoint.X() << ", " << tePoint.Y() << ", " << tePoint.Z() << ")";
    DisplayPoint(tePoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
    text.str("");

    gp_Lin gpline = gce_MakeLin(lePoint, tePoint);
    // Handle(Geom_Line) geomLine = new Geom_Line(gpline); // unlimited line, which can be used in intersection calcs

    // Lets make a limited line for display reasons
    Standard_Real length = lePoint.Distance(tePoint);
    if (length > 0.0)
    {
        Handle(Geom_TrimmedCurve) trimmedLine = GC_MakeSegment(gpline, -length * 0.2, length * 1.2);
        TopoDS_Edge le_te_edge = BRepBuilderAPI_MakeEdge(trimmedLine);
        Handle(AIS_Shape) lineShape = new AIS_Shape(le_te_edge);
        lineShape->SetColor(Quantity_NOC_GOLD /*Quantity_NOC_WHITE*/);
        myAISContext->Display(lineShape, Standard_True);
    }

    if(profile.GetCoordinateContainer().size() < 15) {
        for(unsigned int i = 0; i < profile.GetCoordinateContainer().size(); ++i){
            tigl::CTiglPoint * p = profile.GetCoordinateContainer().at(i);
            std::stringstream str;
            str << i << ": (" << p->x << ", " << p->y << ", " << p->z << ")";
            gp_Pnt pnt = p->Get_gp_Pnt();
            DisplayPoint(pnt, str.str().c_str(), Standard_False, 0., 0., 0., 6.);
        }

    }
    else {
      // Draw some points on the wing profile
      for (double xsi = 0.0; xsi <= 1.0; xsi = xsi + 0.2)
      {
        try {
            gp_Pnt chordPoint = profile.GetChordPoint(xsi);
            text << "CPT(" << xsi << ")";
            text << "(" << chordPoint.X() << ", " << chordPoint.Y() << ", " << chordPoint.Z() << ")";
            DisplayPoint(chordPoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
            text.str("");
        }
        catch (tigl::CTiglError& ex) {
            std::cerr << ex.getError() << std::endl;
        }

        try {
            gp_Pnt upperPoint = profile.GetUpperPoint(xsi);
            text << "UPT(" << xsi << ")";
            text << "(" << upperPoint.X() << ", " << upperPoint.Y() << ", " << upperPoint.Z() << ")";
            DisplayPoint(upperPoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
            text.str("");
        }
        catch (tigl::CTiglError& ex) {
            std::cerr << ex.getError() << std::endl;
        }

        try {
            gp_Pnt lowerPoint = profile.GetLowerPoint(xsi);
            text << "LPT(" << xsi << ")";
            text << "(" << lowerPoint.X() << ", " << lowerPoint.Y() << ", " << lowerPoint.Z() << ")";
            DisplayPoint(lowerPoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
            text.str("");
        }
        catch (tigl::CTiglError& ex) {
            std::cerr << ex.getError() << std::endl;
        }
      }
    }
}

void TIGLViewerDocument::drawWingOverlayProfilePoints()
{
	QString wingUid = dlgGetWingSelection();
	if(wingUid == "")
		return;

	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());

	for (int i = 1; i <= wing.GetSegmentCount(); i++)
	{
		// Draw inner profile points
		tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
		std::vector<tigl::CTiglPoint*> innerPoints = segment.GetRawInnerProfilePoints();
		for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < innerPoints.size(); i++)
		{
			gp_Pnt pnt = innerPoints[i]->Get_gp_Pnt();
			pnt = wing.GetWingTransformation().Transform(pnt);
			DisplayPoint(pnt, "", Standard_False, 0.0, 0.0, 0.0, 2.0);
		}

		// Draw outer profile points
		std::vector<tigl::CTiglPoint*> outerPoints = segment.GetRawOuterProfilePoints();
		for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < outerPoints.size(); i++)
		{
			gp_Pnt pnt = outerPoints[i]->Get_gp_Pnt();
			pnt = wing.GetWingTransformation().Transform(pnt);
			DisplayPoint(pnt, "", Standard_False, 0.0, 0.0, 0.0, 2.0);
		}
	}
}



void TIGLViewerDocument::drawFuselageProfiles()
{
	QString fuselageProfile = dlgGetFuselageProfileSelection();
	if(fuselageProfile == "")
		return;

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSFuselageProfile& profile = GetConfiguration().GetFuselageProfile(fuselageProfile.toStdString());
    TopoDS_Wire wire        = profile.GetWire(true);
    Handle(AIS_Shape) shape = new AIS_Shape(wire);
    shape->SetColor(Quantity_NOC_WHITE);
    myAISContext->Display(shape, Standard_True);

    if(profile.GetCoordinateContainer().size() < 15) {
        for(unsigned int i = 0; i < profile.GetCoordinateContainer().size(); ++i){
            tigl::CTiglPoint * p = profile.GetCoordinateContainer().at(i);
            std::stringstream str;
            str << i << ": (" << p->x << ", " << p->y << ", " << p->z << ")";
            gp_Pnt pnt = p->Get_gp_Pnt();
            DisplayPoint(pnt, str.str().c_str(), Standard_False, 0., 0., 0., 6.);
        }

    }
    else {
        for (double zeta = 0.0; zeta <= 1.0; zeta += 0.1)
        {
          try {
            gp_Pnt wirePoint = profile.GetPoint(zeta);
            std::ostringstream text;
            text << "PT(" << zeta << ")";
            DisplayPoint(wirePoint, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
            text.str("");
          }
          catch (tigl::CTiglError& ex) {
            std::cerr << ex.getError() << std::endl;
          }
        }
    }
}


void TIGLViewerDocument::drawWing()
{
	QString wingUid = dlgGetWingSelection();
    if(wingUid=="")
        return;

	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());

    myAISContext->EraseAll(Standard_False);

    for (int i = 1; i <= wing.GetSegmentCount(); i++)
    {
        // Draw segment loft
        const TopoDS_Shape& loft = wing.GetSegment(i).GetLoft();
        displayShape(loft);
    }
}



void TIGLViewerDocument::drawFuselage()
{
	QString fuselageUid = dlgGetFuselageSelection();
	if(fuselageUid == "")
		return;

	QApplication::setOverrideCursor( Qt::WaitCursor );
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

    myAISContext->EraseAll(Standard_False);

    for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
    {
        // Draw segment loft
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
        TopoDS_Shape loft = segment.GetLoft();
        // Transform by fuselage transformation
        loft = fuselage.GetFuselageTransformation().Transform(loft);
        displayShape(loft);
    }
	QApplication::restoreOverrideCursor();
}

void TIGLViewerDocument::drawWingTriangulation()
{
	QString wingUid = dlgGetWingSelection();
    // cancel on abort
    if (wingUid == "")
        return;

	QApplication::setOverrideCursor( Qt::WaitCursor );
	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());

    //clear screen
    myAISContext->EraseAll(Standard_False);

    //we do not fuse segments anymore but build it from scratch with the profiles
    TopoDS_Shape& fusedWing = wing.GetLoft();

	TopoDS_Compound compound;
	createShapeTriangulation(fusedWing, compound);
	
	displayShape(compound);
	QApplication::restoreOverrideCursor();
}


void TIGLViewerDocument::drawFuselageTriangulation()
{
	QString fuselageUid = dlgGetFuselageSelection();
	if(fuselageUid == "")
		return;

	QApplication::setOverrideCursor( Qt::WaitCursor );
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

    //clear screen
    myAISContext->EraseAll(Standard_False);
    TopoDS_Shape fusedWing = fuselage.GetLoft();

    TopoDS_Compound triangulation;
    createShapeTriangulation(fusedWing, triangulation);

    displayShape(triangulation);
    QApplication::restoreOverrideCursor();
}


void TIGLViewerDocument::drawWingSamplePoints()
{
	QString wingUid = dlgGetWingSelection();
	if(wingUid == "")
		return;

	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());

    myAISContext->EraseAll(Standard_False);

    for (int segmentIndex = 1; segmentIndex <= wing.GetSegmentCount(); segmentIndex++)
    {
        // Draw segment loft
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        const TopoDS_Shape& loft = segment.GetLoft();

        Handle(AIS_Shape) shape = new AIS_Shape(loft);
        shape->SetColor(Quantity_NOC_BLUE2);
        myAISContext->Display(shape, Standard_True);

        // Draw some points on the wing segment
        for (double eta = 0.0; eta <= 1.0; eta += 0.1)
        {
            for (double xsi = 0.0; xsi <= 1.0; xsi += 0.1)
            {
                double x, y, z;
                tiglWingGetUpperPoint(
                    m_cpacsHandle,
                    1,	// TODO: we need to implement that function to use UID instead of index!
                    segmentIndex,
                    eta,
                    xsi,
                    &x,
                    &y,
                    &z);

                Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
                myAISContext->Display(aGraphicPoint, Standard_False);

                tiglWingGetLowerPoint( m_cpacsHandle,
										1,	// TODO: we need to implement that function to use UID instead of index!
										segmentIndex,
										eta,
										xsi,
										&x,
										&y,
										&z);

                aGraphicPoint = new ISession_Point(x, y, z);
                myAISContext->Display(aGraphicPoint, Standard_False);
            }
        }
    }
}


void TIGLViewerDocument::drawFuselageSamplePoints()
{
	QString fuselageUid = dlgGetFuselageSelection();
	if(fuselageUid == "")
		return;

	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

	myAISContext->EraseAll(Standard_False);

	for (int segmentIndex = 1; segmentIndex <= fuselage.GetSegmentCount(); segmentIndex++)
	{
		// Draw segment loft
		tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
		TopoDS_Shape loft = segment.GetLoft();
		// Transform by fuselage transformation
		loft = fuselage.GetFuselageTransformation().Transform(loft);
		Handle(AIS_Shape) shape = new AIS_Shape(loft);
		shape->SetColor(Quantity_NOC_BLUE2);
		myAISContext->Display(shape, Standard_True);

		// Draw some points on the fuselage segment
		for (double eta = 0.0; eta <= 1.0; eta += 0.25)
		{
			for (double zeta = 0.0; zeta <= 1.0; zeta += 0.1)
			{
				double x, y, z;
				tiglFuselageGetPoint(m_cpacsHandle,
										1,	// TODO: we need to implement that function to use UID instead of index!
										segmentIndex,
										eta,
										zeta,
										&x,
										&y,
										&z);

                Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
				myAISContext->Display(aGraphicPoint, Standard_False);
			}
		}
	}
}



void TIGLViewerDocument::drawFuselageSamplePointsAngle()
{
	int fuselageIndex = 1;
	int segmentIndex = 15;
	double angle = 45.0; // turn through 45 degrees
	double x, y, z;

	myAISContext->EraseAll(Standard_False);
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageIndex);

	// Draw the fuselage
	for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
	{
		tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
		TopoDS_Shape loft = segment.GetLoft();

		// Transform by fuselage transformation
		loft = fuselage.GetFuselageTransformation().Transform(loft);

		displayShape(loft);
	}

	// Display the intersection point
	tiglFuselageGetPointAngle(m_cpacsHandle,
								fuselageIndex,
								segmentIndex,
								0.5,
								angle,
								&x,
								&y,
								&z);

    Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
	myAISContext->Display(aGraphicPoint, Standard_False);
}


void TIGLViewerDocument::drawAllFuselagesAndWingsSurfacePoints()
{
	 myAISContext->EraseAll(Standard_False);
	std::ostringstream text;

	// Draw all wings
	for (int wingIndex = 1; wingIndex <= GetConfiguration().GetWingCount(); wingIndex++)
	{
		tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingIndex);

		for (int segmentIndex = 1; segmentIndex <= wing.GetSegmentCount(); segmentIndex++)
		{
			tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
			const TopoDS_Shape& loft = segment.GetLoft();

			Handle(AIS_Shape) shape = new AIS_Shape(loft);
			shape->SetColor(Quantity_NOC_BLUE2);
			myAISContext->Display(shape, Standard_True);

			for (double eta = 0.0; eta <= 1.0; eta += 0.1)
			{
				for (double xsi = 0.0; xsi <= 1.0; xsi += 0.1)
				{
					double x, y, z;
					tiglWingGetUpperPoint(
						m_cpacsHandle,
						wingIndex,
						segmentIndex,
						eta,
						xsi,
						&x,
						&y,
						&z);

                    Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
					myAISContext->Display(aGraphicPoint,Standard_False);
					//text << "PT(" << xsi << ", " << eta << ")";
					//text << "(" << x << ", " << y << ", " << z << ")";
					//DisplayPoint(gp_Pnt(x, y, z), const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
					//text.str("");

					tiglWingGetLowerPoint(
						m_cpacsHandle,
						wingIndex,
						segmentIndex,
						eta,
						xsi,
						&x,
						&y,
						&z);

                    aGraphicPoint = new ISession_Point(x, y, z);
					myAISContext->Display(aGraphicPoint,Standard_False);
				}
			}
		}
	}

	// Draw all fuselages
	for (int fuselageIndex = 1; fuselageIndex <= GetConfiguration().GetFuselageCount(); fuselageIndex++)
	{
		tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageIndex);

		for (int segmentIndex = 1; segmentIndex <= fuselage.GetSegmentCount(); segmentIndex++)
		{
			// Draw segment loft
			tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
			TopoDS_Shape loft = segment.GetLoft();
			// Transform by fuselage transformation
			loft = fuselage.GetFuselageTransformation().Transform(loft);
			Handle(AIS_Shape) shape = new AIS_Shape(loft);
			shape->SetColor(Quantity_NOC_BLUE2);
			myAISContext->Display(shape, Standard_True);

			// Draw some points on the fuselage segment
			for (double eta = 0.0; eta <= 1.0; eta += 0.25)
			{
				for (double zeta = 0.0; zeta <= 1.0; zeta += 0.1)
				{
					double x, y, z;
					tiglFuselageGetPoint(
										m_cpacsHandle,
										fuselageIndex,
										segmentIndex,
										eta,
										zeta,
										&x,
										&y,
										&z);

                    Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
					myAISContext->Display(aGraphicPoint,Standard_False);
				}
			}
		}
	}
}


// -----------------------
// Export Functions
// -----------------------
void TIGLViewerDocument::exportAsIges()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;

	TIGLViewerInputOutput writer;

	writeToStatusBar(tr("Saving as IGES file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export IGES(*.iges *.igs)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportIGES(m_cpacsHandle, qstringToCstring(fileName));
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportIGES</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}


void TIGLViewerDocument::exportFusedAsIges()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;

	TIGLViewerInputOutput writer;

	writeToStatusBar(tr("Saving fused model as IGES file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export IGES(*.iges *.igs)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportFusedWingFuselageIGES(m_cpacsHandle, qstringToCstring(fileName));
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportFusedWingFuselageIGES</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}

void TIGLViewerDocument::exportMeshedWingSTL()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	/*QString wingUid = dlgGetWingSelection(); 
	if(wingUid == "")
		return;*/

	writeToStatusBar(tr("Saving meshed Wing as STL file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedWingSTL(m_cpacsHandle, 1 /* wingUid.toStdString().c_str() */, qstringToCstring(fileName), 0.01);
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportMeshedWingSTL</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}



void TIGLViewerDocument::exportMeshedFuselageSTL()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	/*QString fuselageUid = dlgGetFuselageSelection(); 
	if(fuselageUid == "")
		return;*/

	writeToStatusBar(tr("Saving meshed Fuselage as STL file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedFuselageSTL(m_cpacsHandle, 1 /*fuselageUid*/, qstringToCstring(fileName), 0.01);
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportMeshedFuselageSTL</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}


void TIGLViewerDocument::exportMeshedWingVTK()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	QString wingUid = dlgGetWingSelection();
	if(wingUid == "")
		return;

	writeToStatusBar(tr("Saving meshed Wing as VTK file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedWingVTKByUID(m_cpacsHandle, wingUid.toStdString().c_str(), qstringToCstring(fileName), 0.1);
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportMeshedWingVTKByIndex</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}


void TIGLViewerDocument::exportMeshedWingVTKsimple()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	QString wingUid = dlgGetWingSelection();
	if(wingUid == "")
		return;

	writeToStatusBar(tr("Saving meshed Wing as simple VTK file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		tigl::CCPACSWing& wing = GetConfiguration().GetWing(qstringToCstring(wingUid));
		double deflection = wing.GetWingspan()/2. * _settings.triangulationAccuracy();

		TiglReturnCode err = tiglExportMeshedWingVTKSimpleByUID(m_cpacsHandle, qstringToCstring(wingUid), qstringToCstring(fileName), deflection);
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportMeshedWingVTKSimpleByUID</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}



void TIGLViewerDocument::exportMeshedFuselageVTK()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	QString wingUid = dlgGetFuselageSelection();
	if(wingUid == "")
		return;

	writeToStatusBar(tr("Saving meshed Fuselage as VTK file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedFuselageVTKByUID(m_cpacsHandle, wingUid.toStdString().c_str(), qstringToCstring(fileName), 0.1);
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportMeshedFuselageVTKByIndex</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}


void TIGLViewerDocument::exportMeshedFuselageVTKsimple()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	QString fuselageUid = dlgGetFuselageSelection();

	writeToStatusBar(tr("Saving meshed Fuselage as simple VTK file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export VTK(*.vtp)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedFuselageVTKSimpleByUID(m_cpacsHandle, qstringToCstring(fuselageUid), qstringToCstring(fileName), 0.1);
		QApplication::restoreOverrideCursor();
        if(err != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglExportMeshedFuselageVTKSimpleByUID</u>. Error code: %1").arg(err), "TIGL Error");
		}
	}
}


void TIGLViewerDocument::drawFusedFuselage()
{
	QString fuselageUid = dlgGetFuselageSelection();
	if(fuselageUid == "")
		return;

	myAISContext->EraseAll(Standard_False);
	QApplication::setOverrideCursor( Qt::WaitCursor );
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());
	displayShape(fuselage.GetLoft());
	QApplication::restoreOverrideCursor();
}


void TIGLViewerDocument::drawFusedWing()
{
	QString wingUid = dlgGetWingSelection();
    if(wingUid=="")
        return;

	myAISContext->EraseAll(Standard_False);
	QApplication::setOverrideCursor( Qt::WaitCursor );
	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());
	TopoDS_Shape& loft = wing.GetLoft();
	displayShape(loft);
	QApplication::restoreOverrideCursor();
}


void TIGLViewerDocument::drawFusedAircraft()
{
	QApplication::setOverrideCursor( Qt::WaitCursor );
	try {
		TopoDS_Shape& airplane = GetConfiguration().GetFusedAirplane();
		myAISContext->EraseAll(Standard_False);
		displayShape(airplane);
	}
	catch(tigl::CTiglError & error){
		std::cerr << error.getError() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown Exception" << std::endl;
	}
	QApplication::restoreOverrideCursor();
}

void TIGLViewerDocument::drawFusedAircraftTriangulation()
{
	QApplication::setOverrideCursor( Qt::WaitCursor );
	TopoDS_Shape& airplane = GetConfiguration().GetFusedAirplane();
	myAISContext->EraseAll(Standard_False);
	TopoDS_Compound triangulation;
	createShapeTriangulation(airplane, triangulation);

	displayShape(triangulation);
	QApplication::restoreOverrideCursor();
}


void TIGLViewerDocument::drawWingFuselageIntersectionLine()
{
	QString wingUid = dlgGetWingSelection();
	if(wingUid == "")
		return;
	QString fuselageUid = dlgGetFuselageSelection();
	if(fuselageUid == "")
		return;

	/* now calculate intersection and display single points */
	for (double eta = 0.0; eta <= 1.0; eta += 0.1)
	{
		double x, y, z;

		TiglReturnCode res = tiglComponentIntersectionPoint(
							m_cpacsHandle,
							qstringToCstring(fuselageUid),	//fuselage UID
							qstringToCstring(wingUid),	//wing uid
							1,	// wireID
							eta,
							&x,
							&y,
							&z);

        ISession_Point* aGraphicPoint = new ISession_Point(x, y, z);
		myAISContext->Display(aGraphicPoint,Standard_False);
        if(res != TIGL_SUCCESS) {
			displayError(QString("Error in function <u>tiglComponentIntersectionPoint</u>. Error code: %1").arg(res), "TIGL Error");
		}
	}
}


void TIGLViewerDocument::drawWingComponentSegment()
{
	QString wingUid = dlgGetWingComponentSegmentSelection();
	if(wingUid == "")
		return;

	TopoDS_Shape* pComponentSegment = NULL;
	myAISContext->EraseAll(Standard_False);
	QApplication::setOverrideCursor( Qt::WaitCursor );
	for(int i = 1; i <= GetConfiguration().GetWingCount();++i){
		tigl::CCPACSWing& wing = GetConfiguration().GetWing(i);
		for(int j = 1; j <= wing.GetComponentSegmentCount();++j){
			tigl::CTiglAbstractSegment& segment = wing.GetComponentSegment(j);
			if(segment.GetUID() == wingUid.toStdString()){
				pComponentSegment = &(segment.GetLoft());
				break;
			}
		}
	}

	if(pComponentSegment)
		displayShape(*pComponentSegment);
	else {
		cerr << "Component segment \"" << wingUid.toStdString() << "\" not found" << endl;
	}
	QApplication::restoreOverrideCursor();
}

/*
 * Reads traingles from Mesh of shape and creates vertices and triangular faces
 */
void TIGLViewerDocument::createShapeTriangulation(const TopoDS_Shape& shape, TopoDS_Compound& compound){
    meshShape(shape, _settings.triangulationAccuracy());
    BRep_Builder builder;
    builder.MakeCompound(compound);

    TopExp_Explorer shellExplorer;
    TopExp_Explorer faceExplorer;
    for (shellExplorer.Init(shape, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next())
    {
        TopoDS_Shell shell = TopoDS::Shell(shellExplorer.Current());

        for (faceExplorer.Init(shell, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next())
        {
            TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
            TopLoc_Location location;
            Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
            if (triangulation.IsNull())
                continue;

            gp_Trsf nodeTransformation = location;
            const TColgp_Array1OfPnt& nodes = triangulation->Nodes();

            int index1, index2, index3;
            const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
            for (int j = triangles.Lower(); j <= triangles.Upper(); j++)
            {
                const Poly_Triangle& triangle = triangles(j);
                triangle.Get(index1, index2, index3);
                gp_Pnt point1 = nodes(index1).Transformed(nodeTransformation);
                gp_Pnt point2 = nodes(index2).Transformed(nodeTransformation);
                gp_Pnt point3 = nodes(index3).Transformed(nodeTransformation);

                BRepBuilderAPI_MakePolygon poly;
                poly.Add(point1);
                poly.Add(point2);
                poly.Add(point3);
                poly.Close();

                TopoDS_Face triangleFace = BRepBuilderAPI_MakeFace(poly.Wire());
                if(!triangleFace.IsNull())
                    builder.Add(compound, triangleFace);

                BRepBuilderAPI_MakeEdge edge1(point1, point2);
                BRepBuilderAPI_MakeEdge edge2(point2, point3);
                BRepBuilderAPI_MakeEdge edge3(point3, point1);
                if(edge1.IsDone())
                    builder.Add(compound, edge1);
                if(edge2.IsDone())
                    builder.Add(compound, edge2);
                if(edge3.IsDone())
                    builder.Add(compound, edge3);
            }
        }
    }
}

TiglCPACSConfigurationHandle TIGLViewerDocument::getCpacsHandle(void) const {
    return this->m_cpacsHandle;
}


