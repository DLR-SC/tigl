/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-11 Jan Kleinert <Jan.Kleinert@dlr.de>
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
#include "CTiglUIDManager.h"
#include "CTiglEngineNacelleBuilder.h"
#include "generated/CPACSEngine.h"

/******************************************************************************/

class EngineNacelleBuilder : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/simpletest-pylon-nacelle.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ(tixiRet, SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "SimpleTest", &tiglHandle);
        ASSERT_EQ(tiglRet, TIGL_SUCCESS);

        uidMgr = &tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    }

    void TearDown() override
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    tigl::CTiglUIDManager*       uidMgr;
};

class EngineNacelleBuilderSimple : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* filename = "TestData/simpletest-simplenacelle.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ(tixiRet, SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "SimpleTest", &tiglHandle);
        ASSERT_EQ(tiglRet, TIGL_SUCCESS);

        uidMgr = &tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    }

    void TearDown() override
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    tigl::CTiglUIDManager*       uidMgr;
};


/******************************************************************************/


TEST_F(EngineNacelleBuilder, integrationTest)
{
    tigl::CCPACSConfiguration& config = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    tigl::CCPACSEngine& engine = config.GetEngine("SimpleEngine");
    boost::optional<tigl::CCPACSEngineNacelle>& nacelle = engine.GetNacelle();
    tigl::CTiglEngineNacelleBuilder builder(*nacelle);
    PNamedShape shape = builder.BuildShape();
}

TEST_F(EngineNacelleBuilderSimple, integrationTest)
{
    // check if a nacelle with just one profile can be built
    tigl::CCPACSConfiguration& config = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    tigl::CCPACSEngine& engine = config.GetEngine("SimpleEngine");
    boost::optional<tigl::CCPACSEngineNacelle>& nacelle = engine.GetNacelle();
    tigl::CTiglEngineNacelleBuilder builder(*nacelle);
    PNamedShape shape = builder.BuildShape();
}


