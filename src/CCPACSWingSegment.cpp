/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
/**
* @file
* @brief  Implementation of CPACS wing segment handling routines.
*/

#include <math.h>
#include <iostream>
#include <string>
#include <cassert>
#include <cfloat>

#include "CCPACSWingSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingProfile.h"
#include "CTiglError.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shell.hxx"
#include "TopoDS_Wire.hxx"
#include "gp_Trsf.hxx"
#include "gp_Lin.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GC_MakeSegment.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Geom_Surface.hxx"
#include "Geom_Line.hxx"
#include "gce_MakeLin.hxx"
#include "IntCurvesFace_Intersector.hxx"
#include "Precision.hxx"
#include "TopLoc_Location.hxx"
#include "Poly_Triangulation.hxx"
#include "Poly_Array1OfTriangle.hxx"
#include "BRep_Tool.hxx"
#include "BRep_Builder.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepMesh.hxx"
#include "BRepTools.hxx"
#include "BRepBndLib.hxx"
#include "BRepGProp.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "Bnd_Box.hxx"
#include "BRepLib_FindSurface.hxx"
#include "ShapeAnalysis_Surface.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "GeomAdaptor_Surface.hxx"
#include "GC_MakeLine.hxx"
#include "BRepTools_WireExplorer.hxx"

#include "Geom_BSplineCurve.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "GeomAdaptor_HCurve.hxx"
#include "GeomFill_SimpleBound.hxx"
#include "GeomFill_BSplineCurves.hxx"
#include "GeomFill_FillingStyle.hxx"
#include "Geom_BSplineSurface.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "BRepTools.hxx"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace {
    gp_Pnt transformProfilePoint(const tigl::CTiglTransformation& wingTransform, const tigl::CCPACSWingConnection& connection, const gp_Pnt& pointOnProfile){
        gp_Pnt transformedPoint(pointOnProfile);

         // Do section element transformation on points
        transformedPoint = connection.GetSectionElementTransformation().Transform(transformedPoint);

        // Do section transformations
        transformedPoint = connection.GetSectionTransformation().Transform(transformedPoint);

        // Do positioning transformations
        transformedPoint = connection.GetPositioningTransformation().Transform(transformedPoint);
        
        transformedPoint = wingTransform.Transform(transformedPoint);

        return transformedPoint;
    }
    
    TopoDS_Wire transformProfileWire(const tigl::CTiglTransformation& wingTransform, const tigl::CCPACSWingConnection& connection, const TopoDS_Wire& wire){
        TopoDS_Shape transformedWire(wire);

         // Do section element transformation on points
        transformedWire = connection.GetSectionElementTransformation().Transform(transformedWire);

        // Do section transformations
        transformedWire = connection.GetSectionTransformation().Transform(transformedWire);

        // Do positioning transformations
        transformedWire = connection.GetPositioningTransformation().Transform(transformedWire);
        
        transformedWire = wingTransform.Transform(transformedWire);
        
        // Cast shapes to wires, see OpenCascade documentation
        if (transformedWire.ShapeType() != TopAbs_WIRE)
            throw tigl::CTiglError("Error: Wrong shape type in CCPACSWingSegment::transformProfileWire", TIGL_ERROR);

        return TopoDS::Wire(transformedWire);
    }
}

namespace tigl {

    // Constructor
    CCPACSWingSegment::CCPACSWingSegment(CCPACSWing* aWing, int aSegmentIndex)
        : CTiglAbstractSegment(aSegmentIndex)
        , innerConnection(this)
        , outerConnection(this)
        , wing(aWing)
        , surfacesAreValid(false)
    {
        Cleanup();
    }

    // Destructor
    CCPACSWingSegment::~CCPACSWingSegment(void)
    {
        Cleanup();
    }

    // Invalidates internal state
    void CCPACSWingSegment::Invalidate(void)
    {
        CTiglAbstractSegment::Invalidate();
        surfacesAreValid = false;
    }

