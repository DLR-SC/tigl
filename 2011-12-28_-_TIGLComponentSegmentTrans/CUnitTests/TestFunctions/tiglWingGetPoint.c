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
* @brief Tests for testing behavior of the routines for retrieving surface points.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

int preTiglWingGetPoint(void)
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

int postTiglWingGetPoint(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/**
* Tests tiglWingGetUpperPoint with invalid CPACS handle.
*/
void tiglWingGetUpperPoint_invalidHandle(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetUpperPoint(-1, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetLowerPoint with invalid CPACS handle.
*/
void tiglWingGetLowerPoint_invalidHandle(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetLowerPoint(-1, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetUpperPoint with invalid wing indices.
*/
void tiglWingGetUpperPoint_invalidWing(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle,      0, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 100000, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid wing indices.
*/
void tiglWingGetLowerPoint_invalidWing(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle,      0, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 100000, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with invalid segment indices.
*/
void tiglWingGetUpperPoint_invalidSegment(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1,     0, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 10000, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid segment indices.
*/
void tiglWingGetLowerPoint_invalidSegment(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1,     0, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 10000, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with invalid eta.
*/
void tiglWingGetUpperPoint_invalidEta(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, -1.0, 0.0, &x, &y, &z) == TIGL_ERROR);
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1,  1.1, 0.0, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid eta.
*/
void tiglWingGetLowerPoint_invalidEta(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, -1.0, 0.0, &x, &y, &z) == TIGL_ERROR);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1,  1.1, 0.0, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with invalid xsi.
*/
void tiglWingGetUpperPoint_invalidXsi(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, -1.0, &x, &y, &z) == TIGL_ERROR);
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0,  1.1, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid xsi.
*/
void tiglWingGetLowerPoint_invalidXsi(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, -1.0, &x, &y, &z) == TIGL_ERROR);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0,  1.1, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with null pointer arguments.
*/
void tiglWingGetUpperPoint_nullPointerArgument(void) 
{
    double x, y, z;
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, NULL, &y, &z) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, NULL, &z) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests tiglWingGetLowerPoint with null pointer arguments.
*/
void tiglWingGetLowerPoint_nullPointerArgument(void) 
{
    double x, y, z;
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, NULL, &y, &z) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, NULL, &z) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call to tiglWingGetUpperPoint.
*/
void tiglWingGetUpperPoint_success(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_SUCCESS);   // leading root
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.5, 0.5, &x, &y, &z) == TIGL_SUCCESS);   // middle
    CU_ASSERT(tiglWingGetUpperPoint(tiglHandle, 1, 1, 1.0, 1.0, &x, &y, &z) == TIGL_SUCCESS);   // trailing edge
}

/**
* Tests successfull call to tiglWingGetLowerPoint.
*/
void tiglWingGetLowerPoint_success(void)
{
    double x, y, z;
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_SUCCESS);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.5, 0.5, &x, &y, &z) == TIGL_SUCCESS);
    CU_ASSERT(tiglWingGetLowerPoint(tiglHandle, 1, 1, 1.0, 1.0, &x, &y, &z) == TIGL_SUCCESS);
}
