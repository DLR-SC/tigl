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

#include "tiglcommonfunctions.h"

#include "CTiglError.h"
#include "CNamedShape.h"

#include "Geom_Curve.hxx"
#include "Geom_Surface.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "TopExp_Explorer.hxx"
#include "TopExp.hxx"
#include "TopoDS.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRep_Builder.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "BRepTools.hxx"

#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>

#include "ShapeAnalysis_FreeBounds.hxx"

#include <list>
#include <algorithm>
#include <cassert>

// OCAF
#include "TDF_Label.hxx"
#include "TDataStd_Name.hxx"
#ifdef TIGL_USE_XCAF
#include "XCAFDoc_ShapeTool.hxx"
#endif

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

Standard_Real GetWireLength(const TopoDS_Wire& wire)
{
#if 0
    Standard_Real wireLength = 0.0;
    // explore all edges of result wire
    BRepTools_WireExplorer wireExplorer;
    for (wireExplorer.Init(wire); wireExplorer.More(); wireExplorer.Next())
    {   
        Standard_Real firstParam;
        Standard_Real lastParam;
        TopoDS_Edge edge = wireExplorer.Current();
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);
        GeomAdaptor_Curve adaptorCurve(curve, firstParam, lastParam);
        wireLength += GCPnts_AbscissaPoint::Length(adaptorCurve);
    }
    return(wireLength);
#else
    BRepAdaptor_CompCurve aCompoundCurve(wire, Standard_True);
    return GCPnts_AbscissaPoint::Length( aCompoundCurve );
#endif
}

unsigned int GetNumberOfEdges(const TopoDS_Shape& shape)
{
    TopExp_Explorer edgeExpl(shape, TopAbs_EDGE);
    unsigned int iEdges = 0;
    for (;edgeExpl.More(); edgeExpl.Next()) {
        iEdges++;
    }

    return iEdges;
}

// Gets a point on the wire line in dependence of a parameter alpha with
// 0.0 <= alpha <= 1.0. For alpha = 0.0 this is the line starting point,
// for alpha = 1.0 the last point on the intersection line.
gp_Pnt WireGetPoint(const TopoDS_Wire &wire, double alpha)
{
    gp_Pnt point; gp_Vec normal;
    WireGetPointNormal(wire, alpha, point, normal);
    return point;
}

void WireGetPointNormal(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& normal)
{
    if (alpha < 0.0 || alpha > 1.0) {
        throw tigl::CTiglError("Error: Parameter alpha not in the range 0.0 <= alpha <= 1.0 in WireGetPoint", TIGL_ERROR);
    }

    //TopoDS_Wire wire;
    double length = GetWireLength(wire) * alpha;

    // Get the first edge of the wire
    BRepTools_WireExplorer wireExplorer( wire );
    if (!wireExplorer.More()) {
        throw tigl::CTiglError("Error: Not enough edges found in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
    }

    Standard_Real firstParam, lastParam;
    TopoDS_Edge edge = wireExplorer.Current();
    wireExplorer.Next();
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);

    // Length of current edge
    GeomAdaptor_Curve adaptorCurve;
    adaptorCurve.Load(curve, firstParam, lastParam);
    double currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

    // Length of complete wire up to now
    double sumLength = currLength;

    while (length > sumLength) {
        if (!wireExplorer.More()) {
            break;
        }
        
        edge = wireExplorer.Current();
        wireExplorer.Next();

        curve = BRep_Tool::Curve(edge, firstParam, lastParam);
        adaptorCurve.Load(curve, firstParam, lastParam);

        // Length of current edge
        currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

        // Length of complete wire up to now
        sumLength += currLength;
    }

    // Distance of searched point from end point of current edge
    double currEndDelta = std::max((sumLength - length), 0.0);

    // Distance of searched point from start point of current edge
    double currDist = std::max((currLength - currEndDelta), 0.0);

    GCPnts_AbscissaPoint abscissaPoint(adaptorCurve, currDist, adaptorCurve.FirstParameter());
    adaptorCurve.D1(abscissaPoint.Parameter(), point, normal);
}


gp_Pnt WireGetPoint2(const TopoDS_Wire &wire, double alpha)
{
    gp_Pnt point; gp_Vec normal;
    WireGetPointNormal2(wire, alpha, point, normal);
    return point;
}

