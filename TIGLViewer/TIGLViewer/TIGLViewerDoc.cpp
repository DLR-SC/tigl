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
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "stdafx.h"
#include "TIGLViewer.h"
#include "TIGLViewerDoc.h"
#include "TIGLViewerView.h"
#include "WingProfileDialog.h"
#include "FuselageProfileDialog.h"
#include "WingDialog.h"
#include "WingUIDDialog.h"
#include "FuselageDialog.h"
#include "FuselageUIDDialog.h"
#include "ConfigurationDialog.h"
#include "InputBox.h"
#include "ISession_Point.h"
#include "CTiglIntersectionCalculation.h"
#include "ISession_Text.h"
#include <AIS_ListOfInteractive.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <AIS_Shape.hxx>
#include <gp_Lin.hxx> 
#include <Geom_Line.hxx>
#include <gce_MakeLin.hxx>
#include <GC_MakeSegment.hxx>
#include <StlAPI_Writer.hxx>
#include "TopoDS.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "Geom2d_Line.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "BRep_Tool.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI.hxx"
#include "gp_Pln.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "TopLoc_Location.hxx"
#include "Poly_Triangulation.hxx"
#include "BRep_Builder.hxx"
#include "Poly_Array1OfTriangle.hxx"
#include "gp_Trsf.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "ShapeFix_Wireframe.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepMesh.hxx"
#include "BRepTools.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "BRepCheck_Analyzer.hxx"
#include "TopoDS_Compound.hxx"
#include "CTiglTransformation.h"
#include "TopoDS_Solid.hxx"
#include "ShapeAnalysis_ShapeContents.hxx"
#include "BRepBuilderAPI_Transform.hxx"


// CTIGLViewerDoc
IMPLEMENT_DYNCREATE(CTIGLViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CTIGLViewerDoc, OCC_3dBaseDoc)
    ON_COMMAND(ID_SHOW_WING_PROFILES, OnShowWingProfiles)
    ON_COMMAND(ID_SHOW_WINGS, OnShowWings)
    ON_COMMAND(ID_SHOW_WING_TRIANGULATION, OnShowWingTriangulation)
    ON_COMMAND(ID_SHOW_WING_POINTS, OnShowWingPoints)
    ON_COMMAND(ID_SHOW_ALL_POINTS, OnShowAllPoints)
    ON_COMMAND(ID_CPACS_DRAWOVERLAYCPACSPROFILEPOINTS, OnDrawOverlayCPACSProfilePoints)
    ON_COMMAND(ID_SHOW_ALL, OnShowAll)
    ON_COMMAND(ID_EXPORT_AS_IGES, OnExportAsIges)
	ON_COMMAND(ID_EXPORT_FUSED_IGES, OnExportAsFusedIges)
	ON_COMMAND(ID_EXPORTASSTL_WINGBYINDEX, OnExportMeshedWingSTL)
	ON_COMMAND(ID_EXPORTASSTL_FUSELAGEBYINDEX, OnExportMeshedFuselageSTL)
	ON_COMMAND(ID_EXPORTASVTK_WINGBYINDEX, OnExportMeshedWingVTKIndex)
	ON_COMMAND(ID_EXPORTASVTK_WINGBYUID, OnExportMeshedWingVTKSimple)
	ON_COMMAND(ID_EXPORTASVTK_FUSELAGEBYINDEX, OnExportMeshedFuselageVTKIndex)
	ON_COMMAND(ID_EXPORTASVTK_FUSELAGEBYUID, OnExportMeshedFuselageVTKSimple)
    ON_COMMAND(ID_INTERPOLATE_BSPLINE_WIRE, OnInterpolateBsplineWire)
    ON_COMMAND(ID_INTERPOLATE_LINEAR_WIRE, OnInterpolateLinearWire)
    ON_COMMAND(ID_APPROXIMATE_BSPLINE_WIRE, OnApproximateBsplineWire)
    ON_UPDATE_COMMAND_UI(ID_INTERPOLATE_BSPLINE_WIRE, OnUpdateInterpolateBsplineWire)
    ON_UPDATE_COMMAND_UI(ID_INTERPOLATE_LINEAR_WIRE, OnUpdateInterpolateLinearWire)
    ON_UPDATE_COMMAND_UI(ID_APPROXIMATE_BSPLINE_WIRE, OnUpdateApproximateBsplineWire)
    ON_COMMAND(ID_SHOW_FUSELAGE_PROFILES, OnShowFuselageProfiles)
    ON_COMMAND(ID_SHOW_FUSELAGES, OnShowFuselages)
    ON_COMMAND(ID_SHOW_FUSELAGE_TRIANGULATION, OnShowFuselageTriangulation)
    ON_COMMAND(ID_SHOW_FUSELAGE_POINTS, OnShowFuselagePoints)
	ON_COMMAND(ID_SHOW_FUSELAGE_POINTS_ANGLE, OnShowFuselagePointsAngle)
	ON_COMMAND(ID_CPACS_DRAW_MINIMUM_DISTANCE_GROUND, OnDrawMinumumDistanceToGround)
	ON_COMMAND(ID_CPACS_DRAWFUSEDFUSELAGES, OnShowFusedFuselages)
	ON_COMMAND(ID_CPACS_DRAWFUSEDALL, OnShowFusedAll)		
	ON_COMMAND(ID_CPACS_DRAWFUSEDWINGS, OnShowFusedWings)
	ON_COMMAND(ID_CPACS_SHOWINTERSECTIONLINE, OnShowIntersectionLine)
