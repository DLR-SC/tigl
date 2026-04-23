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
#include "generated/CPACSComponents.h"
#include "CCPACSComponent.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"
#include "CCPACSExternalObject.h"
#include "CTiglPoint.h"

#include "generated/CPACSSystemArchitectures.h"
#include "CCPACSSystemArchitecture.h"
#include "CCPACSSystemConnection.h"
#include "generated/CPACSSourceTarget_externalElement.h"

#include "generated/CPACSElementGeometry.h"
#include "generated/CPACSVehicleElementBase.h"

class Systems : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-systems.cpacs.xml";
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

    const tigl::CTiglUIDManager& GetUIDManager() const
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    }

    const tigl::CCPACSConfiguration& GetConfig() const
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    }

    tigl::CCPACSConfiguration& GetConfig()
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    }

    const tigl::CCPACSGenericSystem& GetSystem(const std::string& uid) const
    {
        return GetUIDManager().ResolveObject<tigl::CCPACSGenericSystem>(uid);
    }

    const tigl::CCPACSComponent& GetComponent(const std::string& uid) const
    {
        return GetUIDManager().ResolveObject<tigl::CCPACSComponent>(uid);
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle Systems::tixiHandle           = 0;
TiglCPACSConfigurationHandle Systems::tiglHandle = 0;

void CheckExceptionMessage(const std::function<void()>& func, const char* expectedMessage)
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
    const auto& system    = GetSystem("genSys_1");
    const auto& component = GetComponent("cuboid_1");

    // defaulted UID check
    EXPECT_EQ(system.GetDefaultedUID(), "genSys_1");
    EXPECT_EQ(component.GetDefaultedUID(), "cuboid_1");

    // check system's type and intent
    EXPECT_EQ(system.GetComponentType(), TIGL_COMPONENT_GENERICSYSTEM);
    EXPECT_EQ(system.GetComponentType(), 12);
    EXPECT_EQ(system.GetComponentIntent(), TIGL_INTENT_PHYSICAL);
    EXPECT_EQ(system.GetComponentIntent(), 1);

    // check components' type and intent
    EXPECT_EQ(component.GetComponentType(), TIGL_COMPONENT_SYSTEM_COMPONENT);
    EXPECT_EQ(component.GetComponentType(), 29);
    EXPECT_EQ(component.GetComponentIntent(), TIGL_INTENT_PHYSICAL);
    EXPECT_EQ(component.GetComponentIntent(), 1);

    // available in configuration
    EXPECT_EQ(system.GetConfiguration().GetUID(), "testAircraft");
}

TEST_F(Systems, SystemsGeometry)
{
    const auto& system = GetSystem("genSys_1");

    const PNamedShape shape = system.GetLoft();
    ASSERT_TRUE(shape);

    // all component shapes should be included in grouped system shape
    unsigned shapeCount = 0;
    for (TopoDS_Iterator it(shape->Shape()); it.More(); it.Next()) {
        ++shapeCount;
    }
    EXPECT_EQ(shapeCount, system.GetComponents().GetComponents().size());
}

TEST_F(Systems, SystemMass)
{
    const auto& system = GetSystem("genSys_1");
    const double eps   = 1e-6;

    // ---- Mass values ----
    const double mAll = system.GetMassAllComponents();
    const double mPos = system.GetMassPositionedComponents();

    EXPECT_NEAR(mAll, 2.8995165, eps);
    EXPECT_NEAR(mPos, 2.7761165, eps);

    // ---- Mass location ----
    const auto cog = system.GetCenterOfGravity();
    ASSERT_TRUE(cog);

    EXPECT_NEAR(cog->x, 16.4246386, eps);
    EXPECT_NEAR(cog->y, 7.0952247, eps);
    EXPECT_NEAR(cog->z, 0.2864855, eps);
}

