#include "CControlSurfaceBoarderBuilder.h"

#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "CTiglIntersectionCalculation.h"

#include <gp_Vec.hxx>
#include <GeomAPI.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAPI_Interpolate.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepLib.hxx>
#include <TopoDS_Face.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <IntRes2d_IntersectionPoint.hxx>
#include <TColgp_HArray1OfPnt2d.hxx>

#include <vector>

#include <cassert>

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

CControlSurfaceBoarderBuilder::CControlSurfaceBoarderBuilder(const CSCoordSystem& coords, TopoDS_Shape wingShape)
    : _coords(coords), _wingShape(wingShape)
{
    computeWingCutWire();
}

CControlSurfaceBoarderBuilder::~CControlSurfaceBoarderBuilder()
{
}

TopoDS_Wire CControlSurfaceBoarderBuilder::boarderWithLEShape(double rLEHeight, double xsiUpper, double xsiLower)
{
    // compute position of the leading and trailing edge in local coords
    gp_Pln plane = _coords.getPlane();
    _le2d = ProjectPointOnPlane(plane, _coords.getLe());
    _te2d = ProjectPointOnPlane(plane, _coords.getTe());
    
    // get the points on the skin
    computeSkinPoint(xsiUpper, true, _up2d, _upTan2d);
    computeSkinPoint(xsiLower, false, _lp2d, _loTan2d);
    
    // determine the relative height
    gp_Vec2d dummyVec;
    gp_Pnt2d upTmp, loTmp;
    computeSkinPoint(1.0, true, upTmp, dummyVec);
    computeSkinPoint(1.0, false, loTmp, dummyVec);
    
    gp_Pnt2d lep2d = loTmp.XY() * (1. - rLEHeight) + upTmp.XY()*rLEHeight;
    
    gp_Pnt2d lp2d = _lp2d;
    gp_Pnt2d up2d = _up2d;

    
    gp_Vec2d upTan2d = _upTan2d;
    gp_Vec2d loTan2d = _loTan2d;
    
    gp_Vec2d upNorm2d(-upTan2d.Y(), upTan2d.X());
    gp_Vec2d loNorm2d(-loTan2d.Y(), loTan2d.X());
    
    if (upNorm2d.Y() < 0) {
        upNorm2d.Multiply(-1.);
    }
    
    if (loNorm2d.Y() > 0) {
        loNorm2d.Multiply(-1.);
    }
    
    // Compute the leading edge by interpolating upper, lower and leading edge point
    Handle(TColgp_HArray1OfPnt2d) points = new TColgp_HArray1OfPnt2d(1,3);
    points->SetValue(1, lp2d);
    points->SetValue(2, lep2d);
    points->SetValue(3, up2d);
    
    Geom2dAPI_Interpolate interp(points, false, Precision::Confusion());

    // only create c1 continous cutouts, if leading edge is in front of upper and lower points
    if (lep2d.X() > up2d.X()) {
        if (upTan2d.X() > 0) {
            upTan2d.Multiply(-1);
        }
    }
    else {
        if (upTan2d.X() < 0) {
            upTan2d.Multiply(-1);
        }
    }
    
    if (lep2d.X() > lp2d.X()) {
        if (loTan2d.X() < 0) {
            loTan2d.Multiply(-1);
        }
    }
    else {
        if (loTan2d.X() > 0) {
            loTan2d.Multiply(-1);
        }
    }
    interp.Load(loTan2d, upTan2d);
    interp.Perform();
    
    TopoDS_Edge leadEdge = BRepBuilderAPI_MakeEdge(interp.Curve(), new Geom_Plane(plane));
    BRepLib::BuildCurves3d(leadEdge);
    
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
    wiremaker.Add(leadEdge);
    wiremaker.Add(pointsToEdge( surf, up2d, upFront2d));
    wiremaker.Add(pointsToEdge( surf, upFront2d, upBack2d));
    wiremaker.Add(pointsToEdge( surf, upBack2d, loBack2d));
    wiremaker.Add(pointsToEdge( surf, loBack2d, loFront2d));
    wiremaker.Add(pointsToEdge( surf, loFront2d, lp2d));
    
    TopoDS_Wire result = wiremaker.Wire();
    
    BRepTools::Write(result, "resultwire.brep");
    
    return result;
}