END_MESSAGE_MAP()


#define CPACS_XPATH_AIRCRAFT 			"/cpacs/vehicles/aircraft"
#define CPACS_XPATH_ROTORCRAFT 			"/cpacs/vehicles/rotorcraft"
#define CPACS_XPATH_AIRCRAFT_MODEL 		"/cpacs/vehicles/aircraft/model"
#define CPACS_XPATH_ROTORCRAFT_MODEL 	"/cpacs/vehicles/rotorcraft/model"

CTIGLViewerDoc::CTIGLViewerDoc()
{
    myAISContext->SetDisplayMode(AIS_Shaded,Standard_False);
    m_cpacsHandle = -1;
}

CTIGLViewerDoc::~CTIGLViewerDoc()
{
}

// CTIGLViewerDoc-Befehle
BOOL CTIGLViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    if (!OCC_3dBaseDoc::OnOpenDocument(lpszPathName))
        return FALSE;

    TixiDocumentHandle tixiHandle = -1;

	std::cerr << "Used TIXI version: " << tixiGetVersion() << std::endl;
	std::cerr << "Used TIGL version: " << tiglGetVersion() << std::endl;

    ReturnCode tixiRet = tixiOpenDocument(static_cast<char*>(const_cast<LPTSTR>(lpszPathName)), &tixiHandle);
    if (tixiRet != SUCCESS)
        return FALSE;

	// read configuration names
	int countAircrafts = 0;
	int countRotorcrafts = 0;
	tixiRet = tixiGetNamedChildrenCount( tixiHandle, CPACS_XPATH_AIRCRAFT, "model", &countAircrafts );
	tixiRet = tixiGetNamedChildrenCount( tixiHandle, CPACS_XPATH_ROTORCRAFT, "model", &countRotorcrafts );
	std::vector<char*> configurationsVector(countAircrafts + countRotorcrafts); 
	int listIndex = 0;
	for(int i=0; i<countAircrafts; i++)
	{
		char *text;
		tixiRet = tixiGetTextAttribute( tixiHandle, CPACS_XPATH_AIRCRAFT_MODEL, "uID", &text);
		configurationsVector[listIndex] = text;
		listIndex++;
	}
	for(int i=0; i<countRotorcrafts; i++)
	{
		char *text;
		tixiRet = tixiGetTextAttribute( tixiHandle, CPACS_XPATH_ROTORCRAFT_MODEL, "uID", &text);
		configurationsVector[listIndex] = text;
		listIndex++;
	}

	CConfigurationDialog dlg(&configurationsVector);
     if (dlg.DoModal() != IDOK)
       return FALSE;

    int index = dlg.GetConfigurationIndex();
    if (index < 0)
        return FALSE;

	// open correct cpacs configuration
    TiglReturnCode tiglRet = tiglOpenCPACSConfiguration(tixiHandle, configurationsVector.at(index), &m_cpacsHandle);
    if (tiglRet != TIGL_SUCCESS)
    {
        tixiCloseDocument(tixiHandle);
        return FALSE;
    }

    OnShowAll();
    
    return TRUE;
}

// Called by the MFC framework before destroying or reusing a document.
void CTIGLViewerDoc::DeleteContents()
{
    myAISContext->EraseAll(Standard_False);

    try {
        if (m_cpacsHandle != -1) 
        {
            tigl::CCPACSConfiguration& config = GetConfiguration();
            TixiDocumentHandle tixiHandle = config.GetTixiDocumentHandle();
            tiglCloseCPACSConfiguration(m_cpacsHandle);
            tixiCloseDocument(tixiHandle);
            m_cpacsHandle = -1;
        }
    }
    catch (...) { }

    OCC_3dBaseDoc::DeleteContents();
}