TEST_F(Systems, ComponentsGeometry)
{
    const double eps = 1e-6;

    // cuboid_1 -> predefined cuboid 1x1x1 => expect extents ~1.0 in x,y,z
    {
        const auto& component = GetComponent("cuboid_1");
        PNamedShape shape     = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->Name(), "cuboid_1_cuboid_1");

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
        const auto& component = GetComponent("wedge_1");
        PNamedShape shape     = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->GetFaceCount(), 6u);

        Bnd_Box box;
        BRepBndLib::Add(shape->Shape(), box);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        EXPECT_NEAR(xmax - xmin, 1.0, eps);
        EXPECT_NEAR(ymax - ymin, 1.2, eps);
        EXPECT_NEAR(zmax - zmin, 1.0, eps);
    }

    // electric motor (specialized element) -> ensure builder supports it
    {
        const auto& component = GetComponent("electricMotor");
        PNamedShape shape     = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->GetFaceCount(), 1u);
    }

    // external geometry (STEP) -> ensure import happened
    {
        const auto& component = GetComponent("external");
        PNamedShape shape     = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_GT(shape->GetFaceCount(), 0u);
    }

    // combined element -> ensure all subelements are built by evaluating the bounding box
    {
        const auto& combined = GetComponent("combinedComponent");
        PNamedShape shape    = combined.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->GetFaceCount(), 33u);

        Bnd_Box box;
        BRepBndLib::Add(shape->Shape(), box);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        EXPECT_NEAR(xmax - xmin, 4.2, eps);
        EXPECT_NEAR(ymax - ymin, 0.88, eps);
        EXPECT_NEAR(zmax - zmin, 0.88, eps);
    }

    // multiSegmentShape
    {
        const auto& multiSegment = GetComponent("multiSegmentComponent");
        PNamedShape shape        = multiSegment.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->GetFaceCount(), 3u);

        Bnd_Box box;
        BRepBndLib::Add(shape->Shape(), box);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        EXPECT_NEAR(xmax - xmin, 1.0, eps);
        EXPECT_NEAR(ymax - ymin, 2.25, eps);
        EXPECT_NEAR(zmax - zmin, 2.125, eps);
    }

    // wing-like multiSegmentShape
    {
        const auto& multiSegment = GetComponent("wing");
        PNamedShape shape        = multiSegment.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->GetFaceCount(), 4u);

        Bnd_Box box;
        BRepBndLib::Add(shape->Shape(), box);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        EXPECT_NEAR(xmax - xmin, 1.0027087, eps);
        EXPECT_NEAR(ymax - ymin, 1.5, eps);
        EXPECT_NEAR(zmax - zmin, 0.0903533, eps);
    }

    // multiSegmentShape with 2 segments and super ellipses
    {
        const auto& multiSegment = GetComponent("multiSegmentComponent3");
        PNamedShape shape        = multiSegment.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->GetFaceCount(), 10u);
    }
}

TEST_F(Systems, Representation)
{
    // cuboid_1 is explicitly set to "physical"
    {
        const auto& component = GetComponent("cuboid_1");
        EXPECT_EQ(component.GetComponentRepresentation(), TIGL_GEOMREP_PHYSICAL);
        EXPECT_EQ(component.GetComponentRepresentationAsString(), "physical");
    }

    // cuboid_2 is explicitly set to "envelope"
    {
        const auto& component = GetComponent("cuboid_2");
        EXPECT_EQ(component.GetComponentRepresentation(), TIGL_GEOMREP_ENVELOPE);
        EXPECT_EQ(component.GetComponentRepresentationAsString(), "envelope");
    }

    // wedge_1 has no attribute "representation", so it must default to "physical"
    {
        const auto& component = GetComponent("wedge_1");
        EXPECT_EQ(component.GetComponentRepresentation(), TIGL_GEOMREP_PHYSICAL);
        EXPECT_EQ(component.GetComponentRepresentationAsString(), "physical");
    }
}

