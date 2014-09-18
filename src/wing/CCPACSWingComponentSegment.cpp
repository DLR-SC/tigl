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
* @brief  Implementation of CPACS wing ComponentSegment handling routines.
*/

#include <math.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <limits>

#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingProfile.h"
#include "CTiglLogging.h"
#include "CCPACSWingCell.h"
#include "CTiglApproximateBsplineWire.h"
#include "CTiglMakeLoft.h"
#include "tiglcommonfunctions.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Wire.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Geom_Plane.hxx"
#include "gp_Pln.hxx"
//#include "Geom_Surface.hxx"
#include "GeomLib.hxx"
#include "Precision.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "BRepTools.hxx"
#include "BRepGProp.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "Geom_BSplineCurve.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "BRepClass3d_SolidClassifier.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "TColgp_Array1OfPnt.hxx"

namespace tigl
{

namespace
{
    bool inBetween(const gp_Pnt& p, const gp_Pnt& p1, const gp_Pnt& p2)
    {
        gp_Vec b(p1, p2);
        gp_Vec v1(p, p1);
        gp_Vec v2(p, p2);

        double res = (b*v1)*(b*v2);
        return res <= 0.;
    }

    double GetNearestValidParameter(double p)
    {
        if (p < 0.) {
            return 0.;
        }
        else if ( p > 1.) {
            return 1.;
        }
        return p;
    }
}


// Constructor
CCPACSWingComponentSegment::CCPACSWingComponentSegment(CCPACSWing* aWing, int aSegmentIndex)
    : CTiglAbstractSegment(aSegmentIndex)
    , wing(aWing)
    , surfacesAreValid(false)
{
    Cleanup();
}

// Destructor
CCPACSWingComponentSegment::~CCPACSWingComponentSegment(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWingComponentSegment::Invalidate(void)
{
    invalidated = true;
    surfacesAreValid = false;
    projLeadingEdge.Nullify();
    wingSegments.clear();

}

// Cleanup routine
void CCPACSWingComponentSegment::Cleanup(void)
{
    name = "";
    fromElementUID = "";
    toElementUID   = "";
    myVolume       = 0.;
    mySurfaceArea  = 0.;
    surfacesAreValid = false;
    CTiglAbstractSegment::Cleanup();
    structure.Cleanup();
    projLeadingEdge.Nullify();
    wingSegments.clear();
}

// Update internal segment data
void CCPACSWingComponentSegment::Update(void)
{
    Invalidate();
}

// Read CPACS segment elements
void CCPACSWingComponentSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
{
    //Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = segmentXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name          = ptrName;
    }

    // Get attribute "uid"
    char* ptrUID = NULL;
    tempString   = "uID";
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(segmentXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get fromElementUID
    char* ptrFromElementUID = NULL;
    tempString = segmentXPath + "/fromElementUID";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrFromElementUID) == SUCCESS) {
        fromElementUID = ptrFromElementUID;
    }

    // Get toElementUID
    char* ptrToElementUID = NULL;
    tempString = segmentXPath + "/toElementUID";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrToElementUID) == SUCCESS) {
        toElementUID = ptrToElementUID;
    }
        
    // read structure
    tempString = segmentXPath + "/structure";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS){
        structure.ReadCPACS(tixiHandle, elementPath);
    }

    Update();
}

// Returns the wing this segment belongs to
CCPACSWing& CCPACSWingComponentSegment::GetWing(void) const
{
    return *wing;
}

    
SegmentList& CCPACSWingComponentSegment::GetSegmentList()
{
    if (wingSegments.size() == 0) {
        std::vector<int> path;
        path = findPath(fromElementUID, toElementUID, path, true);

        if (path.size() == 0) {
            // could not find path from fromUID to toUID
            // try the other way around
            path = findPath(toElementUID, fromElementUID, path, true);
        }

        if (path.size() == 0) {
            LOG(WARNING) << "Could not determine segment list to component segment from \""
                            << GetFromElementUID() << "\" to \"" << GetToElementUID() << "\"!";
        }

        std::vector<int>::iterator it;
        for (it = path.begin(); it != path.end(); ++it) {
            CCPACSWingSegment* pSeg = static_cast<CCPACSWingSegment*>(&(GetWing().GetSegment(*it)));
            wingSegments.push_back(pSeg);
        }
    }

    return wingSegments;
}
    
