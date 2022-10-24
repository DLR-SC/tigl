/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Class to compute intersection algorithms.
*/

#include "CTiglIntersectionCalculation.h"
#include "CTiglError.h"
#include "CTiglShapeCache.h"
#include "tiglcommonfunctions.h"

#include "GeomAPI_IntSS.hxx"
#include "BRep_Tool.hxx"
#include "BRep_Builder.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Solid.hxx"
#include "TopoDS_Compound.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "Geom2d_Line.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI.hxx"
#include "gp_Pln.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "BRepAlgoAPI_Section.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepPrimAPI_MakeHalfSpace.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "BRepProj_Projection.hxx"

#include <sstream>
#include <boost/functional/hash.hpp>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) > (b)) ? (b) : (a))
#endif

// WARNING: boost::hash gets incorrectly compiled with gcc 4.3.4 - the optimization will produce indeterministic hash values
// To workaround the problem, we have to avoid optimization of this file

namespace
{
    class HashablePlane
    {
    public:
        HashablePlane(const gp_Pnt& p, const gp_Dir& n)
        {
            gp_Dir nlocal(n);
            // compute hesse normal form
            double d = gp_Vec(n)*gp_Vec(p.XYZ());
            if (d < 0) {
                d *= -1.;
                nlocal.Reverse();
            }

            hash = boost::hash<double>()(d);
            boost::hash_combine(hash, nlocal.X());
            boost::hash_combine(hash, nlocal.Y());
            boost::hash_combine(hash, nlocal.Z());
        }

        size_t HashValue()
        {
            return hash;
        }

    private:
        size_t hash;
    };

    class HashableProjection
    {
    public:
        HashableProjection(const gp_Pnt& x, const gp_Dir& d)
        {
            gp_Vec xvec = gp_Vec(x.XYZ());
            gp_Vec dvec = gp_Vec(d);
            double coeff  = -dvec.Dot(xvec)/dvec.Dot(dvec);
            gp_Vec result = xvec + coeff*dvec;
            hash = boost::hash<double>()(result.X());
            boost::hash_combine(hash, result.Y());
            boost::hash_combine(hash, result.Z());
        }

        size_t HashValue()
        {
            return hash;
        }
    private:
        size_t hash;
    };

    template <typename L, typename R>
    size_t hash_combine_symmetric( L const& LHS, R const& RHS)
    {
        size_t lhs = boost::hash_value(LHS);
        size_t rhs = boost::hash_value(RHS);

        if ( lhs != rhs ) {
            size_t result = max(lhs, rhs);
            boost::hash_combine(result, min(lhs, rhs));
            return result;
        }
        else {
            return lhs;
        }
    }
}


