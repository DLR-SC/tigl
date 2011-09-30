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
* @brief Tests for testing behavior of the configuration handle management and implementation.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;


int preTiglOpenCPACSConfiguration(void)
{
	char* filename = "../TestData/VFW-614-cpacs_0_9.xml";
	ReturnCode tixiRet;

	tiglHandle = -1;
	tixiHandle = -1;
	tixiRet = tixiOpenDocument(filename, &tixiHandle);
	return (tixiRet == SUCCESS ? 0 : 1);
}

int postTiglOpenCPACSConfiguration(void)
{
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}


/**
* Tests tiglOpenCPACSConfiguration with null pointer arguments.
*/
void tiglOpenCPACSConfiguration_nullPointerArgument(void) 
{
	CU_ASSERT(tiglOpenCPACSConfiguration(tixiHandle, "VFW-614", NULL) == TIGL_NULL_POINTER);
}

/**
* Tests a successfull run of tiglOpenCPACSConfiguration.
*/
void tiglOpenCPACSConfiguration_success(void) 
{
	CU_ASSERT(tiglOpenCPACSConfiguration(tixiHandle, "VFW-614", &tiglHandle) == TIGL_SUCCESS);
	CU_ASSERT(tiglHandle > 0);
	CU_ASSERT(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
}

/**
* Tests a successfull open of tiglOpenCPACSConfiguration with specifiing the uid of the configuration.
*/
void tiglOpenCPACSConfiguration_without_uid(void) 
{
	// Test with NULL argument
	CU_ASSERT(tiglOpenCPACSConfiguration(tixiHandle, NULL, &tiglHandle) == TIGL_SUCCESS);
	CU_ASSERT(tiglHandle > 0);
	CU_ASSERT(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);

	// Test with empty string argument
	CU_ASSERT(tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle) == TIGL_SUCCESS);
	CU_ASSERT(tiglHandle > 0);
	CU_ASSERT(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
}

/******************************************************************************/

int preTiglGetCPACSTixiHandle(void)
{
	char* filename = "../TestData/VFW-614-cpacs_0_9.xml";
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

int postTiglGetCPACSTixiHandle(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/**
* Tests tiglGetCPACSTixiHandle with null pointer arguments.
*/
void tiglGetCPACSTixiHandle_nullPointerArgument(void)
{
	CU_ASSERT(tiglGetCPACSTixiHandle(tiglHandle, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests tiglGetCPACSTixiHandle with an invalid tigl handle.
*/
void tiglGetCPACSTixiHandle_notFound(void)
{
	CU_ASSERT(tiglGetCPACSTixiHandle(tiglHandle + 1, &tixiHandle) == TIGL_NOT_FOUND);
}

/**
* Tests a successfull run of tiglGetCPACSTixiHandle.
*/
void tiglGetCPACSTixiHandle_success(void)
{
	TixiDocumentHandle tempHandle;
	CU_ASSERT(tiglGetCPACSTixiHandle(tiglHandle, &tempHandle) == TIGL_SUCCESS);
	CU_ASSERT_EQUAL(tempHandle, tixiHandle);
}

/******************************************************************************/

int preTiglIsCPACSConfigurationHandleValid(void)
{
	char* filename = "../TestData/VFW-614-cpacs_0_9.xml";
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

int postTiglIsCPACSConfigurationHandleValid(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/**
* Tests tiglIsCPACSConfigurationHandleValid with an invalid CPACS handle.
*/
void tiglIsCPACSConfigurationHandleValid_invalidHandle(void)
{
    TiglBoolean isValid;
    CU_ASSERT(tiglIsCPACSConfigurationHandleValid(-1, &isValid) == TIGL_SUCCESS);
    CU_ASSERT(isValid == TIGL_FALSE);
}

/**
* Tests tiglIsCPACSConfigurationHandleValid with a valid CPACS handle.
*/
void tiglIsCPACSConfigurationHandleValid_validHandle(void)
{
    TiglBoolean isValid;
    CU_ASSERT(tiglIsCPACSConfigurationHandleValid(tiglHandle, &isValid) == TIGL_SUCCESS);
    CU_ASSERT(isValid == TIGL_TRUE);
}


/**
* Tests tiglGetVersion.
*/
void tiglGetVersion_valid(void)
{
	CU_ASSERT(strcmp(tiglGetVersion(), TIGL_VERSION) == 0);
}