// Determines, which segments belong to the component segment
std::vector<int> CCPACSWingComponentSegment::findPath(const std::string& fromUID, const::std::string& toUID, const std::vector<int>& curPath, bool forward) const
{
    if ( fromUID == toUID ) {
        return curPath;
    }
        
    // find all segments with InnerSectionUID == fromUID
    std::vector<int> segList;
    for (int i = 1; i <= wing->GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(i);
        std::string startUID = forward ? segment.GetInnerSectionElementUID() : segment.GetOuterSectionElementUID();
        if (startUID == fromUID) {
            segList.push_back(i);
        }
    }
        
    std::vector<int>::iterator segIt = segList.begin();
    for (; segIt != segList.end(); ++segIt) {
        int iseg = *segIt;
        CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(iseg);
        std::vector<int> newpath(curPath);
        newpath.push_back(iseg);
        std::string segEndUID = forward ? segment.GetOuterSectionElementUID() : segment.GetInnerSectionElementUID();
        std::vector<int> result = findPath(segEndUID, toUID, newpath, forward);
        if (result.size() != 0) {
            return result;
        }
    }
        
    // return empty list as path could not be found
    std::vector<int> result;
    return result;
}
    
TopoDS_Wire CCPACSWingComponentSegment::GetCSLine(double eta1, double xsi1, double eta2, double xsi2, int NSTEPS)
{
    BRepBuilderAPI_MakeWire wireBuilder;
        
    gp_Pnt  old_point = GetPoint(eta1,xsi1);
    for (int istep = 1; istep < NSTEPS; ++istep) {
        double eta = eta1 + (double) istep/(double) (NSTEPS-1) * (eta2-eta1);
        double xsi = xsi1 + (double) istep/(double) (NSTEPS-1) * (xsi2-xsi1);
        gp_Pnt point = GetPoint(eta,xsi);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(old_point, point);
        wireBuilder.Add(edge);
        old_point = point;
    }
    return wireBuilder.Wire();
}
    
