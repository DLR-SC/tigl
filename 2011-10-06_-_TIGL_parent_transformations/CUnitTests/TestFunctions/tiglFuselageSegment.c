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
* @brief Tests for testing behavior of the routines for fuselage segment handling/query.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>


/***************************************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

int preTiglFuselageSegment(void)
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

int postTiglFuselageSegment(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/***************************************************************************************************/

/**
* Tests tiglGetFuselageCount with invalid CPACS handle.
*/
void tiglGetFuselageCount_invalidHandle(void)
{
	int fuselageCount;
	CU_ASSERT(tiglGetFuselageCount(-1, &fuselageCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglGetFuselageCount with null pointer argument.
*/
void tiglGetFuselageCount_nullPointerArgument(void)
{
	CU_ASSERT(tiglGetFuselageCount(tiglHandle, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglGetFuselageCount.
*/
void tiglGetFuselageCount_success(void)
{
	int fuselageCount;
	CU_ASSERT(tiglGetFuselageCount(tiglHandle, &fuselageCount) == TIGL_SUCCESS);
	CU_ASSERT(fuselageCount == 1);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetSegmentCount with invalid CPACS handle.
*/
void tiglFuselageGetSegmentCount_invalidHandle(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetSegmentCount(-1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetSegmentCount with invalid fuselage index.
*/
void tiglFuselageGetSegmentCount_invalidFuselage(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetSegmentCount(tiglHandle, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetSegmentCount with null pointer argument for segmentCountPtr.
*/
void tiglFuselageGetSegmentCount_nullPointerArgument(void)
{
	CU_ASSERT(tiglFuselageGetSegmentCount(tiglHandle, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetSegmentCount.
*/
void tiglFuselageGetSegmentCount_success(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetSegmentCount(tiglHandle, 1, &segmentCount) == TIGL_SUCCESS);
	CU_ASSERT(segmentCount == 50);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with invalid CPACS handle.
*/
void tiglFuselageGetStartConnectedSegmentCount_invalidHandle(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentCount(-1, 1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with invalid fuselage index.
*/
void tiglFuselageGetStartConnectedSegmentCount_invalidFuselage(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 0, 1, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with invalid segment index.
*/
void tiglFuselageGetStartConnectedSegmentCount_invalidSegment(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with null pointer argument.
*/
void tiglFuselageGetStartConnectedSegmentCount_nullPointerArgument(void)
{
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetStartConnectedSegmentCount.
*/
void tiglFuselageGetStartConnectedSegmentCount_success(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 1, &segmentCount) == TIGL_SUCCESS);
	CU_ASSERT(segmentCount == 0);
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 2, &segmentCount) == TIGL_SUCCESS);
	CU_ASSERT(segmentCount == 1);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with invalid CPACS handle.
*/
void tiglFuselageGetEndConnectedSegmentCount_invalidHandle(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentCount(-1, 1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with invalid fuselage index.
*/
void tiglFuselageGetEndConnectedSegmentCount_invalidFuselage(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 0, 1, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with invalid segment index.
*/
void tiglFuselageGetEndConnectedSegmentCount_invalidSegment(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with null pointer argument.
*/
void tiglFuselageGetEndConnectedSegmentCount_nullPointerArgument(void)
{
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetEndConnectedSegmentCount.
*/
void tiglFuselageGetEndConnectedSegmentCount_success(void)
{
	int segmentCount;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 1, &segmentCount) == TIGL_SUCCESS);
	CU_ASSERT(segmentCount == 1);
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 50, &segmentCount) == TIGL_SUCCESS);
	CU_ASSERT(segmentCount == 0);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid CPACS handle.
*/
void tiglFuselageGetStartConnectedSegmentIndex_invalidHandle(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(-1, 1, 2, 1, &connectedSegment) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid fuselage index.
*/
void tiglFuselageGetStartConnectedSegmentIndex_invalidFuselage(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 0, 2, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid segment index.
*/
void tiglFuselageGetStartConnectedSegmentIndex_invalidSegment(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 0, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid segment n-parameter.
*/
void tiglFuselageGetStartConnectedSegmentIndex_invalidN(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 1, 1, &connectedSegment) == TIGL_INDEX_ERROR);
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 1, 0, &connectedSegment) == TIGL_INDEX_ERROR);
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 2, 2, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with null pointer argument.
*/
void tiglFuselageGetStartConnectedSegmentIndex_nullPointerArgument(void)
{
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 2, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetStartConnectedSegmentIndex.
*/
void tiglFuselageGetStartConnectedSegmentIndex_success(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 2, 1, &connectedSegment) == TIGL_SUCCESS);
	CU_ASSERT(connectedSegment == 1);
	CU_ASSERT(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 3, 1, &connectedSegment) == TIGL_SUCCESS);
	CU_ASSERT(connectedSegment == 2);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid CPACS handle.
*/
void tiglFuselageGetEndConnectedSegmentIndex_invalidHandle(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(-1, 1, 1, 1, &connectedSegment) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid fuselage index.
*/
void tiglFuselageGetEndConnectedSegmentIndex_invalidFuselage(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 0, 1, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid segment index.
*/
void tiglFuselageGetEndConnectedSegmentIndex_invalidSegment(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 0, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid segment n-parameter.
*/
void tiglFuselageGetEndConnectedSegmentIndex_invalidN(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 2, &connectedSegment) == TIGL_INDEX_ERROR);
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 0, &connectedSegment) == TIGL_INDEX_ERROR);
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 50, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with null pointer argument.
*/
void tiglFuselageGetEndConnectedSegmentIndex_nullPointerArgument(void)
{
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetEndConnectedSegmentIndex.
*/
void tiglFuselageGetEndConnectedSegmentIndex_success(void)
{
	int connectedSegment;
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 1, &connectedSegment) == TIGL_SUCCESS);
	CU_ASSERT(connectedSegment == 2);
	CU_ASSERT(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 2, 1, &connectedSegment) == TIGL_SUCCESS);
	CU_ASSERT(connectedSegment == 3);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with invalid CPACS handle.
*/
void tiglFuselageGetStartSectionAndElementIndex_invalidHandle(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(-1, 1, 1, &sectionIndex, &elementIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with invalid fuselage index.
*/
void tiglFuselageGetStartSectionAndElementIndex_invalidFuselage(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 0, 1, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with invalid segment index.
*/
void tiglFuselageGetStartSectionAndElementIndex_invalidSegment(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 0, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with null pointer argument.
*/
void tiglFuselageGetStartSectionAndElementIndex_nullPointerArgument(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, NULL, &elementIndex) == TIGL_NULL_POINTER);
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, NULL) == TIGL_NULL_POINTER);
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, NULL, NULL)          == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetStartSectionAndElementIndex.
*/
void tiglFuselageGetStartSectionAndElementIndex_success(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
	CU_ASSERT(sectionIndex == 1);
	CU_ASSERT(elementIndex == 1);
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 2, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
	CU_ASSERT(sectionIndex == 2);
	CU_ASSERT(elementIndex == 1);
	CU_ASSERT(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 3, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
	CU_ASSERT(sectionIndex == 3);
	CU_ASSERT(elementIndex == 1);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with invalid CPACS handle.
*/
void tiglFuselageGetEndSectionAndElementIndex_invalidHandle(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(-1, 1, 1, &sectionIndex, &elementIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with invalid fuselage index.
*/
void tiglFuselageGetEndSectionAndElementIndex_invalidFuselage(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 0, 1, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with invalid segment index.
*/
void tiglFuselageGetEndSectionAndElementIndex_invalidSegment(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 0, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with null pointer argument.
*/
void tiglFuselageGetEndSectionAndElementIndex_nullPointerArgument(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, NULL, &elementIndex) == TIGL_NULL_POINTER);
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, NULL) == TIGL_NULL_POINTER);
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, NULL, NULL)          == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetEndSectionAndElementIndex.
*/
void tiglFuselageGetEndSectionAndElementIndex_success(void)
{
	int sectionIndex;
	int elementIndex;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
	CU_ASSERT(sectionIndex == 2);
	CU_ASSERT(elementIndex == 1);
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 2, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
	CU_ASSERT(sectionIndex == 3);
	CU_ASSERT(elementIndex == 1);
	CU_ASSERT(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 3, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
	CU_ASSERT(sectionIndex == 4);
	CU_ASSERT(elementIndex == 1);
}




/***************************************************************************************************/
/*       Test of tiglFuselageGet[Inner|Outer]SectionAndElementUID-Functions      */ 
/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartSectionAndElementUID with invalid CPACS handle.
*/
void tiglFuselageGetStartSectionAndElementUID_invalidHandle(void)
{
	char* sectionUID;
	char* elementUID;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementUID(-1, 1, 1, &sectionUID, &elementUID) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartSectionAndElementUID with invalid fuselage index.
*/
void tiglFuselageGetStartSectionAndElementUID_invalidFuselage(void)
{
	char* sectionUID;
	char* elementUID;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 0, 1, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartSectionAndElementUID with invalid segment index.
*/
void tiglFuselageGetStartSectionAndElementUID_invalidSegment(void)
{
	char* sectionUID;
	char* elementUID;
	CU_ASSERT(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 0, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}


/**
* Tests successfull call of tiglFuselageGetStartSectionAndElementUID.
*/
void tiglFuselageGetStartSectionAndElementUID_success(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 1, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SFX0.0174") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMX0.0174") == 0);
	free(sectionUID);
	free(elementUID);

    CU_ASSERT(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 2, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SFX0.4207") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMX0.4207") == 0);
	free(sectionUID);
	free(elementUID);

    CU_ASSERT(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 3, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SFX0.8241") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMX0.8241") == 0);
	free(sectionUID);
	free(elementUID);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndSectionAndElementUID with invalid CPACS handle.
*/
void tiglFuselageGetEndSectionAndElementUID_invalidHandle(void)
{
	char* sectionUID;
	char* elementUID;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementUID(-1, 1, 1, &sectionUID, &elementUID) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndSectionAndElementUID with invalid fuselage index.
*/
void tiglFuselageGetEndSectionAndElementUID_invalidFuselage(void)
{
	char* sectionUID;
	char* elementUID;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 0, 1, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndSectionAndElementUID with invalid segment index.
*/
void tiglFuselageGetEndSectionAndElementUID_invalidSegment(void)
{
	char* sectionUID;
	char* elementUID;
	CU_ASSERT(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 0, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests successfull call of tiglFuselageGetEndSectionAndElementUID.
*/
void tiglFuselageGetEndSectionAndElementUID_success(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 1, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SFX0.4207") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMX0.4207") == 0);
	free(sectionUID);
	free(elementUID);

    CU_ASSERT(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 2, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SFX0.8241") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMX0.8241") == 0);
	free(sectionUID);
	free(elementUID);

    CU_ASSERT(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 3, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SFX1.2274") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMX1.2274") == 0);
	free(sectionUID);
	free(elementUID);
}



