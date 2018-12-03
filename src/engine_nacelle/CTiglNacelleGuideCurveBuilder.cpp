/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-30 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "tiglcommonfunctions.h"
#include "CTiglNacelleGuideCurveBuilder.h"
#include "CCPACSNacelleCowl.h"
#include "CCPACSNacelleGuideCurves.h"
#include "CCPACSNacelleSections.h"
#include "CCPACSNacelleSection.h"
#include "CCPACSWingProfileGetPointAlgo.h"
#include "CCPACSGuideCurveProfile.h"
#include "CTiglBSplineAlgorithms.h"
#include "CTiglPointsToBSplineInterpolation.h"
#include "CTiglUIDManager.h"

#include "GeomAPI_Interpolate.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "Precision.hxx"

namespace  {
tigl::NacelleGuideCurveParameters GetGuideCurveParametersFromCPACS(const tigl::CCPACSNacelleGuideCurve& curve);
}

namespace tigl {

CTiglNacelleGuideCurveBuilder::CTiglNacelleGuideCurveBuilder(const CCPACSNacelleGuideCurve& curve)
   : CTiglNacelleGuideCurveBuilder(GetGuideCurveParametersFromCPACS(curve))
{}

CTiglNacelleGuideCurveBuilder::CTiglNacelleGuideCurveBuilder(const NacelleGuideCurveParameters& params)
   : parameters(params)
{}

TopoDS_Wire CTiglNacelleGuideCurveBuilder::GetWire()
{
    // get first point of guide curve in cartesian coordinates
    TopTools_SequenceOfShape startWire;
    startWire.Append(parameters.fromSection->GetTransformedLowerWire());
    startWire.Append(parameters.fromSection->GetTransformedUpperWire());
    tigl::CCPACSWingProfileGetPointAlgo pointAlgo1(startWire);
    gp_Pnt point_start_cartesian;
    gp_Vec tangent;
    pointAlgo1.GetPointTangent(parameters.fromZeta, point_start_cartesian, tangent);

    // radius and angle in YZ-plane
    double y = point_start_cartesian.Y();
    double z = point_start_cartesian.Z();

    double startX   = point_start_cartesian.X();
    double startR   = sqrt(z*z + y*y);
    double startPhi = 0.;

    if ( fabs(startR) > Precision::Confusion() ) {
        if ( y > 0 ) { startPhi = -acos(z/startR); }
        else         { startPhi =  acos(z/startR); }
        startPhi *= 180/M_PI;
    }

    // get last point of guide curve in cartesian coordinates
    TopTools_SequenceOfShape endWire;
    endWire.Append(parameters.toSection->GetTransformedLowerWire());
    endWire.Append(parameters.toSection->GetTransformedUpperWire());
    tigl::CCPACSWingProfileGetPointAlgo pointAlgo2(endWire);
    gp_Pnt point_end_cartesian;
    pointAlgo2.GetPointTangent(parameters.toZeta, point_end_cartesian, tangent);

    // radius and angle in YZ-plane
    y = point_end_cartesian.Y();
    z = point_end_cartesian.Z();

    double endX   = point_end_cartesian.X();
    double endR   = sqrt(z*z + y*y);
    double endPhi = 0.;

    if ( fabs(endR) > 1e-10 ) {
        if ( y > 0 ) { endPhi = -acos(z/endR); }
        else         { endPhi =  acos(z/endR); }
        endPhi *= 180/M_PI;
    }

    // get all relative polar points of guide curve as gp_Pnt
    std::vector<gp_Pnt> points;

    // add first point, if it is not in the list already (it is assumed that the profile points are sorted)
    if ( parameters.profilePoints.size() == 0 || fabs( parameters.profilePoints[0].x ) > Precision::Confusion() ) {
        points.push_back(gp_Pnt(0., 0., 0.));
    }

    // add profile points
    for ( size_t i=0; i< parameters.profilePoints.size(); ++i ) {
        points.push_back(parameters.profilePoints[i].Get_gp_Pnt());
    }

    // add last point, if it is not in the list already (it is assumed that the profile points are sorted)
    if ( parameters.profilePoints.size() ==0 || fabs( 1. - parameters.profilePoints.back().x ) > Precision::Confusion() ) {
        points.push_back(gp_Pnt(1., 0., 0.));
    }

    // Interpolate profile (relative and polar) points
    Handle(TColgp_HArray1OfPnt) interpRelativePolar = new TColgp_HArray1OfPnt(1, points.size());
    for( size_t i = 1; i<= points.size(); ++i ) {
        interpRelativePolar->SetValue(i, points[i-1]);
    }
    CTiglPointsToBSplineInterpolation interp(interpRelativePolar);
    Handle(Geom_BSplineCurve) spline = interp.Curve();
    CTiglBSplineAlgorithms::reparametrizeBSpline(*spline, 0., 1.);

    // sample profile points on interpolated curve
    std::vector<gp_Pnt> cartesianPoints;
    double phi = startPhi;
    if (  endPhi - startPhi  < Precision::Confusion() ) {
        endPhi += 360.;
    }

    while( phi <= endPhi ) {
        double u = (phi- startPhi)/(endPhi - startPhi);
        gp_Pnt sampledPoint;
        spline->D0(u, sampledPoint);

        // linear interpolation of angles (actually it should hold phi = phii)
        double phii = (1-u)*startPhi + u*endPhi;

        // add cubic step function as "baseline" for relative x coordinate (y-component of sampled point)
        double dx = endX - startX;
        double xi   = -2*dx*u*u*u + 3*dx*u*u + startX + sampledPoint.Y();

        // add cubic step function as "baseline" to sampled relative radius (z-component of sampled point)
        double dr = endR - startR;
        double ri   = -2*dr*u*u*u + 3*dr*u*u + startR + sampledPoint.Z();

        // transform to cartesian coordinates
        cartesianPoints.push_back(gp_Pnt( xi, -ri*sin(Radians(phii)), ri*cos(Radians(phii)) ) );

        if ( fabs(phi -endPhi) < Precision::Confusion() ) {
            break;
        }
        if( phi + dPhi >= endPhi ) {
            phi += (endPhi - phi);
        }
        else {
            phi += dPhi;
        }
    }

    // interpolate with prescribed tangents [cos(startPhi),-sin(startPhi)], [cos(endPhi),-sin(endPhi)]
    gp_Vec startTangent(0., -cos(Radians(startPhi)), -sin(Radians(startPhi)));
    gp_Vec endTangent  (0., -cos(Radians(endPhi  )), -sin(Radians(endPhi  )));
    Handle(TColgp_HArray1OfPnt) interpAbsoluteCartesian = new TColgp_HArray1OfPnt(1, cartesianPoints.size());
    for( size_t i = 1; i<= cartesianPoints.size(); ++i ) {
        interpAbsoluteCartesian->SetValue(i, cartesianPoints[i-1]);
    }
    GeomAPI_Interpolate interPol(interpAbsoluteCartesian, false, Precision::Confusion());
    interPol.Load(startTangent, endTangent);
    interPol.Perform();
    Handle(Geom_BSplineCurve) hcurve = interPol.Curve();

    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hcurve);
    BRepBuilderAPI_MakeWire wireBuilder(edge);
    if (!wireBuilder.IsDone()) {
        throw CTiglError("Wire construction failed in CTiglNacelleGuideCurveBuilder::GetWire", TIGL_ERROR);
    }

