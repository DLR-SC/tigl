/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-11-08 Jan Kleinert <jan.kleinert@dlr.de>
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
* @brief Tests for rotor functions.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include "CCPACSConfigurationManager.h"
#include "CNamedShape.h"
#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"

/******************************************************************************/

// returns the number of faces for a geometric component with given uid
int GetNumFaces(TiglCPACSConfigurationHandle const& handle, std::string const& uid){
    const tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(handle);
    tigl::ITiglGeometricComponent& component = config.GetUIDManager().GetGeometricComponent(uid);
    TopoDS_Shape loft = component.GetLoft()->Shape();
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(loft, TopAbs_FACE, map);
    return map.Extent();
}

/******************************************************************************/

class fuselageWallEx1 : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/test_fuselage_walls_ex1.xml";
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

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle fuselageWallEx1::tixiHandle = 0;
TiglCPACSConfigurationHandle fuselageWallEx1::tiglHandle = 0;

/******************************************************************************/

class fuselageWallEx2 : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/test_fuselage_walls_ex2.xml";
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

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle fuselageWallEx2::tixiHandle = 0;
TiglCPACSConfigurationHandle fuselageWallEx2::tiglHandle = 0;

/******************************************************************************/

class fuselageWallLabyrinth : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/test_fuselage_walls_ex3_labyrinth.xml";
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

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle fuselageWallLabyrinth::tixiHandle = 0;
TiglCPACSConfigurationHandle fuselageWallLabyrinth::tiglHandle = 0;

/******************************************************************************/

class fuselageWallCompartment : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/test_fuselage_walls_ex4_compartment.xml";
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

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle fuselageWallCompartment::tixiHandle = 0;
TiglCPACSConfigurationHandle fuselageWallCompartment::tiglHandle = 0;

/******************************************************************************/

TEST_F(fuselageWallEx1, numberOfFaces)
{
    ASSERT_EQ(2, GetNumFaces(tiglHandle, "Wall1"));
    ASSERT_EQ(1, GetNumFaces(tiglHandle, "Wall2"));
}

TEST_F(fuselageWallEx2, numberOfFaces)
{
    ASSERT_EQ(1, GetNumFaces(tiglHandle, "Wall1"));
    ASSERT_EQ(1, GetNumFaces(tiglHandle, "Wall2"));
    ASSERT_EQ(1, GetNumFaces(tiglHandle, "Wall3"));
}

TEST_F(fuselageWallLabyrinth, numberOfFaces)
{
    ASSERT_EQ(5, GetNumFaces(tiglHandle, "Wall1"));
    ASSERT_EQ(5, GetNumFaces(tiglHandle, "Wall2"));
    ASSERT_EQ(9, GetNumFaces(tiglHandle, "Wall3"));
    ASSERT_EQ(9, GetNumFaces(tiglHandle, "Wall4"));
}

TEST_F(fuselageWallCompartment, numberOfFaces)
{
    ASSERT_EQ(1, GetNumFaces(tiglHandle, "Wall1"));
    ASSERT_EQ(6, GetNumFaces(tiglHandle, "Wall2"));
}