TopoDS_Wire CControlSurfaceBoarderBuilder::boarderSimple()
{
    // compute position of the leading and trailing edge in local coords
    gp_Pln plane = _coords.getPlane();
    _le2d = ProjectPointOnPlane(plane, _coords.getLe());
    _te2d = ProjectPointOnPlane(plane, _coords.getTe());
    
    // get the points on the skin
    computeSkinPoint(1.0, true, _up2d, _upTan2d);
    computeSkinPoint(1.0, false, _lp2d, _loTan2d);
    
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
    
    BRepTools::Write(result, "resultwire.brep");
    
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



void CControlSurfaceBoarderBuilder::computeWingCutWire()
{
    gp_Pln pln = _coords.getPlane();
    _wingCutFace =  BRepBuilderAPI_MakeFace(pln).Face();
    
    assert(!_wingCutFace.IsNull());
    
    BRepTools::Write(_wingCutFace, "cutface.brep");
    BRepTools::Write(_wingShape, "wingshape.brep");

    // TODO: shape cache
    CTiglIntersectionCalculation intersector(0, "wing", _wingShape, _coords.getLe(), _coords.getNormal().XYZ());

    // TODO: There might be more faces, e.g. in case of a box wing
    if (intersector.GetCountIntersectionLines() >= 1) {
        _wingCutWire = intersector.GetWire(1);
        
        if (intersector.GetCountIntersectionLines() > 1) {
            throw CTiglError("More than one plane-wing section found. Not yet implemented");
        }
    }
    else {
        throw CTiglError("No plane-wing section found.");
    }
}

void CControlSurfaceBoarderBuilder::computeSkinPoint(double xsi, bool getUpper, gp_Pnt2d& pnt, gp_Vec2d& tangent)
{
    assert(!_wingCutWire.IsNull());
    
    BRepTools::Write(_wingCutWire, "cutwire.brep");
    
    gp_Pnt2d start = _te2d.XY() * (1-xsi) + _le2d.XY()*xsi;
    
    assert(fabs(start.Y()) < 1e-10);
    
    Handle(Geom2d_Curve) line2d = new Geom2d_Line(start, gp_Dir2d(0,1));
    
    
    // compute intersection of wire with line
    TopExp_Explorer edgeExplorer(_wingCutWire, TopAbs_EDGE);
    std::vector<Intersection2dResult> intersections;
    for (; edgeExplorer.More(); edgeExplorer.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
        
        Standard_Real firstParam;
        Standard_Real lastParam;
        Handle(Geom2d_Curve) curve2d = BRep_Tool::CurveOnSurface(edge, _wingCutFace, firstParam, lastParam);
        curve2d = new Geom2d_TrimmedCurve(curve2d, firstParam, lastParam);
        
        Geom2dAPI_InterCurveCurve intersection(line2d, curve2d);
        for (int n = 1; n <= intersection.NbPoints(); n++) {
            IntRes2d_IntersectionPoint pnt = intersection.Intersector().Point(n);
            Intersection2dResult result;
            result.parmOnFirst = pnt.ParamOnFirst();
            curve2d->D1(pnt.ParamOnSecond(), result.pnt, result.tangent);
            intersections.push_back(result);
        }
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
    
    if (getUpper) {
        pnt = upperInters.pnt;
        tangent = upperInters.tangent;
    }
    else {
        pnt = lowerInters.pnt;
        tangent = lowerInters.tangent;
    }
}



CSCoordSystem::CSCoordSystem(gp_Pnt le, gp_Pnt te, gp_Vec yDir)
    : _le(le), _te(te), _ydir(yDir)
{
}

gp_Vec CSCoordSystem::getNormal() const
{
    gp_Vec xDir = getXDir();
    xDir.Normalize();
    
    gp_Vec normal = xDir.Crossed(_ydir.Normalized());
    return normal;
}

gp_Pnt CSCoordSystem::getLe() const
{
    return _le;
}

gp_Pnt CSCoordSystem::getTe() const
{
    return _te;
}

gp_Vec CSCoordSystem::getXDir() const
{
    return _te.XYZ() - _le.XYZ();
}

gp_Vec CSCoordSystem::getYDir() const
{
    return _ydir;
}

gp_Pln CSCoordSystem::getPlane() const
{
    gp_Pln plane(gp_Ax3(_le, getNormal().XYZ(), getXDir().XYZ()));
    return plane;
}



} // namespace tigl