void WireGetPointNormal2(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& normal)
{
    // ETA 3D point
    BRepAdaptor_CompCurve aCompoundCurve(wire, Standard_True);
    
    Standard_Real len =  GCPnts_AbscissaPoint::Length( aCompoundCurve );
    aCompoundCurve.D1( len * alpha, point, normal );
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
    for (int iwire = 0; iwire <= edgeIndex; ++iwire){
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
    return partLength/GetWireLength(wire);
}

gp_Pnt GetCentralFacePoint(const TopoDS_Face& face)
{
    // compute point on face
    Standard_Real umin, umax, vmin, vmax;

    gp_Pnt p;

    Handle_Geom_Surface surface = BRep_Tool::Surface(face);
    BRepTools::UVBounds(face, umin, umax, vmin, vmax);
    Standard_Real umean = 0.5*(umin+umax);
    Standard_Real vmean = 0.5*(vmin+vmax);


    // compute intersection of u-iso line with face boundaries
    Handle_Geom2d_Curve uiso = new Geom2d_Line(
                gp_Pnt2d(umean,0.),
                gp_Dir2d(0., 1.)
                );

    TopExp_Explorer exp (face,TopAbs_EDGE);
    std::list<double> intersections;
    for (; exp.More(); exp.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(exp.Current());
        Standard_Real first, last;

        // Get geomteric curve from edge
        Handle_Geom2d_Curve hcurve = BRep_Tool::CurveOnSurface(edge, face, first, last);
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

ShapeMap MapFacesToShapeGroups(const PNamedShape shape)
{
    ShapeMap map;
    if (!shape) {
        return map;
    }

    BRep_Builder b;
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
    for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
        TopoDS_Face face = TopoDS::Face(faceMap(iface));
        std::string name = shape->ShortName();
        PNamedShape origin = shape->GetFaceTraits(iface-1).Origin();
        if (origin){
            name = origin->ShortName();
        }
        ShapeMap::iterator it = map.find(name);
        if (it == map.end()) {
            TopoDS_Compound c;
            b.MakeCompound(c);
            b.Add(c, face);
            map[name] = c;
        }
        else {
            TopoDS_Shape& c = it->second;
            b.Add(c, face);
        }
    }
    return map;
}

// projects a point onto the line (lineStart<->lineStop) and returns the projection parameter
Standard_Real ProjectPointOnLine(gp_Pnt p, gp_Pnt lineStart, gp_Pnt lineStop) 
{
    return gp_Vec(lineStart, p) * gp_Vec(lineStart, lineStop) / gp_Vec(lineStart, lineStop).SquareMagnitude();
}


#ifdef TIGL_USE_XCAF
void GroupAndInsertShapeToCAF(Handle(XCAFDoc_ShapeTool) myAssembly, const PNamedShape shape, tigl::ShapeStoreType storeType)
{
    if (!shape) {
        return;
    }

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
    // any faces?
    if (faceMap.Extent() > 0) {
        if (storeType == tigl::WHOLE_SHAPE){
            TDF_Label shapeLabel = myAssembly->NewShape();
            myAssembly->SetShape(shapeLabel, shape->Shape());
            TDataStd_Name::Set(shapeLabel, shape->Name());
        }
        else if (storeType == tigl::NAMED_COMPOUNDS) {
            // create compounds with the same name as origin
            ShapeMap map =  MapFacesToShapeGroups(shape);
            // add compounds to document
            ShapeMap::iterator it = map.begin();
            for (; it != map.end(); ++it){
                TDF_Label faceLabel = myAssembly->NewShape();
                myAssembly->SetShape(faceLabel, it->second);
                TDataStd_Name::Set(faceLabel, it->first.c_str());
            }
        }
        else if (storeType == tigl::FACES) {
            for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
                TopoDS_Face face = TopoDS::Face(faceMap(iface));
                std::string name = shape->ShortName();
                PNamedShape origin = shape->GetFaceTraits(iface-1).Origin();
                if (origin){
                    name = origin->ShortName();
                }
                TDF_Label faceLabel = myAssembly->NewShape();
                myAssembly->SetShape(faceLabel, face);
                TDataStd_Name::Set(faceLabel, name.c_str());
            }
        }
    }
    else {
        // no faces, export edges as wires
        Handle(TopTools_HSequenceOfShape) Edges = new TopTools_HSequenceOfShape();
        TopExp_Explorer myEdgeExplorer (shape->Shape(), TopAbs_EDGE);
        while (myEdgeExplorer.More()) {
            Edges->Append(TopoDS::Edge(myEdgeExplorer.Current()));
            myEdgeExplorer.Next();
        }
        ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Edges, 1e-7, false, Edges);
        for (int iwire = 1; iwire <= Edges->Length(); ++iwire) {
            TDF_Label wireLabel = myAssembly->NewShape();
            myAssembly->SetShape(wireLabel, Edges->Value(iwire));
            TDataStd_Name::Set(wireLabel, shape->Name());
        }
    }
}

#endif

