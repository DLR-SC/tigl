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

#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingProfile.h"
#include "CTiglLogging.h"
#include "CCPACSWingCell.h"
#include "CTiglApproximateBsplineWire.h"
#include "tiglcommonfunctions.h"

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
#include "Precision.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepTools.hxx"
#include "BRepBndLib.hxx"
#include "BRepGProp.hxx"
#include "Bnd_Box.hxx"
#include "BRepLib_FindSurface.hxx"
#include "ShapeAnalysis_Surface.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "GeomAdaptor_Surface.hxx"
#include "GC_MakeLine.hxx"
#include "Geom_BSplineCurve.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "GeomFill_SimpleBound.hxx"
#include "GeomFill_BSplineCurves.hxx"
#include "GeomFill_FillingStyle.hxx"
#include "Geom_BSplineSurface.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "BRepClass3d_SolidClassifier.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "TColgp_Array1OfPnt.hxx"


#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace {
bool inBetween(const gp_Pnt& p, const gp_Pnt& p1, const gp_Pnt& p2){
    bool inx = (p1.X() <= p.X() && p.X() <= p2.X()) || (p2.X() <= p.X() && p.X() <= p1.X());
    bool iny = (p1.Y() <= p.Y() && p.Y() <= p2.Y()) || (p2.Y() <= p.Y() && p.Y() <= p1.Y());
    bool inz = (p1.Z() <= p.Z() && p.Z() <= p2.Z()) || (p2.Z() <= p.Z() && p.Z() <= p1.Z());
    
    return inx && iny && inz;
}
}