    return wireBuilder.Wire();
}

CTiglNacelleGuideCurveBuilder::operator TopoDS_Wire()
{
    return GetWire();
};

} //namespace tigl


namespace  {

tigl::NacelleGuideCurveParameters GetGuideCurveParametersFromCPACS(const tigl::CCPACSNacelleGuideCurve& curve)
{
    tigl::NacelleGuideCurveParameters params;

    // get profile points.
    const tigl::CTiglUIDManager& uidMgr = curve.GetParent()->GetParent()->GetUIDManager();
    tigl::CCPACSGuideCurveProfile& guideCurveProfile = uidMgr.ResolveObject<tigl::CCPACSGuideCurveProfile>(curve.GetGuideCurveProfileUID());
    params.profilePoints = guideCurveProfile.GetGuideCurveProfilePoints();

    // get phi,x,r of start point
    const tigl::CCPACSNacelleSections& sections = curve.GetParent()->GetParent()->GetSections();

    params.fromSection = &sections.GetSection(curve.GetStartSectionUID());
    params.fromZeta = curve.GetFromZeta();

    size_t startSectionIdx = sections.GetSectionIndex(curve.GetStartSectionUID());
    size_t endSectionIdx = startSectionIdx + 1;
    if ( endSectionIdx > sections.GetSectionCount() ) {
        endSectionIdx = 1;
    }
    params.toSection = &sections.GetSection(endSectionIdx);
    params.toZeta = curve.GetToZeta();

    return params;
}

} // anonymous namespace