void CCPACSWingComponentSegment::GetSegmentIntersection(const std::string& segmentUID, double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, double &xsi)
{
    // number of component segment point samples per line
    int NSTEPS = 11;
        
    CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(segmentUID);
    bool hasIntersected = false;
        
    // we do an iterative procedure to find the segment intersection
    // by trying to find out, what the exact intersection of the component
    // segment with the segment is
    int iter = 0;
    int maxiter = 10;
    gp_Pnt result(0,0,0);
    gp_Pnt oldresult(100,0,0);
        
    while (result.Distance(oldresult) > 1e-6 && iter < maxiter){
        oldresult = result;
            
        double deta = (csEta2-csEta1)/double(NSTEPS-1);
        double dxsi = (csXsi2-csXsi1)/double(NSTEPS-1);
        
        std::vector<gp_Pnt> points;
        for (int istep = 0; istep < NSTEPS; ++istep) {
            double eta = csEta1 + (double) istep * deta;
            double xsi = csXsi1 + (double) istep * dxsi;
            gp_Pnt point = GetPoint(eta,xsi);
            points.push_back(point);
        }
            
        BRepBuilderAPI_MakeWire wireBuilder;
        for (int istep = 0; istep < NSTEPS-1; ++istep) {
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(points[istep], points[istep+1]);
            wireBuilder.Add(edge);
        }
            
        //return wireGen.BuildWire(container,false);
        TopoDS_Wire csLine = wireBuilder.Wire();
        
        // create segments outer chord line
        gp_Pnt leadingPoint  = segment.GetChordPoint(eta, 0.);
        gp_Pnt trailingPoint = segment.GetChordPoint(eta, 1.);
            
        TopoDS_Wire outerChord = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(leadingPoint,trailingPoint));
            
        BRepExtrema_DistShapeShape extrema(csLine, outerChord);
        extrema.Perform();
            
        double dist = 0;
        if (extrema.IsDone() && extrema.NbSolution() > 0) {
            gp_Pnt p1 = extrema.PointOnShape1(1);
            gp_Pnt p2 = extrema.PointOnShape2(1);
            dist = p1.Distance(p2);
            result = p2;
            // check if the lines were really intersecting (1cm accuracy should be enough)
            if (dist < 1e-2) {
                hasIntersected = true;
            }
        }

            
        // now lets check in between which points the intersection lies
        int ifound = 0;
        for (int i = 0; i < NSTEPS-1; ++i) {
            if (inBetween(result, points[i], points[i+1])) {
                ifound = i;
                break;
            }
        }
            
        // calculate new search field
        csEta2 = csEta1 + deta*(ifound + 1);
        csEta1 = csEta1 + deta*(ifound);
        csXsi2 = csXsi1 + dxsi*(ifound + 1);
        csXsi1 = csXsi1 + dxsi*(ifound);
        ++iter;
    }
        
    if (hasIntersected) {
        // now check if we found an intersection
        double etaTmp;
        segment.GetEtaXsi(result, etaTmp, xsi);
        // by design, result is inside the segment
        // However due to numerics, eta and xsi might
        // be a bit larger than 1 or smaller than 0
        if (etaTmp > 1.) {
            etaTmp = 1.;
        }
        else if (etaTmp < 0) {
            etaTmp = 0.;
        }
        if (xsi > 1.) {
            xsi = 1.;
        }
        else if (xsi < 0) {
            xsi = 0.;
        }

        // check that etaTmp coordinate is correct
        if (fabs(etaTmp - eta) > 1e-6) {
            throw CTiglError("Error determining proper eta, xsi coordinates in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
        }
    }
    else {
        throw CTiglError("Component segment line does not intersect outer segment border in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
    }
        
    // test if eta,xsi is valid
    if (segment.GetChordPoint(eta,xsi).Distance(result) > 1e-6) {
        throw CTiglError("Error determining proper eta, xsi coordinates in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
    }
}

// Builds the loft between the two segment sections
TopoDS_Shape CCPACSWingComponentSegment::BuildLoft(void)
{
    SegmentList& segments = GetSegmentList();
    if (segments.size() == 0) {
        throw CTiglError("Error: Could not find segments in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
    }
    
    CTiglMakeLoft lofter;
    lofter.setMakeSolid(true);

    for (SegmentList::iterator it=segments.begin(); it != segments.end(); ++it) {
        CCPACSWingSegment& segment = **it;
        const TopoDS_Shape& startWire = segment.GetInnerWire();
        lofter.addProfiles(startWire);
    }
    // add outer wire
    CCPACSWingSegment& segment = *segments[segments.size()-1];
    TopoDS_Wire endWire =  segment.GetOuterWire();
    lofter.addProfiles(endWire);

    TopoDS_Shape loft = lofter.Shape();

    BRepTools::Clean(loft);

    Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
    sfs->Init ( loft );
    sfs->Perform();
    loft = sfs->Shape();
    
    return loft;
}

void CCPACSWingComponentSegment::UpdateProjectedLeadingEdge()
{
    if ( !projLeadingEdge.IsNull() ) {
        return;
    }

    // add inner sections of each segment
    SegmentList& segments = GetSegmentList();

    if (segments.size() < 1) {
        std::stringstream str;
        str << "Wing component " << GetUID() << " does not contain any segments (CCPACSWingComponentSegment::updateProjectedLeadingEdge)!";
        throw CTiglError(str.str(), TIGL_ERROR);
    }
    std::vector<gp_Pnt> LEPointsProjected;
    SegmentList::iterator segmentIt = segments.begin();
    int pointIndex = 1;
    for (; segmentIt != segments.end(); ++segmentIt) {
        tigl::CCPACSWingSegment& segment = **segmentIt;

        // build iso xsi line
        gp_Pnt lep = segment.GetChordPoint(0.,0.);

        // build leading edge projected to x=0 plane
        gp_Pnt lep_proj(0., lep.Y(), lep.Z());
        LEPointsProjected.push_back(lep_proj);

        if (segmentIt == segments.end()-1) {
            // add outer section of last segment
            gp_Pnt lep = segment.GetChordPoint(1., 0.);
            gp_Pnt lep_proj(0., lep.Y(), lep.Z());
            LEPointsProjected.push_back(lep_proj);
            // the break should not be necessary here, since the loop is
            break;
        }
        pointIndex++;
    }

    // check if we have to extend the leading edge at wing tip
    int nPoints = LEPointsProjected.size();
    tigl::CCPACSWingSegment& outerSegment = *segments[segments.size()-1];
    tigl::CCPACSWingSegment& innerSegment = *segments[0];

    // project outer point of trailing edge on leading edge
    gp_Pnt pOuterTrailingEdge = outerSegment.GetChordPoint(1.0, 1.0);
    Standard_Real uout = ProjectPointOnLine(pOuterTrailingEdge, LEPointsProjected[nPoints-2], LEPointsProjected[nPoints-1]);


    // project outer point of trailing edge on leading edge
    gp_Pnt pInnerTrailingEdge = innerSegment.GetChordPoint(0.0, 1.0);
    Standard_Real uin = ProjectPointOnLine(pInnerTrailingEdge, LEPointsProjected[0], LEPointsProjected[1]);

    gp_Pnt outnew =  LEPointsProjected[nPoints-1];
    if (uout > 1.0) {
        // extend outer leading edge
        outnew = LEPointsProjected[nPoints-2].XYZ()*(1. - uout) + LEPointsProjected[nPoints-1].XYZ()*uout;
    }

    gp_Pnt innew  = LEPointsProjected[0];
    if (uin < 0.0) {
        // extend inner leading edge
        innew  = LEPointsProjected[0].XYZ()*(1. - uin) + LEPointsProjected[1].XYZ()*uin;
    }

    // set new leading edge points
    LEPointsProjected[nPoints-1] = outnew;
    LEPointsProjected[0]         = innew;

    // build projected leading edge wire
    BRepBuilderAPI_MakeWire wireBuilder;
    for (unsigned int j = 1; j < LEPointsProjected.size(); j++) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(LEPointsProjected[j - 1], LEPointsProjected[j]);
        wireBuilder.Add(edge);
    }
    projLeadingEdge = wireBuilder.Wire();
}


// Gets a point in relative wing coordinates for a given eta and xsi
gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi)
{
    // search for ETA coordinate
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Error: Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }

    UpdateProjectedLeadingEdge();

    SegmentList& segments = GetSegmentList();
    if (segments.size() < 1) {
        std::stringstream str;
        str << "Wing component " << GetUID() << " does not contain any segments (CCPACSWingComponentSegment::GetPoint)!";
        throw CTiglError(str.str(), TIGL_ERROR);
    }

    // build up iso xsi line control points
    TColgp_Array1OfPnt xsiPoints(1,segments.size()+1);
    SegmentList::iterator segmentIt = segments.begin();
    int pointIndex = 1;
    for (; segmentIt != segments.end(); ++segmentIt) {
        tigl::CCPACSWingSegment& segment = **segmentIt;

        // build iso xsi line
        gp_Pnt p = segment.GetChordPoint(0,xsi);
        xsiPoints.SetValue(pointIndex, p);
        if (segmentIt == segments.end()-1) {
            // add outer section of last segment
            gp_Pnt p = segment.GetChordPoint(1., xsi);
            xsiPoints.SetValue(pointIndex+1, p);
            // the break should not be necessary here, since the loop is
            break;
        }
        pointIndex++;
    }

    double wingLenApprox = GetWireLength(projLeadingEdge);

    // create iso xsi line as linear interpolation between xsi points
    GeomAPI_PointsToBSpline linearInterpolation(xsiPoints, 1, 1, GeomAbs_C0, Precision::Confusion());
    Handle_Geom_BoundedCurve xsiCurve = linearInterpolation.Curve();

    // extend iso xsi line, so that we can still intersect it
    // even if we have twisted sections
    gp_Pnt p; gp_Vec v;
    xsiCurve->D1(xsiCurve->LastParameter(), p, v);
    GeomLib::ExtendCurveToPoint(xsiCurve, p.XYZ() + v.Normalized().XYZ()*wingLenApprox, 1, true);
    xsiCurve->D1(xsiCurve->FirstParameter(), p, v);
    GeomLib::ExtendCurveToPoint(xsiCurve, p.XYZ() - v.Normalized().XYZ()*wingLenApprox, 1, false);

    // compute eta point and normal on the projected LE
    gp_Pnt etaPnt; gp_Vec etaNormal;
    WireGetPointTangent(projLeadingEdge, eta, etaPnt, etaNormal);

    // plane normal to projected leading edge
    gp_Pln gpPlane(etaPnt, gp_Dir(etaNormal.XYZ()));
    Handle_Geom_Surface plane = new Geom_Plane(gpPlane);

    // compute intersection of plane with iso-xsi line
    GeomAPI_IntCS Intersector(xsiCurve, plane);
    if (Intersector.IsDone() && Intersector.NbPoints() > 0) {
        gp_Pnt p = Intersector.Point(1);
        return p;
    }
    else {
        // Fallback mode, if they are not intersecting
        // we don't need it, if we make leading/trailing edge longer
        TopoDS_Shape xsiWire = BRepBuilderAPI_MakeEdge(xsiCurve);
        TopoDS_Shape etaFace = BRepBuilderAPI_MakeFace(gpPlane);
        BRepExtrema_DistShapeShape extrema(xsiWire, etaFace);
        extrema.Perform();
        return extrema.PointOnShape1(1);
    }
}

void CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi(const std::string& segmentUID, double seta, double sxsi, double& eta, double& xsi)
{
    // search for ETA coordinate
        
    if (seta < 0.0 || seta > 1.0) {
        throw CTiglError("Error: Parameter seta not in the range 0.0 <= seta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
    if (sxsi < 0.0 || sxsi > 1.0) {
        throw CTiglError("Error: Parameter sxsi not in the range 0.0 <= sxsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
        
    SegmentList& segments = GetSegmentList();
    // check that segment belongs to component segment
    CCPACSWingSegment* segment = NULL;
    for (SegmentList::iterator it = segments.begin(); it != segments.end(); ++it) {
        if (segmentUID == (*it)->GetUID()) {
            segment = *it;
            break;
        }
    }
    if (!segment) {
        throw CTiglError("Error: segment does not belong to component segment in CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi", TIGL_ERROR);
    }

    gp_Pnt point3d = segment->GetChordPoint(seta, sxsi);
    xsi = sxsi;

    UpdateProjectedLeadingEdge();
    eta = ProjectPointOnWire(projLeadingEdge, point3d);
}

//    // Returns an upper or lower point on the segment surface in
//    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
//    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
//    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
//    // edge on the outer wing profile. If fromUpper is true, a point
//    // on the upper surface is returned, otherwise from the lower.
//    gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi, bool fromUpper)
//    {
//        if (eta < 0.0 || eta > 1.0)
//        {
//            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
//        }
//
//        CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
//        CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
//
//        // Compute points on wing profiles for the given xsi
//        gp_Pnt innerProfilePoint;
//        gp_Pnt outerProfilePoint;
//        if (fromUpper == true)
//        {
//            innerProfilePoint = innerProfile.GetUpperPoint(xsi);
//            outerProfilePoint = outerProfile.GetUpperPoint(xsi);
//        }
//        else
//        {
//            innerProfilePoint = innerProfile.GetLowerPoint(xsi);
//            outerProfilePoint = outerProfile.GetLowerPoint(xsi);
//        }
//
//        // Do section element transformation on points
//        innerProfilePoint = innerConnection.GetSectionElementTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetSectionElementTransformation().Transform(outerProfilePoint);
//
//        // Do section transformations
//        innerProfilePoint = innerConnection.GetSectionTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetSectionTransformation().Transform(outerProfilePoint);
//
//        // Do positioning transformations
//        innerProfilePoint = innerConnection.GetPositioningTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetPositioningTransformation().Transform(outerProfilePoint);
//
//        // Get point on wing segment in dependence of eta by linear interpolation
//        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(innerProfilePoint, outerProfilePoint);
//        Standard_Real firstParam = profileLine->FirstParameter();
//        Standard_Real lastParam  = profileLine->LastParameter();
//        Standard_Real param = (lastParam - firstParam) * eta;
//        gp_Pnt profilePoint;
//        profileLine->D0(param, profilePoint);
//
//        return profilePoint;
//    }
//

// Gets the fromElementUID of this segment
const std::string & CCPACSWingComponentSegment::GetFromElementUID(void) const
{
    return fromElementUID;
}

// Gets the toElementUID of this segment
const std::string & CCPACSWingComponentSegment::GetToElementUID(void) const
{
    return toElementUID;
}

// Returns the segment to a given point on the componentSegment. 
// Returns null if the point is not an that wing!
const CTiglAbstractSegment* CCPACSWingComponentSegment::findSegment(double x, double y, double z, gp_Pnt& nearestPoint)
{
    CTiglAbstractSegment* result = NULL;
    gp_Pnt pnt(x, y, z);


    SegmentList& segments = GetSegmentList();

    double minDist = std::numeric_limits<double>::max();
    // now discover to which segment the point belongs
    for (SegmentList::iterator segit = segments.begin(); segit != segments.end(); ++segit) {
        try {
            double eta, xsi;
            (*segit)->GetEtaXsi(pnt, eta, xsi);
            gp_Pnt pointProjected = (*segit)->GetChordPoint(eta,xsi);

            // Get nearest point on this segment
            double nextEta = GetNearestValidParameter(eta);
            double nextXsi = GetNearestValidParameter(xsi);
            gp_Pnt currentPoint = (*segit)->GetChordPoint(nextEta, nextXsi);
            
            double currentDist = currentPoint.Distance(pointProjected);
            if (currentDist < minDist) {
                minDist   = currentDist;
                nearestPoint = currentPoint;
                result = *segit;
            }
        }
        catch(...) {
            // do nothing
        }
    }

    // check if pnt lies on component segment shape with 1cm tolerance
    if (minDist > 1.e-2) {
        return NULL;
    }

    return result;
}

MaterialList CCPACSWingComponentSegment::GetMaterials(double eta, double xsi, TiglStructureType type)
{
    MaterialList list;
        
    if (!structure.IsValid()) {
        // return empty list
        return list;
    }
        
    if (type != UPPER_SHELL && type != LOWER_SHELL) {
        LOG(ERROR) << "Cannot compute materials for inner structure in CCPACSWingComponentSegment::GetMaterials (not yet implemented)";
        return list;
    }
    else {
        CCPACSWingShell* shell = (type == UPPER_SHELL? &structure.GetUpperShell() : &structure.GetLowerShell());
        int ncells = shell->GetCellCount();
        for (int i = 1; i <= ncells; ++i){
            CCPACSWingCell& cell = shell->GetCell(i);
            if (!cell.GetMaterial().IsValid()) {
                continue;
            }
                
            if (cell.IsInside(eta,xsi)) {
                list.push_back(&(cell.GetMaterial()));
            }
        }
            
        // add complete skin, only if no cells are defined
        if (list.empty() && shell->GetMaterial().IsValid()){
            list.push_back(&(shell->GetMaterial()));
        }
        
    }
    return list;
}

} // end namespace tigl
