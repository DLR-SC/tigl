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

#include "CTiglAlgorithmManager.h"
#include "CTiglError.h"

namespace tigl {

	// Constructor
	CTiglAlgorithmManager::CTiglAlgorithmManager()
	{
		wireAlgorithm = &bsplineInterpolationWireAlgorithm;
	}

	// Destructor
	CTiglAlgorithmManager::~CTiglAlgorithmManager(void)
	{
	}

	// Returns a pointer to the only instance of this class
	CTiglAlgorithmManager& CTiglAlgorithmManager::GetInstance(void)
	{
		static CTiglAlgorithmManager instance;
		return instance;
	}

	// Sets an algorithm to use
	void CTiglAlgorithmManager::SetAlgorithm(TiglAlgorithmCode anAlgorithm)
	{
		switch (anAlgorithm) 
		{
			case TIGL_INTERPOLATE_LINEAR_WIRE:
				wireAlgorithm = &linearInterpolationWireAlgorithm;
				break;
			case TIGL_INTERPOLATE_BSPLINE_WIRE:
				wireAlgorithm = &bsplineInterpolationWireAlgorithm;
				break;
			case TIGL_APPROXIMATE_BSPLINE_WIRE:
				wireAlgorithm = &bsplineApproximationWireAlgorithm;
				break;
			default:
				throw CTiglError("Error: Invalid algorithm code or algorithm not implemented in CTiglAlgorithmManager::SetAlgorithm", TIGL_ERROR);
		}
	}

	// Returns a reference to the current wire algorithm
	const ITiglWireAlgorithm& CTiglAlgorithmManager::GetWireAlgorithm(void) const
	{
		return (*wireAlgorithm);
	}


} // end namespace tigl
