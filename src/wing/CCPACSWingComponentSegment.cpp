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
#include "CPointsToLinearBSpline.h"
#include "tiglcommonfunctions.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Wire.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "Extrema_ExtCC.hxx"
#include "Geom_Plane.hxx"
#include "Geom_Line.hxx"
#include "gp_Pln.hxx"
#include "Precision.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "BRepTools.hxx"
#include "BRepGProp.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "BRepClass3d_SolidClassifier.hxx"
#include "TColgp_Array1OfPnt.hxx"
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <cassert>

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

    // Set the face traits
    void SetFaceTraits (PNamedShape loft, unsigned int nSegments) 
    { 
        // designated names of the faces
        std::vector<std::string> names(3);
        names[0]="Bottom";
        names[1]="Top";
        names[2]="TrailingEdge";
        std::vector<std::string> endnames(2);
        endnames[0]="Inside";
        endnames[1]="Outside";

        // map of faces
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(loft->Shape(),   TopAbs_FACE, map);

        unsigned int nFaces = map.Extent();
        // check if number of faces without inside and outside surface (nFaces-2) 
        // is a multiple of 2 (without Trailing Edges) or 3 (with Trailing Edges)
        if (!((nFaces-2)/nSegments == 2 || (nFaces-2)/nSegments == 3) || nFaces < 4) {
            LOG(ERROR) << "CCPACSWingComponentSegment: Unable to determine name face names from component segmetn loft";
            return;
        }
        // remove trailing edge name if there is no trailing edge
        if ((nFaces-2)/nSegments == 2) {
            names.erase(names.begin()+2);
        }
        // assign "Top" and "Bottom" to face traits
        for (unsigned int i = 0; i < nFaces-2; i++) {
            CFaceTraits traits = loft->GetFaceTraits(i);
            traits.SetName(names[i%names.size()].c_str());
            loft->SetFaceTraits(i, traits);
        }
        // assign "Inside" and "Outside" to face traits
        for (unsigned int i = nFaces-2; i < nFaces; i++) {
            CFaceTraits traits = loft->GetFaceTraits(i);
            traits.SetName(endnames[i-nFaces+2].c_str());
            loft->SetFaceTraits(i, traits);
        }
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
    CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(segmentUID);

    // compute component segment line
    gp_Pnt p1 = GetPoint(csEta1, csXsi1);
    gp_Pnt p2 = GetPoint(csEta2, csXsi2);
    double csLen = p1.Distance(p2);

    gp_Lin csLine(p1, p2.XYZ() - p1.XYZ());

    // compute iso eta line of segment
    gp_Pnt pLE = segment.GetChordPoint(eta, 0.);
    gp_Pnt pTE = segment.GetChordPoint(eta, 1.);
    double chordDepth = pTE.Distance(pLE);

    gp_Lin etaLine(pLE, pTE.XYZ() - pLE.XYZ());

    // check, if both lines are parallel
    if (etaLine.Direction().IsParallel(csLine.Direction(), M_PI/180.)) {
        throw CTiglError("Component segment line does not intersect iso eta line of segment in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
    }

    Handle_Geom_Curve csCurve = new Geom_Line(csLine);
    Handle_Geom_Curve etaCurve = new Geom_Line(etaLine);
    GeomAdaptor_Curve csAdptAcuve(csCurve);
    GeomAdaptor_Curve etaAdptCurve(etaCurve);

    // find point on etaLine, that minimizes distance to csLine
    Extrema_ExtCC minimizer(csAdptAcuve, etaAdptCurve);
    minimizer.Perform();

    if (!minimizer.IsDone()) {
        throw CTiglError("Component segment line does not intersect iso eta line of segment in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
    }

    // there should be exactly on minimum between two lines
    // if they are not parallel
    assert(minimizer.NbExt() == 1);

    Extrema_POnCurv pOnCSLine, pOnEtaLine;
    minimizer.Points(1, pOnCSLine, pOnEtaLine);

    // If parameter on CS line is < 0 or larger than 
    // Length of line, there is not actual intersection,
    // i.e. the CS Line is choosen to small
    // We use a tolerance here, to account for small user errors
    double tol = 1e-5;
    if (pOnCSLine.Parameter() < -tol || pOnCSLine.Parameter() > csLen + tol) {
        throw CTiglError("Component segment line does not intersect iso eta line of segment in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
    }

    // compute xsi value
    xsi = pOnEtaLine.Parameter()/chordDepth;
}

// get short name for loft
std::string CCPACSWingComponentSegment::GetShortShapeName() 
{
    unsigned int windex = 0;
    unsigned int wcsindex = 0;
    for (int i = 1; i <= wing->GetConfiguration().GetWingCount(); ++i) {
        tigl::CCPACSWing& w = wing->GetConfiguration().GetWing(i);
        if (wing->GetUID() == w.GetUID()) {
            windex = i;
            for (int j = 1; j <= w.GetComponentSegmentCount(); j++) {
                tigl::CTiglAbstractSegment& wcs = w.GetComponentSegment(j);
                if (GetUID() == wcs.GetUID()) {
                    wcsindex = j;
                    std::stringstream shortName;
                    shortName << "W" << windex << "CS" << wcsindex;
                    return shortName.str();
                }
            }
        }
    }
    return "UNKNOWN";
}

// Builds the loft between the two segment sections
PNamedShape CCPACSWingComponentSegment::BuildLoft(void)
{

    BRepOffsetAPI_ThruSections generator(Standard_True, Standard_True, Precision::Confusion() );

    SegmentList& segments = GetSegmentList();
    if (segments.size() == 0) {
        throw CTiglError("Error: Could not find segments in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
    }
        
    for (SegmentList::iterator it=segments.begin(); it != segments.end(); ++it) {
        CCPACSWingSegment& segment = **it;
        TopoDS_Wire startWire = segment.GetInnerWire();
        generator.AddWire(startWire);
    }

    // add outer wire
    CCPACSWingSegment& segment = *segments[segments.size()-1];
    TopoDS_Wire endWire = segment.GetOuterWire();
    generator.AddWire(endWire);

    generator.CheckCompatibility(Standard_False);
    generator.Build();
    TopoDS_Shape loftShape = generator.Shape();

    BRepTools::Clean(loftShape);

    Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
    sfs->Init ( loftShape );
    sfs->Perform();
    loftShape = sfs->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(loftShape, System);
    myVolume = System.Mass();

    // Calculate surface area
    GProp_GProps AreaSystem;
    BRepGProp::SurfaceProperties(loftShape, AreaSystem);
    mySurfaceArea = AreaSystem.Mass();
        
    // Set Names
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft (new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft, segments.size());
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

    // create projection plane
    gp_GTrsf wingTrafo = wing->GetTransformation().Get_gp_GTrsf();
    gp_XYZ pCenter(0,0,0);
    gp_XYZ pDirX(1,0,0);
    wingTrafo.Transforms(pCenter);
    wingTrafo.Transforms(pDirX);
    Handle(Geom_Plane) projPlane = new Geom_Plane(pCenter, pDirX-pCenter);
    
    std::vector<gp_Pnt> LEPointsProjected;
    SegmentList::iterator segmentIt = segments.begin();
    int pointIndex = 1;
    for (; segmentIt != segments.end(); ++segmentIt) {
        tigl::CCPACSWingSegment& segment = **segmentIt;

        // build iso xsi line
        gp_Pnt lep = segment.GetChordPoint(0.,0.);

        // build leading edge projected to the plane
        gp_Pnt lep_proj = GeomAPI_ProjectPointOnSurf(lep, projPlane).NearestPoint();
        LEPointsProjected.push_back(lep_proj);

        if (segmentIt == segments.end()-1) {
            // add outer section of last segment
            gp_Pnt lep = segment.GetChordPoint(1., 0.);
            gp_Pnt lep_proj = GeomAPI_ProjectPointOnSurf(lep, projPlane).NearestPoint();
            LEPointsProjected.push_back(lep_proj);
            // the break should not be necessary here, since the loop is
            break;
        }
        pointIndex++;
    }

    // check if we have to extend the leading edge at wing tip
    unsigned int nPoints = LEPointsProjected.size();
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

    // build projected leading edge curve
    projLeadingEdge = CPointsToLinearBSpline(LEPointsProjected);
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
    TColgp_Array1OfPnt xsiPoints(1,(Standard_Integer) segments.size() + 1);
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

    // compute eta point and normal on the projected LE
    gp_Pnt etaPnt; gp_Vec etaNormal;
    projLeadingEdge->D1(eta, etaPnt, etaNormal);

    // compute intersection with line strips
    bool haveIntersection = false;
    gp_Pnt intersectionPoint;
    for (unsigned int i = 1; i <= segments.size(); ++i) {
        gp_Pnt p1 = xsiPoints.Value(i);
        gp_Pnt p2 = xsiPoints.Value(i+1);

        double denominator = gp_Vec(p2.XYZ() - p1.XYZ())*etaNormal;
        if (fabs(denominator) < 1e-8) {
            continue;
        }
        double alpha = gp_Vec(etaPnt.XYZ() - p1.XYZ())*etaNormal / denominator ;
        if (i == 1 && alpha < 0.) {
            haveIntersection = true;
            intersectionPoint = p1.XYZ() + alpha*(p2.XYZ()-p1.XYZ());
        }
        if (alpha >= 0. && alpha <= 1.) {
            haveIntersection = true;
            intersectionPoint = p1.XYZ() + alpha*(p2.XYZ()-p1.XYZ());
            break;
        }
        if (i == segments.size() && alpha > 1.) {
            haveIntersection = true;
            intersectionPoint = p1.XYZ() + alpha*(p2.XYZ()-p1.XYZ());
        }
    }

    if (haveIntersection) {
        return intersectionPoint;
    }
    else {
        throw CTiglError("Can not compute point in CCPACSWingComponentSegment::GetPoint.", TIGL_MATH_ERROR);
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
    GeomAPI_ProjectPointOnCurve proj(point3d, projLeadingEdge);
    if (proj.NbPoints() > 0) {
        eta = proj.LowerDistanceParameter();
    }
    else {
        throw CTiglError("Cannot compute eta value in CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi", TIGL_ERROR);
    }
}


// Returns the volume of this segment
double CCPACSWingComponentSegment::GetVolume(void)
{
    GetLoft();
    return( myVolume );
}

// Returns the surface area of this segment
double CCPACSWingComponentSegment::GetSurfaceArea(void)
{
    GetLoft();
    return( mySurfaceArea );
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
const CTiglAbstractSegment* CCPACSWingComponentSegment::findSegment(double x, double y, double z, gp_Pnt& nearestPoint, double& deviation)
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

    deviation = minDist;

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
