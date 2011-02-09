/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
			numWires++;
		}

		// connect edges to wires and save them to Wire-sequence
		ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Edges, tolerance, false, Edges);

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
		numWires = Wires.size() + 1;
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


    // Computes the length of the intersection line
    double CTiglIntersectionCalculation::ComputeWireLength(int wireID)
    {
        double wireLength = 0.0;
		TopoDS_Wire intersectionWire = (Wires[--wireID]);

		// explore all edges of result wire
		BRepTools_WireExplorer wireExplorer;
		for (wireExplorer.Init(intersectionWire); wireExplorer.More(); wireExplorer.Next())       
		{   
			Standard_Real firstParam;
			Standard_Real lastParam;
			TopoDS_Edge edge = wireExplorer.Current();
			Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);
			GeomAdaptor_Curve adaptorCurve(curve);
			wireLength += GCPnts_AbscissaPoint::Length(adaptorCurve);
		}
		return(wireLength);
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
		int i=1;
        double length = ComputeWireLength(wireID) * zeta;
		TopoDS_Wire intersectionWire = (Wires[--wireID]);

        // Get the first edge of the wire
		BRepTools_WireExplorer wireExplorer( intersectionWire );
        if (!wireExplorer.More()) 
        {
            throw CTiglError("Error: Not enough edges found in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
        }
        Standard_Real firstParam;
        Standard_Real lastParam;
        TopoDS_Edge edge = wireExplorer.Current();
        wireExplorer.Next();
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);

        // Length of current edge
        GeomAdaptor_Curve adaptorCurve;
        adaptorCurve.Load(curve);
        double currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

        // Length of complete wire up to now
        double sumLength = currLength;

        while (length > sumLength) 
        {
            if (!wireExplorer.More())
                break;
            edge = wireExplorer.Current();
            wireExplorer.Next();

            curve = BRep_Tool::Curve(edge, firstParam, lastParam);
            adaptorCurve.Load(curve);

            // Length of current edge
            currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

            // Length of complete wire up to now
            sumLength += currLength;
        }

        // Distance of searched point from end point of current edge
        double currEndDelta = max((sumLength - length), 0.0);

        // Distance of searched point from start point of current edge
        double currDist = max((currLength - currEndDelta), 0.0);

        GCPnts_AbscissaPoint abscissaPoint(adaptorCurve, currDist, adaptorCurve.FirstParameter());
        gp_Pnt point = adaptorCurve.Value(abscissaPoint.Parameter());
		return (point);       
    }

	// gives the number of wires of the intersection calculation
	int CTiglIntersectionCalculation::GetNumWires()
	{
		return( numWires );
	}

} // end namespace tigl

