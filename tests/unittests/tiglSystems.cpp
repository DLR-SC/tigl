/*
 * Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
 *
 * Created: 2023-12-29 Marko Alder <marko.alder@dlr.de>
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
  * @brief Tests for testing duct functions.
  */

#include "test.h"
#include "tigl.h"

#include "CCPACSGenericSystem.h"
#include "CCPACSComponents.h"
#include "CCPACSComponent.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"

// #include "CCPACSVessel.h"

class Systems : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-systems.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE(tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "testAircraft", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    tigl::CTiglUIDManager& uidMgr =
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(Systems::tiglHandle).GetUIDManager();

    tigl::CCPACSGenericSystem const* genericSystem = &uidMgr.ResolveObject<tigl::CCPACSGenericSystem>("genSys_1");
    tigl::CCPACSComponent const* rectCube_1        = &uidMgr.ResolveObject<tigl::CCPACSComponent>("rectCube_1");
    tigl::CCPACSComponent const* wedge_1           = &uidMgr.ResolveObject<tigl::CCPACSComponent>("wedge_1");
    tigl::CCPACSComponent const* cylinder_1        = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cylinder_1");
    tigl::CCPACSComponent const* cone_1            = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cone_1");

    // std::cout << "Is UID registered: " << uidMgr->IsUIDRegistered("predefinedElectricMotor") << std::endl;
};

TixiDocumentHandle Systems::tixiHandle           = 0;
TiglCPACSConfigurationHandle Systems::tiglHandle = 0;

void CheckExceptionMessage(std::function<void()> func, const char* expectedMessage)
{
    try {
        func();
        FAIL() << "Expected tigl::CTiglError but no exception was thrown.";
    }
    catch (const tigl::CTiglError& e) {
        EXPECT_STREQ(e.what(), expectedMessage);
    }
    catch (...) {
        FAIL() << "Expected tigl::CTiglError but a different exception was thrown.";
    }
}

TEST_F(Systems, temp)
{
    genericSystem->GetComponents();
    auto& name = rectCube_1->GetName();
    EXPECT_EQ(name, "Component 1 of System 1");

    auto& loft  = rectCube_1->GetLoft();
    auto& loft2 = wedge_1->GetLoft();
    auto& loft3 = cylinder_1->GetLoft();
    auto& loft4 = cone_1->GetLoft();

    EXPECT_EQ(1., 1.);
}
