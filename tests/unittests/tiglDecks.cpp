/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-18 Marko Alder <marko.alder@dlr.de>
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

#include "CCPACSConfigurationManager.h"
#include "CCPACSDeck.h"
#include "CCPACSDeckComponentBase.h"

#include "CNamedShape.h"
#include <TopExp_Explorer.hxx>
#include <BRepBndLib.hxx>

class Decks : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-decks.cpacs.xml";
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

    const tigl::CCPACSDeck& GetDeck(const std::string& uid) const
    {
        return GetUIDManager().ResolveObject<tigl::CCPACSDeck>(uid);
    }

    const tigl::CCPACSDeckComponentBase& GetComponent(const std::string& uid) const
    {
        return GetUIDManager().ResolveObject<tigl::CCPACSDeckComponentBase>(uid);
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle Decks::tixiHandle           = 0;
TiglCPACSConfigurationHandle Decks::tiglHandle = 0;

TEST_F(Decks, Basics)
{
    const auto& deck      = GetDeck("deck1");
    const auto& component = GetComponent("seatModule1_rh");

    EXPECT_EQ(deck.GetDefaultedUID(), "deck1");
    EXPECT_EQ(component.GetDefaultedUID(), "seatModule1_rh");

    EXPECT_EQ(deck.GetComponentType(), TIGL_COMPONENT_DECK);
    EXPECT_EQ(deck.GetComponentIntent(), TIGL_INTENT_PHYSICAL);

    EXPECT_EQ(component.GetComponentType(), TIGL_COMPONENT_DECK_COMPONENT);
    EXPECT_EQ(component.GetComponentIntent(), TIGL_INTENT_PHYSICAL);

    EXPECT_EQ(deck.GetConfiguration().GetUID(), "testAircraft");
    EXPECT_EQ(component.GetConfiguration().GetUID(), "testAircraft");
}

TEST_F(Decks, DeckGeometry)
{
    const auto& deck = GetDeck("deck1");

    const PNamedShape shape = deck.GetLoft();
    ASSERT_TRUE(shape);
    EXPECT_EQ(shape->Name(), "deck1");

    // 2 seat modules + 8 further deck components
    unsigned shapeCount = 0;
    for (TopoDS_Iterator it(shape->Shape()); it.More(); it.Next()) {
        ++shapeCount;
    }
    EXPECT_EQ(shapeCount, 11u);
}

TEST_F(Decks, ComponentRepresentation)
{
    const auto& seat     = GetComponent("seatModule1_rh");
    const auto& ceiling  = GetComponent("ceilingPanel1");
    const auto& galley   = GetComponent("galley");
    const auto& lavatory = GetComponent("lavatory");

    EXPECT_EQ(seat.GetComponentRepresentation(), TIGL_GEOMREP_ENVELOPE);
    EXPECT_EQ(seat.GetComponentRepresentationAsString(), "envelope");

    EXPECT_EQ(ceiling.GetComponentRepresentation(), TIGL_GEOMREP_PHYSICAL);
    EXPECT_EQ(ceiling.GetComponentRepresentationAsString(), "physical");
    EXPECT_EQ(galley.GetComponentRepresentation(), TIGL_GEOMREP_PHYSICAL);
    EXPECT_EQ(lavatory.GetComponentRepresentation(), TIGL_GEOMREP_PHYSICAL);
}

TEST_F(Decks, ComponentGeometryBuilders)
{
    {
        const auto& component = GetComponent("seatModule1_rh");
        const auto shape      = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->Name(), "seatModule1_rh_cuboid_1");
        EXPECT_EQ(shape->GetFaceCount(), 6u);
    }

    {
        const auto& component = GetComponent("luggageCompartment");
        const auto shape      = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->Name(), "luggageCompartment_cylinder_1");
        EXPECT_EQ(shape->GetFaceCount(), 3u);
    }

    {
        const auto& component = GetComponent("classDivider");
        const auto shape      = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->Name(), "classDivider_ellipsoid_1");
        EXPECT_EQ(shape->GetFaceCount(), 1u);
    }

    {
        const auto& component = GetComponent("galley");
        const auto shape      = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->Name(), "galley_multiSegmentShape_1");
        EXPECT_GT(shape->GetFaceCount(), 0u);
    }

    {
        const auto& component = GetComponent("lavatory");
        const auto shape      = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->Name(), "lavatory_external_1");
        EXPECT_GT(shape->GetFaceCount(), 0u);
    }

    {
        const auto& component = GetComponent("floorModule");
        const auto shape      = component.GetLoft();
        ASSERT_TRUE(shape);
        EXPECT_EQ(shape->Name(), "floorModule_cone_1");
        EXPECT_GT(shape->GetFaceCount(), 0u);
    }
}

TEST_F(Decks, MultiPrimitiveComponentGeometry)
{
    const double eps = 1e-6;

    const auto& component = GetComponent("cargoContainer");
    const auto shape      = component.GetLoft();
    ASSERT_TRUE(shape);

    // cargoContainer is composed of two cuboids
    EXPECT_GT(shape->GetFaceCount(), 6u);

    Bnd_Box box;
    BRepBndLib::Add(shape->Shape(), box);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    EXPECT_NEAR(xmax - xmin, 2, eps);
    EXPECT_NEAR(ymax - ymin, 1.5, eps);
    EXPECT_NEAR(zmax - zmin, 1.4, eps);
}

