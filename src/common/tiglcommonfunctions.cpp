/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-06-01 Martin Siggel <Martin.Siggel@dlr.de>
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
 * @brief This file implements some commonly used functions/algorithms on opencascade
 * data structures.
 */

#define NOMINMAX

#if defined _WIN32 || defined __WIN32__
#include <Shlwapi.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "tiglcommonfunctions.h"

#include "CTiglError.h"
#include "CNamedShape.h"
#include "boolean_operations/CBooleanOperTools.h"
#include "boolean_operations/BRepSewingToBRepBuilderShapeAdapter.h"
#include "ListPNamedShape.h"
#include "CNamedShape.h"
#include "PNamedShape.h"
#include "CTiglRelativelyPositionedComponent.h"

#include "Geom_Curve.hxx"
#include "Geom_Surface.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "TopExp_Explorer.hxx"
#include "TopExp.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Edge.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "BRepAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRep_Builder.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "BRepTools.hxx"
#include "BRepBuilderAPI_Sewing.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "BRepExtrema_ExtCF.hxx"
#include "BRepIntCurveSurface_Inter.hxx"
#include "BRepFill.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepClass3d_SolidClassifier.hxx"

#include <Approx_Curve3d.hxx>
#include <BRepAdaptor_HCompCurve.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include "BRepExtrema_ExtCC.hxx"
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepFill_CompatibleWires.hxx>
#include <BRepFill_Filling.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_EdgeConnect.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <GEOMAlgo_Splitter.hxx>
#include <ShapeFix_Wire.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <Standard_Version.hxx>

#include <ShapeAnalysis_FreeBounds.hxx>


#include <list>
#include <algorithm>
#include <cassert>
#include <limits>

#include "Debugging.h"

namespace
{
    struct IsSame
    {
        IsSame(double tolerance) : _tol(tolerance) {}
        bool operator() (double first, double second)
        {
            return (fabs(first-second)<_tol);
        }

        double _tol;
    };

} // anonymous namespace

// calculates a wire's circumference
Standard_Real GetLength(const TopoDS_Wire& wire)
{
    BRepAdaptor_CompCurve aCompoundCurve(wire, Standard_True);
    return GCPnts_AbscissaPoint::Length( aCompoundCurve );
}

Standard_Real GetLength(const TopoDS_Edge &edge)
{
    Standard_Real umin, umax;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, umin, umax);
    GeomAdaptor_Curve adaptorCurve(curve, umin, umax);
    Standard_Real length = GCPnts_AbscissaPoint::Length(adaptorCurve, umin, umax);
    return length;
}

unsigned int GetNumberOfEdges(const TopoDS_Shape& shape)
{
    TopExp_Explorer edgeExpl(shape, TopAbs_EDGE);
    unsigned int iEdges = 0;
    for (; edgeExpl.More(); edgeExpl.Next()) {
        iEdges++;
    }

    return iEdges;
}

unsigned int GetNumberOfFaces(const TopoDS_Shape& shape)
{
    TopExp_Explorer faceExpl(shape, TopAbs_FACE);
    unsigned int iFaces = 0;
    for (; faceExpl.More(); faceExpl.Next()) {
        iFaces++;
    }

    return iFaces;
}

unsigned int GetNumberOfSubshapes(const TopoDS_Shape &shape)
{
    if (shape.ShapeType() == TopAbs_COMPOUND) {
        unsigned int n = 0;
        for (TopoDS_Iterator anIter(shape); anIter.More(); anIter.Next()) {
            n++;
        }
        return n;
    }
    else {
        return 0;
    }
}

// Gets a point on the wire line in dependence of a parameter alpha with
// 0.0 <= alpha <= 1.0. For alpha = 0.0 this is the line starting point,
// for alpha = 1.0 the last point on the intersection line.
gp_Pnt WireGetPoint(const TopoDS_Wire& wire, double alpha)
{
    gp_Pnt point;
    gp_Vec tangent;
    WireGetPointTangent(wire, alpha, point, tangent);
    return point;
}

void WireGetPointTangent(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& tangent)
{
    if (alpha < 0.0 || alpha > 1.0) {
        throw tigl::CTiglError("Parameter alpha not in the range 0.0 <= alpha <= 1.0 in WireGetPointTangent", TIGL_ERROR);
    }
    // ETA 3D point
    BRepAdaptor_CompCurve aCompoundCurve(wire, Standard_True);

    Standard_Real len =  GCPnts_AbscissaPoint::Length( aCompoundCurve );
    if (len < Precision::Confusion()) {
        throw tigl::CTiglError("WireGetPointTangent: Unable to compute tangent on zero length wire", TIGL_MATH_ERROR);
    }
    GCPnts_AbscissaPoint algo(aCompoundCurve, len*alpha, aCompoundCurve.FirstParameter());
    if (algo.IsDone()) {
        double par = algo.Parameter();
        aCompoundCurve.D1( par, point, tangent );
        // normalize tangent to length of the curve
        tangent = len*tangent/tangent.Magnitude();
    }
    else {
        throw tigl::CTiglError("WireGetPointTangent: Cannot compute point on curve.", TIGL_MATH_ERROR);
    }
}

gp_Pnt EdgeGetPoint(const TopoDS_Edge& edge, double alpha)
{
    gp_Pnt point;
    gp_Vec tangent;
    EdgeGetPointTangent(edge, alpha, point, tangent);
    return point;
}

void EdgeGetPointTangent(const TopoDS_Edge& edge, double alpha, gp_Pnt& point, gp_Vec& tangent)
{
    if (alpha < 0.0 || alpha > 1.0) {
        throw tigl::CTiglError("Parameter alpha not in the range 0.0 <= alpha <= 1.0 in EdgeGetPointTangent", TIGL_ERROR);
    }
    // ETA 3D point
    Standard_Real umin, umax;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, umin, umax);
    GeomAdaptor_Curve adaptorCurve(curve, umin, umax);
    Standard_Real len =  GCPnts_AbscissaPoint::Length( adaptorCurve, umin, umax );
    if (len < Precision::Confusion()) {
        throw tigl::CTiglError("EdgeGetPointTangent: Unable to compute tangent on zero length edge", TIGL_MATH_ERROR);
    }
    GCPnts_AbscissaPoint algo(adaptorCurve, len*alpha, umin);
    if (algo.IsDone()) {
        double par = algo.Parameter();
        adaptorCurve.D1( par, point, tangent );
        // normalize tangent to length of the curve
        tangent = len*tangent/tangent.Magnitude();
    }
    else {
        throw tigl::CTiglError("EdgeGetPointTangent: Cannot compute point on curve.", TIGL_MATH_ERROR);
    }
}

