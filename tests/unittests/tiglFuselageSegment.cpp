/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#define _USE_MATH_DEFINES
#include "math.h"


/***************************************************************************************************/

class  TiglFuselageSegment : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/CPACS_30_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};


TixiDocumentHandle  TiglFuselageSegment::tixiHandle = 0;
TiglCPACSConfigurationHandle  TiglFuselageSegment::tiglHandle = 0;


/***************************************************************************************************/

class  TiglFuselageSegmentSimple : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};


TixiDocumentHandle  TiglFuselageSegmentSimple::tixiHandle = 0;
TiglCPACSConfigurationHandle  TiglFuselageSegmentSimple::tiglHandle = 0;


/***************************************************************************************************/

/**
* Tests tiglGetFuselageCount with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageCount_invalidHandle)
{
    int fuselageCount;
    ASSERT_TRUE(tiglGetFuselageCount(-1, &fuselageCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglGetFuselageCount with null pointer argument.
*/
TEST_F( TiglFuselageSegment, FuselageCount_nullPointerArgument)
{
    ASSERT_TRUE(tiglGetFuselageCount(tiglHandle, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglGetFuselageCount.
*/
TEST_F( TiglFuselageSegment, FuselageCount_success)
{
    int fuselageCount;
    ASSERT_TRUE(tiglGetFuselageCount(tiglHandle, &fuselageCount) == TIGL_SUCCESS);
    ASSERT_TRUE(fuselageCount == 1);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetSegmentCount with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageSegmentCount_invalidHandle)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetSegmentCount(-1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetSegmentCount with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageSegmentCount_invalidFuselage)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetSegmentCount(tiglHandle, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetSegmentCount with null pointer argument for segmentCountPtr.
*/
TEST_F( TiglFuselageSegment, FuselageSegmentCount_nullPointerArgument)
{
    ASSERT_TRUE(tiglFuselageGetSegmentCount(tiglHandle, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetSegmentCount.
*/
TEST_F( TiglFuselageSegment, FuselageSegmentCount_success)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetSegmentCount(tiglHandle, 1, &segmentCount) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentCount == 59);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentCount_invalidHandle)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentCount(-1, 1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentCount_invalidFuselage)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 0, 1, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentCount_invalidSegment)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentCount with null pointer argument.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentCount_nullPointerArgument)
{
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetStartConnectedSegmentCount.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentCount_success)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 1, &segmentCount) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentCount == 0);
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentCount(tiglHandle, 1, 2, &segmentCount) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentCount == 1);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentCount_invalidHandle)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentCount(-1, 1, 1, &segmentCount) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentCount_invalidFuselage)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 0, 1, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentCount_invalidSegment)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 0, &segmentCount) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentCount with null pointer argument.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentCount_nullPointerArgument)
{
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetEndConnectedSegmentCount.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentCount_success)
{
    int segmentCount;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 1, &segmentCount) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentCount == 1);
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentCount(tiglHandle, 1, 59, &segmentCount) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentCount == 0);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentIndex_invalidHandle)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(-1, 1, 2, 1, &connectedSegment) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentIndex_invalidFuselage)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 0, 2, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentIndex_invalidSegment)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 0, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with invalid segment n-parameter.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentIndex_invalidN)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 1, 1, &connectedSegment) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 1, 0, &connectedSegment) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 2, 2, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartConnectedSegmentIndex with null pointer argument.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentIndex_nullPointerArgument)
{
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 2, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetStartConnectedSegmentIndex.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartConnectedSegmentIndex_success)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 2, 1, &connectedSegment) == TIGL_SUCCESS);
    ASSERT_TRUE(connectedSegment == 1);
    ASSERT_TRUE(tiglFuselageGetStartConnectedSegmentIndex(tiglHandle, 1, 3, 1, &connectedSegment) == TIGL_SUCCESS);
    ASSERT_TRUE(connectedSegment == 2);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentIndex_invalidHandle)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(-1, 1, 1, 1, &connectedSegment) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentIndex_invalidFuselage)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 0, 1, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentIndex_invalidSegment)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 0, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with invalid segment n-parameter.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentIndex_invalidN)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 2, &connectedSegment) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 0, &connectedSegment) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 59, 1, &connectedSegment) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndConnectedSegmentIndex with null pointer argument.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentIndex_nullPointerArgument)
{
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 1, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetEndConnectedSegmentIndex.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndConnectedSegmentIndex_success)
{
    int connectedSegment;
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 1, 1, &connectedSegment) == TIGL_SUCCESS);
    ASSERT_TRUE(connectedSegment == 2);
    ASSERT_TRUE(tiglFuselageGetEndConnectedSegmentIndex(tiglHandle, 1, 2, 1, &connectedSegment) == TIGL_SUCCESS);
    ASSERT_TRUE(connectedSegment == 3);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementIndex_invalidHandle)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(-1, 1, 1, &sectionIndex, &elementIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementIndex_invalidFuselage)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 0, 1, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementIndex_invalidSegment)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 0, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartSectionAndElementIndex with null pointer argument.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementIndex_nullPointerArgument)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, NULL, &elementIndex) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, NULL) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, NULL, NULL)          == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetStartSectionAndElementIndex.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementIndex_success)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(sectionIndex == 1);
    ASSERT_TRUE(elementIndex == 1);
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 2, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(sectionIndex == 2);
    ASSERT_TRUE(elementIndex == 1);
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementIndex(tiglHandle, 1, 3, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(sectionIndex == 3);
    ASSERT_TRUE(elementIndex == 1);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementIndex_invalidHandle)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(-1, 1, 1, &sectionIndex, &elementIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementIndex_invalidFuselage)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 0, 1, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementIndex_invalidSegment)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 0, &sectionIndex, &elementIndex) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndSectionAndElementIndex with null pointer argument.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementIndex_nullPointerArgument)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, NULL, &elementIndex) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, NULL) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, NULL, NULL)          == TIGL_NULL_POINTER);
}