    // Cleanup routine
    void CCPACSWingSegment::Cleanup(void)
    {
        name = "";
        upperShape.Nullify();
        lowerShape.Nullify();
        surfacesAreValid = false;
        CTiglAbstractSegment::Cleanup();
    }

    // Update internal segment data
    void CCPACSWingSegment::Update(void)
    {
        Invalidate();
    }

    // Read CPACS segment elements
    void CCPACSWingSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
    {
        Cleanup();

        char*       elementPath;
        std::string tempString;

        // Get subelement "name"
        char* ptrName = NULL;
        tempString    = segmentXPath + "/name";
        elementPath   = const_cast<char*>(tempString.c_str());
        if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS)
            name          = ptrName;

        // Get attribute "uid"
        char* ptrUID = NULL;
        tempString   = "uID";
        elementPath  = const_cast<char*>(tempString.c_str());
        if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(segmentXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS)
            SetUID(ptrUID);

        // Inner connection
        tempString = segmentXPath + "/fromElementUID";
        innerConnection.ReadCPACS(tixiHandle, tempString);

        // Inner connection
        tempString = segmentXPath + "/toElementUID";
        outerConnection.ReadCPACS(tixiHandle, tempString);

        Update();
    }

    // Returns the wing this segment belongs to
    CCPACSWing& CCPACSWingSegment::GetWing(void) const
    {
        return *wing;
    }

    // helper function to get the inner transformed chord line wire
    TopoDS_Wire CCPACSWingSegment::GetInnerWire(void)
    {
        CCPACSWingProfile & innerProfile = innerConnection.GetProfile();
        return transformProfileWire(GetWing().GetTransformation(), innerConnection, innerProfile.GetFusedUpperLowerWire());
    }

    // helper function to get the outer transformed chord line wire
    TopoDS_Wire CCPACSWingSegment::GetOuterWire(void)
    {
        CCPACSWingProfile & outerProfile = outerConnection.GetProfile();
        return transformProfileWire(GetWing().GetTransformation(), outerConnection, outerProfile.GetFusedUpperLowerWire());
    }
    
    // helper function to get the inner closing of the wing segment
    TopoDS_Shape CCPACSWingSegment::GetInnerClosure()
    {
        TopoDS_Wire wire = GetInnerWire();
        return BRepBuilderAPI_MakeFace(wire).Face();
    }
    
    // helper function to get the inner closing of the wing segment
    TopoDS_Shape CCPACSWingSegment::GetOuterClosure()
    {
        TopoDS_Wire wire = GetOuterWire();
        return BRepBuilderAPI_MakeFace(wire).Face();
    }

    // Builds the loft between the two segment sections
    TopoDS_Shape CCPACSWingSegment::BuildLoft(void)
    {
        TopoDS_Wire innerWire = GetInnerWire();
        TopoDS_Wire outerWire = GetOuterWire();

        // Build loft
        //BRepOffsetAPI_ThruSections generator(Standard_False, Standard_False, Precision::Confusion());
        BRepOffsetAPI_ThruSections generator(/* is solid (else shell) */ Standard_True, /* ruled (else smoothed out) */ Standard_False, Precision::Confusion());
        generator.AddWire(innerWire);
        generator.AddWire(outerWire);
        generator.CheckCompatibility(/* check (defaults to true) */ Standard_False);
        generator.Build();
        TopoDS_Shape loft = generator.Shape();

        Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
        sfs->Init ( loft );
        sfs->Perform();
        loft = sfs->Shape();

        // Calculate volume
        GProp_GProps System;
        BRepGProp::VolumeProperties(loft, System);
        myVolume = System.Mass();

        // Calculate surface area
        GProp_GProps AreaSystem;
        BRepGProp::SurfaceProperties(loft, AreaSystem);
        mySurfaceArea = AreaSystem.Mass();
        
        return loft;
    }

    // Gets the upper point in relative wing coordinates for a given eta and xsi
    gp_Pnt CCPACSWingSegment::GetUpperPoint(double eta, double xsi)
    {
        return GetPoint(eta, xsi, true);
    }

    // Gets the lower point in relative wing coordinates for a given eta and xsi
    gp_Pnt CCPACSWingSegment::GetLowerPoint(double eta, double xsi)
    {
        return GetPoint(eta, xsi, false);
    }

    // Returns the inner section UID of this segment
    std::string CCPACSWingSegment::GetInnerSectionUID(void)
    {
        return innerConnection.GetSectionUID();
    }

    // Returns the outer section UID of this segment
    std::string CCPACSWingSegment::GetOuterSectionUID(void)
    {
        return outerConnection.GetSectionUID();
    }

    // Returns the inner section element UID of this segment
    std::string CCPACSWingSegment::GetInnerSectionElementUID(void)
    {
        return innerConnection.GetSectionElementUID();
    }

    // Returns the outer section element UID of this segment
    std::string CCPACSWingSegment::GetOuterSectionElementUID(void)
    {
        return outerConnection.GetSectionElementUID();
    }

    // Returns the inner section index of this segment
    int CCPACSWingSegment::GetInnerSectionIndex(void)
    {
        return innerConnection.GetSectionIndex();
    }

    // Returns the outer section index of this segment
    int CCPACSWingSegment::GetOuterSectionIndex(void)
    {
        return outerConnection.GetSectionIndex();
    }

    // Returns the inner section element index of this segment
    int CCPACSWingSegment::GetInnerSectionElementIndex(void)
    {
        return innerConnection.GetSectionElementIndex();
    }

    // Returns the outer section element index of this segment
    int CCPACSWingSegment::GetOuterSectionElementIndex(void)
    {
        return outerConnection.GetSectionElementIndex();
    }

    // Returns the start section element index of this segment
    CCPACSWingConnection& CCPACSWingSegment::GetInnerConnection(void)
    {
        return( innerConnection );
    }

    // Returns the end section element index of this segment
    CCPACSWingConnection& CCPACSWingSegment::GetOuterConnection(void)
    {
        return( outerConnection );
    }

    // Returns the volume of this segment
    double CCPACSWingSegment::GetVolume(void)
    {
        Update();
        return( myVolume );
    }

    // Returns the surface area of this segment
    double CCPACSWingSegment::GetSurfaceArea(void)
    {
        Update();
        return( mySurfaceArea );
    }

    // Gets the count of segments connected to the inner section of this segment // TODO can this be optimized instead of iterating over all segments?
    int CCPACSWingSegment::GetInnerConnectedSegmentCount(void)
    {
        int count = 0;
        for (int i = 1; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetInnerSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetInnerSectionUID())
            {
                count++;
            }
        }
        return count;
    }

    // Gets the count of segments connected to the outer section of this segment
    int CCPACSWingSegment::GetOuterConnectedSegmentCount(void)
    {
        int count = 0;
        for (int i = 1; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetOuterSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetOuterSectionUID())
            {
                count++;
            }
        }
        return count;
    }

    // Gets the index (number) of the n-th segment connected to the inner section
    // of this segment. n starts at 1.
    int CCPACSWingSegment::GetInnerConnectedSegmentIndex(int n)
    {
        if (n < 1 || n > GetInnerConnectedSegmentCount())
        {
            throw CTiglError("Error: Invalid value for parameter n in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_INDEX_ERROR);
        }

        for (int i = 1, count = 0; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetInnerSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetInnerSectionUID())
            {
                if (++count == n) return nextSegment.GetSegmentIndex();
            }
        }

        throw CTiglError("Error: No connected segment found in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    // Gets the index (number) of the n-th segment connected to the outer section
    // of this segment. n starts at 1.
    int CCPACSWingSegment::GetOuterConnectedSegmentIndex(int n)
    {
        if (n < 1 || n > GetOuterConnectedSegmentCount())
        {
            throw CTiglError("Error: Invalid value for parameter n in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_INDEX_ERROR);
        }

        for (int i = 1, count = 0; i <= GetWing().GetSegmentCount(); i++)
        {
            CCPACSWingSegment& nextSegment = (CCPACSWingSegment &) GetWing().GetSegment(i);
            if (nextSegment.GetSegmentIndex() == mySegmentIndex)
                continue;
            if (nextSegment.GetInnerSectionUID() == GetOuterSectionUID() ||
                nextSegment.GetOuterSectionUID() == GetOuterSectionUID())
            {
                if (++count == n) return nextSegment.GetSegmentIndex();
            }
        }

        throw CTiglError("Error: No connected segment found in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    // Returns an upper or lower point on the segment surface in
    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
    // edge on the outer wing profile. If fromUpper is true, a point
    // on the upper surface is returned, otherwise from the lower.
    gp_Pnt CCPACSWingSegment::GetPoint(double eta, double xsi, bool fromUpper)
    {
        if (eta < 0.0 || eta > 1.0)
        {
            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
        }

        CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
        CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

        // Compute points on wing profiles for the given xsi
        gp_Pnt innerProfilePoint;
        gp_Pnt outerProfilePoint;
        if (fromUpper == true)
        {
            innerProfilePoint = innerProfile.GetUpperPoint(xsi);
            outerProfilePoint = outerProfile.GetUpperPoint(xsi);
        }
        else
        {
            innerProfilePoint = innerProfile.GetLowerPoint(xsi);
            outerProfilePoint = outerProfile.GetLowerPoint(xsi);
        }

        innerProfilePoint = transformProfilePoint(wing->GetTransformation(), innerConnection, innerProfilePoint);
        outerProfilePoint = transformProfilePoint(wing->GetTransformation(), outerConnection, outerProfilePoint);

        // Get point on wing segment in dependence of eta by linear interpolation
        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(innerProfilePoint, outerProfilePoint);
        Standard_Real firstParam = profileLine->FirstParameter();
        Standard_Real lastParam  = profileLine->LastParameter();
        Standard_Real param = firstParam + (lastParam - firstParam) * eta;
        gp_Pnt profilePoint;
        profileLine->D0(param, profilePoint);

        return profilePoint;
    }
    
    gp_Pnt CCPACSWingSegment::GetPointAngles(double eta, double xsi, double xangle, double yangle, double zangle, bool fromUpper)
    {
        if (eta < 0.0 || eta > 1.0)
        {
            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
        }

        if(!surfacesAreValid) {
            MakeSurfaces();
        }
        
        CTiglPoint tiglPoint, tiglNormal;
        cordSurface.translate(eta, xsi, &tiglPoint);
        cordSurface.getNormal(eta,xsi, &tiglNormal);
        
        // calc vector
        CTiglTransformation trafo;
        trafo.AddRotationX(xangle);
        trafo.AddRotationY(yangle);
        trafo.AddRotationZ(zangle);
        
        gp_Vec normalVec(trafo.Transform(tiglNormal.Get_gp_Pnt()).XYZ());
        normalVec.Normalize();
        
        gp_Pnt start(tiglPoint.Get_gp_Pnt().XYZ() + 1e3*normalVec.XYZ());
        gp_Pnt stop (tiglPoint.Get_gp_Pnt().XYZ() - 1e3*normalVec.XYZ());
        
        BRepBuilderAPI_MakeWire wireBuilder;
        TopoDS_Edge normalEdge = BRepBuilderAPI_MakeEdge(start, stop);
        wireBuilder.Add(normalEdge);
        TopoDS_Wire normalWire = wireBuilder.Wire();
        
        BRepExtrema_DistShapeShape extrema;
        extrema.LoadS1(normalWire);
        if(fromUpper)
            extrema.LoadS2(wing->GetUpperShape());
        else 
            extrema.LoadS2(wing->GetLowerShape());

        extrema.Perform();
        if (!extrema.IsDone())
            throw CTiglError("Could not calculate intersection of line with wing shell in CCPACSWingSegment::GetPointAngles", TIGL_NOT_FOUND);
        
        gp_Pnt p1 = extrema.PointOnShape1(1);
        gp_Pnt p2 = extrema.PointOnShape2(1);
        
        if (p1.Distance(p2) > 1e-7)
            throw CTiglError("Could not calculate intersection of line with wing shell in CCPACSWingSegment::GetPointAngles", TIGL_NOT_FOUND);
        
        return p2;
    }

    gp_Pnt CCPACSWingSegment::GetChordPoint(double eta, double xsi)
    {
        if (eta < 0.0 || eta > 1.0)
        {
            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
        }

        CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
        CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

        // Compute points on wing profiles for the given xsi
        gp_Pnt innerProfilePoint = innerProfile.GetChordPoint(xsi);
        gp_Pnt outerProfilePoint = outerProfile.GetChordPoint(xsi);

        // Do section element transformation on points
        innerProfilePoint = transformProfilePoint(wing->GetTransformation(), innerConnection, innerProfilePoint);
        outerProfilePoint = transformProfilePoint(wing->GetTransformation(), outerConnection, outerProfilePoint);

        // Get point on wing segment in dependence of eta by linear interpolation
        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(innerProfilePoint, outerProfilePoint);
        Standard_Real firstParam = profileLine->FirstParameter();
        Standard_Real lastParam  = profileLine->LastParameter();
        Standard_Real param = (lastParam - firstParam) * eta;
        gp_Pnt profilePoint;
        profileLine->D0(param, profilePoint);

        return profilePoint;
    }

    // Returns the inner profile points as read from TIXI. The points are already transformed.
    std::vector<CTiglPoint*> CCPACSWingSegment::GetRawInnerProfilePoints()
    {
        CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
        std::vector<CTiglPoint*> points = innerProfile.GetCoordinateContainer();
        std::vector<CTiglPoint*> pointsTransformed;
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < points.size(); i++) {
            
            gp_Pnt pnt = points[i]->Get_gp_Pnt();

            pnt = innerConnection.GetSectionElementTransformation().Transform(pnt);
            pnt = innerConnection.GetSectionTransformation().Transform(pnt);
            pnt = innerConnection.GetPositioningTransformation().Transform(pnt);

            CTiglPoint *tiglPoint = new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z());
            pointsTransformed.push_back(tiglPoint);

        }
        return pointsTransformed;
    }


    // Returns the outer profile points as read from TIXI. The points are already transformed.
    std::vector<CTiglPoint*> CCPACSWingSegment::GetRawOuterProfilePoints()
    {
        CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
        std::vector<CTiglPoint*> points = outerProfile.GetCoordinateContainer();
        std::vector<CTiglPoint*> pointsTransformed;
        for (std::vector<tigl::CTiglPoint*>::size_type i = 0; i < points.size(); i++) {
            
            gp_Pnt pnt = points[i]->Get_gp_Pnt();

            pnt = outerConnection.GetSectionElementTransformation().Transform(pnt);
            pnt = outerConnection.GetSectionTransformation().Transform(pnt);
            pnt = outerConnection.GetPositioningTransformation().Transform(pnt);

            CTiglPoint *tiglPoint = new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z());
            pointsTransformed.push_back(tiglPoint);

        }
        return pointsTransformed;
    }


    double CCPACSWingSegment::GetEta(gp_Pnt pnt, double xsi)
    {
        // Build virtual eta line.
        // eta is in x = 0
        gp_Pnt pnt0 = GetChordPoint(0, xsi);
        pnt0 = wing->GetWingTransformation().Transform(pnt0);        
        pnt0 = gp_Pnt(0, pnt0.Y(), pnt0.Z());

        gp_Pnt pnt1 = GetChordPoint(1, xsi);
        pnt1 = wing->GetWingTransformation().Transform(pnt1);
        pnt1 = gp_Pnt(0, pnt1.Y(), pnt1.Z());

        BRepBuilderAPI_MakeWire etaWireBuilder;
        TopoDS_Edge etaEdge = BRepBuilderAPI_MakeEdge(pnt0, pnt1);
        etaWireBuilder.Add(etaEdge);
        TopoDS_Wire etaLine = etaWireBuilder.Wire();
        
        // intersection line
        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(pnt, gp_Pnt(-1e9, 0, 0));
        BRepBuilderAPI_MakeEdge ME(profileLine);     
        TopoDS_Shape aCrv(ME.Edge());
        
        // now find intersection point
        BRepExtrema_DistShapeShape extrema(etaLine, aCrv);
        extrema.Perform();
        gp_Pnt intersectionPoint = extrema.PointOnShape1(1);

        //length of the eta line
        Standard_Real len1 = pnt0.Distance(pnt1);
        // now the small line, a fraction of the original eta line
        Standard_Real len2 = pnt0.Distance(intersectionPoint);

        assert(len1 != 0.);

        return len2/len1;
    }


    // Returns eta as parametric distance from a given point on the surface
    double CCPACSWingSegment::GetEta(gp_Pnt pnt, bool isUpper)
    {
        double eta = 0., xsi = 0.;
        GetEtaXsi(pnt, isUpper, eta, xsi);
        return eta;
    }

    // Returns xsi as parametric distance from a given point on the surface
    double CCPACSWingSegment::GetXsi(gp_Pnt pnt, bool isUpper)
    {
        double eta = 0., xsi = 0.;
        GetEtaXsi(pnt, isUpper, eta, xsi);
        return xsi;
    }

    // Returns xsi as parametric distance from a given point on the surface
    void CCPACSWingSegment::GetEtaXsi(gp_Pnt pnt, bool isUpper, double& eta, double& xsi)
    {
        MakeSurfaces();
        CTiglPoint tmpPnt(pnt.XYZ());
        if(cordSurface.translate(tmpPnt, &eta, &xsi) != TIGL_SUCCESS)
            throw tigl::CTiglError("Cannot determine eta, xsi coordinates of current point in CCPACSWingSegment::GetEtaXsi!", TIGL_MATH_ERROR);
    }

    // Returns if the given point is ont the Top of the wing or on the lower side.
    bool CCPACSWingSegment::GetIsOnTop(gp_Pnt pnt)
    {
        double tolerance = 0.03; // 3cm

        MakeSurfaces();

        GeomAPI_ProjectPointOnSurf Proj(pnt, upperSurface);
        if(Proj.NbPoints() > 0 && Proj.LowerDistance() < tolerance)
            return true;
        else
            return false;
    }

    // Builds upper/lower surfaces as shapes
    // we split the wing profile into upper and lower wire.
    // To do so, we have to determine, what is up
    void CCPACSWingSegment::MakeSurfaces()
    {
        if(surfacesAreValid)
            return;
        
        gp_Pnt inner_lep = GetChordPoint(0.0, 0.0);
        gp_Pnt outer_lep = GetChordPoint(1.0, 0.0);
        
        gp_Pnt inner_tep = GetChordPoint(0.0, 1.0);
        gp_Pnt outer_tep = GetChordPoint(1.0, 1.0);
        
        cordSurface.setQuadriangle(inner_lep.XYZ(), outer_lep.XYZ(), inner_tep.XYZ(), outer_tep.XYZ());

        TopoDS_Wire iu_wire = innerConnection.GetProfile().GetUpperWire();
        TopoDS_Wire ou_wire = outerConnection.GetProfile().GetUpperWire();
        TopoDS_Wire il_wire = innerConnection.GetProfile().GetLowerWire();
        TopoDS_Wire ol_wire = outerConnection.GetProfile().GetLowerWire();
        
        iu_wire = transformProfileWire(GetWing().GetTransformation(), innerConnection, iu_wire);
        ou_wire = transformProfileWire(GetWing().GetTransformation(), outerConnection, ou_wire);
        il_wire = transformProfileWire(GetWing().GetTransformation(), innerConnection, il_wire);
        ol_wire = transformProfileWire(GetWing().GetTransformation(), outerConnection, ol_wire);
        

        BRepOffsetAPI_ThruSections upperSections(Standard_False,Standard_True);
        upperSections.AddWire(iu_wire);
        upperSections.AddWire(ou_wire);
        upperSections.Build();
        
        BRepOffsetAPI_ThruSections lowerSections(Standard_False,Standard_True);
        lowerSections.AddWire(il_wire);
        lowerSections.AddWire(ol_wire);
        lowerSections.Build();
        
        upperShape = upperSections.Shape();
        lowerShape = lowerSections.Shape();

        // we select the largest face to be the upperface (we can not include the trailing edge)
        TopExp_Explorer faceExplorer;
        double maxArea = DBL_MIN;
        for(faceExplorer.Init(upperShape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
            const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
            GProp_GProps System;
            BRepGProp::SurfaceProperties(face, System);
            double surfaceArea = System.Mass();
            if(surfaceArea > maxArea){
                const BRepLib_FindSurface findSurface(face, /* tolerance */1.01);
                upperSurface = findSurface.Surface();
                maxArea = surfaceArea;
            }
        }

        // we select the largest face to be the lowerface (we can not include the trailing edge)
        maxArea = DBL_MIN;
        for(faceExplorer.Init(lowerShape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
            const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
            GProp_GProps System;
            BRepGProp::SurfaceProperties(face, System);
            double surfaceArea = System.Mass();
            if(surfaceArea > maxArea){
                const BRepLib_FindSurface findSurface(face, /* tolerance */1.01);
                lowerSurface = findSurface.Surface();
                maxArea = surfaceArea;
            }
        }

        surfacesAreValid = true;
    }



    // Returns the reference area of this wing.
    // Here, we always take the reference wing area to be that of the trapezoidal portion of the wing projected into the centerline.
    // The leading and trailing edge chord extensions are not included in this definition and for some airplanes, such as Boeing's Blended
    // Wing Body, the difference can be almost a factor of two between the "real" wing area and the "trap area". Some companies use reference
    // wing areas that include portions of the chord extensions, and in some studies, even tail area is included as part of the reference area.
    // For simplicity, we use the trapezoidal area here.
    double CCPACSWingSegment::GetReferenceArea()
    {
        double refArea = 0.0;
        MakeSurfaces();

        // Calculate surface area
        GProp_GProps System;
        BRepGProp::SurfaceProperties(upperShape, System);
        refArea = System.Mass();
        return refArea;
    }

    // Returns the lower Surface of this Segment
    Handle(Geom_Surface) CCPACSWingSegment::GetLowerSurface()
    {
        if(!surfacesAreValid) {
            MakeSurfaces();
        }
        return lowerSurface;
    }

    // Returns the upper Surface of this Segment
    Handle(Geom_Surface) CCPACSWingSegment::GetUpperSurface()
    {
        if(!surfacesAreValid) {
            MakeSurfaces();
        }
        return upperSurface;
    }
    
    // Returns the upper wing shape of this Segment
    TopoDS_Shape& CCPACSWingSegment::GetUpperShape()
    {
        if(!surfacesAreValid) {
            MakeSurfaces();
        }
        return upperShape;
    }

    // Returns the lower wing shape of this Segment
    TopoDS_Shape& CCPACSWingSegment::GetLowerShape()
    {
        if(!surfacesAreValid) {
            MakeSurfaces();
        }
        return lowerShape;
    }

    // builds data structure for a TDocStd_Application
    // mostly used for export
    TDF_Label CCPACSWingSegment::ExportDataStructure(Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label)
    {
        TDF_Label subLabel;
        return subLabel;
    }

} // end namespace tigl