Standard_Real ProjectPointOnWire(const TopoDS_Wire& wire, gp_Pnt p)
{
    double smallestDist = DBL_MAX;
    double alpha  = 0.;
    int edgeIndex = 0;

    // find edge with closest dist to point p
    BRepTools_WireExplorer wireExplorer;
    int iwire = 0;
    for (wireExplorer.Init(wire); wireExplorer.More(); wireExplorer.Next(), iwire++) {
        Standard_Real firstParam, lastParam;
        TopoDS_Edge edge = wireExplorer.Current();
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);

        GeomAPI_ProjectPointOnCurve proj(p, curve, firstParam, lastParam);
        if (proj.NbPoints() > 0 && proj.LowerDistance() < smallestDist) {
            smallestDist = proj.LowerDistance();
            edgeIndex = iwire;
            alpha = proj.LowerDistanceParameter();
        }
    }

    // compute partial length of wire until projection point is reached
    wireExplorer.Init(wire);
    double partLength = 0.;
    for (int iwire = 0; iwire <= edgeIndex; ++iwire) {
        Standard_Real firstParam;
        Standard_Real lastParam;
        TopoDS_Edge edge = wireExplorer.Current();
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);
        GeomAdaptor_Curve adaptorCurve(curve, firstParam, lastParam);
        if (iwire == edgeIndex) {
            lastParam = alpha;
        }

        partLength += GCPnts_AbscissaPoint::Length(adaptorCurve, firstParam, lastParam);
        wireExplorer.Next();
    }

    // return relative coordinate
    double normalizedLength = partLength/GetLength(wire);
    if (normalizedLength > 1.0) {
        normalizedLength = 1.0;
    }
    else if (normalizedLength < 0.0) {
        normalizedLength = 0.0;
    }
    return normalizedLength;
}

gp_Pnt GetCentralFacePoint(const TopoDS_Face& face)
{
    // compute point on face
    Standard_Real umin, umax, vmin, vmax;

    gp_Pnt p;

    Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
    BRepTools::UVBounds(face, umin, umax, vmin, vmax);
    Standard_Real umean = 0.5*(umin+umax);
    Standard_Real vmean = 0.5*(vmin+vmax);


    // compute intersection of u-iso line with face boundaries
    Handle(Geom2d_Curve) uiso = new Geom2d_Line(
                gp_Pnt2d(umean,0.),
                gp_Dir2d(0., 1.)
                );

    TopExp_Explorer exp (face,TopAbs_EDGE);
    std::list<double> intersections;
    for (; exp.More(); exp.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(exp.Current());
        Standard_Real first, last;

        // Get geomteric curve from edge
        Handle(Geom2d_Curve) hcurve = BRep_Tool::CurveOnSurface(edge, face, first, last);
        hcurve = new Geom2d_TrimmedCurve(hcurve, first, last);

        Geom2dAPI_InterCurveCurve intersector(uiso, hcurve);
        for (int ipoint = 0; ipoint < intersector.NbPoints(); ++ipoint) {
            gp_Pnt2d p = intersector.Point(ipoint+1);
            intersections.push_back(p.Y());
        }
    }

    // remove duplicate solutions defined by tolerance
    double tolerance = 1e-5;
    intersections.sort();
    intersections.unique(IsSame((vmax-vmin)*tolerance));

    // normally we should have at least two intersections
    // also the number of sections should be even - else something is really strange
    //assert(intersections.size() % 2 == 0);
    if (intersections.size() >= 2) {
        std::list<double>::iterator it = intersections.begin();
        double int1 = *it++;
        double int2 = *it;
        vmean = (int1 + int2)/2.;
    }

    surface->D0(umean, vmean, p);

    return p;
}

ListPNamedShape GroupFaces(const PNamedShape shape, tigl::ShapeGroupMode groupType)
{
    ListPNamedShape shapeList;
    if (!shape) {
        return shapeList;
    }

    if (groupType == tigl::NAMED_COMPOUNDS) {
        BRep_Builder b;
        TopTools_IndexedMapOfShape faceMap;
        std::map<PNamedShape, TopoDS_Shape> map;
        TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
        if (faceMap.Extent() == 0) {
            // return the shape as is
            shapeList.push_back(shape);
            return shapeList;
        }
        
        for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
            TopoDS_Face face = TopoDS::Face(faceMap(iface));
            PNamedShape origin = shape->GetFaceTraits(iface-1).Origin();

            std::map<PNamedShape, TopoDS_Shape>::iterator it = map.find(origin);
            if (it == map.end()) {
                TopoDS_Compound c;
                b.MakeCompound(c);
                b.Add(c, face);
                map[origin] = c;
            }
            else {
                TopoDS_Shape& c = it->second;
                b.Add(c, face);
            }
        }

        // create Named Shapes
        std::map<PNamedShape, TopoDS_Shape>::iterator it;
        for (it = map.begin(); it != map.end(); ++it) {
            PNamedShape  origin     = it->first;
            TopoDS_Shape toposhape  = it->second;
            PNamedShape curshape;
            if (origin) {
                curshape = PNamedShape(new CNamedShape(toposhape, origin->Name()));
                curshape->SetShortName(origin->ShortName());
            }
            else {
                curshape = PNamedShape(new CNamedShape(toposhape, shape->Name()));
                curshape->SetShortName(shape->ShortName());
            }
            // set the original face traits
            CBooleanOperTools::AppendNamesToShape(shape, curshape);

            // make shells
            curshape = CBooleanOperTools::Shellify(curshape);
            shapeList.push_back(curshape);
        }
    }
    else if (groupType == tigl::WHOLE_SHAPE) {
        shapeList.push_back(shape);
    }
    else if (groupType == tigl::FACES) {
        // store each face as an own shape
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape->Shape(), TopAbs_FACE, faceMap);
        if (faceMap.Extent() == 0) {
            // return the shape as is
            shapeList.push_back(shape);
            return shapeList;
        }
        
        for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
            TopoDS_Face face = TopoDS::Face(faceMap(iface));
            const CFaceTraits& traits = shape->GetFaceTraits(iface-1);

            PNamedShape faceShape;
            if (traits.Origin()) {
                faceShape = PNamedShape(new CNamedShape(face, traits.Origin()->Name()));
                faceShape->SetShortName(traits.Origin()->ShortName());
            }
            else {
                faceShape = PNamedShape(new CNamedShape(face, shape->Name()));
                faceShape->SetShortName(shape->ShortName());
            }
            faceShape->SetFaceTraits(0, shape->GetFaceTraits(iface-1));
            shapeList.push_back(faceShape);
        }
        
    }
    return shapeList;
}

// projects a point onto the line (lineStart<->lineStop) and returns the projection parameter
Standard_Real ProjectPointOnLine(gp_Pnt p, gp_Pnt lineStart, gp_Pnt lineStop)
{
    return gp_Vec(lineStart, p) * gp_Vec(lineStart, lineStop) / gp_Vec(lineStart, lineStop).SquareMagnitude();
}

IntStatus IntersectLinePlane(gp_Pnt p1, gp_Pnt p2, gp_Pln plane, gp_Pnt& result)
{
    gp_Vec normal = plane.Axis().Direction();
    gp_Pnt center = plane.Axis().Location();

    double denominator = gp_Vec(p2.XYZ() - p1.XYZ())*normal;
    if (fabs(denominator) < 1e-8) {
        return NoIntersection;
    }

    double alpha = gp_Vec(center.XYZ() - p1.XYZ())*normal / denominator ;
    result = p1.XYZ() + alpha*(p2.XYZ()-p1.XYZ());

    if (alpha >= 0. && alpha <= 1.) {
        return BetweenPoints;
    }
    else if (alpha < 0.) {
        return OutsideBefore;
    }
    else {
        // alpha > 1.
        return OutsideAfter;
    }
}

