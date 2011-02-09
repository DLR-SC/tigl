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

#ifndef CTIGLINTERSECTIONCALCULATION
#define CTIGLINTERSECTIONCALCULATION

#include "tigl.h"
#include "CCPACSHeader.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"

#include "GeomAPI_IntSS.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "Handle_TopTools_HSequenceOfShape.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"


namespace tigl {

	class CTiglIntersectionCalculation
	{

	public:
		// Constructor
		CTiglIntersectionCalculation( TopoDS_Shape compoundOne,
									  TopoDS_Shape compoundTwo );

		// Destructor
		virtual ~CTiglIntersectionCalculation(void);

		// returns total number of intersection lines
		int GetCountIntersectionLines(void);

		// Gets a point on the intersection line in dependence of a parameter zeta with
		// 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the line starting point,
		// for zeta = 1.0 the last point on the intersection line.
		// numIntersecLine is the number of the Intersection line.
		gp_Pnt GetPoint(double zeta, int wireID);

		// gives the number of wires of the intersection calculation
		int GetNumWires();

	protected:
		// Computes the length of the intersection line
		double ComputeWireLength(int wireID);


	private:		
		Standard_Real tolerance;
		int numWires;							/* The number of intersection lines */
		TopoDS_Shape intersectionResult;		/* The full Intersection result */
		std::vector<TopoDS_Wire> Wires;	    /* All intersection wires */
		Handle(TopTools_HSequenceOfShape) Edges;/* All intersection edges */
		
	};

} // end namespace tigl

#endif // CTIGLINTERSECTIONCALCULATION