TEST_F(Systems, ComponentMasses)
{
    const double eps = 1e-6;

    // ---- Mass values ----
    {
        const auto& component = GetComponent("cuboid_1");
        const auto m          = component.GetMass();
        ASSERT_TRUE(m);
        EXPECT_NEAR(*m, 0.1234, eps);
    }

    const auto& cuboid_2 = GetComponent("cuboid_2");
    const auto& cuboid_3 = GetComponent("cuboid_3");

    const auto m2 = cuboid_2.GetMass();
    const auto m3 = cuboid_3.GetMass();
    ASSERT_TRUE(m2);
    ASSERT_TRUE(m3);
    EXPECT_NEAR(*m2, 0.375, eps);
    EXPECT_NEAR(*m2, *m3, eps);

    {
        const auto& component = GetComponent("wedge_1");
        const auto m          = component.GetMass();
        EXPECT_FALSE(m);
    }

    {
        const auto& component = GetComponent("external");
        const auto m          = component.GetMass();
        ASSERT_TRUE(m);
        EXPECT_NEAR(*m, 0.2476386, eps);
    }

    {
        const auto& component = GetComponent("electricMotor");
        const auto m          = component.GetMass();
        ASSERT_TRUE(m);
        EXPECT_NEAR(*m, 0.123, eps);
    }

    // ---- Mass location ----
    {
        const auto& cuboid_2 = GetComponent("cuboid_2");
        const auto cogLocal  = cuboid_2.GetCenterOfGravityLocal();
        ASSERT_TRUE(cogLocal);
        EXPECT_NEAR(cogLocal->x, 1.5, eps);
        EXPECT_NEAR(cogLocal->y, 0.25, eps);
        EXPECT_NEAR(cogLocal->z, 0.25, eps);

        const auto cogGlobal = cuboid_2.GetCenterOfGravityGlobal();
        EXPECT_TRUE(cuboid_2.IsPositioned());
        ASSERT_TRUE(cogGlobal);
        EXPECT_NEAR(cogGlobal->x, 1.5, eps);
        EXPECT_NEAR(cogGlobal->y, 5.25, eps);
        EXPECT_NEAR(cogGlobal->z, 0.25, eps);
    }

    {
        const auto& cuboid_4 = GetComponent("cuboid_4");

        const auto cogLocal = cuboid_4.GetCenterOfGravityLocal();
        ASSERT_TRUE(cogLocal);
        EXPECT_NEAR(cogLocal->x, 0.3, eps);
        EXPECT_NEAR(cogLocal->y, 0.25, eps);
        EXPECT_NEAR(cogLocal->z, 0.4, eps);

        const auto cogGlobal = cuboid_4.GetCenterOfGravityGlobal();
        EXPECT_TRUE(cuboid_4.IsPositioned());
        ASSERT_TRUE(cogGlobal);
        EXPECT_NEAR(cogGlobal->x, -0.0707107, eps);
        EXPECT_NEAR(cogGlobal->y, 15.25, eps);
        EXPECT_NEAR(cogGlobal->z, 0.49497475, eps);
    }

    const auto& unpositionedCuboid = GetComponent("unpositionedCuboid");
    EXPECT_FALSE(unpositionedCuboid.IsPositioned());
    EXPECT_FALSE(unpositionedCuboid.GetCenterOfGravityGlobal());

    // ---- Mass inertia ----
    {
        const auto& component = GetComponent("cuboid_1");
        const auto mi         = component.GetMassInertiaLocal();
        ASSERT_TRUE(mi);
        EXPECT_EQ(mi->Jxx, 1);
        EXPECT_EQ(mi->Jyy, 2);
        EXPECT_EQ(mi->Jzz, 3);
        EXPECT_FALSE(mi->Jxy);
        EXPECT_FALSE(mi->Jxz);
        EXPECT_FALSE(mi->Jyz);
    }

    {
        const auto& component = GetComponent("cuboid_2");
        const auto mi         = component.GetMassInertiaLocal();
        EXPECT_FALSE(mi);
    }
}

TEST_F(Systems, ComponentCentroid)
{
    const double eps = 1e-6;

    const auto& component = GetComponent("cuboid_2");

    const auto centroidLocal = component.GetCentroidLocal();
    EXPECT_NEAR(centroidLocal.x, 1.5, eps);
    EXPECT_NEAR(centroidLocal.y, 0.25, eps);
    EXPECT_NEAR(centroidLocal.z, 0.25, eps);

    const auto centroidGlobal = component.GetCentroidGlobal();
    ASSERT_TRUE(centroidGlobal);
    EXPECT_NEAR(centroidGlobal->x, 1.5, eps);
    EXPECT_NEAR(centroidGlobal->y, 5.25, eps);
    EXPECT_NEAR(centroidGlobal->z, 0.25, eps);
}

TEST_F(Systems, ConfigurationAccess)
{
    const auto& config = GetConfig();

    // genericSystem
    EXPECT_EQ(config.GetGenericSystemCount(), 1u);

    const auto& system = config.GetGenericSystem(1);
    EXPECT_EQ(system.GetDefaultedUID(), "genSys_1");
    EXPECT_EQ(system.GetName(), "Generic System 1");

    const auto& systemByUid = config.GetGenericSystem("genSys_1");
    EXPECT_EQ(systemByUid.GetDefaultedUID(), "genSys_1");
    EXPECT_EQ(systemByUid.GetName(), "Generic System 1");
    EXPECT_EQ(&system, &systemByUid);

    // systemArchitecture
    EXPECT_EQ(config.GetSystemArchitecturesCount(), 1u);

    const auto& sa = config.GetSystemArchitecture(1);
    EXPECT_EQ(sa.GetName(), "Test system architecture");

    const auto& saByUid = config.GetSystemArchitecture("systemArchitecture1");
    EXPECT_EQ(saByUid.GetName(), "Test system architecture");
    EXPECT_EQ(&sa, &saByUid);
}

