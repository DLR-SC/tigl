/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-25 Marko Alder <marko.alder@dlr.de>
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

#include "test.h"
#include "tigl.h"

#include <boost/optional/optional_io.hpp>

#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>

#include <TopExp_Explorer.hxx>
#include <TopoDS_Solid.hxx>

#include "CNamedShape.h"
#include "CCPACSGenericSystem.h"
#include "CCPACSComponent.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"
#include "CCPACSExternalObject.h"
#include "CTiglPoint.h"

#include "generated/CPACSElementGeometry.h"
#include "CTiglVehicleElementBuilder.h"
#include "generated/CPACSVehicleElementBase.h"

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
    tigl::CCPACSComponent const* cuboid_1          = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cuboid_1");
    tigl::CCPACSComponent const* wedge_1           = &uidMgr.ResolveObject<tigl::CCPACSComponent>("wedge_1");
    tigl::CCPACSComponent const* cylinder_1        = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cylinder_1");
    tigl::CCPACSComponent const* cone_1            = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cone_1");
    tigl::CCPACSComponent const* external          = &uidMgr.ResolveObject<tigl::CCPACSComponent>("external");

    tigl::CCPACSComponent const* eMotor = &uidMgr.ResolveObject<tigl::CCPACSComponent>("electricMotor");
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

TEST_F(Systems, Basics)
{
    // defaulted UID check
    EXPECT_EQ(genericSystem->GetDefaultedUID(), "genSys_1");
    EXPECT_EQ(cuboid_1->GetDefaultedUID(), "cuboid_1");

    // check components' type and intent
    EXPECT_EQ(cuboid_1->GetComponentType(), TIGL_COMPONENT_SYSTEM_COMPONENT);
    EXPECT_EQ(cuboid_1->GetComponentIntent(), TIGL_INTENT_PHYSICAL);
}

TEST_F(Systems, SystemsGeometry)
{
    PNamedShape shape = genericSystem->GetLoft();
    ASSERT_NE(shape, nullptr);

    // all component shapes should be included in grouped system shape
    unsigned shapeCount = 0;
    for (TopoDS_Iterator it(shape->Shape()); it.More(); it.Next()) {
        ++shapeCount;
    }
    EXPECT_EQ(shapeCount, genericSystem->GetComponents().GetComponents().size());
}

