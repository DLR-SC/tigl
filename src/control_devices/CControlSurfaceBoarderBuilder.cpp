/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-19 Martin Siggel <martin.siggel@dlr.de>
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

#include "CControlSurfaceBoarderBuilder.h"

#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "CTiglIntersectionCalculation.h"

#include <gp_Vec.hxx>
#include <Geom_Plane.hxx>
#include <Geom2dAPI_Interpolate.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepIntCurveSurface_Inter.hxx>
#include <BRepLib.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Edge.hxx>
#include <TColgp_HArray1OfPnt2d.hxx>

#include <vector>

#include <cassert>
#include <algorithm>

namespace
{

    struct Intersection2dResult
    {
        gp_Vec2d tangent;
        gp_Pnt2d pnt;
        double parmOnFirst;
    };
    
    TopoDS_Edge pointsToEdge(Handle(Geom_Surface) surf, gp_Pnt2d p1, gp_Pnt2d p2)
    {
        gp_Pnt p13d = surf->Value(p1.X(), p1.Y());
        gp_Pnt p23d = surf->Value(p2.X(), p2.Y());
        BRepBuilderAPI_MakeEdge edgemaker(p13d, p23d);
        TopoDS_Edge edge =  edgemaker.Edge();
        return edge;
    }

}

namespace tigl
{

CControlSurfaceBoarderBuilder::CControlSurfaceBoarderBuilder(const CTiglControlSurfaceBorderCoordinateSystem& coords, TopoDS_Shape wingShape)
    : _coords(coords), _wingShape(wingShape)
{
}

CControlSurfaceBoarderBuilder::~CControlSurfaceBoarderBuilder()
{
}

TopoDS_Wire CControlSurfaceBoarderBuilder::boarderWithLEShape(double rLEHeight, double xsiNose, double xsiUpper, double xsiLower)
{
    return boarderWithInnerShapeImpl(rLEHeight, xsiNose, xsiUpper, xsiLower, 5.0);
}

TopoDS_Wire CControlSurfaceBoarderBuilder::boarderWithInnerShape(double rTEHeight, double xsiTail, double xsiTEUpper, double xsiTELower)
{
    return boarderWithInnerShapeImpl(rTEHeight, xsiTail, xsiTEUpper, xsiTELower, -5.0);
}

TopoDS_Wire CControlSurfaceBoarderBuilder::boarderSimple(double xsiUpper, double xsiLower)
{
    // compute position of the leading and trailing edge in local coords
    gp_Pln plane = _coords.getPlane();
    gp_Pnt2d _le2d = ProjectPointOnPlane(plane, _coords.getLe());
    gp_Pnt2d _te2d = ProjectPointOnPlane(plane, _coords.getTe());
    
    computeSkinPoints(xsiUpper, xsiLower);
    
    gp_Pnt2d lp2d = _lp2d;
    gp_Pnt2d up2d = _up2d;
    
    gp_Vec2d upNorm2d = up2d.XY() - lp2d.XY();
    gp_Vec2d loNorm2d = -upNorm2d.XY();
    
    if (upNorm2d.Y() < 0) {
        upNorm2d.Multiply(-1.);
    }
    
    if (loNorm2d.Y() > 0) {
        loNorm2d.Multiply(-1.);
    }
    
    // compute some extra points with enough offset, to close the wire outside the wing
    double offset_factor = 5.;
    double ymax = std::max(fabs(lp2d.Y()), fabs(up2d.Y())) * offset_factor;
    double alphaUp = (ymax - up2d.Y())/upNorm2d.Y();
    double alphaLo = (-ymax - lp2d.Y())/loNorm2d.Y();

    gp_Pnt2d upFront2d = up2d.XY() + alphaUp*upNorm2d.XY();
    gp_Pnt2d loFront2d = lp2d.XY() + alphaLo*loNorm2d.XY();

    double xmax = _le2d.X() + offset_factor*(_te2d.X() - _le2d.X());

    gp_Pnt2d upBack2d(xmax, ymax);
    gp_Pnt2d loBack2d(xmax, -ymax);

    Handle(Geom_Surface) surf = new Geom_Plane(plane);
    
    // create the wire
    BRepBuilderAPI_MakeWire wiremaker;
    wiremaker.Add(pointsToEdge( surf, loFront2d, upFront2d));
    wiremaker.Add(pointsToEdge( surf, upFront2d, upBack2d));
    wiremaker.Add(pointsToEdge( surf, upBack2d, loBack2d));
    wiremaker.Add(pointsToEdge( surf, loBack2d, loFront2d));
    
    TopoDS_Wire result = wiremaker.Wire();
    return result;
}

gp_Pnt2d CControlSurfaceBoarderBuilder::upperPoint()
{
    return _up2d;
}

gp_Pnt2d CControlSurfaceBoarderBuilder::lowerPoint()
{
    return _lp2d;
}

gp_Vec2d CControlSurfaceBoarderBuilder::upperTangent()
{
    return _upTan2d;
}

gp_Vec2d CControlSurfaceBoarderBuilder::lowerTangent()
{
    return _loTan2d;
}

TopoDS_Wire CControlSurfaceBoarderBuilder::boarderWithInnerShapeImpl(double relHeightCenterPoint, double xsiCenterPoint, double xsiEdgeUpper, double xsiEdgeLower, double offset_factor)
{
    // compute position of the leading and trailing edge in local coords
    gp_Pln plane = _coords.getPlane();
    gp_Pnt2d _le2d = ProjectPointOnPlane(plane, _coords.getLe());
    gp_Pnt2d _te2d = ProjectPointOnPlane(plane, _coords.getTe());

    computeSkinPoints(xsiEdgeUpper, xsiEdgeLower);

    // Determine the center point of the slat defined by relHeightCenterPoint
    gp_Pnt2d upTmp, loTmp;
    gp_Vec2d dummyVec;
    computeSkinPointsImpl(xsiCenterPoint, upTmp, dummyVec, loTmp, dummyVec);
    gp_Pnt2d centerPoint2d = loTmp.XY() * (1. - relHeightCenterPoint) + upTmp.XY()*relHeightCenterPoint;

    // Compute normals
    gp_Vec2d upNorm2d(-_upTan2d.Y(), _upTan2d.X());
    gp_Vec2d loNorm2d(-_loTan2d.Y(), _loTan2d.X());

    if (upNorm2d.Y() < 0) {
        upNorm2d.Multiply(-1.);
    }

    if (loNorm2d.Y() > 0) {
        loNorm2d.Multiply(-1.);
    }

    // Compute the inner edge by interpolating upper, lower and center point
    Handle(TColgp_HArray1OfPnt2d) points = new TColgp_HArray1OfPnt2d(1,3);
    points->SetValue(1, _lp2d);
    points->SetValue(2, centerPoint2d);
    points->SetValue(3, _up2d);

    Geom2dAPI_Interpolate interp(points, false, Precision::Confusion());

    // make the upper tangent point away from the inner edge
    if ((centerPoint2d.X() > _up2d.X()) != (_upTan2d.X() < 0)) {
        _upTan2d.Multiply(-1.);
    }

    // make the lower tangent point to the inner edge
    if ((centerPoint2d.X() > _lp2d.X()) != (_loTan2d.X() > 0)) {
        _loTan2d.Multiply(-1.);
    }

    interp.Load(_loTan2d, _upTan2d);
    interp.Perform();

    TopoDS_Edge innerEdge = BRepBuilderAPI_MakeEdge(interp.Curve(), new Geom_Plane(plane));
    BRepLib::BuildCurves3d(innerEdge);

    // compute some extra points with enough offset, to close the wire outside the wing
    double ymax = std::max(fabs(_lp2d.Y()), fabs(_up2d.Y())) * fabs(offset_factor);
    double alphaUp = (ymax - _up2d.Y())/upNorm2d.Y();
    double alphaLo = (-ymax - _lp2d.Y())/loNorm2d.Y();

    gp_Pnt2d upFront2d = _up2d.XY() + alphaUp*upNorm2d.XY();
    gp_Pnt2d loFront2d = _lp2d.XY() + alphaLo*loNorm2d.XY();

    double xmax = _le2d.X() + offset_factor*(_te2d.X() - _le2d.X());

    gp_Pnt2d upBack2d(xmax, ymax);
    gp_Pnt2d loBack2d(xmax, -ymax);

    Handle(Geom_Surface) surf = new Geom_Plane(plane);

    // create the wire
    BRepBuilderAPI_MakeWire wiremaker;
    wiremaker.Add(innerEdge);
    wiremaker.Add(pointsToEdge( surf, _up2d, upFront2d));
    wiremaker.Add(pointsToEdge( surf, upFront2d, upBack2d));
    wiremaker.Add(pointsToEdge( surf, upBack2d, loBack2d));
    wiremaker.Add(pointsToEdge( surf, loBack2d, loFront2d));
    wiremaker.Add(pointsToEdge( surf, loFront2d, _lp2d));

    TopoDS_Wire result = wiremaker.Wire();
    return result;
}


void CControlSurfaceBoarderBuilder::computeSkinPointsImpl(double xsi, gp_Pnt2d& pntUp, gp_Vec2d& tanUp, gp_Pnt2d& pntLo, gp_Vec2d& tanLo)
{
    gp_Pnt start3d = _coords.getTe().XYZ() * (1-xsi) + _coords.getLe().XYZ()*xsi;
    gp_Lin line3d(start3d, _coords.getYDir().XYZ());

    std::vector<Intersection2dResult> intersections;

    BRepIntCurveSurface_Inter intersector;
    for(intersector.Init(_wingShape, line3d, 1e-6); intersector.More(); intersector.Next()) {
        // compute normal vector to the face
        const TopoDS_Face& face = intersector.Face();

        gp_Pnt pnt3d;
        gp_Vec du, dv, faceNormal3d;
        BRep_Tool::Surface(face)->D1(intersector.U(), intersector.V(), pnt3d, du, dv);
        faceNormal3d = du.Crossed(dv);

        // compute tangent, wich is a cross product of both face normals
        gp_Vec tangent3d = _coords.getNormal().Crossed(faceNormal3d);

        // convert to 2d
        gp_Pnt2d pnt2d = ProjectPointOnPlane(_coords.getPlane(), pnt3d);
        gp_Pnt2d stop2d = ProjectPointOnPlane(_coords.getPlane(), pnt3d.XYZ() + tangent3d.XYZ());

        gp_Vec2d tan2d = stop2d.XY() - pnt2d.XY();

        // store results
        Intersection2dResult result;
        result.parmOnFirst = intersector.W();
        result.pnt = pnt2d;
        result.tangent = tan2d;
        intersections.push_back(result);
    }

    if (intersections.size() != 2) {
        throw CTiglError("Number of intersections is not 2");
    }

    Intersection2dResult lowerInters = intersections[0];
    Intersection2dResult upperInters = intersections[1];

    // sort according to direction of line2d
    if (lowerInters.parmOnFirst > upperInters.parmOnFirst) {
        Intersection2dResult tmp = upperInters;
        upperInters = lowerInters;
        lowerInters = tmp;
    }

    pntUp = upperInters.pnt;
    pntLo = lowerInters.pnt;
    tanUp = upperInters.tangent;
    tanLo = lowerInters.tangent;
}

void CControlSurfaceBoarderBuilder::computeSkinPoints(double xsiUpper, double xsiLower)
{
    // get the points on the skin
    if (fabs(xsiUpper-xsiLower) < 1e-10) {
        computeSkinPointsImpl(xsiUpper, _up2d, _upTan2d, _lp2d, _loTan2d);
    }
    else {
        gp_Pnt2d tmpPnt;
        gp_Vec2d tmpVec;
        computeSkinPointsImpl(xsiUpper, _up2d, _upTan2d, tmpPnt, tmpVec);
        computeSkinPointsImpl(xsiLower,  tmpPnt, tmpVec, _lp2d, _loTan2d);
    }
}


} // namespace tigl