// Returns the coordinates of the bounding box of the shape
void GetShapeExtension(const TopoDS_Shape& shape,
                       double& minx, double& maxx,
                       double& miny, double& maxy,
                       double& minz, double& maxz)
{
    Bnd_Box boundingBox;
    BRepBndLib::Add(shape, boundingBox);
    boundingBox.Get(minx, miny, minz, maxx, maxy, maxz);
}

// Returns a unique Hashcode for a specific geometric component
int GetComponentHashCode(tigl::ITiglGeometricComponent& component)
{
    const TopoDS_Shape& loft = (*component.GetLoft()).Shape();
    if (!loft.IsNull()) {
        return loft.HashCode(2294967295);
    }
    else {
        return 0;
    }
}

// Creates an Edge from the given Points by B-Spline interpolation
TopoDS_Edge EdgeSplineFromPoints(const std::vector<gp_Pnt>& points)
{
    unsigned int pointCount = static_cast<int>(points.size());
    
    Handle(TColgp_HArray1OfPnt) hpoints = new TColgp_HArray1OfPnt(1, pointCount);
    for (unsigned int j = 0; j < pointCount; j++) {
        hpoints->SetValue(j + 1, points[j]);
    }

    GeomAPI_Interpolate interPol(hpoints, Standard_False, Precision::Confusion());
    interPol.Perform();
    Handle(Geom_BSplineCurve) hcurve = interPol.Curve();
    
    return BRepBuilderAPI_MakeEdge(hcurve);
}

TopoDS_Edge GetEdge(const TopoDS_Shape &shape, int iEdge)
{
    TopTools_IndexedMapOfShape edgeMap;
    TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
    
    if (iEdge < 0 || iEdge >= edgeMap.Extent()) {
        return TopoDS_Edge();
    }
    else {
        return TopoDS::Edge(edgeMap(iEdge+1));
    }
}

TopoDS_Face GetFace(const TopoDS_Shape &shape, int iFace)
{
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);

    if (iFace < 0 || iFace >= faceMap.Extent()) {
        return TopoDS_Face();
    }
    else {
        return TopoDS::Face(faceMap(iFace + 1));
    }
}

Handle(Geom_BSplineCurve) GetBSplineCurve(const TopoDS_Edge& e)
{
    double u1, u2;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(e, u1, u2);
    curve = new Geom_TrimmedCurve(curve, u1, u2);
    
    // convert to bspline
    Handle(Geom_BSplineCurve) bspl =  GeomConvert::CurveToBSplineCurve(curve);
    return bspl;
}

namespace {
    bool GetIntersectionPoint_impl(const TopoDS_Face& face, const TopoDS_Edge& edge, gp_Pnt& dst, double tolerance)
    {
        BRepIntCurveSurface_Inter faceCurveInter;

        double umin = 0., umax = 0.;
        const Handle(Geom_Curve)& curve = BRep_Tool::Curve(edge, umin, umax);

        faceCurveInter.Init(face, GeomAdaptor_Curve(curve, umin, umax), tolerance);


        if (faceCurveInter.More()) {
            dst = faceCurveInter.Pnt();
            faceCurveInter.Next();
            while (faceCurveInter.More()) {
                if (!dst.IsEqual(faceCurveInter.Pnt(), Precision::Confusion())) {
                    LOG(WARNING) << "Multiple Intersections found in GetIntersectionPoint";
                }
                faceCurveInter.Next();
            }
            return true;
        }

        return false;
    }
}

bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Edge& edge, gp_Pnt& dst, double tolerance)
{
    const bool intersection = GetIntersectionPoint_impl(face, edge, dst, tolerance);
    if (!intersection) {
        LOG(WARNING) << "No Intersections found in GetIntersectionPoint";
    }
    return intersection;
}


bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Wire& wire, gp_Pnt& dst, double tolerance)
{
    DEBUG_SCOPE(debug);
    debug.addShape(face, "face");
    debug.addShape(wire, "wire");
    BRepTools_WireExplorer wireExp;
    for (wireExp.Init(wire); wireExp.More(); wireExp.Next()) {
        const TopoDS_Edge& edge = wireExp.Current();
        if (GetIntersectionPoint_impl(face, edge, dst, tolerance)) {
            return true;
        }
    }

    LOG(WARNING) << "No Intersections found in GetIntersectionPoint(face, wire)";
    return false;
}

TIGL_EXPORT bool GetIntersectionPoint(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, intersectionPointList& intersectionPoints, const double tolerance)
{
    BRepExtrema_ExtCC Intersector;

    // explore the edges of both wires
    TopExp_Explorer EdgeExplorer1(wire1, TopAbs_EDGE);
    TopExp_Explorer EdgeExplorer2(wire2, TopAbs_EDGE);

    while ( EdgeExplorer2.More() ) {

        TopoDS_Edge edge2 = TopoDS::Edge(EdgeExplorer2.Current());
        Intersector.Initialize(edge2);

        while ( EdgeExplorer1.More() ) {

            TopoDS_Edge edge1 = TopoDS::Edge(EdgeExplorer1.Current());

            //calculate intersection point
            Intersector.Perform(edge1);

            if (!Intersector.IsDone()) {
                LOG(ERROR) << "Error intersecting two wires in GetIntersectionPoint!";
                throw tigl::CTiglError("Error intersecting two wires in GetIntersectionPoint!");
            }

            // now go through all extremal distances of the current edge pair
            for( int i=1; i<=Intersector.NbExt(); i++ ) {
                if( Intersector.SquareDistance(i) < tolerance ) {

                    IntersectionPoint intersectionPoint;

                    intersectionPoint.SquareDistance=Intersector.SquareDistance(i);
                    intersectionPoint.Center = Intersector.PointOnE1(i);
                    intersectionPoint.Center.BaryCenter(0.5,Intersector.PointOnE2(i),0.5);

                    //make sure the intersectionPoints are unique
                    bool foundIntersectionPoint = false;
                    for (unsigned int i=0;i<intersectionPoints.size(); i++ ) {
                        if ( intersectionPoint.Center.Distance( intersectionPoints[i].Center ) < tolerance ) {
                            foundIntersectionPoint = true;
                            if ( intersectionPoint.SquareDistance< intersectionPoints[i].SquareDistance ) {
                                intersectionPoints[i]=intersectionPoint;
                            }
                        }
                    }
                    if ( !foundIntersectionPoint ) {
                        intersectionPoints.push_back(intersectionPoint);
                    }
                }
            }
            EdgeExplorer1.Next();
        }
        EdgeExplorer1.ReInit();
        EdgeExplorer2.Next();
    }

    if ( intersectionPoints.size() == 0 ) {
        LOG(INFO) << "tiglIntersectCurves: The curves do not intersect to the specified tolerance";
        return false;
    }

    return true;
}