namespace tigl 
{

// Constructor
CTiglIntersectionCalculation::CTiglIntersectionCalculation(CTiglShapeCache * cache,
                                                           const std::string& idOne,
                                                           const std::string& idTwo,
                                                           TopoDS_Shape compoundOne,
                                                           TopoDS_Shape compoundTwo)
    : tolerance(1.0e-7)
{
    size_t hash1 = boost::hash<std::string>()(idOne);
    size_t hash2 = boost::hash<std::string>()(idTwo);
    computeIntersection(cache, hash1, hash2, compoundOne, compoundTwo);
}

// Compute intersection of a shape with a plane
CTiglIntersectionCalculation::CTiglIntersectionCalculation(CTiglShapeCache* cache,
                                                           const std::string& shapeID,
                                                           TopoDS_Shape shape,
                                                           gp_Pnt point,
                                                           gp_Dir normal)
    : tolerance(1.0e-7)
{

    size_t hash1 = boost::hash<std::string>()(shapeID);
    size_t hash2 = HashablePlane(point, normal).HashValue();

    // create plane
    TopoDS_Shape plane = BRepBuilderAPI_MakeFace(gp_Pln(point, normal));
    computeIntersection(cache, hash1, hash2, shape, plane);
}

// Computes the intersection of a shape with a plane segment,
// that is defined by two points p1 and p2 and a vector w in
// the following way: p(u,v) = p1 (1-u) + p2u + wv, with
// u in [0,1]. Ideally w should be perpendicular to (p1 - p2)
// cache variable can be NULL
CTiglIntersectionCalculation::CTiglIntersectionCalculation(CTiglShapeCache* cache,
                                                           const std::string& shapeID,
                                                           TopoDS_Shape shape,
                                                           gp_Pnt point1,
                                                           gp_Pnt point2,
                                                           gp_Dir normal,
                                                           bool forceOrthogonal)
    : tolerance(1.0e-7)
{

    // create hash
    size_t hash = boost::hash<std::string>()(shapeID);
    size_t hashP1P2 = hash_combine_symmetric( HashableProjection(point1, normal).HashValue(), HashableProjection(point2, normal).HashValue() );
    boost::hash_combine(hash, hashP1P2);
    //TODO make hash independent of sign of normal
    boost::hash_combine(hash, normal.X());
    boost::hash_combine(hash, normal.Y());
    boost::hash_combine(hash, normal.Z());

    std::stringstream s;
    s << "int" << hash;
    id = s.str();

    bool inCache = false;
    if (cache) {
        // check, if result is already in cache
        if (cache->HasShape(id)) {
            intersectionResult = TopoDS::Compound(cache->GetShape(id));
            inCache = true;
        }
    }

    if (!inCache) {

        TopoDS_Wire p2p1 = BRepBuilderAPI_MakeWire( BRepBuilderAPI_MakeEdge(point1, point2) );

        if (forceOrthogonal) {
            // force direction orthogonal to P2-P1, i.e. use direction
            //       d =  ( (P2 - P1) x w ) x (P2 - P1)
            gp_Dir n = gp_Dir( gp_Vec(point1, point2).Crossed( gp_Vec(normal) ) );
            gp_Dir d = gp_Dir( gp_Vec(n).Crossed( gp_Vec(point1, point2) ) );
            BRepProj_Projection projector = BRepProj_Projection(p2p1, shape, d);
            intersectionResult = projector.Shape();
        }
        else {
            // use direction normal
            BRepProj_Projection projector = BRepProj_Projection(p2p1, shape, normal);
            intersectionResult = projector.Shape();
        }

        //add to cache
        if (cache) {
            cache->Insert(intersectionResult, id);
        }
    }
}

CTiglIntersectionCalculation::CTiglIntersectionCalculation(CTiglShapeCache* cache,
                                                           const std::string& wireID1,
                                                           int wireIdx1,
                                                           const std::string& wireID2,
                                                           int wireIdx2,
                                                           double tol)
    : tolerance(tol)
{
    size_t hash = hash_combine_symmetric( boost::hash<std::string>()(wireID1),
                                          boost::hash<std::string>()(wireID2) );

    std::stringstream s;
    s << "int" << hash;
    id = s.str();

    bool inCache = false;
    if (cache) {
        // check, if result is already in cache
        if (cache->HasShape(id)) {
            intersectionResult = TopoDS::Compound(cache->GetShape(id));
            inCache = true;
        }
    }

    if (!inCache) {

        // get first wire
        CTiglIntersectionCalculation Intersector1(*cache, wireID1);
        TopoDS_Wire wire1 = Intersector1.GetWire(wireIdx1);

        // get second wire
        CTiglIntersectionCalculation Intersector2(*cache, wireID2);
        TopoDS_Wire wire2 = Intersector2.GetWire(wireIdx2);

        // calculate the intersections
        intersectionPointList intersectionPoints;
        GetIntersectionPoint(wire1, wire2, intersectionPoints, tolerance);

        // save each intersection point as a vertex in intersectionResult
        intersectionResult.Nullify();
        BRep_Builder builder;
        builder.MakeCompound(intersectionResult);
        for( unsigned int i = 0; i< intersectionPoints.size(); i++ ) {
            builder.Add(intersectionResult,
                        BRepBuilderAPI_MakeVertex (intersectionPoints[i].Center) );
        }


        //add to cache
        if (cache) {
            cache->Insert(intersectionResult, id);
        }
    }

}

CTiglIntersectionCalculation::CTiglIntersectionCalculation(CTiglShapeCache& cache,
                                                           const std::string& intersectionID)
    : tolerance(1.0e-7)
{
    // check that intersectionID is in cache
    if (!cache.HasShape(intersectionID)) {
        throw CTiglError("The given intersectionID can not be found.", TIGL_NOT_FOUND);
    }
    
    try {
        intersectionResult = TopoDS::Compound(cache.GetShape(intersectionID));
    }
    catch (Standard_TypeMismatch&) {
        throw CTiglError("The given intersectionID is invalid.", TIGL_NOT_FOUND);
    }

    id = intersectionID;
}

void CTiglIntersectionCalculation::computeIntersection(CTiglShapeCache * cache,
                                                       size_t hashOne,
                                                       size_t hashTwo,
                                                       TopoDS_Shape compoundOne,
                                                       TopoDS_Shape compoundTwo)
{
    // create some identification id to store intersection in cache
    // it should not matter, if the arguments One and Two are interchanged
    // the xor commutes, so this should work
    size_t tmpid;
    if (hashOne != hashTwo) {
        tmpid = hash_combine_symmetric(hashOne, hashTwo);
    }
    else
    {
        tmpid = hashOne;
    }
    std::stringstream s;
    s << "int" << tmpid;
    id = s.str();

    bool inCache = false;
    if (cache) {
        // check, if result is already in cache
        if (cache->HasShape(id)) {
            intersectionResult = TopoDS::Compound(cache->GetShape(id));
            inCache = true;
        }
    }

    if (!inCache) {
        Standard_Boolean PerformNow=Standard_False;
        BRepAlgoAPI_Section section(compoundOne, compoundTwo, PerformNow);
        section.ComputePCurveOn1(Standard_True);
        section.Approximation(Standard_True);
        section.Build();
        TopoDS_Shape result = section.Shape();
        TopExp_Explorer myEdgeExplorer (result, TopAbs_EDGE);

        Handle(TopTools_HSequenceOfShape) Edges = new TopTools_HSequenceOfShape();

        while (myEdgeExplorer.More()) {
            Edges->Append(TopoDS::Edge(myEdgeExplorer.Current()));
            myEdgeExplorer.Next();
        }

        // connect all connected edges to wires and save them in container Edges again
        ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Edges, tolerance, false, Edges);
        int numWires = Edges->Length();

        intersectionResult.Nullify();
        BRep_Builder builder;
        builder.MakeCompound(intersectionResult);
        std::vector<TopoDS_Wire> Wires;

        // filter duplicated wires
        for (int wireID=1; wireID <= numWires; wireID++) {
            bool found = false;
            TopoDS_Wire wire = TopoDS::Wire(Edges->Value(wireID));
            for (std::vector<TopoDS_Wire>::size_type i = 0; i < Wires.size(); i++) {
                if (Wires[i].HashCode(200000) == wire.HashCode(200000)) {
                        found = true;
                }
            }

            if (!found) {
                Wires.push_back(wire);
                builder.Add(intersectionResult, wire);
            }
        }

        // add to cache
        if (cache) {
            cache->Insert(intersectionResult, id);
        }
    }
}

