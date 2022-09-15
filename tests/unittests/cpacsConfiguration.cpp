/* 
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
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
* @brief Tests for testing behavior of the configuration handle management and implementation.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include "tigl_version.h"

/******************************************************************************/

class tiglOpenCpacsConfiguration : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/CPACS_30_D150.xml";
        ReturnCode tixiRet;

        tiglHandle = -1;
        tixiHandle = -1;
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE( tixiRet == SUCCESS);
    }

    void TearDown() override
    {
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};




/**
* Tests tiglOpenCPACSConfiguration with null pointer arguments.
*/
TEST_F(tiglOpenCpacsConfiguration, nullPointerArgument) 
{
    ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", NULL) == TIGL_NULL_POINTER);
}

/**
* Tests a successful run of tiglOpenCPACSConfiguration.
*/
TEST_F(tiglOpenCpacsConfiguration, openSuccess) 
{
    ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle) == TIGL_SUCCESS);
    ASSERT_TRUE(tiglHandle > 0);
    ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
}

/**
* Tests a successful open of tiglOpenCPACSConfiguration with specifiing the uid of the configuration.
*/
TEST_F(tiglOpenCpacsConfiguration, open_without_uid) 
{
    // Test with NULL argument
    ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, NULL, &tiglHandle) == TIGL_SUCCESS);
    ASSERT_TRUE(tiglHandle > 0);
    ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);

    // Test with empty string argument
    ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle) == TIGL_SUCCESS);
    ASSERT_TRUE(tiglHandle > 0);
    ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
}


/******************************************************************************/


class TiglGetCPACSTixiHandle : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/CPACS_30_D150.xml";
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        ASSERT_TRUE(tixiOpenDocument(filename, &tixiHandle) == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE (tiglRet == TIGL_SUCCESS );
    }

    void TearDown() override
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle)== TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle)== SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};


/**
* Tests tiglGetCPACSTixiHandle with null pointer arguments.
*/
TEST_F(TiglGetCPACSTixiHandle, nullPointerArgument)
{
    ASSERT_TRUE(tiglGetCPACSTixiHandle(tiglHandle, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests tiglGetCPACSTixiHandle with an invalid tigl handle.
*/
TEST_F(TiglGetCPACSTixiHandle, hanlde_notFound)
{
    ASSERT_TRUE(tiglGetCPACSTixiHandle(tiglHandle + 1, &tixiHandle) == TIGL_NOT_FOUND);
}

/**
* Tests a successful run of tiglGetCPACSTixiHandle.
*/
TEST_F(TiglGetCPACSTixiHandle, handle_success)
{
    TixiDocumentHandle tempHandle;
    ASSERT_TRUE(tiglGetCPACSTixiHandle(tiglHandle, &tempHandle) == TIGL_SUCCESS);
}

/******************************************************************************/

class tiglCPACSConfigurationHandleValid : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/CPACS_30_D150.xml";
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        ASSERT_TRUE( tixiOpenDocument(filename, &tixiHandle) == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    void TearDown() override
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};

/**
* Tests tiglIsCPACSConfigurationHandleValid with an invalid CPACS handle.
*/
TEST_F(tiglCPACSConfigurationHandleValid, invalidHandle)
{
    TiglBoolean isValid;
    ASSERT_TRUE(tiglIsCPACSConfigurationHandleValid(-1, &isValid) == TIGL_SUCCESS);
    ASSERT_TRUE(isValid == TIGL_FALSE);
}

/**
* Tests tiglIsCPACSConfigurationHandleValid with a valid CPACS handle.
*/
TEST_F(tiglCPACSConfigurationHandleValid,validHandle)
{
    TiglBoolean isValid;
    ASSERT_TRUE(tiglIsCPACSConfigurationHandleValid (tiglHandle, &isValid) == TIGL_SUCCESS);
    ASSERT_TRUE(isValid == TIGL_TRUE);
}


/**
* Tests tiglGetVersion.
*/
TEST_F(tiglCPACSConfigurationHandleValid, version_valid)
{
    ASSERT_EQ(0, std::string(tiglGetVersion()).find(TIGL_VERSION_STRING));
}


class SimpleConfigurationTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        ASSERT_TRUE( tixiOpenDocument(filename, &tixiHandle) == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    void TearDown() override
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};


TEST_F(SimpleConfigurationTests, GetAircraftLength)
{
    double len = 0.;
    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationGetLength(tiglHandle, &len));

    EXPECT_NEAR(2., len, 1e-1);
}

TEST_F(SimpleConfigurationTests, GetBoundingBox)
{
    double minX, minY, minZ, maxX, maxY, maxZ;
    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationGetBoundingBox(tiglHandle, &minX, &minY, &minZ, &maxX, &maxY, &maxZ));

    EXPECT_NEAR(-0.5, minX, 1e-1);
    EXPECT_NEAR( 1.5, maxX, 1e-1);

    EXPECT_NEAR( -2., minY, 1e-1);
    EXPECT_NEAR(  2., maxY, 1e-1);

    EXPECT_NEAR(-0.5, minZ, 1e-1);
    EXPECT_NEAR( 0.5, maxZ, 1e-1);
}

TEST_F(SimpleConfigurationTests, GetComponentType)
{
    TiglGeometricComponentType type;
    EXPECT_EQ(TIGL_SUCCESS, tiglComponentGetType(tiglHandle, "Wing", &type));
    EXPECT_EQ(TIGL_COMPONENT_WING, type);

    EXPECT_EQ(TIGL_SUCCESS, tiglComponentGetType(tiglHandle, "Cpacs2Test_Wing_Seg_1_2", &type));
    EXPECT_EQ(TIGL_COMPONENT_WINGSEGMENT, type);

    EXPECT_EQ(TIGL_SUCCESS, tiglComponentGetType(tiglHandle, "WING_CS1", &type));
    EXPECT_EQ(TIGL_COMPONENT_WINGCOMPSEGMENT, type);

    EXPECT_EQ(TIGL_SUCCESS, tiglComponentGetType(tiglHandle, "SimpleFuselage", &type));
    EXPECT_EQ(TIGL_COMPONENT_FUSELAGE, type);

    EXPECT_EQ(TIGL_SUCCESS, tiglComponentGetType(tiglHandle, "segmentD150_Fuselage_1Segment2ID", &type));
    EXPECT_EQ(TIGL_COMPONENT_FUSELSEGMENT, type);

    // test error codes
    EXPECT_EQ(TIGL_UID_ERROR, tiglComponentGetType(tiglHandle, "this_uid_does_not_exist", &type));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglComponentGetType(tiglHandle, nullptr, &type));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglComponentGetType(tiglHandle, "SimpleFuselage", nullptr));
    EXPECT_EQ(TIGL_NOT_FOUND, tiglComponentGetType(-1, "SimpleFuselage", &type));
}