TopoDS_Face GetSingleFace(const TopoDS_Shape& shape)
{
    unsigned numFaces = GetNumberOfFaces(shape);
    if (numFaces < 1) {
        LOG(ERROR) << "unable to get single face from shape: shape contains no faces";
        throw tigl::CTiglError("unable to get single face from shape: shape contains no faces!");
    }
    else if (numFaces > 1) {
        LOG(ERROR) << "unable to get single face from shape: shape contains more than one face";
        throw tigl::CTiglError("unable to get single face from shape: shape contains more than one face!");
    }
    return GetFace(shape, 0);
}

TopoDS_Face BuildFace(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Pnt& p4)
{
    BRepBuilderAPI_MakeEdge me1(p1, p2);
    TopoDS_Edge e1 = me1.Edge();
    BRepBuilderAPI_MakeEdge me2(p3, p4);
    TopoDS_Edge e2 = me2.Edge();

    TopoDS_Face face = BRepFill::Face(e1, e2);
    return face;
}

bool IsPathRelative(const std::string& path)
{
#if defined _WIN32 || defined __WIN32__
    return PathIsRelative(path.c_str()) == 1;
#else
    if (path.size() > 0 && path[0] == '/') {
        return false;
    }
    else {
        return true;
    }
#endif
}

bool IsFileReadable(const std::string& filename)
{
#ifdef _MSC_VER
    return _access(filename.c_str(), 4) == 0;
#else
    return access(filename.c_str(), R_OK) == 0;
#endif
}

std::string FileExtension(const std::string &filename)
{
    if(filename.find_last_of(".") != std::string::npos) {
        return filename.substr(filename.find_last_of(".")+1);
    }
    else {
        return "";
    }
}

gp_Pnt GetFirstPoint(const TopoDS_Shape& wireOrEdge)
{
    if (wireOrEdge.ShapeType() == TopAbs_WIRE)
        return GetFirstPoint(TopoDS::Wire(wireOrEdge));
    if (wireOrEdge.ShapeType() == TopAbs_EDGE)
        return GetFirstPoint(TopoDS::Edge(wireOrEdge));
    throw tigl::CTiglError("Shape must be wire or edge");
}

gp_Pnt GetFirstPoint(const TopoDS_Wire& w)
{
    TopTools_IndexedMapOfShape wireMap;
    TopExp::MapShapes(w, TopAbs_EDGE, wireMap);
    TopoDS_Edge e = TopoDS::Edge(wireMap(1));
    return GetFirstPoint(e);
}

gp_Pnt GetFirstPoint(const TopoDS_Edge& e)
{
    double u1, u2;
    Handle_Geom_Curve c = BRep_Tool::Curve(e, u1, u2);

    if (e.Orientation() == TopAbs_REVERSED) {
        return c->Value(u2);
    }
    else {
        return c->Value(u1);
    }
}

gp_Pnt GetLastPoint(const TopoDS_Shape& wireOrEdge)
{
    if (wireOrEdge.ShapeType() == TopAbs_WIRE)
        return GetLastPoint(TopoDS::Wire(wireOrEdge));
    if (wireOrEdge.ShapeType() == TopAbs_EDGE)
        return GetLastPoint(TopoDS::Edge(wireOrEdge));
    throw tigl::CTiglError("Shape must be wire or edge");
}

gp_Pnt GetLastPoint(const TopoDS_Wire& w)
{
    TopTools_IndexedMapOfShape wireMap;
    TopExp::MapShapes(w, TopAbs_EDGE, wireMap);
    TopoDS_Edge e = TopoDS::Edge(wireMap(wireMap.Extent()));
    return GetLastPoint(e);
}

gp_Pnt GetLastPoint(const TopoDS_Edge& e)
{
    double u1, u2;
    Handle_Geom_Curve c = BRep_Tool::Curve(e, u1, u2);

    if (e.Orientation() == TopAbs_REVERSED) {
        return c->Value(u1);
    }
    else {
        return c->Value(u2);
    }
}

TiglContinuity getEdgeContinuity(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2)
{
    // **********************************************************************************
    // Create a wire from both edges
    // **********************************************************************************
    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(edge1);
    if (!wireMaker.IsDone()) {
        throw tigl::CTiglError("checkEdgeContinuity: Unable to create connected wire", TIGL_ERROR);
    }
    wireMaker.Add(edge2);
    if (!wireMaker.IsDone()) {
        throw tigl::CTiglError("checkEdgeContinuity: Unable to create connected wire", TIGL_ERROR);
    }
    TopoDS_Vertex commonVertex;
    TopoDS_Wire wire = wireMaker.Wire();

    // **********************************************************************************
    // Fix connectivity: Create common vertex
    // **********************************************************************************
    ShapeFix_Wire wireFixer;
    wireFixer.Load(wire);
    wireFixer.FixConnected();
    wireFixer.Perform();
    wire = wireFixer.Wire();
    if (wire.IsNull()) {
        throw tigl::CTiglError("checkEdgeContinuity: Unable to create common vertex in connected wire", TIGL_ERROR);
    }

    // **********************************************************************************
    // Get new edges with common vertex
    // **********************************************************************************
    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(wire, TopAbs_EDGE, edges);
    if (edges.Extent() != 2) {
        throw tigl::CTiglError("checkEdgeContinuity: Unexpected error in connected wire", TIGL_ERROR);
    }
    TopoDS_Edge newedge1 = TopoDS::Edge(edges(1));
    TopoDS_Edge newedge2 = TopoDS::Edge(edges(2));

    TopExp::CommonVertex(newedge1, newedge2, commonVertex);
    if (commonVertex.IsNull()) {
        throw tigl::CTiglError("checkEdgeContinuity: Unable to find common vertex", TIGL_ERROR);
    }

    // **********************************************************************************
    // Get derivatives at common vertex
    // **********************************************************************************
    BRepAdaptor_Curve firstCurve;
    BRepAdaptor_Curve secondCurve;
    if (TopExp::FirstVertex(newedge1).IsSame(commonVertex) && TopExp::LastVertex(newedge2).IsSame(commonVertex)) {
        secondCurve.Initialize(newedge1);
        firstCurve.Initialize(newedge2);
    }
    else if (TopExp::LastVertex(newedge1).IsSame(commonVertex) && TopExp::FirstVertex(newedge2).IsSame(commonVertex)) {
        firstCurve.Initialize(newedge1);
        secondCurve.Initialize(newedge2);
    }
    else {
        throw tigl::CTiglError("checkEdgeContinuity: Unexpected error: Unable to find common vertex in connected wire", TIGL_ERROR);
    }

    gp_Pnt point;
    gp_Vec firstD1;
    gp_Vec firstD2;
    gp_Vec secondD1;
    gp_Vec secondD2;
    firstCurve.D2(firstCurve.LastParameter(), point, firstD1, firstD2);
    secondCurve.D2(secondCurve.FirstParameter(), point, secondD1, secondD2);

    // **********************************************************************************
    // Get continuity depending on derivatives at the common vertex
    // **********************************************************************************
    if ((firstD1.Magnitude() >= gp::Resolution() && secondD1.Magnitude() >= gp::Resolution())
        && gp_Dir(firstD1).IsEqual(gp_Dir(secondD1), Precision::Confusion())) {
        if ((firstD2.Magnitude() >= gp::Resolution() && secondD2.Magnitude() >= gp::Resolution())
            && gp_Dir(firstD2).IsEqual(gp_Dir(secondD2), Precision::Confusion())) {
            return C2;
        }
        else {
            return C1;
        }
    }
    else {
        return C0;
    }
}

