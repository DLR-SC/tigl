/*
* Copyright (C) 2022 German Aerospace Center
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

#include "CTiglMakeLoft.h"
#include "test.h"
#include "tigl.h"
#include "Debugging.h"
#include "tiglcommonfunctions.h"
#include "CCPACSFuselage.h"
#include "BRepBuilderAPI_Transform.hxx"
#include <BRepCheck_Analyzer.hxx>
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CNamedShape.h"

class FuselageStandardProfileSuperEllipse : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // Test case on standardProfile, mixed profiles: rectangle, rectangle with rounded corners, circle, superellipse, guidecurves

        const char* filename = "TestData/simpletest_standard_profile_superellipse_guides.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // Test case on standardProfile rectangle with guide curves

        const char* filename1 = "TestData/simpletest_standard_profile_superellipse_kink.cpacs.xml";
        ReturnCode tixiRet1;
        TiglReturnCode tiglRet1;

        tiglHandle1 = -1;
        tixiHandle1 = -1;

        tixiRet1 = tixiOpenDocument(filename1, &tixiHandle1);
        ASSERT_TRUE (tixiRet1 == SUCCESS);
        tiglRet1 = tiglOpenCPACSConfiguration(tixiHandle1, "", &tiglHandle1);
        ASSERT_TRUE(tiglRet1 == TIGL_SUCCESS);

        // Test case on standardProfile, invalid elements

        ReturnCode tixiRet2;
        TiglReturnCode tiglRet2;

        tiglHandle2 = -1;
        tixiHandle2 = -1;

        tixiRet2 = tixiOpenDocument(filename1, &tixiHandle2);
        ASSERT_TRUE (tixiRet2 == SUCCESS);
        tiglRet2 = tiglOpenCPACSConfiguration(tixiHandle2, "", &tiglHandle2);
        ASSERT_TRUE(tiglRet2 == TIGL_SUCCESS);


    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;

        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle1) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle1) == SUCCESS);
        tiglHandle1 = -1;
        tixiHandle1 = -1;

        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle2) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle2) == SUCCESS);
        tiglHandle2 = -1;
        tixiHandle2 = -1;

    }

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    static TixiDocumentHandle           tixiHandle1;
    static TiglCPACSConfigurationHandle tiglHandle1;


    static TixiDocumentHandle           tixiHandle2;
    static TiglCPACSConfigurationHandle tiglHandle2;
};

TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle = 0;
TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle1 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle1 = 0;
TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle2 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle2 = 0;


TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesWithGuides_Superellipse)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}


TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesWithKinks_Superellipse)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle1);
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}

TEST_F(FuselageStandardProfileSuperEllipse, UntrimmedLoftFaceNames_MultipleAeroFaces)
{
    // Regression test for the untrimmed fuselage face naming.
    //
    // The guides model produces an untrimmed loft with several aerodynamic faces
    // (one per guide-curve sector) followed by the Front/Rear cap faces. The old
    // SetFaceTraitsUntrimmed assumed a single aero face and cycled the cap names
    // (loftName, "symmetry", "Front", "Rear") over every remaining face, which
    // mislabeled aero faces as Front/Rear/symmetry and gave the caps wrong names.
    //
    // Buggy output was: SimpleFuselage, Front, Rear, SimpleFuselage, symmetry,
    // Front, Rear, SimpleFuselage  (3 aero / 1 symmetry / 2 Front / 2 Rear).
    // Correct layout is [aero...][symmetry?][Front][Rear]: 6 aero, 0 symmetry,
    // 1 Front, 1 Rear.
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    tigl::CCPACSFuselage& fuselage = config.GetFuselage(1);
    PNamedShape loft               = fuselage.GetUntrimmedLoft();
    ASSERT_TRUE(loft != nullptr);

    const std::string aeroName = loft->Name();

    int nAero = 0, nSymmetry = 0, nFront = 0, nRear = 0;
    for (int i = 0; i < loft->GetFaceCount(); ++i) {
        const std::string name = loft->GetFaceTraits(i).Name();
        if (name == "Front") {
            ++nFront;
        }
        else if (name == "Rear") {
            ++nRear;
        }
        else if (name == "symmetry") {
            ++nSymmetry;
        }
        else if (name == aeroName) {
            ++nAero;
        }
    }

    EXPECT_EQ(nAero, 6);
    EXPECT_EQ(nSymmetry, 0);
    EXPECT_EQ(nFront, 1);
    EXPECT_EQ(nRear, 1);
    EXPECT_EQ(loft->GetFaceCount(), 8);
}

TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesInvalidInput)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    //add invalid element
    tixiCreateElementAtIndex(tixiHandle2, "/cpacs/vehicles/profiles/fuselageProfiles", "fuselageProfile", 1);
    tixiCreateElement(tixiHandle2,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "invalidType");
    tixiAddTextAttribute(tixiHandle2,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "uID", "std2");

    // change uid of one segment to invalid profile type
    tixiUpdateTextElement(tixiHandle2, "/cpacs/vehicles/aircraft/model/fuselages/fuselage[1]/sections/section[1]/elements/element[1]/profileUID", "std2");
    tiglOpenCPACSConfiguration(tixiHandle2, "", &tiglHandle2);
    tigl::CCPACSConfiguration& config1         = manager.GetConfiguration(tiglHandle2);

    // fuselage cannot be build with invalid profile
    ASSERT_THROW(config1.GetFuselage(1).GetLoft(),tigl::CTiglError);
}


TEST(FuselageStandardProfileSuperEllipse_kinks, issue_1094)
{
    const char* filename = "TestData/superellipses_kink.xml";
    ReturnCode tixiRet;
    TiglReturnCode tiglRet;

    TixiDocumentHandle tiglHandle = -1;
    TiglCPACSConfigurationHandle tixiHandle = -1;

    tixiRet = tixiOpenDocument(filename, &tixiHandle);
    ASSERT_TRUE (tixiRet == SUCCESS);
    tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
    ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);

    // check number of faces. It should be Front, Rear and additionally four faces, one face per quadrant.
    // If there are additional kinks, there are more faces
    auto fuselage = config.GetFuselage(1).GetTrimmedLoft();
    int face_count = 0;
    for (int i=0; i < fuselage->GetFaceCount(); ++i) {
        if (fuselage->GetFaceTraits(i).Name() != "Front" && fuselage->GetFaceTraits(i).Name() != "Rear") {
            face_count++;
        }
    }
    ASSERT_EQ(face_count, 4);

    ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
    ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
    tiglHandle = -1;
    tixiHandle = -1;
}