// Popups wings selection dialog and draws the profile point from the CPACS file
void CTIGLViewerDoc::OnDrawOverlayCPACSProfilePoints()
{
	CWingDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetWingIndex() + 1;
    if (index < 1)
        return;

    tigl::CCPACSWing& wing = GetConfiguration().GetWing(index);

    for (int i = 1; i <= wing.GetSegmentCount(); i++)
    {
        // Draw inner profile points
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
		std::vector<tigl::CTiglPoint*> innerPoints = segment.GetRawInnerProfilePoints();
		for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < innerPoints.size(); i++) 
		{
			gp_Pnt pnt = innerPoints[i]->Get_gp_Pnt();
			pnt = wing.GetWingTransformation().Transform(pnt);
			Handle(ISession_Point) aGraphicPoint = new ISession_Point(pnt.X(), pnt.Y(), pnt.Z());
			myAISContext->Display(aGraphicPoint, Standard_False);
		}

		// Draw outer profile points
		std::vector<tigl::CTiglPoint*> outerPoints = segment.GetRawOuterProfilePoints();
		for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < outerPoints.size(); i++) 
		{
			gp_Pnt pnt = outerPoints[i]->Get_gp_Pnt();			
			pnt = wing.GetWingTransformation().Transform(pnt);
			Handle(ISession_Point) aGraphicPoint = new ISession_Point(pnt.X(), pnt.Y(), pnt.Z());
			myAISContext->Display(aGraphicPoint, Standard_False);
		}
    }
}


void CTIGLViewerDoc::OnShowWingProfiles()
{
    CWingProfileDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetProfileIndex() + 1;
    if (index < 1)
        return;

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSWingProfile& profile = GetConfiguration().GetWingProfile(index);
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

    DrawXYZAxis();

}

void CTIGLViewerDoc::OnShowFuselageProfiles()
{
    CFuselageProfileDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetProfileIndex() + 1;
    if (index < 1)
        return;

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSFuselageProfile& profile = GetConfiguration().GetFuselageProfile(index);
    TopoDS_Wire wire        = profile.GetWire(false);
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

    DrawXYZAxis();
}

void CTIGLViewerDoc::OnShowWings()
{
    CWingDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetWingIndex() + 1;
    if (index < 1)
        return;

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSWing& wing = GetConfiguration().GetWing(index);

    for (int i = 1; i <= wing.GetSegmentCount(); i++)
    {
        // Draw segment loft
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
        TopoDS_Shape loft = segment.GetLoft();
        // Transform by wing transformation
        loft = wing.GetWingTransformation().Transform(loft);
        Handle(AIS_Shape) shape = new AIS_Shape(loft);
        shape->SetColor(Quantity_NOC_BLUE2);
        myAISContext->Display(shape, Standard_False);
    }

    DrawXYZAxis();
}

void CTIGLViewerDoc::OnShowFuselages()
{
    CFuselageDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetFuselageIndex() + 1;
    if (index < 1)
        return;

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(index);


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

    DrawXYZAxis();
}

void CTIGLViewerDoc::OnShowWingTriangulation()
{
    CWingDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetWingIndex() + 1;
    if (index < 1)
        return;

    //clear screen
    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSWing& wing = GetConfiguration().GetWing(index);
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

    DrawXYZAxis();
}



void CTIGLViewerDoc::OnShowFuselageTriangulation()
{
    CFuselageDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetFuselageIndex() + 1;
    if (index < 1)
        return;

    // clean screen
    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(index);
	tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(1);
	TopoDS_Shape fusedFuselage = segment.GetLoft();

    for (int i = 2; i <= fuselage.GetSegmentCount(); i++)
    {
		tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
        // Draw segment loft
        const TopoDS_Shape loft = segment.GetLoft();        
		fusedFuselage = BRepAlgoAPI_Fuse(fusedFuselage, loft);
	}

    BRepMesh::Mesh(fusedFuselage, 0.01);

        // Draw segment loft      
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);

        TopExp_Explorer shellExplorer;
        TopExp_Explorer faceExplorer;
        for (shellExplorer.Init(fusedFuselage, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next())
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

                    // Transform by fuselage transformation
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

    DrawXYZAxis();
}

