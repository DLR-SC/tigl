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
#include "BRepAlgoAPI_Section.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"

#include <sstream>
#include <boost/functional/hash.hpp>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
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

            hash = 0;
            boost::hash_combine(hash, d);
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
        tmpid = hashOne ^ hashTwo;
    }
    else {
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

// Gets a point on the intersection line in dependence of a parameter zeta with
// 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the line starting point,
// for zeta = 1.0 the last point on the intersection line.
// numIntersecLine is the number of the Intersection line.
gp_Pnt CTiglIntersectionCalculation::GetPoint(double zeta, int wireID = 1)
{
    if (zeta < 0.0 || zeta > 1.0) {
        throw CTiglError("Error: Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
    }

    TopoDS_Wire wire = GetWire(wireID);
    return WireGetPoint(wire, zeta);
}

TopoDS_Wire CTiglIntersectionCalculation::GetWire(int wireID)
{
    if (wireID > GetCountIntersectionLines() || wireID < 1){
        throw CTiglError("Error: Invalid wireID in CTiglIntersectionCalculation::GetWire", TIGL_INDEX_ERROR);
    }

    TopExp_Explorer wireExplorer(intersectionResult, TopAbs_WIRE);
    int currentWireID = 1;
    for (; wireExplorer.More(); wireExplorer.Next()) {
        if (wireID == currentWireID) {
            return TopoDS::Wire(wireExplorer.Current());
        }
        currentWireID++;
    }

    throw CTiglError("Cannot retrieve intersection wire in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
}

const std::string& CTiglIntersectionCalculation::GetID()
{
    return id;
}

} // end namespace tigl