TEST_F(Systems, ComponentsGeometry)
{
    const double eps = 1e-6;

    // cuboid_1 -> predefined cuboid 1x1x1 => expect extents ~1.0 in x,y,z
    {
        PNamedShape shape = cuboid_1->GetLoft();
        ASSERT_NE(shape, nullptr);
        EXPECT_EQ(shape->Name(), "cuboid_1");

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

    // wedge_1 -> must have 6 faces and bounding box extents ~1.0 in x,y,z
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

TEST_F(Systems, Masses)
{
    const double eps = 1e-6;

    // ---- Mass values ----
    EXPECT_NEAR(cuboid_1->GetMass(), 1234.0, eps);

    auto const* cuboid_2 = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cuboid_2");
    auto const* cuboid_3 = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cuboid_3");
    EXPECT_NEAR(cuboid_2->GetMass(), 0.375, eps);
    EXPECT_NEAR(cuboid_2->GetMass(), cuboid_3->GetMass(), eps);

    EXPECT_THROW(wedge_1->GetMass(), tigl::CTiglError);

    EXPECT_NEAR(external->GetMass(), 0.2476386, eps);
    EXPECT_NEAR(eMotor->GetMass(), 123., eps);

    // ---- Mass location ----
    auto const* cuboid_4 = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cuboid_4");

    const auto cogLocal = cuboid_4->GetCenterOfGravityLocal();
    EXPECT_NEAR(cogLocal.x, 0.3, eps);
    EXPECT_NEAR(cogLocal.y, 0.25, eps);
    EXPECT_NEAR(cogLocal.z, 0.4, eps);

    const auto cogGlobal = cuboid_4->GetCenterOfGravityGlobal();
    EXPECT_TRUE(cuboid_4->IsPositioned());
    ASSERT_TRUE(cogGlobal);
    EXPECT_NEAR(cogGlobal->x, -0.0707107, eps);
    EXPECT_NEAR(cogGlobal->y, 15.25, eps);
    EXPECT_NEAR(cogGlobal->z, 0.49497475, eps);

    auto const* unpositionedCuboid = &uidMgr.ResolveObject<tigl::CCPACSComponent>("unpositionedCuboid");
    EXPECT_FALSE(unpositionedCuboid->IsPositioned());
    EXPECT_FALSE(unpositionedCuboid->GetCenterOfGravityGlobal());
}

class InvalidSystems : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-invalid-systems.cpacs.xml";
        ASSERT_EQ(tixiOpenDocument(filename, &tixiHandle), SUCCESS);
        ASSERT_EQ(tiglOpenCPACSConfiguration(tixiHandle, "testAircraft", &tiglHandle), TIGL_SUCCESS);
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
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(InvalidSystems::tiglHandle).GetUIDManager();

    tigl::CCPACSGenericSystem const* testSystem = &uidMgr.ResolveObject<tigl::CCPACSGenericSystem>("testSystem");
};

TEST_F(InvalidSystems, Exceptions)
{

    // Exception for wrong UID reference
    {
        auto const* wrongReference = &uidMgr.ResolveObject<tigl::CCPACSComponent>("wrongUIDReference");
        ASSERT_NE(wrongReference, nullptr);

        CheckExceptionMessage([&] { (void)wrongReference->GetLoft(); },
                              "Unsupported system element for uid \"NACA0009\" in CCPACSComponent::BuildLoft");
    }
}

TEST_F(InvalidSystems, VehicleElementBuilderExceptions)
{
    // Ensures that building a vehicle element fails if no geometry choice is set.
    {
        tigl::CCPACSVehicleElementBase* parent = nullptr;
        tigl::CTiglUIDManager* uidMgr          = nullptr;

        tigl::CCPACSElementGeometry geom(parent, uidMgr);
        tigl::CTiglTransformation transformation;

        tigl::CTiglVehicleElementBuilder builder(geom, transformation, "testShapeName", "");
        EXPECT_THROW((void)builder.BuildShape(), tigl::CTiglError);
    }

    // Check get name from NextUIDParent
    {
        const std::string uID = "predCuboid_1";
        auto const* ve        = &uidMgr.ResolveObject<tigl::CCPACSVehicleElementBase>(uID);
        ASSERT_NE(ve, nullptr);

        const tigl::CCPACSElementGeometry& geom = ve->GetGeometry();

        tigl::CTiglTransformation transformation;
        tigl::CTiglVehicleElementBuilder builder(geom, transformation, "", "");

        PNamedShape loft = builder.BuildShape();
        ASSERT_TRUE(loft != nullptr);

        EXPECT_EQ(uID, loft->Name());
    }
}

TEST_F(InvalidSystems, InvalidShapes)
{
    {
        auto const* invalidShape = &uidMgr.ResolveObject<tigl::CCPACSComponent>("invalidCuboid");
        CheckExceptionMessage(
            [&] { (void)invalidShape->GetLoft(); },
            "Invalid cuboid parameters for uID=\"invalidPredCuboid\": lengthX, depthY and heightZ must be positive.");
    }

    {
        auto const* invalidShape = &uidMgr.ResolveObject<tigl::CCPACSComponent>("invalidCylinder");
        CheckExceptionMessage(
            [&] { (void)invalidShape->GetLoft(); },
            "Invalid cylinder parameters for uID=\"invalidPredCylinder\": Radius and height must be positive.");
    }

    {
        auto const* invalidShape = &uidMgr.ResolveObject<tigl::CCPACSComponent>("invalidCone");
        CheckExceptionMessage([&] { (void)invalidShape->GetLoft(); },
                              "Invalid cone parameters for uID=\"invalidPredCone\": At least one radius must be "
                              "positive and height must be positive.");
    }

    {
        auto const* invalidShape = &uidMgr.ResolveObject<tigl::CCPACSComponent>("invalidEllipsoid");
        CheckExceptionMessage([&] { (void)invalidShape->GetLoft(); },
                              "Invalid ellipsoid parameters: All radii must be positive.");
    }

    {
        auto const* invalidShape = &uidMgr.ResolveObject<tigl::CCPACSComponent>("zeroVolumeEllipsoid");
        CheckExceptionMessage([&] { (void)invalidShape->GetLoft(); },
                              "Invalid ellipsoid diskAngle: must be in range (0, 2*pi].");
    }

    // It's ok to build a cone as cylinder (ToDo: checking the warning would be nice)
    {
        auto const* cylinder = &uidMgr.ResolveObject<tigl::CCPACSComponent>("cylinderCone");
        ASSERT_NE(cylinder->GetLoft(), nullptr);
    }

    {
        auto const* face = &uidMgr.ResolveObject<tigl::CCPACSComponent>("zeroHeightComponent");
        CheckExceptionMessage([&] { (void)face->GetMass(); },
                              "Cannot compute geometric center (zero volume) for uid \"predFace\".");
    }
}

TEST_F(InvalidSystems, InvalidFileHandling)
{
    {
        auto const* invalidShape = &uidMgr.ResolveObject<tigl::CCPACSComponent>("invalidExternal1");
        CheckExceptionMessage([&] { (void)invalidShape->GetLoft(); },
                              "Cannot open external file. No file format specified.");
    }

    {
        auto const* invalidShape = &uidMgr.ResolveObject<tigl::CCPACSComponent>("invalidExternal2");
        CheckExceptionMessage([&] { (void)invalidShape->GetLoft(); },
                              "Cannot open external element. Unknown file format: Stl");
    }
}

TEST_F(InvalidSystems, Masses)
{
    auto const* cuboid = &uidMgr.ResolveObject<tigl::CCPACSComponent>("invalidMass");
    CheckExceptionMessage([&] { (void)cuboid->GetMass(); },
                          "Invalid mass definition (no mass and no density) for uid \"invalidPredCuboid_2\".");
}

TixiDocumentHandle InvalidSystems::tixiHandle           = 0;
TiglCPACSConfigurationHandle InvalidSystems::tiglHandle = 0;
