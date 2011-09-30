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
* @brief  Implementation of routines for managing calculation algorithms like
*         linear interpolation or bspline interpolation to build wires.
*         Implements the singleton design pattern.
*/

#ifndef CTIGLALGORITHMMANAGER_H
#define CTIGLALGORITHMMANAGER_H

#include "tigl.h"
#include "ITiglWireAlgorithm.h"
#include "CTiglInterpolateLinearWire.h"
#include "CTiglInterpolateBsplineWire.h"
#include "CTiglApproximateBsplineWire.h"

namespace tigl {

	class CTiglAlgorithmManager
	{

	public:
		// Returns a reference to the only instance of this class
		static CTiglAlgorithmManager& GetInstance(void);

		// Sets an algorithm to use
		void SetAlgorithm(TiglAlgorithmCode anAlgorithm);

		// Returns a reference to the current wire building algorithm
		const ITiglWireAlgorithm& GetWireAlgorithm(void) const;

		// Destructor
		~CTiglAlgorithmManager(void);

	private:
		// Constructor
		CTiglAlgorithmManager(void);

		// Copy constructor
		CTiglAlgorithmManager(const CTiglAlgorithmManager& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CTiglAlgorithmManager& )        { /* Do nothing */ }

	private:
		ITiglWireAlgorithm*         wireAlgorithm;                     // Pointer to current wing profile wire calculation algorithm
		CTiglInterpolateLinearWire  linearInterpolationWireAlgorithm;  // Instance of a linear interpolation algorithm for building a wire
		CTiglInterpolateBsplineWire bsplineInterpolationWireAlgorithm; // Instance of a BSpline interpolation algorithm for building a wire
		CTiglApproximateBsplineWire bsplineApproximationWireAlgorithm; // Instance of a BSpline approximation algorithm for building a wire

	};

} // end namespace tigl

#endif // CTIGLALGORITHMMANAGER_H
