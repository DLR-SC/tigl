/*
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
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

#include <qnamespace.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>
#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>

#include <string.h>

#include <Handle_AIS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <TopoDS_Shell.hxx>

#include "TIGLViewerDocument.h"
#include "TIGLViewerInternal.h"
#include "TIGLViewerWindow.h"
#include "OCC_Point.h"
#include "OCC_Text.h"


TIGLViewerDocument::TIGLViewerDocument( QWidget *parentWidget, const Handle_AIS_InteractiveContext& ic )
{
	parent = parentWidget;
	myAISContext = ic;
	myOCC = ((TIGLViewerWindow*) parent)->getMyOCC();
}

void TIGLViewerDocument::writeToStatusBar(QString text)
{
	((TIGLViewerWindow*) parent)->statusBar()->showMessage(text);
}

char* TIGLViewerDocument::qstringToCstring(QString text)
{
	return _strdup((const char*)text.toLatin1());
}

void TIGLViewerDocument::openCpacsConfiguration(const QString fileName)
{
	QStringList configurations;

	myAISContext->SetDisplayMode(AIS_Shaded,Standard_False);

	TixiDocumentHandle tixiHandle = -1;

	ReturnCode tixiRet = tixiOpenDocument(qstringToCstring(fileName), &tixiHandle);
	if (tixiRet != SUCCESS) {
		//return FALSE;
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
	TiglReturnCode tiglRet;
	if (countRotorcrafts + countAircrafts == 1)
	{
		tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &m_cpacsHandle);
	} else
	{
		bool ok;
		QString item = QInputDialog::getItem(parent, tr("Select CPACS Configuration"),
											 tr("Available Configurations:"), configurations, 0, false, &ok);
		if (ok && !item.isEmpty())
		{
			tiglRet = tiglOpenCPACSConfiguration(tixiHandle, _strdup((const char*)item.toLatin1()), &m_cpacsHandle);
		}
	}

	if (tiglRet != TIGL_SUCCESS)
	{
		tixiCloseDocument(tixiHandle);
		m_cpacsHandle = -1;
		QMessageBox::critical(parent, tr("TIGL Error"),
								tr("Error opening a CPACS configuration"),
								QMessageBox::Abort );
	}
	drawAllFuselagesAndWings();
}


TIGLViewerDocument::~TIGLViewerDocument( )
{
	m_cpacsHandle = -1;
}

// Returns the CPACS configuration
tigl::CCPACSConfiguration& TIGLViewerDocument::GetConfiguration(void) const
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    return manager.GetConfiguration(m_cpacsHandle);
}


// a small helper when we just want to display a shape
Handle(AIS_Shape) TIGLViewerDocument::displayShape(TopoDS_Shape loft)
{
	Handle(AIS_Shape) shape = new AIS_Shape(loft);
	shape->SetColor(Quantity_NOC_BLUE2);
	myAISContext->Display(shape, Standard_True);
	return shape;
}


// Displays a point on the screen
void TIGLViewerDocument::DisplayPoint(gp_Pnt& aPoint,
                                  char* aText,
                                  Standard_Boolean UpdateViewer,
                                  Standard_Real anXoffset,
                                  Standard_Real anYoffset,
                                  Standard_Real aZoffset,
                                  Standard_Real TextScale)
{
    Handle(OCC_Point) aGraphicPoint = new OCC_Point(aPoint);
    myAISContext->Display(aGraphicPoint,UpdateViewer);
    Handle(OCC_Text) aGraphicText = new OCC_Text(aText, aPoint.X() + anXoffset,
    											 aPoint.Y() + anYoffset,
    											 aPoint.Z()+aZoffset);
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

	return QInputDialog::getItem(parent, tr("Select Wing"), tr("Available Wings:"), wings, 0, false, &ok);
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

	return QInputDialog::getItem(parent, tr("Select Wing Profile"), tr("Available Wing Profiles:"), wingProfiles, 0, false, &ok);
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
	return QInputDialog::getItem(parent, tr("Select Fuselage"), tr("Available Fuselages:"), fuselages, 0, false, &ok);
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

	return QInputDialog::getItem(parent, tr("Select Fuselage Profile"), tr("Available Fuselage Profiles:"), fuselageProfiles, 0, false, &ok);
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
			TopoDS_Shape loft = segment.GetLoft();

			// Transform by wing transformation
			loft = wing.GetWingTransformation().Transform(loft);
			displayShape(loft);
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
	}
}



void TIGLViewerDocument::drawWingProfiles()
{
	QString wingProfile = dlgGetWingProfileSelection();

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

void TIGLViewerDocument::drawWingOverlayProfilePoints()
{
	QString wingUid = dlgGetWingSelection();

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

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSFuselageProfile& profile = GetConfiguration().GetFuselageProfile(fuselageProfile.toStdString());
    TopoDS_Wire wire        = profile.GetWire(true);
    Handle(AIS_Shape) shape = new AIS_Shape(wire);
    shape->SetColor(Quantity_NOC_WHITE);
    myAISContext->Display(shape, Standard_True);

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


void TIGLViewerDocument::drawWing()
{
	QString wingUid = dlgGetWingSelection();
	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());

    myAISContext->EraseAll(Standard_False);

    for (int i = 1; i <= wing.GetSegmentCount(); i++)
    {
        // Draw segment loft
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
        TopoDS_Shape loft = segment.GetLoft();
        // Transform by wing transformation
        loft = wing.GetWingTransformation().Transform(loft);
        displayShape(loft);
    }
}



void TIGLViewerDocument::drawFuselage()
{
	QString fuselageUid = dlgGetFuselageSelection();
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

    myAISContext->EraseAll(Standard_False);

    for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
    {
        // Draw segment loft
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
        TopoDS_Shape loft = segment.GetLoft();
        // Transform by fuselage transformation
        loft = fuselage.GetFuselageTransformation().Transform(loft);
        Handle(AIS_Shape) shape = new AIS_Shape(loft);
        shape->SetColor(Quantity_NOC_BLUE2);
        myAISContext->Display(shape, Standard_False);
    }
}


void TIGLViewerDocument::drawWingTriangulation()
{
	QString wingUid = dlgGetWingSelection();
	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());

    //clear screen
    myAISContext->EraseAll(Standard_False);

	tigl::CCPACSWingSegment& firstSegment = (tigl::CCPACSWingSegment &) wing.GetSegment(1);
	TopoDS_Shape fusedWing = firstSegment.GetLoft();

    for (int i = 2; i <= wing.GetSegmentCount(); i++)
    {
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
        TopoDS_Shape loft = segment.GetLoft();

        TopExp_Explorer shellExplorer;
        TopExp_Explorer faceExplorer;

		fusedWing = BRepAlgoAPI_Fuse(fusedWing, loft);
    }

	BRepMesh::Mesh(fusedWing, 0.01);

	BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    TopExp_Explorer shellExplorer;
    TopExp_Explorer faceExplorer;
    for (shellExplorer.Init(fusedWing, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next())
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

                // Transform by wing transformation
                point1 = wing.GetWingTransformation().Transform(point1);
                point2 = wing.GetWingTransformation().Transform(point2);
                point3 = wing.GetWingTransformation().Transform(point3);

                BRepBuilderAPI_MakePolygon poly;
                poly.Add(point1);
                poly.Add(point2);
                poly.Add(point3);
                poly.Close();

                TopoDS_Face triangleFace = BRepBuilderAPI_MakeFace(poly.Wire());
                builder.Add(compound, triangleFace);

                BRepBuilderAPI_MakeEdge edge1(point1, point2);
                BRepBuilderAPI_MakeEdge edge2(point2, point3);
                BRepBuilderAPI_MakeEdge edge3(point3, point1);
                builder.Add(compound, edge1);
                builder.Add(compound, edge2);
                builder.Add(compound, edge3);
            }
        }
    }
    Handle(AIS_Shape) triangulation = new AIS_Shape(compound);
    myAISContext->SetDisplayMode(triangulation, 1);
    myAISContext->SetColor(triangulation, Quantity_NOC_BLUE2);
    myAISContext->Display(triangulation);
}


void TIGLViewerDocument::drawFuselageTriangulation()
{
	QString fuselageUid = dlgGetFuselageSelection();
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageUid.toStdString());

    //clear screen
    myAISContext->EraseAll(Standard_False);

	tigl::CCPACSWingSegment& firstSegment = (tigl::CCPACSWingSegment &) fuselage.GetSegment(1);
	TopoDS_Shape fusedWing = firstSegment.GetLoft();

    for (int i = 2; i <= fuselage.GetSegmentCount(); i++)
    {
        tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
        TopoDS_Shape loft = segment.GetLoft();

        TopExp_Explorer shellExplorer;
        TopExp_Explorer faceExplorer;

		fusedWing = BRepAlgoAPI_Fuse(fusedWing, loft);
    }

	BRepMesh::Mesh(fusedWing, 0.01);

	BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    TopExp_Explorer shellExplorer;
    TopExp_Explorer faceExplorer;
    for (shellExplorer.Init(fusedWing, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next())
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

                // Transform by wing transformation
                point1 = fuselage.GetFuselageTransformation().Transform(point1);
                point2 = fuselage.GetFuselageTransformation().Transform(point2);
                point3 = fuselage.GetFuselageTransformation().Transform(point3);

                BRepBuilderAPI_MakePolygon poly;
                poly.Add(point1);
                poly.Add(point2);
                poly.Add(point3);
                poly.Close();

                TopoDS_Face triangleFace = BRepBuilderAPI_MakeFace(poly.Wire());
                builder.Add(compound, triangleFace);

                BRepBuilderAPI_MakeEdge edge1(point1, point2);
                BRepBuilderAPI_MakeEdge edge2(point2, point3);
                BRepBuilderAPI_MakeEdge edge3(point3, point1);
                builder.Add(compound, edge1);
                builder.Add(compound, edge2);
                builder.Add(compound, edge3);
            }
        }
    }
    Handle(AIS_Shape) triangulation = new AIS_Shape(compound);
    myAISContext->SetDisplayMode(triangulation, 1);
    myAISContext->SetColor(triangulation, Quantity_NOC_BLUE2);
    myAISContext->Display(triangulation);
}


void TIGLViewerDocument::drawWingSamplePoints()
{
	QString wingUid = dlgGetWingSelection();
	tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingUid.toStdString());

    myAISContext->EraseAll(Standard_False);

    for (int segmentIndex = 1; segmentIndex <= wing.GetSegmentCount(); segmentIndex++)
    {
        // Draw segment loft
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(segmentIndex);
        TopoDS_Shape loft = segment.GetLoft();
        // Transform by wing transformation
        loft = wing.GetWingTransformation().Transform(loft);
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

                Handle(OCC_Point) aGraphicPoint = new OCC_Point(x, y, z);
                myAISContext->Display(aGraphicPoint, Standard_False);

                tiglWingGetLowerPoint( m_cpacsHandle,
										1,	// TODO: we need to implement that function to use UID instead of index!
										segmentIndex,
										eta,
										xsi,
										&x,
										&y,
										&z);

                aGraphicPoint = new OCC_Point(x, y, z);
                myAISContext->Display(aGraphicPoint, Standard_False);
            }
        }
    }
}


void TIGLViewerDocument::drawFuselageSamplePoints()
{
	QString fuselageUid = dlgGetFuselageSelection();
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

				Handle(OCC_Point) aGraphicPoint = new OCC_Point(x, y, z);
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

	Handle(OCC_Point) aGraphicPoint = new OCC_Point(x, y, z);
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
			TopoDS_Shape loft = segment.GetLoft();
			// Transform by wing transformation
			loft = wing.GetWingTransformation().Transform(loft);
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

					Handle(OCC_Point) aGraphicPoint = new OCC_Point(x, y, z);
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

					aGraphicPoint = new OCC_Point(x, y, z);
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

					Handle(OCC_Point) aGraphicPoint = new OCC_Point(x, y, z);
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
	}
}

void TIGLViewerDocument::exportMeshedWingSTL()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	//QString wingUid = dlgGetWingSelection(); // TODO: switch to uid based export

	writeToStatusBar(tr("Saving meshed Wing as STL file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedWingSTL(m_cpacsHandle, 1 /*wingUid*/, qstringToCstring(fileName), 0.01);
		QApplication::restoreOverrideCursor();
	}
}