Standard_Boolean IsEqual(const TopoDS_Shape& s1, const TopoDS_Shape& s2)
{
    return s1.IsEqual(s2);
}

TopoDS_Face BuildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2)
{
    TopoDS_Wire closedWire = CloseWires(wire1, wire2);
    BRepBuilderAPI_MakeFace mf(closedWire);
    TopoDS_Face face;
    if (mf.IsDone()) {
        face = mf.Face();
    }
    else {
        face = BuildRuledFace(wire1, wire2);
    }
    return face;
}

TopoDS_Face BuildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir)
{
    TopoDS_Wire closedWire = CloseWires(wire1, wire2, dir);
    BRepBuilderAPI_MakeFace mf(closedWire);
    TopoDS_Face face;
    if (mf.IsDone()) {
        face = mf.Face();
    }
    else {
        face = BuildRuledFace(wire1, wire2);
    }
    return face;
}

TopoDS_Face BuildFace(const TopoDS_Wire& wire)
{
    TopoDS_Face result;

    // try building face using BRepLib_MakeFace, which tries to build a plane
    // if a plane is not possible, use BRepFill_Filling
    BRepLib_MakeFace mf(wire, Standard_True);
    if (mf.IsDone()) {
        result = mf.Face();
    }
    else {
        BRepFill_Filling filler;
        TopExp_Explorer exp;
        for (exp.Init(wire, TopAbs_EDGE); exp.More(); exp.Next()) {
            TopoDS_Edge e = TopoDS::Edge(exp.Current());
            filler.Add(e, GeomAbs_C0);
        }
        filler.Build();
        if (!filler.IsDone()) {
            LOG(ERROR) << "Unable to generate face from Wire!";
            throw tigl::CTiglError("BuildFace: Unable to generate face from Wire!");
        }
        result = filler.Face();
    }
    return result;
}

TopoDS_Face BuildRuledFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2)
{
    // fix edge order in wires so that no self-intersecting faces are generated
    TopTools_SequenceOfShape wireSequence;
    wireSequence.Append(wire1);
    wireSequence.Append(wire2);

    BRepFill_CompatibleWires orderWires(wireSequence);
    orderWires.Perform();
    if (!orderWires.IsDone()) {
        LOG(ERROR) << "unable to determine common wire order!";
        throw tigl::CTiglError("unable to determine common wire order!");
    }
    TopTools_SequenceOfShape orderedWireSequence;

    orderedWireSequence = orderWires.Shape();
    if (orderedWireSequence.Length() != 2) {
        LOG(ERROR) << "number of wires in sequence does not match. Expected: 2, Got: " << orderedWireSequence.Length();
        throw tigl::CTiglError("number of wires in sequence does not match. Expected: 2");
    }
    TopoDS_Wire sortedWire1 = TopoDS::Wire(orderedWireSequence.First());
    TopoDS_Wire sortedWire2 = TopoDS::Wire(orderedWireSequence.Last());

    // build curve adaptor, second parameter defines that the length of the single edges is used as u coordinate, 
    // instead normalization of the u coordinates of the single edges of the wire (each edge would have u-coords
    // range from 0 to 1 independent of their length otherwise)
    BRepAdaptor_CompCurve compCurve1(sortedWire1, Standard_True);
    BRepAdaptor_CompCurve compCurve2(sortedWire2, Standard_True);

    // Wrap the adaptor in a class which manages curve access via handle (HCurve). According to doxygen
    // this is required by the algorithms
    Handle(Adaptor3d_HCurve) curve1 = new BRepAdaptor_HCompCurve(compCurve1);
    Handle(Adaptor3d_HCurve) curve2 = new BRepAdaptor_HCompCurve(compCurve2);

    // We have to generate an approximated curve now from the wire using the adaptor
    // NOTE: last parameter value unknown
    Approx_Curve3d approx1(curve1, Precision::Confusion(), GeomAbs_C2, 2000, 12);
    Approx_Curve3d approx2(curve2, Precision::Confusion(), GeomAbs_C2, 2000, 12);

    // Get approximated curve
    if (!approx1.IsDone() || !approx1.HasResult() ||
        !approx2.IsDone() || !approx2.HasResult()) {

        LOG(ERROR) << "unable to approximate wires by curves for building face";
        throw tigl::CTiglError("unable to approximate wires by curves for building face!");
    }
    Handle(Geom_Curve) approxCurve1 = approx1.Curve();
    Handle(Geom_Curve) approxCurve2 = approx2.Curve();

    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(approxCurve1);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(approxCurve2);
    TopoDS_Face face = BRepFill::Face(e1, e2);
    return face;
}

TopoDS_Wire BuildWire(const gp_Pnt& p1, const gp_Pnt& p2)
{
    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Wire w = BRepBuilderAPI_MakeWire(e1);
    return w;
}

TopoDS_Wire BuildWireFromEdges(const TopoDS_Shape& edges)
{
    TopTools_ListOfShape wireList;
    BuildWiresFromConnectedEdges(edges, wireList);
    if (wireList.Extent() != 1) {
        LOG(ERROR) << "Error generating single wire!";
        throw tigl::CTiglError("Error generating single wire in MakeWireFromEdges!");
    }
    TopoDS_Wire result = TopoDS::Wire(wireList.First());
    return result;
}

void BuildWiresFromConnectedEdges(const TopoDS_Shape& shape, TopTools_ListOfShape& wireList)
{
    // get list of edges from passed shape
    TopTools_ListOfShape edgeList;
    GetListOfShape(shape, TopAbs_EDGE, edgeList);

    // iterate until all edges are converted into wires
    while (edgeList.Extent() > 0) {
        // pop first edge from edgeList
        TopoDS_Edge e1 = TopoDS::Edge(edgeList.First());
        edgeList.RemoveFirst();
        // build a list of edges for a single wire
        TopTools_ListOfShape wireEdges;
        wireEdges.Append(e1);
        FindAllConnectedEdges(e1, edgeList, wireEdges);

        // build wire from wireEdges
        BRepBuilderAPI_MakeWire wireBuilder;
        TopTools_ListIteratorOfListOfShape eIt;
        for (eIt.Initialize(wireEdges); eIt.More(); eIt.Next()) {
            wireBuilder.Add(TopoDS::Edge(eIt.Value()));
        }
        if (!wireBuilder.IsDone()) {
            LOG(ERROR) << "Error creating wire!";
            throw tigl::CTiglError("Error creating wire in BuildWiresFromConnectedEdges!");
        }
        TopoDS_Wire wire = SortWireEdges(wireBuilder.Wire());

        wireList.Append(wire);
    }
}

