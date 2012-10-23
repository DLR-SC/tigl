/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2012-10-17 Martin Siggel <Martin.Siggel@dlr.de>
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
* @brief Tests for testing behavior of wing component segment functions.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

int preTiglWingComponentSegment(void)
{
	char* filename = "TestData/CPACS_21_D150.xml";
	ReturnCode tixiRet;
	TiglReturnCode tiglRet;

	tiglHandle = -1;
	tixiHandle = -1;
	
	tixiRet = tixiOpenDocument(filename, &tixiHandle);
	if (tixiRet != SUCCESS) 
		return 1;

	tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
	return (tiglRet == TIGL_SUCCESS ? 0 : 1);
}

int postTiglWingComponentSegment(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

void tiglWingGetComponentSegmentCount_success(void){
	int numCompSeg = 0;
	CU_ASSERT(tiglWingGetComponentSegmentCount(tiglHandle, 1, &numCompSeg) == TIGL_SUCCESS);
	CU_ASSERT(numCompSeg == 1);
}

void tiglWingGetComponentSegmentCount_wrongWing(void){
	int numCompSeg = 0;
	// this example contains only 3 wings
	CU_ASSERT(tiglWingGetComponentSegmentCount(tiglHandle, 4, &numCompSeg) == TIGL_INDEX_ERROR);
}

void tiglWingGetComponentSegmentCount_nullPtr(void){
	// this example contains only 3 wings
	CU_ASSERT(tiglWingGetComponentSegmentCount(tiglHandle, 1, NULL) == TIGL_NULL_POINTER);
}

void tiglWingGetComponentSegmentCount_wrongHandle(void){
	int numCompSeg = 0;
	TiglCPACSConfigurationHandle myWrongHandle = -1234;
	CU_ASSERT(tiglWingGetComponentSegmentCount(myWrongHandle, 1, &numCompSeg) == TIGL_NOT_FOUND);
}


void tiglWingComponentGetEtaXsi_success(void)
{
	double eta = 0.1103;
	double xsi = 0.812922;
	char * wingUID = NULL;
	char * segmentUID = NULL;
	double segmentEta = 0., segmentXsi = 0.;

	TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_VAMP_W1_CompSeg1", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	CU_ASSERT( ret == TIGL_SUCCESS);
	CU_ASSERT(strcmp(wingUID, "D150_VAMP_W1") == 0);

	// test for a bug  in tigl 2.0.2, occurs when component segment does not lie on first wing
	eta = 0.16;
	xsi = 0.577506;
	ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_VAMP_HL1_CompSeg1", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	CU_ASSERT(ret == TIGL_SUCCESS);
	CU_ASSERT(strcmp(wingUID, "D150_VAMP_HL1") == 0);
}

void tiglWingComponentGetEtaXsi_wrongUID(void){
	double eta = 0.1103;
	double xsi = 0.812922;
	char * wingUID = NULL;
	char * segmentUID = NULL;
	double segmentEta = 0., segmentXsi = 0.;

	TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "invalid_comp_seg", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	CU_ASSERT(ret == TIGL_UID_ERROR);
}