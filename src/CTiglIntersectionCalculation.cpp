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
#include "BRepAlgoAPI_Section.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "Handle_TopTools_HSequenceOfShape.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace tigl {

    // Constructor
    CTiglIntersectionCalculation::CTiglIntersectionCalculation( TopoDS_Shape compoundOne,
                                                                TopoDS_Shape compoundTwo)
        : tolerance(1.0e-7),
        numWires(0)
    {
        Standard_Boolean PerformNow=Standard_False; 
        BRepAlgoAPI_Section section(compoundOne, compoundTwo, PerformNow); 
        section.ComputePCurveOn1(Standard_True); 
        section.Approximation(Standard_True); 
        section.Build(); 
        intersectionResult = section.Shape();

        TopExp_Explorer myEdgeExplorer (intersectionResult, TopAbs_EDGE);

        Edges = new TopTools_HSequenceOfShape();

        while (myEdgeExplorer.More())
        {
            Edges->Append(TopoDS::Edge(myEdgeExplorer.Current()));
            myEdgeExplorer.Next();
        }

        // connect all connected edges to wires and save them in container Edges again
        ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Edges, tolerance, false, Edges);
        numWires = Edges->Length();

        // filter duplicated wires
        for (int wireID=1; wireID <= numWires; wireID++)
        {
            bool found = false;
            TopoDS_Wire wire = TopoDS::Wire(Edges->Value(wireID));
            for (std::vector<TopoDS_Wire>::size_type i = 0; i < Wires.size(); i++)
            {
                if (Wires[i].HashCode(200000) == wire.HashCode(200000))
                {
                        found = true;
                }
            }

            if(!found)
            {
                Wires.push_back(wire);
            }
        }
        numWires = Wires.size();
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
        if (zeta < 0.0 || zeta > 1.0) 
        {
            throw CTiglError("Error: Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
        }

        if (wireID > numWires)
        {
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
    
    TopoDS_Wire& CTiglIntersectionCalculation::GetWire(int wireID){
        if (wireID > numWires || wireID < 1){
            throw CTiglError("Error: Invalid wireID in CTiglIntersectionCalculation::GetWire", TIGL_ERROR);
        }
        return Wires.at(wireID-1);
    }

} // end namespace tigl

