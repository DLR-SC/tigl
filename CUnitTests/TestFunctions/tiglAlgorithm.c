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
* @brief Tests for testing behavior of the TIGL algorithm selection routines.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>


/**
* Tests tiglUseAlgorithm with invalid algorithm codes.
*/
void tiglUseAlgorithm_invalidAlgorithm(void)
{
    CU_ASSERT(tiglUseAlgorithm(-1) == TIGL_ERROR);
}

/**
* Tests tiglUseAlgorithm with valid algorithm codes.
*/
void tiglUseAlgorithm_validAlgorithm(void)
{
    CU_ASSERT(tiglUseAlgorithm(TIGL_INTERPOLATE_BSPLINE_WIRE) == TIGL_SUCCESS);
    CU_ASSERT(tiglUseAlgorithm(TIGL_APPROXIMATE_BSPLINE_WIRE) == TIGL_SUCCESS);
    CU_ASSERT(tiglUseAlgorithm(TIGL_INTERPOLATE_LINEAR_WIRE)  == TIGL_SUCCESS);
}