/**
* Tests successfull call of tiglFuselageGetEndSectionAndElementIndex.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementIndex_success)
{
    int sectionIndex;
    int elementIndex;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 1, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(sectionIndex == 2);
    ASSERT_TRUE(elementIndex == 1);
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 2, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(sectionIndex == 3);
    ASSERT_TRUE(elementIndex == 1);
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementIndex(tiglHandle, 1, 3, &sectionIndex, &elementIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(sectionIndex == 4);
    ASSERT_TRUE(elementIndex == 1);
}




/***************************************************************************************************/
/*       Test of tiglFuselageGet[Inner|Outer]SectionAndElementUID-Functions      */ 
/***************************************************************************************************/

/**
* Tests tiglFuselageGetStartSectionAndElementUID with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementUID_invalidHandle)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementUID(-1, 1, 1, &sectionUID, &elementUID) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetStartSectionAndElementUID with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementUID_invalidFuselage)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 0, 1, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetStartSectionAndElementUID with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementUID_invalidSegment)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 0, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}


/**
* Tests successfull call of tiglFuselageGetStartSectionAndElementUID.
*/
TEST_F( TiglFuselageSegment, FuselageGetStartSectionAndElementUID_success)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 1, &sectionUID, &elementUID) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(sectionUID, "D150_VAMP_FL1_Sec1") == 0);
    ASSERT_TRUE(strcmp(elementUID, "D150_VAMP_FL1_Sec1_Elem1") == 0);

    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 2, &sectionUID, &elementUID) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(sectionUID, "D150_VAMP_FL1_Sec2") == 0);
    ASSERT_TRUE(strcmp(elementUID, "D150_VAMP_FL1_Sec2_Elem1") == 0);

    ASSERT_TRUE(tiglFuselageGetStartSectionAndElementUID(tiglHandle, 1, 3, &sectionUID, &elementUID) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(sectionUID, "D150_VAMP_FL1_Sec3") == 0);
    ASSERT_TRUE(strcmp(elementUID, "D150_VAMP_FL1_Sec3_Elem1") == 0);
}

