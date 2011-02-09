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
* @brief  Implementation of routines for building a wire from a std::vector
*         of points by a BSpline interpolation.
*/

#ifndef CTIGLINTERPOLATEBSPLINEWIRE_H
#define CTIGLINTERPOLATEBSPLINEWIRE_H

#include "ITiglWireAlgorithm.h"
#include "Geom_BSplineCurve.hxx"

namespace tigl {

	class CTiglInterpolateBsplineWire : public ITiglWireAlgorithm
	{

	public:
		// Constructor
		CTiglInterpolateBsplineWire(void);

		// Destructor
		virtual ~CTiglInterpolateBsplineWire(void);

		// Builds the wire from the given points
		virtual TopoDS_Wire BuildWire(const CPointContainer& points, bool forceClosed = false) const;

		// Returns the algorithm code identifier for an algorithm
		virtual TiglAlgorithmCode GetAlgorithmCode(void) const;

		// Returns the point on the wire with the smallest x value
		virtual gp_Pnt GetPointWithMinX(const CPointContainer& points) const;

		// Returns the point on the wire with the biggest x value
		virtual gp_Pnt GetPointWithMaxX(const CPointContainer& points) const;

		// Returns the point on the wire with the smallest y value
		virtual gp_Pnt GetPointWithMinY(const CPointContainer& points) const;

		// Returns the point on the wire with the biggest y value
		virtual gp_Pnt GetPointWithMaxY(const CPointContainer& points) const;

	protected:

	private:
		// Copy constructor
		CTiglInterpolateBsplineWire(const CTiglInterpolateBsplineWire& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CTiglInterpolateBsplineWire& )              { /* Do nothing */ }

	};

} // end namespace tigl

#endif // CTIGLINTERPOLATEBSPLINEWIRE_H