void CTIGLViewerDoc::OnShowWingPoints()
{
    CWingDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int wingIndex = dlg.GetWingIndex() + 1;
    if (wingIndex < 1)
        return;

    myAISContext->EraseAll(Standard_False);

    tigl::CCPACSWing& wing = GetConfiguration().GetWing(wingIndex);

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
                TiglReturnCode res = tiglWingGetUpperPoint(
                    m_cpacsHandle,
                    wingIndex,
                    segmentIndex,
                    eta,
                    xsi,
                    &x,
                    &y,
                    &z);
                if (res != TIGL_SUCCESS)
                    TRACE("tiglWingGetUpperPoint failed, Error code = %d\n", res);

                Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
                myAISContext->Display(aGraphicPoint, Standard_False);

                res = tiglWingGetLowerPoint(
                    m_cpacsHandle,
                    wingIndex,
                    segmentIndex,
                    eta,
                    xsi,
                    &x,
                    &y,
                    &z);
                if (res != TIGL_SUCCESS)
                    TRACE("tiglWingGetLowerPoint failed, Error code = %d\n", res);

                aGraphicPoint = new ISession_Point(x, y, z);
                myAISContext->Display(aGraphicPoint, Standard_False);
            }
        }
    }

    DrawXYZAxis();
}

void CTIGLViewerDoc::OnShowFuselagePoints()
{
    CFuselageDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int fuselageIndex = dlg.GetFuselageIndex() + 1;
    if (fuselageIndex < 1)
        return;

    myAISContext->EraseAll(Standard_False);

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
                TiglReturnCode res = tiglFuselageGetPoint(
                    m_cpacsHandle,
                    fuselageIndex,
                    segmentIndex,
                    eta,
                    zeta,
                    &x,
                    &y,
                    &z);
                if (res != TIGL_SUCCESS)
                    TRACE("tiglFuselageGetPoint failed, Error code = %d\n", res);

                Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
                myAISContext->Display(aGraphicPoint, Standard_False);
            }
        }
    }

    DrawXYZAxis();
}



void CTIGLViewerDoc::OnShowFuselagePointsAngle()
{
	int fuselageIndex = 1;
	int segmentIndex = 15;
	double y_cs = 0.0;
	double z_cs = 0.0;
	double angle = 45.0; // turn through 45 degrees 
	double x, y, z;

	myAISContext->EraseAll(Standard_False);
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(fuselageIndex);

	// Draw segment loft
	tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(segmentIndex);
	TopoDS_Shape loft = segment.GetLoft();
	// Transform by fuselage transformation
	loft = fuselage.GetFuselageTransformation().Transform(loft);
	Handle(AIS_Shape) shape = new AIS_Shape(loft);
	shape->SetColor(Quantity_NOC_BLUE2);
	myAISContext->Display(shape, Standard_True);

	// Display the intersection point		
	TiglReturnCode res2 = tiglFuselageGetPointAngle(
		m_cpacsHandle,
		fuselageIndex,
		segmentIndex,
		0.5,
		angle,
		&x,
		&y,
		&z);
	if (res2 != TIGL_SUCCESS)
		TRACE("tiglFuselageGetPointAngle failed, Error code = %d\n", res2);

	Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
	myAISContext->Display(aGraphicPoint, Standard_False);

	DrawXYZAxis();
}

void CTIGLViewerDoc::OnInterpolateBsplineWire()
{
    tiglUseAlgorithm(TIGL_INTERPOLATE_BSPLINE_WIRE);
}

void CTIGLViewerDoc::OnInterpolateLinearWire()
{
    tiglUseAlgorithm(TIGL_INTERPOLATE_LINEAR_WIRE);
}

void CTIGLViewerDoc::OnApproximateBsplineWire()
{
    tiglUseAlgorithm(TIGL_APPROXIMATE_BSPLINE_WIRE);
}

void CTIGLViewerDoc::OnUpdateInterpolateBsplineWire(CCmdUI *pCmdUI)
{
    tigl::CTiglAlgorithmManager& manager = tigl::CTiglAlgorithmManager::GetInstance();
    const tigl::ITiglWireAlgorithm& algorithm = manager.GetWireAlgorithm();
    pCmdUI->SetCheck(algorithm.GetAlgorithmCode() == TIGL_INTERPOLATE_BSPLINE_WIRE);
}

void CTIGLViewerDoc::OnUpdateInterpolateLinearWire(CCmdUI *pCmdUI)
{
    tigl::CTiglAlgorithmManager& manager = tigl::CTiglAlgorithmManager::GetInstance();
    const tigl::ITiglWireAlgorithm& algorithm = manager.GetWireAlgorithm();
    pCmdUI->SetCheck(algorithm.GetAlgorithmCode() == TIGL_INTERPOLATE_LINEAR_WIRE);
}

void CTIGLViewerDoc::OnUpdateApproximateBsplineWire(CCmdUI *pCmdUI)
{
    tigl::CTiglAlgorithmManager& manager = tigl::CTiglAlgorithmManager::GetInstance();
    const tigl::ITiglWireAlgorithm& algorithm = manager.GetWireAlgorithm();
    pCmdUI->SetCheck(algorithm.GetAlgorithmCode() == TIGL_APPROXIMATE_BSPLINE_WIRE);
}