/***************************************************************************************************/

/**
* Tests tiglFuselageGetEndSectionAndElementUID with invalid CPACS handle.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementUID_invalidHandle)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementUID(-1, 1, 1, &sectionUID, &elementUID) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetEndSectionAndElementUID with invalid fuselage index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementUID_invalidFuselage)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 0, 1, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetEndSectionAndElementUID with invalid segment index.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementUID_invalidSegment)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 0, &sectionUID, &elementUID) == TIGL_INDEX_ERROR);
}

/**
* Tests successfull call of tiglFuselageGetEndSectionAndElementUID.
*/
TEST_F( TiglFuselageSegment, FuselageGetEndSectionAndElementUID_success)
{
    char* sectionUID;
    char* elementUID;
    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 1, &sectionUID, &elementUID) == TIGL_SUCCESS);
    ASSERT_STREQ("D150_VAMP_FL1_Sec2", sectionUID);
    ASSERT_STREQ("D150_VAMP_FL1_Sec2_Elem1",elementUID);

    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 2, &sectionUID, &elementUID) == TIGL_SUCCESS);
    ASSERT_STREQ("D150_VAMP_FL1_Sec3", sectionUID);
    ASSERT_STREQ("D150_VAMP_FL1_Sec3_Elem1", elementUID);

    ASSERT_TRUE(tiglFuselageGetEndSectionAndElementUID(tiglHandle, 1, 3, &sectionUID, &elementUID) == TIGL_SUCCESS);
    ASSERT_STREQ("D150_VAMP_FL1_Sec4", sectionUID);
    ASSERT_STREQ("D150_VAMP_FL1_Sec4_Elem1", elementUID);
}

TEST_F(TiglFuselageSegment, GetSegmentVolume)
{
    double volume = 0.;
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSegmentVolume(tiglHandle, 1, 10, &volume));
    ASSERT_GT(volume, 0.);
}

/***************************************************************************************************/

TEST_F(TiglFuselageSegmentSimple, getSectionCenter)
{
    // test first fuselage segment
    double eta = 0.;

    double pointX = 0;
    double pointY = 0;
    double pointZ = 0;

    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, &pointZ));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, &pointZ));
    EXPECT_NEAR(-0.5, pointX, 1e-15);
    EXPECT_NEAR(0, pointY, 1e-6);
    EXPECT_NEAR(0, pointZ, 1e-6);

    eta = 0.5;

    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, &pointZ));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, &pointZ));
    EXPECT_NEAR(0, pointX, 1e-15);
    EXPECT_NEAR(0, pointY, 1e-2);
    EXPECT_NEAR(0, pointZ, 1e-2);

    eta = 1;

    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, &pointZ));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, &pointZ));
    EXPECT_NEAR(0.5, pointX, 1e-15);
    EXPECT_NEAR(0, pointY, 1e-2);
    EXPECT_NEAR(0, pointZ, 1e-2);

    // test second fuselage segment
    eta = 0.;

    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &pointX, &pointY, &pointZ));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &pointX, &pointY, &pointZ));
    EXPECT_NEAR(0.5, pointX, 1e-15);
    EXPECT_NEAR(0, pointY, 1e-2);
    EXPECT_NEAR(0, pointZ, 1e-2);

    eta = 0.5;

    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &pointX, &pointY, &pointZ));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &pointX, &pointY, &pointZ));
    EXPECT_NEAR(1, pointX, 1e-15);
    EXPECT_NEAR(0, pointY, 1e-2);
    EXPECT_NEAR(0, pointZ, 1e-2);

    eta = 1;

    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &pointX, &pointY, &pointZ));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &pointX, &pointY, &pointZ));
    EXPECT_NEAR(1.5, pointX, 1e-15);
    EXPECT_NEAR(0, pointY, 1e-2);
    EXPECT_NEAR(0, pointZ, 1e-2);

    // some other tests: make sure that right error codes are returned in case of the corresponding errors
    ASSERT_EQ(TIGL_UID_ERROR, tiglFuselageGetSectionCenter(tiglHandle, "invalidUID", eta, &pointX, &pointY, &pointZ));

    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, NULL, &pointY, &pointZ));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, NULL, &pointZ));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, NULL));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, NULL, NULL, &pointZ));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, NULL, NULL));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, NULL, NULL, NULL));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetSectionCenter(tiglHandle, NULL, eta, &pointX, &pointY, &pointZ));

    ASSERT_EQ(TIGL_NOT_FOUND, tiglFuselageGetSectionCenter(-1, "segmentD150_Fuselage_1Segment2ID", eta, &pointX, &pointY, &pointZ));

    double eta_out_of_range = -0.5;
    ASSERT_EQ(TIGL_MATH_ERROR, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta_out_of_range, &pointX, &pointY, &pointZ));
    eta_out_of_range = 2;
    ASSERT_EQ(TIGL_MATH_ERROR, tiglFuselageGetSectionCenter(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta_out_of_range, &pointX, &pointY, &pointZ));
}