TopoDS_Wire CloseWire(const TopoDS_Wire& wire)
{
    // get the list of end vertices
    TopTools_ListOfShape endVertices;
    GetEndVertices(wire, endVertices);

    // determine number of end vertices
    int numEndVertices = endVertices.Extent();

    // check if wire is already closed
    if (numEndVertices == 0) {
        return wire;
    }

    // check if we have exatcly two end vertices
    if (numEndVertices != 2) {
        LOG(ERROR) << "invalid number of end vertices found!";
        throw tigl::CTiglError("invalid number of end vertices found in CloseWire!");
    }

    // next generate an edge between the end vertices
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(TopoDS::Vertex(endVertices.First()), TopoDS::Vertex(endVertices.Last()));

    // build new wire
    BRepBuilderAPI_MakeWire makeWire;
    makeWire.Add(wire);
    makeWire.Add(edge);
    if (!makeWire.IsDone()) {
        LOG(ERROR) << "unable to build closed wire!";
        throw tigl::CTiglError("unable to build closed wire in CloseWire!");
    }
    TopoDS_Wire result = SortWireEdges(makeWire.Wire());
    return result;
}

// determine direction from wire1 by using end vertices
TopoDS_Wire CloseWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2)
{
    // get a map of vertices with connected edges
    TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
    TopTools_ListOfShape endVertices1, endVertices2;

    TopExp::MapShapesAndAncestors(wire1, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);
    // filter out all vertices which have only a single edge connected (end vertices)
    for (int i = 1; i <= vertexMap.Extent(); i++) {
        const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
        if (edgeList.Extent() == 1) {
            endVertices1.Append(vertexMap.FindKey(i));
        }
    }

    // check for correct number of end vertices
    if (endVertices1.Extent() != 2) {
        LOG(ERROR) << "Unable to close wires because invalid number of end-vertices found!";
        throw tigl::CTiglError("Unable to close wires because invalid number of end-vertices found!");
    }

    TopoDS_Vertex& v1 = TopoDS::Vertex(endVertices1.First());
    TopoDS_Vertex& v2 = TopoDS::Vertex(endVertices1.Last());
    gp_Pnt p1 = BRep_Tool::Pnt(v1);
    gp_Pnt p2 = BRep_Tool::Pnt(v2);
    gp_Vec dir(p1, p2);

    return CloseWires(wire1, wire2, dir);
}

TopoDS_Wire CloseWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir)
{
    // get a map of vertices with connected edges
    TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
    TopTools_ListOfShape endVertices1, endVertices2;

    TopExp::MapShapesAndAncestors(wire1, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);
    // filter out all vertices which have only a single edge connected (end vertices)
    for (int i = 1; i <= vertexMap.Extent(); i++) {
        const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
        if (edgeList.Extent() == 1) {
            endVertices1.Append(vertexMap.FindKey(i));
        }
    }
    vertexMap.Clear();
    TopExp::MapShapesAndAncestors(wire2, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);
    // filter out all vertices which have only a single edge connected (end vertices)
    for (int i = 1; i <= vertexMap.Extent(); i++) {
        const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
        if (edgeList.Extent() == 1) {
            endVertices2.Append(vertexMap.FindKey(i));
        }
    }

    // check if number of end vertices is correct
    int numEndVertices1 = endVertices1.Extent();
    int numEndVertices2 = endVertices2.Extent();
    if (numEndVertices1 != 2 || numEndVertices2 != 2) {
        LOG(ERROR) << "Unable to close wires because invalid number of end-vertices found!";
        throw tigl::CTiglError("Unable to close wires because invalid number of end-vertices found!");
    }

    // sort end vertices according to direction vector
    TopoDS_Vertex vUpper1 = TopoDS::Vertex(endVertices1.First());
    TopoDS_Vertex vLower1 = TopoDS::Vertex(endVertices1.Last());
    gp_Pnt pUpper = BRep_Tool::Pnt(vUpper1);
    gp_Pnt pLower = BRep_Tool::Pnt(vLower1);
    gp_Vec vDiff(pLower, pUpper);
    if (vDiff.Normalized().Dot(dir.Normalized()) < 0) {
        vUpper1 = TopoDS::Vertex(endVertices1.Last());
        vLower1 = TopoDS::Vertex(endVertices1.First());
    }
    TopoDS_Vertex vUpper2 = TopoDS::Vertex(endVertices2.First());
    TopoDS_Vertex vLower2 = TopoDS::Vertex(endVertices2.Last());
    pUpper = BRep_Tool::Pnt(vUpper2);
    pLower = BRep_Tool::Pnt(vLower2);
    vDiff = gp_Vec(pLower, pUpper);
    if (vDiff.Normalized().Dot(dir.Normalized()) < 0) {
        vUpper2 = TopoDS::Vertex(endVertices2.Last());
        vLower2 = TopoDS::Vertex(endVertices2.First());
    }

    // next build the edges and combine the wires to a single wire
    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(vUpper1, vUpper2);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(vLower1, vLower2);

    // put all edges into a large list
    TopTools_ListOfShape edgeList;
    edgeList.Append(edge1);
    edgeList.Append(edge2);
    TopoDS_Iterator edgeIt;
    for (edgeIt.Initialize(wire1); edgeIt.More(); edgeIt.Next()) {
        edgeList.Append(edgeIt.Value());
    }
    for (edgeIt.Initialize(wire2); edgeIt.More(); edgeIt.Next()) {
        edgeList.Append(edgeIt.Value());
    }

    BRepBuilderAPI_MakeWire makeWire;
    makeWire.Add(edgeList);
    if (!makeWire.IsDone()) {
        LOG(ERROR) << "error during creation of closed wire!";
        throw tigl::CTiglError("error during creation of closed wire in CloseWires!");
    }

    TopoDS_Wire result = SortWireEdges(makeWire.Wire());

    return result;
}

TopoDS_Wire SortWireEdges(const TopoDS_Wire& wire)
{
    ShapeFix_Wire fixWire;
    fixWire.Load(wire);
    fixWire.FixReorder();
    TopoDS_Wire result = fixWire.Wire();
    return result;
}

bool GetMinMaxPoint(const TopoDS_Shape& shape, const gp_Vec& dir, gp_Pnt& minPnt, gp_Pnt& maxPnt)
{
    TopExp_Explorer explorer(shape, TopAbs_VERTEX);

    // return false in case shape has no vertices
    if (!explorer.More()) {
        return false;
    }

    // initialize values
    const TopoDS_Vertex& v = TopoDS::Vertex(explorer.Current());
    gp_Pnt p = BRep_Tool::Pnt(v);
    minPnt = p;
    maxPnt = p;
    explorer.Next();

    for (; explorer.More(); explorer.Next()) {
        const TopoDS_Vertex& v = TopoDS::Vertex(explorer.Current());
        p = BRep_Tool::Pnt(v);
        gp_Vec offset(minPnt, p);
        if (offset.Dot(dir) < 0) {
            minPnt = p;
        }
        offset = gp_Vec(p, maxPnt);
        if (offset.Dot(dir) < 0) {
            maxPnt = p;
        }
    }

    return true;
}

void GetListOfShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, TopTools_ListOfShape& result)
{
    TopTools_IndexedMapOfShape typeMap;
    TopExp::MapShapes(shape, type, typeMap);
    for (int i = 1; i <= typeMap.Extent(); i++) {
        result.Append(typeMap.FindKey(i));
    }
}