// Returns the CPACS configuration
tigl::CCPACSConfiguration& CTIGLViewerDoc::GetConfiguration(void) const
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    return manager.GetConfiguration(m_cpacsHandle);
}

// Displays a point on the screen
void CTIGLViewerDoc::DisplayPoint(gp_Pnt& aPoint,
                                  char* aText,
                                  Standard_Boolean UpdateViewer,
                                  Standard_Real anXoffset,
                                  Standard_Real anYoffset,
                                  Standard_Real aZoffset,
                                  Standard_Real TextScale)
{
    Handle(ISession_Point) aGraphicPoint = new ISession_Point(aPoint);
    myAISContext->Display(aGraphicPoint,UpdateViewer);
    Handle(ISession_Text)  aGraphicText = new ISession_Text(aText,aPoint.X()+anXoffset,aPoint.Y()+anYoffset,aPoint.Z()+aZoffset);
    aGraphicText->SetScale(TextScale);
    myAISContext->Display(aGraphicText,UpdateViewer);
}

void CTIGLViewerDoc::DrawXYZAxis(void)
{
    // X
    Handle(Geom_TrimmedCurve) line = GC_MakeSegment(gp_Pnt(0.0, 0.0, 0.0), gp_Pnt(1.0, 0.0, 0.0));
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(line);
    Handle(AIS_Shape) lineShape = new AIS_Shape(edge);
    lineShape->SetColor(Quantity_NOC_WHITE);
    myAISContext->Display(lineShape, Standard_True);
    DisplayPoint(gp_Pnt(1.0, 0.0, 0.0), "X", Standard_True, 0.0, 0.0, 0.0, 2.0);

    // Y
    line = GC_MakeSegment(gp_Pnt(0.0, 0.0, 0.0), gp_Pnt(0.0, 1.0, 0.0));
    edge = BRepBuilderAPI_MakeEdge(line);
    lineShape = new AIS_Shape(edge);
    lineShape->SetColor(Quantity_NOC_WHITE);
    myAISContext->Display(lineShape, Standard_True);
    DisplayPoint(gp_Pnt(0.0, 1.0, 0.0), "Y", Standard_True, 0.0, 0.0, 0.0, 2.0);

    // Z
    line = GC_MakeSegment(gp_Pnt(0.0, 0.0, 0.0), gp_Pnt(0.0, 0.0, 1.0));
    edge = BRepBuilderAPI_MakeEdge(line);
    lineShape = new AIS_Shape(edge);
    lineShape->SetColor(Quantity_NOC_WHITE);
    myAISContext->Display(lineShape, Standard_True);
    DisplayPoint(gp_Pnt(0.0, 0.0, 1.0), "Z", Standard_True, 0.0, 0.0, 0.0, 2.0);
}


void CTIGLViewerDoc::OnShowAllPoints()
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
                    TiglReturnCode res = tiglWingGetUpperPoint(
                        m_cpacsHandle,
                        wingIndex,
                        segmentIndex,
                        eta,
                        xsi,
                        &x,
                        &y,
                        &z);
                    if (res != TIGL_SUCCESS)
                        TRACE("tiglWingGetUpperPoint failed, Error code = %d\n", res);

                    Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
                    myAISContext->Display(aGraphicPoint,Standard_False);
                    //text << "PT(" << xsi << ", " << eta << ")";
                    //text << "(" << x << ", " << y << ", " << z << ")";
                    //DisplayPoint(gp_Pnt(x, y, z), const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
                    //text.str("");

                    res = tiglWingGetLowerPoint(
                        m_cpacsHandle,
                        wingIndex,
                        segmentIndex,
                        eta,
                        xsi,
                        &x,
                        &y,
                        &z);
                    if (res != TIGL_SUCCESS)
                        TRACE("tiglWingGetLowerPoint failed, Error code = %d\n", res);

                    aGraphicPoint = new ISession_Point(x, y, z);
                    myAISContext->Display(aGraphicPoint,Standard_False);
                    //text << "PT(" << xsi << ", " << eta << ")";
                    //text << "(" << x << ", " << y << ", " << z << ")";
                    //DisplayPoint(gp_Pnt(x, y, z), const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);
                    //text.str("");
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
                    TiglReturnCode res = tiglFuselageGetPoint(
                        m_cpacsHandle,
                        fuselageIndex,
                        segmentIndex,
                        eta,
                        zeta,
                        &x,
                        &y,
                        &z);
                    if (res != TIGL_SUCCESS)
                        TRACE("tiglFuselageGetPoint failed, Error code = %d\n", res);

                    Handle(ISession_Point) aGraphicPoint = new ISession_Point(x, y, z);
                    myAISContext->Display(aGraphicPoint,Standard_False);
                }
            }
        }
    }

    DrawXYZAxis();
}