TEST_F(Systems, ConfigurationAccessNonConst)
{
    auto& config = GetConfig();

    auto& system = config.GetGenericSystem(1);
    EXPECT_EQ(system.GetDefaultedUID(), "genSys_1");

    auto& saByIndex = config.GetSystemArchitecture(1);
    EXPECT_EQ(saByIndex.GetName(), "Test system architecture");

    auto& saByUID = config.GetSystemArchitecture("systemArchitecture1");
    EXPECT_EQ(saByUID.GetName(), "Test system architecture");
}

TEST_F(Systems, SystemArchitectureConnections)
{
    const auto& config = GetConfig();
    const auto& sa     = config.GetSystemArchitecture(1);

    const auto& connections = sa.GetConnections();
    ASSERT_TRUE(connections);

    ASSERT_EQ(connections->GetConnectionCount(), 5u);

    {
        const auto& c     = connections->GetConnection(1);
        const auto source = c.GetSourceComponent();
        const auto target = c.GetTargetComponent();
        ASSERT_TRUE(source);
        ASSERT_TRUE(target);

        EXPECT_EQ(source->GetDefaultedUID(), "cuboid_1");
        EXPECT_EQ(target->GetDefaultedUID(), "cuboid_2");
    }

    {
        const auto& c     = connections->GetConnection(3);
        const auto source = c.GetSourceComponent();
        const auto target = c.GetTargetComponent();
        ASSERT_TRUE(source);

        // target is a fuselage, not a CCPACSComponent
        EXPECT_FALSE(target);
        const auto& targetUid = c.GetTarget().GetComponentUID_choice4();
        ASSERT_TRUE(targetUid);
        EXPECT_EQ(*targetUid, "SimpleFuselage");
    }

    {
        const auto& c              = connections->GetConnection(4);
        const auto targetComponent = c.GetTargetComponent();
        EXPECT_FALSE(targetComponent);

        const auto& targetExternalElement = c.GetTarget().GetExternalElement_choice1();
        ASSERT_TRUE(targetExternalElement);
        EXPECT_EQ(*targetExternalElement, tigl::ECPACSSourceTarget_externalElement::ambient);
    }

    {
        const auto& c             = connections->GetConnection(5);
        const auto sourceComonent = c.GetSourceComponent();
        EXPECT_FALSE(sourceComonent);

        const auto& targetSourceElement = c.GetSource().GetExternalElement_choice1();
        ASSERT_TRUE(targetSourceElement);
        EXPECT_EQ(*targetSourceElement, tigl::ECPACSSourceTarget_externalElement::ambient);
    }
}