std::vector<TopoDS_Shape> GetSubShapes(const TopoDS_Shape& shape, TopAbs_ShapeEnum type) {
    std::vector<TopoDS_Shape> result;

    TopTools_IndexedMapOfShape typeMap;
    TopExp::MapShapes(shape, type, typeMap);
    for (int i = 1; i <= typeMap.Extent(); i++) {
        result.push_back(typeMap.FindKey(i));
    }

    return result;
}

TopoDS_Shape CutShapes(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
    BRepAlgoAPI_Section cutter(shape1, shape2, Standard_False);
    cutter.ComputePCurveOn1(Standard_True);
    cutter.Approximation(Standard_True);
    cutter.Build();
    if (!cutter.IsDone()) {
        LOG(ERROR) << "Error cutting shapes in method CutShapes!";
        throw tigl::CTiglError("Error cutting shapes in method CutShapes!");
    }
    return cutter.Shape();
}

TopoDS_Shape SplitShape(const TopoDS_Shape& src, const TopoDS_Shape& tool)
{
    double fuzzyValue = Precision::Confusion();

    const int c_tries = 3;
    for (int i = 0;; i++) {
        GEOMAlgo_Splitter splitter;
        splitter.AddArgument(src);
        splitter.AddTool(tool);
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
        splitter.SetFuzzyValue(fuzzyValue);
#endif
        try {
            splitter.Perform();
        }
        catch (const Standard_Failure& f) {
            std::stringstream ss;
            ss << "ERROR: splitting of shapes failed: " << f.GetMessageString();
            LOG(ERROR) << ss.str();
            throw tigl::CTiglError(ss.str());
        }

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,2,0)
        if (splitter.HasErrors()) {
            if (i < c_tries - 1) {
                fuzzyValue *= 10;
                LOG(WARNING) << "SplitShape failed, retrying with fuzzyValue: " << fuzzyValue;
                continue;
            }

            std::ostringstream oss;
            splitter.GetReport()->Dump(oss);
            LOG(ERROR) << "unable to split passed shapes: " << oss.str();
            throw tigl::CTiglError("unable to split passed shapes: " + oss.str());
        }
#elif OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
        if (splitter.ErrorStatus() != 0) {
            if (i < c_tries - 1) {
                fuzzyValue *= 10;
                LOG(WARNING) << "SplitShape failed, retrying with fuzzyValue: " << fuzzyValue;
                continue;
            }

            LOG(ERROR) << "unable to split passed shapes!";
            throw tigl::CTiglError("unable to split passed shapes!");
        }
#else
        if (splitter.ErrorStatus() != 0) {
            LOG(ERROR) << "unable to split passed shapes!";
            throw tigl::CTiglError("unable to split passed shapes!");
        }
#endif
        return splitter.Shape();
    }
}

void FindAllConnectedEdges(const TopoDS_Edge& edge, TopTools_ListOfShape& edgeList, TopTools_ListOfShape& targetList)
{
    TopTools_ListIteratorOfListOfShape edgeIt;
    TopoDS_Vertex tempVertex;
    TopoDS_Edge connectedEdge;

    // finished gets true as soon as no more connected edge is found in the edgeList
    bool finished = false;
    // loop until all connected e
    while (!finished) {
        connectedEdge.Nullify();
        // iterate over all edges in edgeList, and break when a connected edge was found
        for (edgeIt.Initialize(edgeList); edgeIt.More(); edgeIt.Next()) {
            TopoDS_Edge testEdge = TopoDS::Edge(edgeIt.Value());

            // check if edges are connected
            if (CheckCommonVertex(edge, testEdge)) {
                connectedEdge = testEdge;
                break;
            }
        }
        // check if a connected edge was found
        if (!connectedEdge.IsNull()) {
            // remove connected edge from edgeList
            edgeList.Remove(edgeIt);
            // append connected edge to connectedEdge list
            targetList.Append(connectedEdge);
            // append all edges which are connected to the new edge to the list
            FindAllConnectedEdges(connectedEdge, edgeList, targetList);
        }
        else {
            finished = true;
        }
    }
}

bool CheckCommonVertex(const TopoDS_Edge& e1, const TopoDS_Edge& e2)
{
    TopoDS_Vertex v1First, v1Last, v2First, v2Last;
    TopExp::Vertices(e1, v1First, v1Last);
    TopExp::Vertices(e2, v2First, v2Last);

    gp_Pnt p1First = BRep_Tool::Pnt(v1First);
    gp_Pnt p1Last = BRep_Tool::Pnt(v1Last);
    gp_Pnt p2First = BRep_Tool::Pnt(v2First);
    gp_Pnt p2Last = BRep_Tool::Pnt(v2Last);

    return (p1First.Distance(p2First) < Precision::Confusion() ||
        p1First.Distance(p2Last) < Precision::Confusion() ||
        p1Last.Distance(p2First) < Precision::Confusion() ||
        p1Last.Distance(p2Last) < Precision::Confusion());
}

void GetEndVertices(const TopoDS_Shape& shape, TopTools_ListOfShape& endVertices)
{
    // get a map of vertices with connected edges
    TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
    TopExp::MapShapesAndAncestors(shape, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);

    // filter out all vertices which have only a single edge connected (end vertices)
    for (int i = 1; i <= vertexMap.Extent(); i++) {
        const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
        if (edgeList.Extent() == 1) {
            endVertices.Append(vertexMap.FindKey(i));
        }
    }
}