TEST_F(Decks, ComponentMass_DensityBased)
{
    const auto& seat = GetComponent("seatModule1_rh");
    const double eps = 1e-6;

    const auto mass = seat.GetMass();
    ASSERT_TRUE(mass);
    EXPECT_NEAR(*mass, 19.2, eps);

    const auto cogLocal = seat.GetCenterOfGravityLocal();
    ASSERT_TRUE(cogLocal);
    // 0.4 geometric center + 0.2 local cuboid translation + 0.3 local geometry translation
    EXPECT_NEAR(cogLocal->x, 0.9, eps);
    EXPECT_NEAR(cogLocal->y, 1.0, eps);
    EXPECT_NEAR(cogLocal->z, 0.5, eps);

    const auto cogGlobal = seat.GetCenterOfGravityGlobal();
    ASSERT_TRUE(cogGlobal);
    EXPECT_NEAR(cogGlobal->x, 3.4, eps); // 0.9 local + 2.5 global translation
    EXPECT_NEAR(cogGlobal->y, 1.0, eps);
    EXPECT_NEAR(cogGlobal->z, 0.5, eps);
}

TEST_F(Decks, ComponentMass_ExplicitMassAndLocation)
{
    const auto& luggage = GetComponent("luggageCompartment");
    const double eps    = 1e-6;

    const auto mass = luggage.GetMass();
    ASSERT_TRUE(mass);
    EXPECT_NEAR(*mass, 79.95, eps);

    const auto cogLocal = luggage.GetCenterOfGravityLocal();
    ASSERT_TRUE(cogLocal);
    EXPECT_NEAR(cogLocal->x, 0.2, eps);
    EXPECT_NEAR(cogLocal->y, 0.3, eps);
    EXPECT_NEAR(cogLocal->z, 0.4, eps);

    const auto cogGlobal = luggage.GetCenterOfGravityGlobal();
    ASSERT_TRUE(cogGlobal);

    EXPECT_NEAR(cogGlobal->x, 7.6, eps);
    EXPECT_NEAR(cogGlobal->y, 1.39, eps);
    EXPECT_NEAR(cogGlobal->z, 0.9, eps);
}

TEST_F(Decks, ComponentMass_ExplicitMassInertia)
{
    const auto& classDivider = GetComponent("classDivider");
    const double eps         = 1e-6;

    const auto mass = classDivider.GetMass();
    ASSERT_TRUE(mass);
    EXPECT_NEAR(*mass, 123.0, eps);

    const auto inertia = classDivider.GetMassInertiaLocal();
    ASSERT_TRUE(inertia);
    EXPECT_NEAR(inertia->Jxx, 1.0, eps);
    EXPECT_NEAR(inertia->Jyy, 2.0, eps);
    EXPECT_NEAR(inertia->Jzz, 3.0, eps);

    EXPECT_FALSE(inertia->Jxy);
    EXPECT_FALSE(inertia->Jxz);
    EXPECT_FALSE(inertia->Jyz);
}

TEST_F(Decks, NoMass)
{
    // Deck component without mass definition (which is ok)
    const auto& comp = GetComponent("galley");
    ASSERT_FALSE(comp.GetMass());
}

// This test serves an intuitive testing of the scaling effect on mass properties at deck level
TEST_F(Decks, MassProperties_ScalingOfDeckInstance)
{
    const auto& comp = GetComponent("seatModule_massScaleTest");
    const double eps = 1e-6;

    const auto mass = comp.GetMass();
    ASSERT_TRUE(mass);
    EXPECT_NEAR(*mass, 2.6666666, eps);

    const auto cogLocal = comp.GetCenterOfGravityLocal();
    ASSERT_TRUE(cogLocal);
    EXPECT_NEAR(cogLocal->x, 0.5, eps);
    EXPECT_NEAR(cogLocal->y, 0.5, eps);
    EXPECT_NEAR(cogLocal->z, 0, eps);

    const auto cogGlobal = comp.GetCenterOfGravityGlobal();
    ASSERT_TRUE(cogGlobal);
    EXPECT_NEAR(cogGlobal->x, 2, eps);
    EXPECT_NEAR(cogGlobal->y, 0.0, eps);
    EXPECT_NEAR(cogGlobal->z, 0.0, eps);
}

class InvalidDecks : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-invalid-decks.cpacs.xml";
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

    const tigl::CCPACSDeckComponentBase& GetComponent(const std::string& uid) const
    {
        return GetUIDManager().ResolveObject<tigl::CCPACSDeckComponentBase>(uid);
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle InvalidDecks::tixiHandle           = 0;
TiglCPACSConfigurationHandle InvalidDecks::tiglHandle = 0;

static void CheckExceptionMessage(const std::function<void()>& func, const char* expectedMessage)
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

TEST_F(InvalidDecks, UnsupportedReferences)
{
    // Deck component references a profile UID
    const auto& comp = GetComponent("seatModule");
    CheckExceptionMessage([&] { (void)comp.GetLoft(); }, "Unsupported deck element for uID \"fuselageCircleProfile\".");
}