void CTIGLViewerDoc::OnShowAll()
{
    myAISContext->EraseAll(Standard_False);

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

            Handle(AIS_Shape) shape = new AIS_Shape(loft);
            shape->SetColor(Quantity_NOC_BLUE2);
            myAISContext->Display(shape, Standard_True);
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

            Handle(AIS_Shape) shape = new AIS_Shape(loft);
            shape->SetColor(Quantity_NOC_BLUE2);
            myAISContext->Display(shape, Standard_True);
        }
    }

    DrawXYZAxis();
}


void CTIGLViewerDoc::OnExportAsIges()
{
    CFileDialog dlg(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "IGES Files (*.iges )|*.iges;|IGES Files (*.igs )| *.igs;||", NULL ); 

    if (dlg.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlg.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
        TiglReturnCode err = tiglExportIGES(m_cpacsHandle, filenamePtr);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting as IGES, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}



void CTIGLViewerDoc::OnExportMeshedWingSTL()
{
	CWingDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetWingIndex() + 1;
    if (index < 1)
        return;

    CFileDialog dlgF(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "STL Files (*.stl )|*.stl;||", NULL ); 

    if (dlgF.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlgF.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
        TiglReturnCode err = tiglExportMeshedWingSTL(m_cpacsHandle, index, filenamePtr, 0.01);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting wing as STL, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}

void CTIGLViewerDoc::OnExportMeshedFuselageSTL()
{
	CFuselageDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetFuselageIndex() + 1;
    if (index < 1)
        return;

    CFileDialog dlgF(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "STL Files (*.stl )|*.stl;||", NULL ); 

    if (dlgF.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlgF.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
        TiglReturnCode err = tiglExportMeshedFuselageSTL(m_cpacsHandle, index, filenamePtr, 0.01);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting fuselage as STL, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}



void CTIGLViewerDoc::OnExportMeshedWingVTKIndex()
{
	CWingDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetWingIndex() + 1;
    if (index < 1)
        return;

    CFileDialog dlgF(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "VTK Files (*.vtp )|*.vtp;||", NULL ); 

    if (dlgF.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlgF.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
        TiglReturnCode err = tiglExportMeshedWingVTKByIndex(m_cpacsHandle, index, filenamePtr, 0.1);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting wing as VTK, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}




void CTIGLViewerDoc::OnExportMeshedWingVTKSimple()
{
	CWingUIDDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

	std::string uid = dlg.GetWingUID();

    CFileDialog dlgF(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "VTK Files (*.vtp )|*.vtp;||", NULL ); 

    if (dlgF.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlgF.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
		char* uidPtr = const_cast<char*>(uid.c_str());
        TiglReturnCode err = tiglExportMeshedWingVTKSimpleByUID(m_cpacsHandle, uidPtr, filenamePtr, 0.1);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting wing as VTK, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}

void CTIGLViewerDoc::OnExportMeshedFuselageVTKIndex()
{
	CFuselageDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetFuselageIndex() + 1;
    if (index < 1)
        return;

    CFileDialog dlgF(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "VTK Files (*.vtp )|*.vtp;||", NULL ); 

    if (dlgF.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlgF.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
        TiglReturnCode err = tiglExportMeshedFuselageVTKByIndex(m_cpacsHandle, index, filenamePtr, 0.1);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting fuselage as VTK, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}



void CTIGLViewerDoc::OnExportMeshedFuselageVTKSimple()
{
	CFuselageUIDDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

	std::string uid = dlg.GetFuselageUID();
    
	CFileDialog dlgF(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "VTK Files (*.vtp )|*.vtp;||", NULL ); 

    if (dlgF.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlgF.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
		char* uidPtr = const_cast<char*>(uid.c_str());
        TiglReturnCode err = tiglExportMeshedFuselageVTKSimpleByUID(m_cpacsHandle, uidPtr, filenamePtr, 0.1);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting fuselage as VTK, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}



void CTIGLViewerDoc::OnExportAsFusedIges()
{
    CFileDialog dlg(FALSE,_T("*.iges"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "IGES Files (*.iges )|*.iges;|IGES Files (*.igs )| *.igs;||", NULL ); 

    if (dlg.DoModal() == IDOK)  
    { 
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        std::string filename(dlg.GetPathName());
        char* filenamePtr = const_cast<char*>(filename.c_str());
        TiglReturnCode err = tiglExportFusedWingFuselageIGES(m_cpacsHandle, filenamePtr);
        SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        if (err != TIGL_SUCCESS)
        {
            std::ostringstream str;
            str << "Error while exporting as IGES, Error code = " << err;
            AfxMessageBox(str.str().c_str(), MB_ICONERROR | MB_OK);
        }
    }
}

void CTIGLViewerDoc::PrintContents (TopoDS_Shape geom)
{
  ShapeAnalysis_ShapeContents cont;
  cont.Clear();
  cont.Perform(geom);

  cout << "OCC CONTENTS" << endl;
  cout << "============" << endl;
  cout << "SOLIDS   : " << cont.NbSolids() << endl;
  cout << "SHELLS   : " << cont.NbShells() << endl;
  cout << "FACES    : " << cont.NbFaces() << endl;
  cout << "WIRES    : " << cont.NbWires() << endl;
  cout << "EDGES    : " << cont.NbEdges() << endl;
  cout << "VERTICES : " << cont.NbVertices() << endl;

  TopExp_Explorer e;
  int count = 0;
  for (e.Init(geom, TopAbs_COMPOUND); e.More(); e.Next())
    count++;
  cout << "Compounds: " << count << endl;

  count = 0;
  for (e.Init(geom, TopAbs_COMPSOLID); e.More(); e.Next())
    count++;
  cout << "CompSolids: " << count << endl;

  cout << endl;

}


void CTIGLViewerDoc::OnShowFusedFuselages()
{
    CFuselageDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetFuselageIndex() + 1;
    if (index < 1)
        return;

    myAISContext->EraseAll(Standard_False);
    tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(index);
	TopoDS_Shape fusedFuselage = fuselage.GetLoft();

    Handle(AIS_Shape) shape = new AIS_Shape(fusedFuselage);
    shape->SetColor(Quantity_NOC_BLUE2);
    myAISContext->Display(shape, Standard_False);   

    DrawXYZAxis();
}


void CTIGLViewerDoc::OnShowFusedWings()
{
    CWingDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;

    int index = dlg.GetWingIndex() + 1;
    if (index < 1)
        return;

    myAISContext->EraseAll(Standard_False);
    tigl::CCPACSWing& wing = GetConfiguration().GetWing(index);
	TopoDS_Shape fusedWing = wing.GetLoft();

    Handle(AIS_Shape) shape = new AIS_Shape(fusedWing);
    shape->SetColor(Quantity_NOC_BLUE2);
    myAISContext->Display(shape, Standard_False);   

    DrawXYZAxis();
}


void CTIGLViewerDoc::OnShowFusedAll()
{
	myAISContext->EraseAll(Standard_False);
	TopoDS_Shape airplane = GetConfiguration().GetFusedAirplane();

	Handle(AIS_Shape) shape = new AIS_Shape(airplane);
    shape->SetColor(Quantity_NOC_BLUE2);
    myAISContext->Display(shape, Standard_False);   

    DrawXYZAxis();
}



// Draws a point where the distance between the selected fuselage and the ground is at minimum.
// The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
void CTIGLViewerDoc::OnDrawMinumumDistanceToGround()
{
	double axisPntX = 0.0;
	double axisPntY = 0.0;
	double axisPntZ = 0.0;
	double axisDirX = 0.0;
	double axisDirY = 0.0;
	double axisDirZ = 0.0;
	double angle = 0.0;

	myAISContext->EraseAll(Standard_False);

	// Get the right fuselage	
	CFuselageUIDDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;
	std::string uid = dlg.GetFuselageUID();
	tigl::CCPACSFuselage& fuselage = GetConfiguration().GetFuselage(uid);
	TopoDS_Shape fusedFuselage = fuselage.GetLoft();

	// get all values for the axis
	InputBoxHelper ibox(AfxGetMainWnd()->m_hWnd);
	
	if (ibox.DoModal("DrawMinumumDistanceToGround", "Please specifiy the X-value of the axis on which the aircraft should be rotated.")) {
			axisPntX = atof(ibox.Text);
	}

	if (ibox.DoModal("DrawMinumumDistanceToGround", "Please specifiy the Y-value of the axis on which the aircraft should be rotated.")) {
		axisPntY = atof(ibox.Text);
	}

	if (ibox.DoModal("DrawMinumumDistanceToGround", "Please specifiy the Z-value of the axis on which the aircraft should be rotated.")) {
		axisPntZ = atof(ibox.Text);
	}

	// get direction
	if (ibox.DoModal("DrawMinumumDistanceToGround", "Please specifiy the X-value of the direction of the axis on which the aircraft should be rotated.")) {
			axisDirX = atof(ibox.Text);
	}

	if (ibox.DoModal("DrawMinumumDistanceToGround", "Please specifiy the Y-value of the direction of the axis on which the aircraft should be rotated.")) {
			axisDirY = atof(ibox.Text);
	}
	
	if (ibox.DoModal("DrawMinumumDistanceToGround", "Please specifiy the Z-value of the direction of the axis on which the aircraft should be rotated.")) {
			axisDirZ = atof(ibox.Text);
	}

	// get the angle	
	if (ibox.DoModal("DrawMinumumDistanceToGround", "Please specifiy the angle (in degree) on which the aircraft should be rotated.")) {
			angle = atof(ibox.Text);
	}


	
	// Definition of the axis of rotation
	 gp_Ax1 RAxis(gp_Pnt(axisPntX, axisPntY, axisPntZ), gp_Dir(axisDirX, axisDirY, axisDirZ));

	Handle(Geom_Line) aGeomAxis= new Geom_Line(RAxis);
	Handle(Geom_TrimmedCurve) trimmedLine = GC_MakeSegment(RAxis, -20 * 0.2, 20 * 1.2);
    TopoDS_Edge le_te_edge = BRepBuilderAPI_MakeEdge(trimmedLine);
    Handle(AIS_Shape) lineShape = new AIS_Shape(le_te_edge);
    lineShape->SetColor(Quantity_NOC_GOLD /*Quantity_NOC_WHITE*/);
    myAISContext->Display(lineShape, Standard_True);

	
	// now rotate fuselage
	gp_Trsf myTrsf;
	myTrsf.SetRotation(RAxis, angle *PI / 180);
	BRepBuilderAPI_Transform xform(fusedFuselage, myTrsf);
	fusedFuselage = xform.Shape();

	// draw rotated fuselage
	Handle(AIS_Shape) fuselageShape = new AIS_Shape(fusedFuselage);
    fuselageShape->SetColor(Quantity_NOC_BLUE2);
    myAISContext->Display(fuselageShape, Standard_False);   


	//build cutting plane for intersection
    gp_Pnt p1(-25, -25, -8);
    gp_Pnt p2( 45, -25, -8);
    gp_Pnt p3( 45,  25, -8);
    gp_Pnt p4(-25,  25, -8);

    Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
    Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
    Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
    Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

    TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
    TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
    TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
    TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

    TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
    TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

	Handle(AIS_Shape) shape = new AIS_Shape(shaft_face);
	shape->SetColor(Quantity_NOC_BROWN);
	myAISContext->Display(shape, Standard_False);   


	gp_Pnt closest_pnt = fuselage.GetMinumumDistanceToGround(RAxis, angle);
	
	// display point on fuselage
	std::ostringstream text;
    text << "LE(" << closest_pnt.X() << ", " << closest_pnt.Y() << ", " << closest_pnt.Z() << ")";
	DisplayPoint(closest_pnt, const_cast<char*>(text.str().c_str()), Standard_False, 0.0, 0.0, 0.0, 2.0);

}





/**
* unvollständig: es musst geklärt werden wie mit den 2 Schnittkurven
* zwischen Flügel und Rumpf umgegangen werden muss oder ob diese eine
* Ungenauigkeit in unserer Beispiel CPACS Datei ist.
* Danach muss das Problem gelöst werden das diese Hilfsklasse nicht
* jedes mal neu erzeugt und berechnet wird.
*/
void CTIGLViewerDoc::OnShowIntersectionLine()
{
	// Get Wing UID
	CWingUIDDialog dlgW(*this);
    if (dlgW.DoModal() != IDOK)
        return;
	std::string wingUid = dlgW.GetWingUID();

	// Get Fuselage UID
	CFuselageUIDDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;
	std::string fuselageUid = dlg.GetFuselageUID();


    /* now calculate intersection and display single points */
    for (double eta = 0.0; eta <= 1.0; eta += 0.1)
    {
		double x, y, z;

		TiglReturnCode res = tiglComponentIntersectionPoint(
							m_cpacsHandle,
							const_cast<char*>(fuselageUid.c_str()),	//fuselage UID
							const_cast<char*>(wingUid.c_str()),	//wing uid
							1,	// wireID
							eta,
							&x,
							&y,
							&z);
		if (res != TIGL_SUCCESS)
			TRACE("tiglComponentIntersectionPoint failed, Error code = %d\n", res);

		ISession_Point* aGraphicPoint = new ISession_Point(x, y, z);
        myAISContext->Display(aGraphicPoint,Standard_False);
	}
    DrawXYZAxis();
}







// CTIGLViewerDoc Diagnose
#ifdef _DEBUG
void CTIGLViewerDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CTIGLViewerDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