TEST_F(Systems, SystemArchitectureComponentList)
{
    const auto& config = GetConfig();
    const auto& sa     = config.GetSystemArchitecture(1);

    const auto components = sa.GetGenericSystemComponents();

    ASSERT_EQ(components.size(), 3u);

    EXPECT_EQ(components[0]->GetDefaultedUID(), "cuboid_1");
    EXPECT_EQ(components[1]->GetDefaultedUID(), "cuboid_2");
    EXPECT_EQ(components[2]->GetDefaultedUID(), "cuboid_3");
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

    const tigl::CTiglUIDManager& GetUIDManager() const
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
    }

    const tigl::CCPACSGenericSystem& GetSystem(const std::string& uid) const
    {
        return GetUIDManager().ResolveObject<tigl::CCPACSGenericSystem>(uid);
    }

    const tigl::CCPACSComponent& GetComponent(const std::string& uid) const
    {
        return GetUIDManager().ResolveObject<tigl::CCPACSComponent>(uid);
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TEST_F(InvalidSystems, Exceptions)
{
    // Exception for wrong UID reference
    const auto& wrongReference = GetComponent("wrongUIDReference");

    CheckExceptionMessage([&] { (void)wrongReference.GetLoft(); }, "Unsupported system element for uID \"NACA0009\".");
}

TEST_F(InvalidSystems, InvalidShapes)
{
    {
        const auto& system = GetSystem("testSystem");
        const auto& comp   = system.GetComponents().GetComponent(12);
        const auto loft    = comp.GetLoft();
        ASSERT_TRUE(loft);
        EXPECT_EQ(loft->Name(), "predCuboid_1_cuboid_1");
    }

    {
        const auto& invalidShape = GetComponent("invalidCuboid");
        CheckExceptionMessage(
            [&] { (void)invalidShape.GetLoft(); },
            "Invalid cuboid parameters for uID=\"invalidPredCuboid\": lengthX, depthY and heightZ must be positive.");
    }

    {
        const auto& invalidShape = GetComponent("invalidCylinder");
        CheckExceptionMessage(
            [&] { (void)invalidShape.GetLoft(); },
            "Invalid cylinder parameters for uID=\"invalidPredCylinder\": Radius and height must be positive.");
    }

    {
        const auto& invalidShape = GetComponent("invalidCone");
        CheckExceptionMessage([&] { (void)invalidShape.GetLoft(); },
                              "Invalid cone parameters for uID=\"invalidPredCone\": At least one radius must be "
                              "positive and height must be positive.");
    }

    {
        const auto& invalidShape = GetComponent("invalidEllipsoid");
        CheckExceptionMessage([&] { (void)invalidShape.GetLoft(); },
                              "Invalid ellipsoid parameters: All radii must be positive.");
    }

    {
        const auto& invalidShape = GetComponent("zeroVolumeEllipsoid");
        CheckExceptionMessage([&] { (void)invalidShape.GetLoft(); },
                              "Invalid ellipsoid diskAngle: must be in range (0, 2*pi].");
    }

    // It's ok to build a cone as cylinder (ToDo: checking the warning would be nice)
    {
        const auto& cylinder = GetComponent("cylinderCone");
        ASSERT_TRUE(cylinder.GetLoft());
    }

    {
        const auto& face = GetComponent("zeroHeightComponent");
        CheckExceptionMessage([&] { (void)face.GetMass(); },
                              "Cannot compute mass properties of component with uID \"predFace\" (zero volume).");
    }

    {
        const auto& mss = GetComponent("mssWithoutSegments");
        CheckExceptionMessage([&] { (void)mss.GetLoft(); }, "Cannot build multi-segment shape: no segments defined.");
    }
}

TEST_F(InvalidSystems, InvalidComponentCentroid)
{
    {
        const auto& comp = GetComponent("zeroHeightComponent");
        CheckExceptionMessage(
            [&] { (void)comp.GetCentroidLocal(); },
            "Cannot compute geometric centroid of component with uID=\"zeroHeightComponent\" (zero volume).");

        CheckExceptionMessage(
            [&] { (void)comp.GetCentroidGlobal(); },
            "Cannot compute geometric centroid of component with uID=\"zeroHeightComponent\" (zero volume).");
    }

    {
        const auto& comp = GetComponent("elementWithoutGeometry");
        CheckExceptionMessage([&] { (void)comp.GetCentroidLocal(); },
                              "No geometry primitives defined for uID=\"predElementWithoutGeometry\"");
    }
}

TEST_F(InvalidSystems, InvalidFileHandling)
{
    {
        const auto& invalidShape = GetComponent("invalidExternal1");
        CheckExceptionMessage([&] { (void)invalidShape.GetLoft(); },
                              "Cannot open external file. No file format specified.");
    }

    {
        const auto& invalidShape = GetComponent("invalidExternal2");
        CheckExceptionMessage([&] { (void)invalidShape.GetLoft(); },
                              "Cannot open external element. Unknown file format: Stl");
    }
}

TEST_F(InvalidSystems, Masses)
{
    const auto& cuboid = GetComponent("invalidMass");

    CheckExceptionMessage([&] { (void)cuboid.GetMass(); },
                          "Invalid mass definition (no mass and no density) for uID \"invalidPredCuboid_2\".");
}

TEST_F(InvalidSystems, InvalidSystemMassProperties)
{
    // Systems with zero mass should not return a center of gravity
    {
        const auto& sys = GetSystem("testSystem2");
        const auto cog  = sys.GetCenterOfGravity();
        ASSERT_FALSE(cog);
    }

    // System referring to an element without geometry
    {
        const auto& sys = GetSystem("testSystem3");
        CheckExceptionMessage([&] { (void)sys.GetCenterOfGravity(); },
                              "No geometry primitives defined for uID=\"predElementWithoutGeometry\"");
    }
}

TixiDocumentHandle InvalidSystems::tixiHandle           = 0;
TiglCPACSConfigurationHandle InvalidSystems::tiglHandle = 0;

class EmptyACSystems : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-systems.cpacs.xml";
        ASSERT_EQ(tixiOpenDocument(filename, &tixiHandle), SUCCESS);
        ASSERT_EQ(tiglOpenCPACSConfiguration(tixiHandle, "emptyAircraft", &tiglHandle), TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    const tigl::CCPACSConfiguration& GetConfig() const
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle EmptyACSystems::tixiHandle           = 0;
TiglCPACSConfigurationHandle EmptyACSystems::tiglHandle = 0;

TEST_F(EmptyACSystems, EmptyGenericSystems)
{
    const auto& config = GetConfig();

    EXPECT_EQ(config.GetGenericSystemCount(), 0u);
    CheckExceptionMessage([&] { (void)config.GetGenericSystem("doesNotExist"); }, "No generic system loaded");
    CheckExceptionMessage([&] { (void)config.GetGenericSystem(1); }, "No generic system loaded");
}

TEST_F(EmptyACSystems, EmptySystemArchitectures)
{
    const auto& config = GetConfig();

    EXPECT_EQ(config.GetSystemArchitecturesCount(), 0u);
    CheckExceptionMessage([&] { (void)config.GetSystemArchitecture("doesNotExist"); }, "No system architecture loaded");
    CheckExceptionMessage([&] { (void)config.GetSystemArchitecture(1); }, "No system architecture loaded");
}

class RCSystems : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-systems.cpacs.xml";
        ASSERT_EQ(tixiOpenDocument(filename, &tixiHandle), SUCCESS);
        ASSERT_EQ(tiglOpenCPACSConfiguration(tixiHandle, "testRotorcraft", &tiglHandle), TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    const tigl::CCPACSConfiguration& GetConfig() const
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TEST_F(RCSystems, SystemsAccess)
{
    const auto& config = GetConfig();

    // genericSystem
    EXPECT_EQ(config.GetGenericSystemCount(), 1u);

    const auto& system = config.GetGenericSystem(1);
    EXPECT_EQ(system.GetDefaultedUID(), "genRCSystem");
    EXPECT_EQ(system.GetName(), "Generic rotorcraft system");

    const auto& systemByUid = config.GetGenericSystem("genRCSystem");
    EXPECT_EQ(systemByUid.GetDefaultedUID(), "genRCSystem");
    EXPECT_EQ(systemByUid.GetName(), "Generic rotorcraft system");
    EXPECT_EQ(&system, &systemByUid);

    // systemArchitecture
    EXPECT_EQ(config.GetSystemArchitecturesCount(), 2u);

    const auto& sa = config.GetSystemArchitecture(1);
    EXPECT_EQ(sa.GetName(), "Rotorcraft system architecture");

    const auto& saByUid = config.GetSystemArchitecture("rcSysArc");
    EXPECT_EQ(saByUid.GetName(), "Rotorcraft system architecture");
    EXPECT_EQ(&sa, &saByUid);
}

TEST_F(RCSystems, EmptySystemArchitecture)
{
    const auto& config = GetConfig();
    const auto& sa     = config.GetSystemArchitecture(2);

    const auto components = sa.GetGenericSystemComponents();

    ASSERT_EQ(components.size(), 0u);
}

TixiDocumentHandle RCSystems::tixiHandle           = 0;
TiglCPACSConfigurationHandle RCSystems::tiglHandle = 0;

class EmptyRCSystems : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-systems.cpacs.xml";
        ASSERT_EQ(tixiOpenDocument(filename, &tixiHandle), SUCCESS);
        ASSERT_EQ(tiglOpenCPACSConfiguration(tixiHandle, "emptyRotorcraft", &tiglHandle), TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    const tigl::CCPACSConfiguration& GetConfig() const
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle EmptyRCSystems::tixiHandle           = 0;
TiglCPACSConfigurationHandle EmptyRCSystems::tiglHandle = 0;

TEST_F(EmptyRCSystems, EmptyGenericSystems)
{
    const auto& config = GetConfig();

    EXPECT_EQ(config.GetGenericSystemCount(), 0u);
    CheckExceptionMessage([&] { (void)config.GetGenericSystem("doesNotExist"); }, "No generic system loaded");
    CheckExceptionMessage([&] { (void)config.GetGenericSystem(1); }, "No generic system loaded");
}

TEST_F(EmptyRCSystems, EmptySystemArchitectures)
{
    const auto& config = GetConfig();

    EXPECT_EQ(config.GetSystemArchitecturesCount(), 0u);
    CheckExceptionMessage([&] { (void)config.GetSystemArchitecture("doesNotExist"); }, "No system architecture loaded");
    CheckExceptionMessage([&] { (void)config.GetSystemArchitecture(1); }, "No system architecture loaded");
}