namespace tigl {

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
        if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS)
            name          = ptrName;

        // Get attribute "uid"
        char* ptrUID = NULL;
        tempString   = "uID";
        elementPath  = const_cast<char*>(tempString.c_str());
        if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(segmentXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS)
            SetUID(ptrUID);

        // Get fromElementUID
        char* ptrFromElementUID = NULL;
        tempString = segmentXPath + "/fromElementUID";
        elementPath   = const_cast<char*>(tempString.c_str());
        if (tixiGetTextElement(tixiHandle, elementPath, &ptrFromElementUID) == SUCCESS)
            fromElementUID = ptrFromElementUID;

        // Get toElementUID
        char* ptrToElementUID = NULL;
        tempString = segmentXPath + "/toElementUID";
        elementPath   = const_cast<char*>(tempString.c_str());
        if (tixiGetTextElement(tixiHandle, elementPath, &ptrToElementUID) == SUCCESS)
            toElementUID = ptrToElementUID;
        
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

    
    std::vector<int> CCPACSWingComponentSegment::GetSegmentList(const std::string& fromElementUID, const std::string& toElementUID) const{
        std::vector<int> path;
        path = findPath(fromElementUID, toElementUID, path, true);
        
        if(path.size() == 0){
            // could not find path from fromUID to toUID
            // try the other way around
            path = findPath(toElementUID, fromElementUID, path, true);
        }
        
        if(path.size() == 0){
            LOG(WARNING) << "Could not determine segment list to component segment from \"" 
                         << GetFromElementUID() << "\" to \"" << GetToElementUID() << "\"!";
        }
        
        return path;
    }
    
    // Determines, which segments belong to the component segment
    std::vector<int> CCPACSWingComponentSegment::findPath(const std::string& fromUID, const::std::string& toUID, const std::vector<int>& curPath, bool forward) const{
        if( fromUID == toUID )
            return curPath;
        
        // find all segments with InnerSectionUID == fromUID
        std::vector<int> segList;
        for(int i = 1; i <= wing->GetSegmentCount(); ++i){
            CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(i);
            std::string startUID = forward ? segment.GetInnerSectionElementUID() : segment.GetOuterSectionElementUID();
            if(startUID == fromUID)
                segList.push_back(i);
        }
        
        std::vector<int>::iterator segIt = segList.begin();
        for(; segIt != segList.end(); ++segIt){
            int iseg = *segIt;
            CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(iseg);
            std::vector<int> newpath(curPath);
            newpath.push_back(iseg);
            std::string segEndUID = forward ? segment.GetOuterSectionElementUID() : segment.GetInnerSectionElementUID();
            std::vector<int> result = findPath(segEndUID, toUID, newpath, forward);
            if(result.size() != 0)
                return result;
        }
        
        // return empty list as path could not be found
        std::vector<int> result;
        return result;
    }
    
    TopoDS_Wire CCPACSWingComponentSegment::GetCSLine(double eta1, double xsi1, double eta2, double xsi2, int NSTEPS) {
        BRepBuilderAPI_MakeWire wireBuilder;
        
        gp_Pnt  old_point = GetPoint(eta1,xsi1);
        for(int istep = 1; istep < NSTEPS; ++istep){
            double eta = eta1 + (double) istep/(double) (NSTEPS-1) * (eta2-eta1);
            double xsi = xsi1 + (double) istep/(double) (NSTEPS-1) * (xsi2-xsi1);
            gp_Pnt point = GetPoint(eta,xsi);
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(old_point, point);
            wireBuilder.Add(edge);
            old_point = point;
        }
        return wireBuilder.Wire();
    }
    
    void CCPACSWingComponentSegment::GetSegmentIntersection(const std::string& segmentUID, double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, double &xsi){
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
            for(int istep = 0; istep < NSTEPS; ++istep){
                double eta = csEta1 + (double) istep * deta;
                double xsi = csXsi1 + (double) istep * dxsi;
                gp_Pnt point = GetPoint(eta,xsi);
                points.push_back(point);
            }
            
            BRepBuilderAPI_MakeWire wireBuilder;
            for(int istep = 0; istep < NSTEPS-1; ++istep){
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
                if(dist < 1e-2)
                    hasIntersected = true;
            }

            
            // now lets check in between which points the intersection lies
            int ifound = 0;
            for(int i = 0; i < NSTEPS-1; ++i){
                if(inBetween(result, points[i], points[i+1])){
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
        
        if(hasIntersected) {
            // now check if we found an intersection
            double etaTmp;
            segment.GetEtaXsi(result, false, etaTmp, xsi);
            // by design, result is inside the segment
            // However due to numerics, eta and xsi might
            // be a bit larger than 1 or smaller than 0
            if(etaTmp > 1.) {
                etaTmp = 1.;
            }
            else if(etaTmp < 0) {
                etaTmp = 0.;
            }
            if(xsi > 1.) {
                xsi = 1.;
            }
            else if(xsi < 0) {
                xsi = 0.;
            }

            // check that etaTmp coordinate is correct
            if(fabs(etaTmp - eta) > 1e-6)
                throw CTiglError("Error determining proper eta, xsi coordinates in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
        }
        else {
            throw CTiglError("Component segment line does not intersect outer segment border in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
        }
        
        // test if eta,xsi is valid
        if(segment.GetChordPoint(eta,xsi).Distance(result) > 1e-6){
            throw CTiglError("Error determining proper eta, xsi coordinates in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
        }
    }

    // Builds the loft between the two segment sections
    TopoDS_Shape CCPACSWingComponentSegment::BuildLoft(void)
    {

        BRepOffsetAPI_ThruSections generator(Standard_True, Standard_True, Precision::Confusion() );
        
        std::vector<int> segments = GetSegmentList(fromElementUID, toElementUID);
        if(segments.size() == 0){
            throw CTiglError("Error: Could not find segments in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
        }
        
        for(std::vector<int>::iterator it=segments.begin(); it != segments.end(); ++it){
            int iseg = *it;
            CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(iseg);
            CCPACSWingConnection& startConnection = segment.GetInnerConnection();
    
            CCPACSWingProfile& startProfile = startConnection.GetProfile();
            TopoDS_Wire startWire = startProfile.GetWire(true);
    
            // Do section element transformations
            TopoDS_Shape startShape = startConnection.GetSectionElementTransformation().Transform(startWire);
    
            // Do section transformations
            startShape = startConnection.GetSectionTransformation().Transform(startShape);
    
            // Do positioning transformations (positioning of sections)
            startShape = startConnection.GetPositioningTransformation().Transform(startShape);
    
            // Cast shapes to wires, see OpenCascade documentation
            if (startShape.ShapeType() != TopAbs_WIRE) {
                throw CTiglError("Error: Wrong shape type in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
            }
            startWire = TopoDS::Wire(startShape);
    
            generator.AddWire(startWire);
        }
        {
           // add outer wire
            CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(segments[segments.size()-1]);
            CCPACSWingConnection& endConnection = segment.GetOuterConnection();
            CCPACSWingProfile& endProfile = endConnection.GetProfile();
            TopoDS_Wire endWire = endProfile.GetWire(true);
            TopoDS_Shape endShape = endConnection.GetSectionElementTransformation().Transform(endWire);
            endShape = endConnection.GetSectionTransformation().Transform(endShape);
            endShape = endConnection.GetPositioningTransformation().Transform(endShape);
            if (endShape.ShapeType() != TopAbs_WIRE) {
                throw CTiglError("Error: Wrong shape type in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
            }
            endWire = TopoDS::Wire(endShape);
            generator.AddWire(endWire);
        }

        generator.CheckCompatibility(Standard_False);
        generator.Build();
        TopoDS_Shape loft = generator.Shape();

        // transform with wing transformation
        loft = wing->GetWingTransformation().Transform(loft);

        BRepTools::Clean(loft);

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


    // Gets a point in relative wing coordinates for a given eta and xsi
    gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi)
    {
        // search for ETA coordinate
        std::vector<gp_Pnt> CPointContainer;
        std::vector<gp_Pnt> CPointContainer2d;
        bool inComponentSection = false;

        if (eta < 0.0 || eta > 1.0)
        {
            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
        }
        if (xsi < 0.0 || xsi > 1.0)
        {
            throw CTiglError("Error: Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
        }

        for (int i = 1; i <= wing->GetSegmentCount(); i++)
        {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(i);


            // if we found the outer section, break...
            if( (segment.GetInnerSectionElementUID() == toElementUID) || (segment.GetOuterSectionElementUID() == toElementUID)) {
                gp_Pnt pnt = segment.GetChordPoint(0,xsi);
                CPointContainer.push_back(pnt);

                pnt = segment.GetChordPoint(1, xsi);
                CPointContainer.push_back(pnt);

                i++;
                break;
            }

            // Ok, we found the first segment of this componentSegment
            if(segment.GetInnerSectionElementUID() == fromElementUID) {
                inComponentSection = true;
            }

            // try next segment if this is not within the componentSegment
            if (!inComponentSection) continue;

            gp_Pnt pnt = segment.GetChordPoint(0, xsi);
            CPointContainer.push_back(pnt);
        }


        // make all point in 2d because its only a projection 
        double x_mean = 0.;
        for (unsigned int j = 0; j < CPointContainer.size(); j++)
        {
            gp_Pnt pnt = CPointContainer[j];
            x_mean += pnt.X();
            pnt = gp_Pnt(0, pnt.Y(), pnt.Z());
            CPointContainer2d.push_back(pnt);
        }
        x_mean = x_mean / double(CPointContainer.size());


        // build virtual ETA-line
        BRepBuilderAPI_MakeWire wireBuilder;
       for (unsigned int j = 1; j < CPointContainer2d.size(); j++)
       {
           TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(CPointContainer2d[j - 1], CPointContainer2d[j]);
           wireBuilder.Add(edge);
       }
       TopoDS_Wire etaLinie = wireBuilder.Wire();
        
        // build virtual XSI-line
        BRepBuilderAPI_MakeWire wireBuilderXsi;
       for (unsigned int j = 1; j < CPointContainer.size(); j++)
       {
           TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(CPointContainer[j - 1], CPointContainer[j]);
           wireBuilderXsi.Add(edge);
       }
       TopoDS_Wire xsiLinie = wireBuilderXsi.Wire();

        // ETA 3D point
        BRepAdaptor_CompCurve aCompoundCurve(etaLinie, Standard_True);
        gp_Pnt etaPnt;
        
        Standard_Real len = GCPnts_AbscissaPoint::Length( aCompoundCurve );
        aCompoundCurve.D0( len * eta, etaPnt );


        // intersection line
        gp_Pnt p1(etaPnt), p2(etaPnt);
        p1.SetX(x_mean - 1e3); p2.SetX(x_mean + 1e3);
        BRepBuilderAPI_MakeEdge ME(p1,p2);
        TopoDS_Shape aCrv(ME.Edge());

        //intersection point
        BRepExtrema_DistShapeShape extrema(xsiLinie, aCrv);
        extrema.Perform();
        return extrema.PointOnShape1(1);
    }

    void CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi(const std::string& segmentUID, double seta, double sxsi, double& eta, double& xsi)
    {
        // search for ETA coordinate
        std::vector<gp_Pnt> CPointContainer2d;
        
        if (seta < 0.0 || seta > 1.0)
        {
            throw CTiglError("Error: Parameter seta not in the range 0.0 <= seta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
        }
        if (sxsi < 0.0 || sxsi > 1.0)
        {
            throw CTiglError("Error: Parameter sxsi not in the range 0.0 <= sxsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
        }
        
        std::vector<int> seglist = GetSegmentList(fromElementUID, toElementUID);
        // check that segment belongs to component segment
        CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(segmentUID);
        std::vector<int>::iterator segit = std::find(seglist.begin(), seglist.end(), segment.GetSegmentIndex());
        if (segit == seglist.end())
            throw CTiglError("Error: segment does not belong to component segment in CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi", TIGL_ERROR);
        
        for(segit = seglist.begin(); segit != seglist.end(); ++segit) {
            int segindex = *segit;
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(segindex);
            gp_Pnt pnt = segment.GetChordPoint(0.0,sxsi);
            pnt = gp_Pnt(0.0, pnt.Y(), pnt.Z());
            CPointContainer2d.push_back(pnt);
        }
        if(seglist.size() > 0){
            int segindex = seglist[seglist.size()-1];
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(segindex);
            gp_Pnt pnt = segment.GetChordPoint(1.0,sxsi);
            pnt = gp_Pnt(0.0, pnt.Y(), pnt.Z());
            CPointContainer2d.push_back(pnt);
        }

        // build virtual ETA-line
        BRepBuilderAPI_MakeWire wireBuilder;
        for (unsigned int j = 1; j < CPointContainer2d.size(); j++)
        {
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(CPointContainer2d[j - 1], CPointContainer2d[j]);
            wireBuilder.Add(edge);
        }
        TopoDS_Wire etaLinie = wireBuilder.Wire();

        gp_Pnt point3d = segment.GetChordPoint(seta, sxsi);
        xsi = sxsi;
        eta = ProjectPointOnWire(etaLinie, point3d);
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
    const CTiglAbstractSegment* CCPACSWingComponentSegment::findSegment(double x, double y, double z)
    {
        CTiglAbstractSegment* result = NULL;
        gp_Pnt pnt(x, y, z);

        // Quick check if the point even is on this wing
        BRepClass3d_SolidClassifier quickClassifier;
        
        quickClassifier.Load(wing->GetLoft());
        quickClassifier.Perform(pnt, 1.0e-2);
        if((quickClassifier.State() != TopAbs_IN) && (quickClassifier.State() != TopAbs_ON)){
            return NULL;
        }

        std::vector<int> segmentIndexList = GetSegmentList(fromElementUID, toElementUID);

        // now discover the right segment
        for (std::vector<int>::iterator segit = segmentIndexList.begin(); segit != segmentIndexList.end(); ++segit)
        {
            //Handle_Geom_Surface aSurf = wing->GetUpperSegmentSurface(i);
            TopoDS_Shape segmentLoft = wing->GetSegment(*segit).GetLoft();

            BRepClass3d_SolidClassifier classifier;
            classifier.Load(segmentLoft);
            classifier.Perform(pnt, 1.0e-3);
            if((classifier.State() == TopAbs_IN) || (classifier.State() == TopAbs_ON)){
                result = &(wing->GetSegment(*segit));
                break;
            }
        }

        return result;
    }


#ifdef TIGL_USE_XCAF
    // builds data structure for a TDocStd_Application
    // mostly used for export
    TDF_Label CCPACSWingComponentSegment::ExportDataStructure(CCPACSConfiguration&, Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label)
    {
        TDF_Label subLabel;
        return subLabel;
    }
#endif
    
    MaterialList CCPACSWingComponentSegment::GetMaterials(double eta, double xsi, TiglStructureType type) {
        MaterialList list;
        
        if (!structure.IsValid())
            // return empty list
            return list;
        
        if(type != UPPER_SHELL && type != LOWER_SHELL) {
            LOG(ERROR) << "Cannot compute materials for inner structure in CCPACSWingComponentSegment::GetMaterials (not yet implemented)";
            return list;
        }
        else {
            CCPACSWingShell* shell = type == UPPER_SHELL? &structure.GetUpperShell() : &structure.GetLowerShell();
            int ncells = shell->GetCellCount();
            for (int i = 1; i <= ncells; ++i){
                CCPACSWingCell& cell = shell->GetCell(i);
                if (!cell.GetMaterial().IsValid())
                    continue;
                
                if (cell.IsInside(eta,xsi)){
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
