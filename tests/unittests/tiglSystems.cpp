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

#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>

#include "CNamedShape.h" 
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
    tigl::CCPACSComponent const* external          = &uidMgr.ResolveObject<tigl::CCPACSComponent>("external");

    tigl::CCPACSComponent const* eMotor = &uidMgr.ResolveObject<tigl::CCPACSComponent>("electricMotor");

    tigl::CCPACSComponent const* wrongReference = &uidMgr.ResolveObject<tigl::CCPACSComponent>("wrongUIDReference");

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

TEST_F(Systems, ComponentsGeometry)
{
    const double eps = 1e-6;

    // rectCube_1 -> predefined cuboid 1x1x1 => expect extents ~1.0 in x,y,z
    {
        PNamedShape shape = rectCube_1->GetLoft();
        ASSERT_NE(shape, nullptr);
        EXPECT_EQ(shape->Name(), "rectCube_1");

        unsigned int faces = shape->GetFaceCount();
        EXPECT_EQ(faces, 6u) << "Cuboid should have 6 faces";

        Bnd_Box box;
        BRepBndLib::Add(shape->Shape(), box);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        EXPECT_NEAR(xmax - xmin, 1.0, eps);
        EXPECT_NEAR(ymax - ymin, 1.0, eps);
        EXPECT_NEAR(zmax - zmin, 1.0, eps);
    }

    // wedge_1 -> at least some faces and non-zero bounding box
    {
        PNamedShape shape = wedge_1->GetLoft();
        ASSERT_NE(shape, nullptr) << "wedge_1 produced a null shape";
        EXPECT_EQ(shape->GetFaceCount(), 6u);

        Bnd_Box box;
        BRepBndLib::Add(shape->Shape(), box);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        EXPECT_NEAR(xmax - xmin, 1.0, eps);
        EXPECT_NEAR(ymax - ymin, 1.0, eps);
        EXPECT_NEAR(zmax - zmin, 1.0, eps);
    }

    // electric motor (specialized element) -> ensure builder supports it
    {
        PNamedShape shape = eMotor->GetLoft();
        ASSERT_NE(shape, nullptr) << "electricMotor produced a null shape";
        EXPECT_EQ(shape->GetFaceCount(), 1u);
    }

    // external geometry (STEP) -> ensure import happened
    {
        PNamedShape shape = external->GetLoft();
        ASSERT_NE(shape, nullptr) << "external produced a null shape (STEP import failed)";
        EXPECT_GT(shape->GetFaceCount(), 0u);
    }
}
