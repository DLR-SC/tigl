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
* @brief Tests for testing non classified wing functions.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>
#include <string.h>


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

int preTiglWing(void)
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

int postTiglWing(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/******************************************************************************/

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
void tiglWingGetProfileName_invalidHandle(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetProfileName(-1, 1, 1, 1, &namePtr) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetProfileName with invalid wing index.
*/
void tiglWingGetProfileName_invalidWing(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetProfileName(tiglHandle, -1, 1, 1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with invalid section index.
*/
void tiglWingGetProfileName_invalidSection(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetProfileName(tiglHandle, 1, -1, 1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with invalid element index.
*/
void tiglWingGetProfileName_invalidElement(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetProfileName(tiglHandle, 1, 1, -1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with null pointer argument.
*/
void tiglWingGetProfileName_nullPointerArgument(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetProfileName(tiglHandle, 1, 1, 1, 0) == TIGL_NULL_POINTER);
}

/**
* Tests tiglWingGetProfileName with null pointer argument.
*/
void tiglWingGetProfileName_success(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetProfileName(tiglHandle, 1, 1, 1, &namePtr) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(namePtr, "VFW614 wing airfoil Y=0.") == 0);
}


/**
* Tests tiglWingGetUID 
*/
void tiglWingGetUID_success(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetUID(tiglHandle, 1, &namePtr) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(namePtr, "MainWing") == 0);
}


/**
* Tests tiglWingGetSegmentUID 
*/
void tiglWingGetSegmentUID_success(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetSegmentUID(tiglHandle, 1, 1, &namePtr) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(namePtr, "MainWingInnerSegment") == 0);
}


/**
* Tests tiglWingGetSectionUID 
*/
void tiglWingGetSectionUID_success(void)
{
    char* namePtr = 0;
    CU_ASSERT(tiglWingGetSectionUID(tiglHandle, 1, 1, &namePtr) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(namePtr, "SMY0.") == 0);
}