void TIGLViewerDocument::exportMeshedFuselageSTL()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	//QString fuselageUid = dlgGetFuselageSelection(); // TODO: switch to uid based export

	writeToStatusBar(tr("Saving meshed Fuselage as STL file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedFuselageSTL(m_cpacsHandle, 1 /*fuselageUid*/, qstringToCstring(fileName), 0.01);
		QApplication::restoreOverrideCursor();
	}
}


void TIGLViewerDocument::exportMeshedWingVTK()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	//QString wingUid = dlgGetWingSelection(); // TODO: switch to uid based export

	writeToStatusBar(tr("Saving meshed Wing as VTK file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedWingVTKByIndex(m_cpacsHandle, 1 /*wingUid*/, qstringToCstring(fileName), 0.1);
		QApplication::restoreOverrideCursor();
	}
}


void TIGLViewerDocument::exportMeshedWingVTKsimple()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	QString wingUid = dlgGetWingSelection();

	writeToStatusBar(tr("Saving meshed Wing as simple VTK file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedWingVTKSimpleByUID(m_cpacsHandle, qstringToCstring(wingUid), qstringToCstring(fileName), 0.1);
		QApplication::restoreOverrideCursor();
	}
}



void TIGLViewerDocument::exportMeshedFuselageVTK()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;
	TIGLViewerInputOutput writer;

	//QString wingUid = dlgGetWingSelection(); // TODO: switch to uid based export

	writeToStatusBar(tr("Saving meshed Fuselage as VTK file with TIGL..."));

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedFuselageVTKByIndex(m_cpacsHandle, 1 /*wingUid*/, qstringToCstring(fileName), 0.1);
		QApplication::restoreOverrideCursor();
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

	fileName = QFileDialog::getSaveFileName(parent, tr("Save as..."), myLastFolder, tr("Export STL(*.stl)"));

	if (!fileName.isEmpty())
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		TiglReturnCode err = tiglExportMeshedFuselageVTKSimpleByUID(m_cpacsHandle, qstringToCstring(fuselageUid), qstringToCstring(fileName), 0.1);
		QApplication::restoreOverrideCursor();
	}
}



