/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2022-09-22 Anton Reiswich <anton.reiswich@dlr.de>
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

#include "CTiglIntersectionCalculationBase.h"
#include "CTiglIntersectionCalculationCommon.h"
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
#include "BRepAlgoAPI_Common.hxx"

#include <sstream>
#include <boost/functional/hash.hpp>


#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) > (b)) ? (b) : (a))
#endif


//// WARNING: boost::hash gets incorrectly compiled with gcc 4.3.4 - the optimization will produce indeterministic hash values
//// To workaround the problem, we have to avoid optimization of this file


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


void CTiglIntersectionCalculationCommon::computeIntersection(CTiglShapeCache * cache,
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
//        Standard_Boolean PerformNow=Standard_False;
//        BRepAlgoAPI_Section section(compoundOne, compoundTwo, PerformNow);
//        section.ComputePCurveOn1(Standard_True);
//        section.Approximation(Standard_True);
//        section.Build();
//        TopoDS_Shape result = section.Shape();
        TopoDS_Shape result = BRepAlgoAPI_Common(compoundOne, compoundTwo);  // contrary to the implementation in the base class, the overwritten function
                                                                             // uses the BRepAlgoAPI_Common class to compute "result"
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

} // end namespace tigl

