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
* @brief Tests for testing behavior of the routines for segment handling/query.
*/

#include "CUnit/CUnit.h"
#include "tigl.h"
#include <stdio.h>


/***************************************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

int preTiglWingSegment(void)
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

int postTiglWingSegment(void)
{
	tiglCloseCPACSConfiguration(tiglHandle);
	tixiCloseDocument(tixiHandle);
	tiglHandle = -1;
	tixiHandle = -1;
	return 0;
}

/***************************************************************************************************/

/**
* Tests tiglGetWingCount with invalid CPACS handle.
*/
void tiglGetWingCount_invalidHandle(void)
{
    int wingCount;
    CU_ASSERT(tiglGetWingCount(-1, &wingCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglGetWingCount with null pointer argument.
*/
void tiglGetWingCount_nullPointerArgument(void)
{
    CU_ASSERT(tiglGetWingCount(tiglHandle, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglGetWingCount.
*/
void tiglGetWingCount_success(void)
{
    int wingCount;
    CU_ASSERT(tiglGetWingCount(tiglHandle, &wingCount) == TIGL_SUCCESS);
    CU_ASSERT(wingCount == 2);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetSegmentCount with invalid CPACS handle.
*/
void tiglWingGetSegmentCount_invalidHandle(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetSegmentCount(-1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetSegmentCount with invalid wing index.
*/
void tiglWingGetSegmentCount_invalidWing(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetSegmentCount(tiglHandle, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetSegmentCount with null pointer argument for segmentCountPtr.
*/
void tiglWingGetSegmentCount_nullPointerArgument(void)
{
    CU_ASSERT(tiglWingGetSegmentCount(tiglHandle, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglWingGetSegmentCount.
*/
void tiglWingGetSegmentCount_success(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetSegmentCount(tiglHandle, 1, &segmentCount) == TIGL_SUCCESS);
    CU_ASSERT(segmentCount == 3);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetInnerConnectedSegmentCount with invalid CPACS handle.
*/
void tiglWingGetInnerConnectedSegmentCount_invalidHandle(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentCount(-1, 1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetInnerConnectedSegmentCount with invalid wing index.
*/
void tiglWingGetInnerConnectedSegmentCount_invalidWing(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentCount(tiglHandle, 0, 1, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerConnectedSegmentCount with invalid segment index.
*/
void tiglWingGetInnerConnectedSegmentCount_invalidSegment(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentCount(tiglHandle, 1, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerConnectedSegmentCount with null pointer argument.
*/
void tiglWingGetInnerConnectedSegmentCount_nullPointerArgument(void)
{
    CU_ASSERT(tiglWingGetInnerConnectedSegmentCount(tiglHandle, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglWingGetInnerConnectedSegmentCount.
*/
void tiglWingGetInnerConnectedSegmentCount_success(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentCount(tiglHandle, 1, 1, &segmentCount) == TIGL_SUCCESS);
    CU_ASSERT(segmentCount == 0);
    CU_ASSERT(tiglWingGetInnerConnectedSegmentCount(tiglHandle, 1, 2, &segmentCount) == TIGL_SUCCESS);
    CU_ASSERT(segmentCount == 1);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetOuterConnectedSegmentCount with invalid CPACS handle.
*/
void tiglWingGetOuterConnectedSegmentCount_invalidHandle(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentCount(-1, 1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetOuterConnectedSegmentCount with invalid wing index.
*/
void tiglWingGetOuterConnectedSegmentCount_invalidWing(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentCount(tiglHandle, 0, 1, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterConnectedSegmentCount with invalid segment index.
*/
void tiglWingGetOuterConnectedSegmentCount_invalidSegment(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentCount(tiglHandle, 1, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterConnectedSegmentCount with null pointer argument.
*/
void tiglWingGetOuterConnectedSegmentCount_nullPointerArgument(void)
{
    CU_ASSERT(tiglWingGetOuterConnectedSegmentCount(tiglHandle, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglWingGetOuterConnectedSegmentCount.
*/
void tiglWingGetOuterConnectedSegmentCount_success(void)
{
    int segmentCount;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentCount(tiglHandle, 1, 1, &segmentCount) == TIGL_SUCCESS);
    CU_ASSERT(segmentCount == 1);
    CU_ASSERT(tiglWingGetOuterConnectedSegmentCount(tiglHandle, 1, 3, &segmentCount) == TIGL_SUCCESS);
    CU_ASSERT(segmentCount == 0);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetInnerConnectedSegmentIndex with invalid CPACS handle.
*/
void tiglWingGetInnerConnectedSegmentIndex_invalidHandle(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(-1, 1, 2, 1, &connectedSegment) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetInnerConnectedSegmentIndex with invalid wing index.
*/
void tiglWingGetInnerConnectedSegmentIndex_invalidWing(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 0, 2, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerConnectedSegmentIndex with invalid segment index.
*/
void tiglWingGetInnerConnectedSegmentIndex_invalidSegment(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 1, 0, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerConnectedSegmentIndex with invalid segment n-parameter.
*/
void tiglWingGetInnerConnectedSegmentIndex_invalidN(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 1, 1, 1, &connectedSegment) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 1, 1, 0, &connectedSegment) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 1, 2, 2, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerConnectedSegmentIndex with null pointer argument.
*/
void tiglWingGetInnerConnectedSegmentIndex_nullPointerArgument(void)
{
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 1, 2, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglWingGetInnerConnectedSegmentIndex.
*/
void tiglWingGetInnerConnectedSegmentIndex_success(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 1, 2, 1, &connectedSegment) == TIGL_SUCCESS);
    CU_ASSERT(connectedSegment == 1);
    CU_ASSERT(tiglWingGetInnerConnectedSegmentIndex(tiglHandle, 1, 3, 1, &connectedSegment) == TIGL_SUCCESS);
    CU_ASSERT(connectedSegment == 2);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetOuterConnectedSegmentIndex with invalid CPACS handle.
*/
void tiglWingGetOuterConnectedSegmentIndex_invalidHandle(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(-1, 1, 1, 1, &connectedSegment) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetOuterConnectedSegmentIndex with invalid wing index.
*/
void tiglWingGetOuterConnectedSegmentIndex_invalidWing(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 0, 1, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterConnectedSegmentIndex with invalid segment index.
*/
void tiglWingGetOuterConnectedSegmentIndex_invalidSegment(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 1, 0, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterConnectedSegmentIndex with invalid segment n-parameter.
*/
void tiglWingGetOuterConnectedSegmentIndex_invalidN(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 1, 1, 2, &connectedSegment) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 1, 1, 0, &connectedSegment) == TIGL_INDEX_ERROR);
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 1, 3, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterConnectedSegmentIndex with null pointer argument.
*/
void tiglWingGetOuterConnectedSegmentIndex_nullPointerArgument(void)
{
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 1, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglWingGetOuterConnectedSegmentIndex.
*/
void tiglWingGetOuterConnectedSegmentIndex_success(void)
{
    int connectedSegment;
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 1, 1, 1, &connectedSegment) == TIGL_SUCCESS);
    CU_ASSERT(connectedSegment == 2);
    CU_ASSERT(tiglWingGetOuterConnectedSegmentIndex(tiglHandle, 1, 2, 1, &connectedSegment) == TIGL_SUCCESS);
    CU_ASSERT(connectedSegment == 3);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetInnerSectionAndElementIndex with invalid CPACS handle.
*/
void tiglWingGetInnerSectionAndElementIndex_invalidHandle(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(-1, 1, 1, &sectionIndex, &elementIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetInnerSectionAndElementIndex with invalid wing index.
*/
void tiglWingGetInnerSectionAndElementIndex_invalidWing(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 0, 1, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerSectionAndElementIndex with invalid segment index.
*/
void tiglWingGetInnerSectionAndElementIndex_invalidSegment(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 1, 0, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerSectionAndElementIndex with null pointer argument.
*/
void tiglWingGetInnerSectionAndElementIndex_nullPointerArgument(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 1, 1, NULL, &elementIndex) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, NULL) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 1, 1, NULL, NULL)          == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglWingGetInnerSectionAndElementIndex.
*/
void tiglWingGetInnerSectionAndElementIndex_success(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    CU_ASSERT(sectionIndex == 1);
    CU_ASSERT(elementIndex == 1);
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 1, 2, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    CU_ASSERT(sectionIndex == 2);
    CU_ASSERT(elementIndex == 1);
    CU_ASSERT(tiglWingGetInnerSectionAndElementIndex(tiglHandle, 1, 3, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    CU_ASSERT(sectionIndex == 3);
    CU_ASSERT(elementIndex == 1);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetOuterSectionAndElementIndex with invalid CPACS handle.
*/
void tiglWingGetOuterSectionAndElementIndex_invalidHandle(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(-1, 1, 1, &sectionIndex, &elementIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetOuterSectionAndElementIndex with invalid wing index.
*/
void tiglWingGetOuterSectionAndElementIndex_invalidWing(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 0, 1, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterSectionAndElementIndex with invalid segment index.
*/
void tiglWingGetOuterSectionAndElementIndex_invalidSegment(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 1, 0, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterSectionAndElementIndex with null pointer argument.
*/
void tiglWingGetOuterSectionAndElementIndex_nullPointerArgument(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 1, 1, NULL, &elementIndex) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, NULL) == TIGL_NULL_POINTER);
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 1, 1, NULL, NULL)          == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglWingGetOuterSectionAndElementIndex.
*/
void tiglWingGetOuterSectionAndElementIndex_success(void)
{
    int sectionIndex;
    int elementIndex;
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    CU_ASSERT(sectionIndex == 2);
    CU_ASSERT(elementIndex == 1);
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 1, 2, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    CU_ASSERT(sectionIndex == 3);
    CU_ASSERT(elementIndex == 1);
    CU_ASSERT(tiglWingGetOuterSectionAndElementIndex(tiglHandle, 1, 3, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    CU_ASSERT(sectionIndex == 4);
    CU_ASSERT(elementIndex == 1);
}



/***************************************************************************************************/
/*       Test of tiglWingGet[Inner|Outer]SectionAndElementUID-Functions      */ 
/***************************************************************************************************/

/**
* Tests tiglWingGetInnerSectionAndElementUID with invalid CPACS handle.
*/
void tiglWingGetInnerSectionAndElementUID_invalidHandle(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglWingGetInnerSectionAndElementUID(-1, 1, 1, &sectionUID, &elementUID) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetInnerSectionAndElementUID with invalid wing index.
*/
void tiglWingGetInnerSectionAndElementUID_invalidWing(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglWingGetInnerSectionAndElementUID(tiglHandle, 0, 1, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetInnerSectionAndElementUID with invalid segment index.
*/
void tiglWingGetInnerSectionAndElementUID_invalidSegment(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglWingGetInnerSectionAndElementUID(tiglHandle, 1, 0, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}


/**
* Tests successfull call of tiglWingGetInnerSectionAndElementUID.
*/
void tiglWingGetInnerSectionAndElementUID_success(void)
{
    char* sectionUID;
    char* elementUID;

    CU_ASSERT(tiglWingGetInnerSectionAndElementUID(tiglHandle, 1, 1, &sectionUID, &elementUID) == TIGL_SUCCESS);
	CU_ASSERT(strcmp(sectionUID, "SMY0.") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMY0.") == 0);
	free(sectionUID);
	free(elementUID);
    
	CU_ASSERT(tiglWingGetInnerSectionAndElementUID(tiglHandle, 1, 2, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SMY3.087") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMY3.087") == 0);
	free(sectionUID);
	free(elementUID);

    CU_ASSERT(tiglWingGetInnerSectionAndElementUID(tiglHandle, 1, 3, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SMY7.453") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMY7.453") == 0);
    free(sectionUID);
	free(elementUID);
}

/***************************************************************************************************/

/**
* Tests tiglWingGetOuterSectionAndElementUID with invalid CPACS handle.
*/
void tiglWingGetOuterSectionAndElementUID_invalidHandle(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglWingGetOuterSectionAndElementUID(-1, 1, 1, &sectionUID, &elementUID) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetOuterSectionAndElementUID with invalid wing index.
*/
void tiglWingGetOuterSectionAndElementUID_invalidWing(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglWingGetOuterSectionAndElementUID(tiglHandle, 0, 1, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetOuterSectionAndElementUID with invalid segment index.
*/
void tiglWingGetOuterSectionAndElementUID_invalidSegment(void)
{
    char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglWingGetOuterSectionAndElementUID(tiglHandle, 1, 0, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests successfull call of tiglWingGetOuterSectionAndElementUID.
*/
void tiglWingGetOuterSectionAndElementUID_success(void)
{
	char* sectionUID;
    char* elementUID;
    CU_ASSERT(tiglWingGetOuterSectionAndElementUID(tiglHandle, 1, 1, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SMY3.087") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMY3.087") == 0);
	free(sectionUID);
	free(elementUID);

    CU_ASSERT(tiglWingGetOuterSectionAndElementUID(tiglHandle, 1, 2, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SMY7.453") == 0);
    CU_ASSERT(strcmp(elementUID, "ME_SMY7.453") == 0);
    free(sectionUID);
	free(elementUID);
	
	CU_ASSERT(tiglWingGetOuterSectionAndElementUID(tiglHandle, 1, 3, &sectionUID, &elementUID) == TIGL_SUCCESS);
    CU_ASSERT(strcmp(sectionUID, "SMY10.724") == 0);
	CU_ASSERT(strcmp(elementUID, "ME_SMY10.724") == 0);
	free(sectionUID);
	free(elementUID);
}