TEST_F(TiglFuselageSegmentSimple, getSectionArea)
{
    // test first fuselage segment
    double eta = 0.;

    double area = 0.;

    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &area));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &area));
    EXPECT_NEAR(M_PI * 0.5 * 0.5, area, 3e-2);

    eta = 0.5;
    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &area));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &area));
    EXPECT_NEAR(M_PI * 0.5 * 0.5, area, 3e-2);

    eta = 1.;
    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &area));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, &area));
    EXPECT_NEAR(M_PI * 0.5 * 0.5, area, 3e-2);

    // test second fuselage segment
    eta = 0.;
    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &area));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &area));
    EXPECT_NEAR(M_PI * 0.5 * 0.5, area, 3e-2);

    eta = 0.5;
    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &area));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &area));
    EXPECT_NEAR(M_PI * 0.5 * 0.5, area, 3e-2);

    eta = 1.;
    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &area));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment3ID", eta, &area));
    EXPECT_NEAR(M_PI * 0.5 * 0.5, area, 3e-2);

    // some other tests: make sure that right error codes are returned in case of the corresponding errors
    ASSERT_EQ(TIGL_UID_ERROR, tiglFuselageGetCrossSectionArea(tiglHandle, "invalidUID", eta, &area));

    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta, NULL));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetCrossSectionArea(tiglHandle, NULL, eta, &area));

    ASSERT_EQ(TIGL_NOT_FOUND, tiglFuselageGetCrossSectionArea(-1, "segmentD150_Fuselage_1Segment2ID", eta, &area));

    double eta_out_of_range = -0.5;
    ASSERT_EQ(TIGL_MATH_ERROR, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta_out_of_range, &area));
    eta_out_of_range = 2;
    ASSERT_EQ(TIGL_MATH_ERROR, tiglFuselageGetCrossSectionArea(tiglHandle, "segmentD150_Fuselage_1Segment2ID", eta_out_of_range, &area));
}

TEST_F(TiglFuselageSegmentSimple, getCenterLineLength)
{
    double centerLineLength = 0.;

    // test value of the result
    ASSERT_NE(TIGL_NULL_POINTER, tiglFuselageGetCenterLineLength(tiglHandle, "SimpleFuselage", &centerLineLength));
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetCenterLineLength(tiglHandle, "SimpleFuselage", &centerLineLength));
    EXPECT_NEAR(centerLineLength, 2., 1e-15);

    // some other tests: make sure that right error codes are returned in case of the corresponding errors
    ASSERT_EQ(TIGL_UID_ERROR, tiglFuselageGetCenterLineLength(tiglHandle, "invalidUID", &centerLineLength));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetCenterLineLength(tiglHandle, "SimpleFuselage", NULL));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglFuselageGetCenterLineLength(tiglHandle, NULL, &centerLineLength));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglFuselageGetCenterLineLength(-1, "SimpleFuselage", &centerLineLength));
}