// Destructor
CTiglIntersectionCalculation::~CTiglIntersectionCalculation( void )
{
}


// returns total number of intersection lines
int CTiglIntersectionCalculation::GetCountIntersectionLines()
{
    TopExp_Explorer wireExplorer(intersectionResult, TopAbs_WIRE);
    int nwires = 0;
    for (; wireExplorer.More(); wireExplorer.Next()) {
        nwires++;
    }
    return nwires;
}

// returns total number of intersection points
int CTiglIntersectionCalculation::GetCountIntersectionPoints()
{
    TopExp_Explorer vertexExplorer(intersectionResult, TopAbs_VERTEX);
    int npoints = 0;
    for (; vertexExplorer.More(); vertexExplorer.Next()) {
        npoints++;
    }
    return npoints;
}

// Gets a point on the intersection line in dependence of a parameter zeta with
// 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the line starting point,
// for zeta = 1.0 the last point on the intersection line.
// numIntersecLine is the number of the Intersection line.
gp_Pnt CTiglIntersectionCalculation::GetPoint(double zeta, int wireID = 1)
{
    if (zeta < 0.0 || zeta > 1.0) {
        throw CTiglError("Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
    }

    TopoDS_Wire wire = GetWire(wireID);
    return WireGetPoint(wire, zeta);
}

TopoDS_Wire CTiglIntersectionCalculation::GetWire(int wireID)
{
    if (wireID > GetCountIntersectionLines() || wireID < 1){
        throw CTiglError("Invalid wireID in CTiglIntersectionCalculation::GetWire", TIGL_INDEX_ERROR);
    }

    TopExp_Explorer wireExplorer(intersectionResult, TopAbs_WIRE);
    int currentWireID = 1;
    for (; wireExplorer.More(); wireExplorer.Next()) {
        if (wireID == currentWireID) {
            return TopoDS::Wire(wireExplorer.Current());
        }
        currentWireID++;
    }

    throw CTiglError("Cannot retrieve intersection wire in CTiglIntersectionCalculation::GetWire", TIGL_ERROR);
}

TopoDS_Vertex CTiglIntersectionCalculation::GetVertex(int vertexID)
{
    if (vertexID > GetCountIntersectionPoints() || vertexID < 1){
        throw CTiglError("Invalid vertexID in CTiglIntersectionCalculation::GetVertex", TIGL_INDEX_ERROR);
    }

    TopExp_Explorer vertexExplorer(intersectionResult, TopAbs_VERTEX);
    int currentVertexID = 1;
    for (; vertexExplorer.More(); vertexExplorer.Next()) {
        if (vertexID == currentVertexID) {
            return TopoDS::Vertex(vertexExplorer.Current());
        }
        currentVertexID++;
    }

    throw CTiglError("Cannot retrieve intersection vertex in CTiglIntersectionCalculation::GetVertex", TIGL_ERROR);
}

const std::string& CTiglIntersectionCalculation::GetID()
{
    return id;
}

} // end namespace tigl

