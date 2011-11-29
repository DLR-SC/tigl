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
* @brief Tests for testing behavior of the routines for retrieving fuselage surface points.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

int preTiglFuselageGetPoint(void)
{
	char* filename = "../TestData/cpacs_25032009.xml";
	ReturnCode tixiRet;
	TiglReturnCode tiglRet;

	tiglHandle = -1;
	tixiHandle = -1;
	
	tixiRet = tixiOpenDocument(filename, &tixiHandle);
	if (tixiRet != SUCCESS) 
		return 1;

	tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "VFW-614", &tiglHandle);
	return (tiglRet == TIGL_SUCCESS ? 0 : 1);
}

int postTiglFuselageGetPoint(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/**
* Tests tiglFuselageGetPoint with invalid CPACS handle.
*/
void tiglFuselageGetPoint_invalidHandle(void)
{
    double x, y, z;
    CU_ASSERT(tiglFuselageGetPoint(-1, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetPoint with invalid fuselage indices.
*/
void tiglFuselageGetPoint_invalidFuselage(void)
{
    double x, y, z;
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle,      0, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 100000, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetPoint with invalid segment indices.
*/
void tiglFuselageGetPoint_invalidSegment(void)
{
    double x, y, z;
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1,     0, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 10000, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetPoint with invalid eta.
*/
void tiglFuselageGetPoint_invalidEta(void)
{
    double x, y, z;
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, -0.1, 0.0, &x, &y, &z) == TIGL_ERROR);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1,  1.1, 0.0, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglFuselageGetPoint with invalid zeta.
*/
void tiglFuselageGetPoint_invalidZeta(void)
{
    double x, y, z;
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, -0.1, &x, &y, &z) == TIGL_ERROR);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0,  1.1, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglFuselageGetPoint with null pointer arguments.
*/
void tiglFuselageGetPoint_nullPointerArgument(void) 
{
    double x, y, z;
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, NULL, &y, &z) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, NULL, &z) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call to tiglFuselageGetPoint.
*/
void tiglFuselageGetPoint_success(void)
{
    double x, y, z;
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_SUCCESS);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.5, 0.5, &x, &y, &z) == TIGL_SUCCESS);
    CU_ASSERT(tiglFuselageGetPoint(tiglHandle, 1, 1, 1.0, 1.0, &x, &y, &z) == TIGL_SUCCESS);
}

