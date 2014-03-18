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
#include "tiglcommonfunctions.h"

#include "GeomAPI_IntSS.hxx"
#include "BRep_Tool.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Solid.hxx"
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
#include "Handle_TopTools_HSequenceOfShape.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "CTiglShapeCache.h"

#include <sstream>
#include <boost/functional/hash.hpp>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace
{
    class HashablePlane
    {
    public:
        HashablePlane(const gp_Pnt& p, const gp_Dir& n)
        {
            vector.push_back(p.X());
            vector.push_back(p.Y());
            vector.push_back(p.Z());
            vector.push_back(n.X());
            vector.push_back(n.Y());
            vector.push_back(n.Z());
        }
        
        operator std::vector<double>& ()
        {
            return vector;
        }
        
    private:
        std::vector<double> vector;
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
    : tolerance(1.0e-7),
    numWires(0)
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
    : tolerance(1.0e-7),
    numWires(0)
{

    size_t hash1 = boost::hash<std::string>()(shapeID);
    size_t hash2 = boost::hash<std::vector<double> >()(HashablePlane(point, normal));
    
    // create plane
    TopoDS_Shape plane = BRepBuilderAPI_MakeFace(gp_Pln(point, normal));
    computeIntersection(cache, hash1, hash2, shape, plane);
}


void CTiglIntersectionCalculation::computeIntersection(CTiglShapeCache * cache,
                                                       size_t idOne,
                                                       size_t idTwo,
                                                       TopoDS_Shape compoundOne,
                                                       TopoDS_Shape compoundTwo)
{
    // create some identification string to store intersection in cache
    // it should not matter, if the arguments One and Two are interchanged
    std::stringstream stream;
    if (idOne < idTwo) {
        stream << idOne << "::" << idTwo;
    }
    else {
        stream << idTwo << "::" << idOne;
    }
    id = stream.str();

    bool inCache = false;
    if (cache) {
        // check, if result is already in cache
        unsigned int nshapes = cache->GetNShapesOfType(id);
        if (nshapes > 0) {
            inCache = true;
            numWires = nshapes;
            for (unsigned int i = 0; i < nshapes; ++i) {
                Wires.push_back(TopoDS::Wire(cache->GetShape(id, i)));
            }
        }
    }

    if (!inCache) {
        Standard_Boolean PerformNow=Standard_False; 
        BRepAlgoAPI_Section section(compoundOne, compoundTwo, PerformNow); 
        section.ComputePCurveOn1(Standard_True); 
        section.Approximation(Standard_True); 
        section.Build(); 
        intersectionResult = section.Shape();

        TopExp_Explorer myEdgeExplorer (intersectionResult, TopAbs_EDGE);

        Handle(TopTools_HSequenceOfShape) Edges = new TopTools_HSequenceOfShape();

        while (myEdgeExplorer.More()) {
            Edges->Append(TopoDS::Edge(myEdgeExplorer.Current()));
            myEdgeExplorer.Next();
        }

        // connect all connected edges to wires and save them in container Edges again
        ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Edges, tolerance, false, Edges);
        numWires = Edges->Length();

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
                if (cache) {
                    cache->Insert(wire, id);
                }
            }
        }
        numWires = Wires.size();
    }
}

// Destructor
CTiglIntersectionCalculation::~CTiglIntersectionCalculation( void )
{
    Wires.clear();
}


// returns total number of intersection lines
int CTiglIntersectionCalculation::GetCountIntersectionLines(void)
{
    return(numWires);
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

    if (wireID > numWires) {
        throw CTiglError("Error: Unknown wireID in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
    }

    //TopoDS_Wire wire;
    TopoDS_Wire& intersectionWire = (Wires[--wireID]);
    return WireGetPoint(intersectionWire, zeta);
}

// gives the number of wires of the intersection calculation
int CTiglIntersectionCalculation::GetNumWires()
{
    return( numWires );
}

std::string CTiglIntersectionCalculation::GetIDString(int wireID) 
{
    std::stringstream stream;
    stream << "Intersect_" << id << "_" << wireID;
    return stream.str();
}

TopoDS_Wire& CTiglIntersectionCalculation::GetWire(int wireID)
{
    if (wireID > numWires || wireID < 1){
        throw CTiglError("Error: Invalid wireID in CTiglIntersectionCalculation::GetWire", TIGL_ERROR);
    }
    return Wires.at(wireID-1);
}

} // end namespace tigl