TopoDS_Face GetNearestFace(const TopoDS_Shape& shape, const gp_Pnt& pnt)
{
    TopExp_Explorer explorer;
    TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(pnt);
    TopoDS_Face resultFace;
    float resultDistance = std::numeric_limits<float>::max();
    for (explorer.Init(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        TopoDS_Face checkFace = TopoDS::Face(explorer.Current());
        BRepExtrema_DistShapeShape extrema(checkFace, v);
        if (!extrema.IsDone() || extrema.NbSolution() < 1) {
            LOG(ERROR) << "unable to determine nearest point between face and vertex!";
            throw tigl::CTiglError("unable to determine nearest point between face and vertex!");
        }
        if (extrema.Value() < resultDistance) {
            resultFace = checkFace;
            resultDistance = static_cast<float>(extrema.Value());
        }
    }
    return resultFace;
}

gp_Pnt GetCenterOfMass(const TopoDS_Shape &shape)
{
    // get linear properties of the shape
     GProp_GProps LProps;
     BRepGProp::LinearProperties(shape, LProps);

     // compute center of the shape
     gp_Pnt centerPoint = LProps.CentreOfMass();

     return centerPoint;
}

double GetArea(const TopoDS_Shape &shape)
{
    // get surface properties of the shape
     GProp_GProps SProps;
     BRepGProp::SurfaceProperties(shape, SProps);

     // compute the area of the shape or the area that is framed by the shape
     double area = SProps.Mass();

     return area;
}

TopoDS_Shape RemoveDuplicateEdges(const TopoDS_Shape& shape)
{
    TopTools_ListOfShape initialEdgeList, newEdgeList;
    TopTools_ListIteratorOfListOfShape initialEdgeIt, newEdgeIt;

    TopoDS_Vertex v1First, v1Last, v2First, v2Last;
    gp_Pnt p1First, p1Mid, p1Last, p2First, p2Mid, p2Last;

    // get list of all edges of passed shape
    GetListOfShape(shape, TopAbs_EDGE, initialEdgeList);

    // iterate over all edges
    for (initialEdgeIt.Initialize(initialEdgeList); initialEdgeIt.More(); initialEdgeIt.Next()) {
        TopoDS_Edge& edge = TopoDS::Edge(initialEdgeIt.Value());
        TopExp::Vertices(edge, v1First, v1Last);
        p1First = BRep_Tool::Pnt(v1First);
        p1Last = BRep_Tool::Pnt(v1Last);

        // if identical edge is not found already in newEdgeList then add it to this list
        bool duplicate = false;
        for (newEdgeIt.Initialize(newEdgeList); newEdgeIt.More(); newEdgeIt.Next()) {
            TopoDS_Edge& checkEdge = TopoDS::Edge(newEdgeIt.Value());
            TopExp::Vertices(checkEdge, v2First, v2Last);
            p2First = BRep_Tool::Pnt(v2First);
            p2Last = BRep_Tool::Pnt(v2Last);

            if ((p1First.Distance(p2First) < Precision::Confusion() &&
                p1Last.Distance(p2Last) < Precision::Confusion()) ||
                (p1First.Distance(p2Last) < Precision::Confusion() &&
                p1Last.Distance(p2First) < Precision::Confusion())) {

                // now check for identical mid point
                Handle(Geom_Curve) curve;
                Standard_Real uStart, uEnd;
                // get midpoint of edge
                curve = BRep_Tool::Curve(edge, uStart, uEnd);
                curve->D0((uStart + uEnd) / 2.0, p1Mid);
                // get midpoint of checkEdge
                curve = BRep_Tool::Curve(checkEdge, uStart, uEnd);
                curve->D0((uStart + uEnd) / 2.0, p2Mid);
                 
                if (p1Mid.Distance(p2Mid) < 1E-5) {
                    duplicate = true;
                    break;
                }
            }
        }
        // only add edge to newEdgeList if no identical edge already exists in there
        if (!duplicate) {
            newEdgeList.Append(edge);
        }
    }

    // finally rebuild a compound of all edges from the newEdgeList
    TopoDS_Compound result;
    BRep_Builder builder;
    builder.MakeCompound(result);

    for (newEdgeIt.Initialize(newEdgeList); newEdgeIt.More(); newEdgeIt.Next()) {
        builder.Add(result, TopoDS::Edge(newEdgeIt.Value()));
    }

    return result;
}


bool IsPointInsideShape(const TopoDS_Shape &solid, gp_Pnt point)
{
    // check if solid
    TopoDS_Solid s;
    try {
        s = TopoDS::Solid(solid);
    }
    catch (Standard_Failure) {
        throw tigl::CTiglError("The shape is not a solid");
    }

    BRepClass3d_SolidClassifier algo(s);

    // test whether a point at infinity lies inside. If yes, then the shape is reversed
    algo.PerformInfinitePoint(1e-3);

    bool shapeIsReversed = (algo.State() == TopAbs_IN);

    algo.Perform(point, 1e-3);

    return ((algo.State() == TopAbs_IN) != shapeIsReversed ) || (algo.State() == TopAbs_ON);
}

std::vector<double> LinspaceWithBreaks(double umin, double umax, size_t n_values, const std::vector<double>& breaks)
{
    double du = (umax - umin) / static_cast<double>(n_values - 1);

    std::vector<double> result(n_values);
    for (int i = 0; i < n_values; ++i) {
        result[i] = i * du + umin;
    }

    // now insert the break

    double eps = 0.3;
    // remove points, that are closer to each break point than du*eps
    for (std::vector<double>::const_iterator it = breaks.begin(); it != breaks.end(); ++it) {
        double breakpoint = *it;
        std::vector<double>::iterator pos = std::find_if(result.begin(), result.end(), IsInsideTolerance(breakpoint, du*eps));
        if (pos != result.end()) {
            // point found, replace it
            *pos = breakpoint;
        }
        else {
            // find closest element
            pos = std::find_if(result.begin(), result.end(), IsInsideTolerance(breakpoint, (0.5 + 1e-8)*du));

            if (*pos > breakpoint) {
                result.insert(pos, breakpoint);
            }
            else {
                result.insert(pos+1, breakpoint);
            }
        }
    }

    return result;
}

template <class T>
T Clamp(T val, T min, T max)
{
    if (min > max) {
        throw tigl::CTiglError("Minimum may not be larger than maximum in clamp!");
    }
    
    return std::max(min, std::min(val, max));
}

double Clamp(double val, double min, double max)
{
    return Clamp<>(val, min, max);
}

int Clamp(int val, int min, int max)
{
    return Clamp<>(val, min, max);
}

size_t Clamp(size_t val, size_t min, size_t max)
{
    return Clamp<>(val, min, max);
}

TopoDS_Shape TransformedShape(const tigl::CTiglTransformation& transformationToGlobal, TiglCoordinateSystem cs, const TopoDS_Shape& shape)
{
    switch (cs) {
    case WING_COORDINATE_SYSTEM:
    case FUSELAGE_COORDINATE_SYSTEM:
        return shape;
    case GLOBAL_COORDINATE_SYSTEM:
        return transformationToGlobal.Transform(shape);
    default:
        throw tigl::CTiglError("Invalid coordinate system");
    }
}

TopoDS_Shape TransformedShape(const tigl::CTiglRelativelyPositionedComponent& component, TiglCoordinateSystem cs, const TopoDS_Shape& shape)
{
    return TransformedShape(component.GetTransformationMatrix(), cs, shape);
}

TopoDS_Shape GetFacesByName(const PNamedShape shape, const std::string &name)
{
    std::vector<TopoDS_Face> faces;
    for (int i = 0; i < static_cast<int>(shape->GetFaceCount()); i++) {
        if (shape->GetFaceTraits(i).Name() == name) {
            faces.push_back(GetFace(shape->Shape(), i));
        }
    }
    
    if (faces.empty())
        throw tigl::CTiglError("Could not find faces named " + name);
    if (faces.size() == 1)
        return faces[0];
    
    TopoDS_Compound c;
    TopoDS_Builder b;
    b.MakeCompound(c);
    for (std::size_t i = 0; i < faces.size(); i++)
        b.Add(c, faces[i]);
    return c;
}

TIGL_EXPORT Handle(TColgp_HArray1OfPnt) OccArray(const std::vector<gp_Pnt>& pnts)
{
    Handle(TColgp_HArray1OfPnt) result = new TColgp_HArray1OfPnt(1, static_cast<int>(pnts.size()));
    int idx = 1;
    for (std::vector<gp_Pnt>::const_iterator it = pnts.begin(); it != pnts.end(); ++it, ++idx) {
        result->SetValue(idx, *it);
    }
    return result;
}
