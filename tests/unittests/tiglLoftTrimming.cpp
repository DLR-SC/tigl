/*
* Copyright (C) 2026 German Aerospace Center
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
* @brief Tests for trimmed vs untrimmed loft disambiguation (PR #1331).
*
* Verifies that GetLoft() defaults to GetUntrimmedLoft(), that trimming
* genuinely changes face count when guide curves are absent, and that
* trimming preserves geometry (bounding box, area, volume).
* Also verifies that guide-curve lofts ignore the trimming flag.
*/

#include "BRepBuilderAPI_Transform.hxx"
#include "BRepCheck_Analyzer.hxx"
#include "BRepGProp.hxx"
#include "BRepBndLib.hxx"
#include "Bnd_Box.hxx"
#include "CTiglError.h"
#include "CCPACSFuselage.h"
#include "CCPACSWing.h"
#include "CCPACSDuct.h"
#include "CCPACSEnginePylon.h"
#include "CCPACSConfigurationManager.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"
#include "test.h"
#include "testUtils.h"

#include <TopExp.hxx>
#include <TopAbs.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <GProp_GProps.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <math.hxx>

namespace {

int countFaces(PNamedShape shape)
{
    if (!shape) {
        return 0;
    }
    int count = 0;
    TopExp_Explorer explorer(shape->Shape(), TopAbs_FACE);
    while (explorer.More()) {
        ++count;
        explorer.Next();
    }
    return count;
}

double surfaceArea(PNamedShape shape)
{
    if (!shape) {
        return 0.0;
    }
    GProp_GProps props;
    BRepGProp::SurfaceProperties(shape->Shape(), props);
    return props.Mass();
}

double volume(PNamedShape shape)
{
    if (!shape) {
        return 0.0;
    }
    GProp_GProps props;
    BRepGProp::VolumeProperties(shape->Shape(), props);
    return props.Mass();
}

bool isClosedSolid(PNamedShape shape)
{
    if (!shape) {
        return false;
    }
    return BRepCheck_Analyzer(shape->Shape()).IsValid();
}

double bboxSize(const Bnd_Box& box)
{
    if (box.IsVoid()) {
        return 0.0;
    }
    return (box.CornerMax().X() - box.CornerMin().X()) *
           (box.CornerMax().Y() - box.CornerMin().Y()) *
           (box.CornerMax().Z() - box.CornerMin().Z());
}

double bboxDiagonal(const Bnd_Box& box)
{
    if (box.IsVoid()) {
        return 0.0;
    }
    return sqrt(
        pow(box.CornerMax().X() - box.CornerMin().X(), 2) +
        pow(box.CornerMax().Y() - box.CornerMin().Y(), 2) +
        pow(box.CornerMax().Z() - box.CornerMin().Z(), 2)
    );
}

void expectNear(double a, double b, double relTol, const std::string& msg)
{
    double maxAbs = std::max(std::abs(a), std::abs(b));
    double diff   = std::abs(a - b);
    double tol    = relTol * (maxAbs > 0 ? maxAbs : 1.0);
    EXPECT_NEAR(a, b, tol) << msg << " (abs diff=" << diff << ", rel tol=" << relTol << ")";
}

class LoftTrimming : public ::testing::Test
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
        ASSERT_TRUE(tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Cpacs2Test", &tiglHandle);
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

TixiDocumentHandle LoftTrimming::tixiHandle = 0;
TiglCPACSConfigurationHandle LoftTrimming::tiglHandle = 0;

TEST_F(LoftTrimming, GetLoftDefaultsToUntrimmed)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    // Wing
    {
        auto& wing = config.GetWing(1);
        auto loft      = wing.GetLoft();
        auto untrimmed = wing.GetUntrimmedLoft();
        EXPECT_EQ(countFaces(loft), countFaces(untrimmed));
        EXPECT_EQ(surfaceArea(loft), surfaceArea(untrimmed));
        EXPECT_EQ(volume(loft), volume(untrimmed));
    }

    // Fuselage
    {
        auto& fuselage = config.GetFuselage(1);
        auto loft      = fuselage.GetLoft();
        auto untrimmed = fuselage.GetUntrimmedLoft();
        EXPECT_EQ(countFaces(loft), countFaces(untrimmed));
        EXPECT_EQ(surfaceArea(loft), surfaceArea(untrimmed));
        EXPECT_EQ(volume(loft), volume(untrimmed));
    }
}

TEST_F(LoftTrimming, TrimmedHasMoreFacesThanUntrimmed_NoGuides)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    // Wing (no guide curves in simpletest)
    {
        auto& wing = config.GetWing(1);
        int trimmedFaces   = countFaces(wing.GetTrimmedLoft());
        int untrimmedFaces = countFaces(wing.GetUntrimmedLoft());
        EXPECT_GT(trimmedFaces, untrimmedFaces) << "Trimmed loft should have more faces than untrimmed when no guide curves";
    }

    // Fuselage (no guide curves in simpletest)
    {
        auto& fuselage = config.GetFuselage(1);
        int trimmedFaces   = countFaces(fuselage.GetTrimmedLoft());
        int untrimmedFaces = countFaces(fuselage.GetUntrimmedLoft());
        EXPECT_GT(trimmedFaces, untrimmedFaces) << "Trimmed loft should have more faces than untrimmed when no guide curves";
    }
}

