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

#include "Geom_Curve.hxx"
#include "Geom_Surface.hxx"
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
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "BRepTools.hxx"
#include "BRepBuilderAPI_Sewing.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "BRepExtrema_ExtCF.hxx"
#include "BRepFill.hxx"

#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>

#include "ShapeAnalysis_FreeBounds.hxx"

#include <list>
#include <algorithm>
#include <cassert>
#include <limits>

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
Standard_Real GetWireLength(const TopoDS_Wire& wire)
{
    BRepAdaptor_CompCurve aCompoundCurve(wire, Standard_True);
    return GCPnts_AbscissaPoint::Length( aCompoundCurve );
}

Standard_Real GetEdgeLength(const TopoDS_Edge &edge)
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
        throw tigl::CTiglError("Error: Parameter alpha not in the range 0.0 <= alpha <= 1.0 in WireGetPointTangent", TIGL_ERROR);
    }
    // ETA 3D point
    BRepAdaptor_CompCurve aCompoundCurve(wire, Standard_True);

    Standard_Real len =  GCPnts_AbscissaPoint::Length( aCompoundCurve );
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
        throw tigl::CTiglError("Error: Parameter alpha not in the range 0.0 <= alpha <= 1.0 in EdgeGetPointTangent", TIGL_ERROR);
    }
    // ETA 3D point
    Standard_Real umin, umax;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, umin, umax);
    GeomAdaptor_Curve adaptorCurve(curve, umin, umax);
    Standard_Real len =  GCPnts_AbscissaPoint::Length( adaptorCurve, umin, umax );
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
    double normalizedLength = partLength/GetWireLength(wire);
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

bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Edge& edge, gp_Pnt& dst)
{
    BRepExtrema_ExtCF edgeFaceIntersect(edge, face);
    if (!edgeFaceIntersect.IsDone()) {
        LOG(ERROR) << "Error intersecting edge and face in GetIntersectionPoint!";
        throw tigl::CTiglError("Error intersecting edge and face in GetIntersectionPoint!");
    }
    double minDistance = 0;//std::numeric_limits<double>::max();
    int minIndex = 0;
    for (int i = 1; i <= edgeFaceIntersect.NbExt(); i++) {
        double distance = edgeFaceIntersect.SquareDistance(i);
        if (distance < minDistance) {
            minDistance = distance;
            minIndex = i;
        }
    }

    if (minDistance <= Precision::Confusion()) {
        dst = edgeFaceIntersect.PointOnEdge(minIndex);
        return true;
    }
    return false;
}

bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Wire& wire, gp_Pnt& dst)
{
    BRepTools_WireExplorer wireExp;
    for (wireExp.Init(wire); wireExp.More(); wireExp.Next()) {
        const TopoDS_Edge& edge = wireExp.Current();
        if (GetIntersectionPoint(face, edge, dst)) {
            return true;
        }
    }
    return false;
}

TopoDS_Face GetSingleFace(const TopoDS_Shape& shape)
{
    unsigned numFaces = GetNumberOfFaces(shape);
    if (numFaces < 1) {
        LOG(ERROR) << "unable to get single face from shape: shape contains no faces";
        throw tigl::CTiglError("ERROR: unable to get single face from shape: shape contains no faces!");
    }
    else if (numFaces > 1) {
        LOG(ERROR) << "unable to get single face from shape: shape contains more than one face";
        throw tigl::CTiglError("ERROR: unable to get single face from shape: shape contains more than one face!");
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