TEST_F(LoftTrimming, TrimmingPreservesGeometry)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    double relTol = 1e-6;

    // Wing
    {
        const auto& wing = config.GetWing(1);
        auto trimmed   = wing.GetTrimmedLoft();
        auto untrimmed = wing.GetUntrimmedLoft();

        EXPECT_TRUE(isClosedSolid(trimmed));
        EXPECT_TRUE(isClosedSolid(untrimmed));

        Bnd_Box trimmedBox, untrimmedBox;
        BRepBndLib::AddOptimal(trimmed->Shape(), trimmedBox);
        BRepBndLib::AddOptimal(untrimmed->Shape(), untrimmedBox);

        double diag = bboxDiagonal(trimmedBox);
        double lenTol = relTol * diag;

        EXPECT_NEAR(trimmedBox.CornerMin().X(), untrimmedBox.CornerMin().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Y(), untrimmedBox.CornerMin().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Z(), untrimmedBox.CornerMin().Z(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().X(), untrimmedBox.CornerMax().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Y(), untrimmedBox.CornerMax().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Z(), untrimmedBox.CornerMax().Z(), lenTol);
    }

    // Fuselage
    {
        const auto& fuselage = config.GetFuselage(1);
        auto trimmed   = fuselage.GetTrimmedLoft();
        auto untrimmed = fuselage.GetUntrimmedLoft();

        EXPECT_TRUE(isClosedSolid(trimmed));
        EXPECT_TRUE(isClosedSolid(untrimmed));

        Bnd_Box trimmedBox, untrimmedBox;
        BRepBndLib::AddOptimal(trimmed->Shape(), trimmedBox);
        BRepBndLib::AddOptimal(untrimmed->Shape(), untrimmedBox);

        double diag = bboxDiagonal(trimmedBox);
        double lenTol = 1e-6 * diag;

        EXPECT_NEAR(trimmedBox.CornerMin().X(), untrimmedBox.CornerMin().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Y(), untrimmedBox.CornerMin().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Z(), untrimmedBox.CornerMin().Z(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().X(), untrimmedBox.CornerMax().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Y(), untrimmedBox.CornerMax().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Z(), untrimmedBox.CornerMax().Z(), lenTol);
    }
}

TEST_F(LoftTrimming, WingTrimmedCleanShape)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    const auto& wing = config.GetWing(1);
    auto trimmedClean = wing.GetTrimmedWingCleanShape();
    auto untrimmedClean = wing.GetWingCleanShape();

    EXPECT_TRUE(trimmedClean != nullptr);
    EXPECT_TRUE(untrimmedClean != nullptr);
}

class LoftTrimmingWithGuides : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-with-guides.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE(tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Cpacs2Test", &tiglHandle);
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

TixiDocumentHandle LoftTrimmingWithGuides::tixiHandle = 0;
TiglCPACSConfigurationHandle LoftTrimmingWithGuides::tiglHandle = 0;

TEST_F(LoftTrimmingWithGuides, GuideCurveLoftIgnoresTrimming)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    // Wing (has guide curves)
    {
        const auto& wing = config.GetWing(1);
        int trimmedFaces   = countFaces(wing.GetTrimmedLoft());
        int untrimmedFaces = countFaces(wing.GetUntrimmedLoft());
        EXPECT_EQ(trimmedFaces, untrimmedFaces) << "Guide-curve loft should ignore trimming flag";
    }
}

TEST_F(LoftTrimmingWithGuides, TrimmedVsUntrimmedPreservesGeometry_WithGuides)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    // Wing
    {
        const auto& wing = config.GetWing(1);
        auto trimmed   = wing.GetTrimmedLoft();
        auto untrimmed = wing.GetUntrimmedLoft();

        EXPECT_TRUE(isClosedSolid(trimmed));
        EXPECT_TRUE(isClosedSolid(untrimmed));

        Bnd_Box trimmedBox, untrimmedBox;
        BRepBndLib::AddOptimal(trimmed->Shape(), trimmedBox);
        BRepBndLib::AddOptimal(untrimmed->Shape(), untrimmedBox);

        double diag = bboxDiagonal(trimmedBox);
        double lenTol = 1e-6 * diag;

        EXPECT_NEAR(trimmedBox.CornerMin().X(), untrimmedBox.CornerMin().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Y(), untrimmedBox.CornerMin().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Z(), untrimmedBox.CornerMin().Z(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().X(), untrimmedBox.CornerMax().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Y(), untrimmedBox.CornerMax().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Z(), untrimmedBox.CornerMax().Z(), lenTol);
    }

    // Fuselage
    {
        const auto& fuselage = config.GetFuselage(1);
        auto trimmed   = fuselage.GetTrimmedLoft();
        auto untrimmed = fuselage.GetUntrimmedLoft();

        EXPECT_TRUE(isClosedSolid(trimmed));
        EXPECT_TRUE(isClosedSolid(untrimmed));

        Bnd_Box trimmedBox, untrimmedBox;
        BRepBndLib::AddOptimal(trimmed->Shape(), trimmedBox);
        BRepBndLib::AddOptimal(untrimmed->Shape(), untrimmedBox);

        double diag = bboxDiagonal(trimmedBox);
        double lenTol = 1e-6 * diag;

        EXPECT_NEAR(trimmedBox.CornerMin().X(), untrimmedBox.CornerMin().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Y(), untrimmedBox.CornerMin().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMin().Z(), untrimmedBox.CornerMin().Z(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().X(), untrimmedBox.CornerMax().X(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Y(), untrimmedBox.CornerMax().Y(), lenTol);
        EXPECT_NEAR(trimmedBox.CornerMax().Z(), untrimmedBox.CornerMax().Z(), lenTol);
    }
}

} // anonymous namespace
