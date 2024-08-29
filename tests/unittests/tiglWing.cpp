/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
* @brief Tests for testing non classified wing functions.
*/

#include "BRepCheck_Analyzer.hxx"
#include "test.h" // Brings in the GTest framework
#include "testUtils.h"
#include "tigl.h"
#include <string.h>
#include <CCPACSConfigurationManager.h>
#include <TiglWingHelperFunctions.h>

/******************************************************************************/

class TiglWing : public ::testing::Test 
{
protected:
    static void SetUpTestCase() 
    {
        const char* filename = "TestData/CPACS_30_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
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


TixiDocumentHandle TiglWing::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglWing::tiglHandle = 0;


class WingSimple : public ::testing::Test 
{
protected:
    static void SetUpTestCase() 
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglSimpleWingHandle = -1;
        tixiSimpleWingHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiSimpleWingHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiSimpleWingHandle, "Cpacs2Test", &tiglSimpleWingHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase() 
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglSimpleWingHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiSimpleWingHandle) == SUCCESS);
        tiglSimpleWingHandle = -1;
        tixiSimpleWingHandle = -1;
    }

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiSimpleWingHandle;
    static TiglCPACSConfigurationHandle tiglSimpleWingHandle;
};

TixiDocumentHandle WingSimple::tixiSimpleWingHandle = 0;
TiglCPACSConfigurationHandle WingSimple::tiglSimpleWingHandle = 0;

/******************************************************************************/

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidHandle)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(-1, 1, 1, 1, &namePtr) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetProfileName with invalid wing index.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidWing)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, -1, 1, 1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with invalid section index.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidSection)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, -1, 1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with invalid element index.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidElement)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, 1, -1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with null pointer argument.
*/
TEST_F(TiglWing, tiglWingGetProfileName_nullPointerArgument)
{
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, 1, 1, 0) == TIGL_NULL_POINTER);
}

/**
* Tests tiglWingGetProfileName with null pointer argument.
*/
TEST_F(TiglWing, tiglWingGetProfileName_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, 1, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_STREQ( "NameD150_VAMP_W_SupCritProf1", namePtr);
}


/**
* Tests tiglWingGetUID 
*/
TEST_F(TiglWing, tiglWingGetUID_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetUID(tiglHandle, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_STREQ( "D150_VAMP_W1", namePtr);
}


/**
* Tests tiglWingGetSegmentUID 
*/
TEST_F(TiglWing, tiglWingGetSegmentUID_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetSegmentUID(tiglHandle, 1, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_STREQ( "D150_VAMP_W1_Seg1", namePtr);
}


/**
* Tests tiglWingGetSegmentUID 
*/
TEST_F(TiglWing, tiglWingGetSegmentUID_indexError)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetSegmentUID(tiglHandle, 1, -1, &namePtr) == TIGL_INDEX_ERROR);
}



/**
* Tests tiglWingGetSectionUID 
*/
TEST_F(TiglWing, tiglWingGetSectionUID_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetSectionUID(tiglHandle, 1, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_STREQ( "D150_VAMP_W1_Sec1", namePtr);
}

/**
* Tests for tiglWingGetIndex
*/
TEST_F(TiglWing, tiglWingGetIndex_success)
{
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "D150_VAMP_W1", &wingIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(wingIndex == 1);

    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "D150_VAMP_HL1", &wingIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(wingIndex == 2);

    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "D150_VAMP_SL1", &wingIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(wingIndex == 3);
}

TEST_F(TiglWing, tiglWingGetIndex_wrongUID)
{
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "invalid_uid", &wingIndex) == TIGL_UID_ERROR);
    ASSERT_TRUE(wingIndex == -1);
}

TEST_F(TiglWing, tiglWingGetIndex_nullPtr)
{
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, NULL , &wingIndex) == TIGL_NULL_POINTER);
}


TEST_F(TiglWing, tiglWingGetIndex_indexNullPtr)
{
    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "D150_VAMP_SL1" , NULL) == TIGL_NULL_POINTER);
}

TEST_F(TiglWing, tiglWingGetIndex_wrongHandle)
{
    TiglCPACSConfigurationHandle myWrongHandle = -1234;
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(myWrongHandle, "D150_VAMP_W1_Seg1" , &wingIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetSegmentIndex
*/
TEST_F(TiglWing, tiglWingGetSegmentIndex_success)
{
    int segmentIndex = 0;
    int wingIndex = 0;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetSegmentIndex(tiglHandle, "D150_VAMP_W1_Seg1", &segmentIndex, &wingIndex));
    ASSERT_EQ(1, segmentIndex);
    ASSERT_EQ(1, wingIndex);

    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetSegmentIndex(tiglHandle, "D150_VAMP_W1_Seg2", &segmentIndex, &wingIndex));
    ASSERT_EQ(2, segmentIndex);
    ASSERT_EQ(1, wingIndex);

    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetSegmentIndex(tiglHandle, "D150_VAMP_W1_Seg3", &segmentIndex, &wingIndex));
    ASSERT_EQ(3, segmentIndex);
    ASSERT_EQ(1, wingIndex);
}

TEST_F(TiglWing, tiglWingGetSegmentIndex_wrongUID)
{
    int segmentIndex = 0;
    int wingIndex    = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, "invalid_seg_name", &segmentIndex, &wingIndex) == TIGL_UID_ERROR);
}

TEST_F(TiglWing, tiglWingGetSegmentIndex_nullPtr)
{
    int segmentIndex = 0;
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, NULL, &segmentIndex, &wingIndex) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, "D150_VAMP_W1_Seg1", NULL, &wingIndex) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, "D150_VAMP_W1_Seg1", &segmentIndex, NULL) == TIGL_NULL_POINTER);
}

TEST_F(TiglWing, tiglWingGetSegmentIndex_wrongHandle)
{
    TiglCPACSConfigurationHandle myWrongHandle = -1234;
    int segmentIndex = 0;
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(myWrongHandle, "D150_VAMP_W1_Seg1", &segmentIndex, &wingIndex) == TIGL_NOT_FOUND);
}

TEST_F(TiglWing, tiglWingGetSpanVTP)
{
    double span = 0.;
    tiglWingGetSpan(tiglHandle, "D150_VAMP_SL1", &span);
    ASSERT_LT(span, 5.9);
    ASSERT_GT(span, 5.8);
}

/**
 * This test replicates issue 849, which showed a strange
 * behaviour while invalidating the wing
 */
TEST_F(TiglWing, bug849)

{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    auto& vtp = config.GetWing(3);

    // somehow, this generates something
    // removing this line maade the test succeed
    // otherwise, the test failed
    vtp.GetWingspan();

    // which is not correctly invalidated now
    // note, that we actually don't change anything
    vtp.SetSymmetryAxis(vtp.GetSymmetryAxis());

    auto span = vtp.GetWingspan();
    EXPECT_NEAR(5.9, span, 0.1); // the reported wing span is 0.62 instead of 5.4
}

/// Regression reported by issue #827
TEST_F(TiglWing, tiglGetAspectRatio)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    const auto& wing = config.GetWing(1);

    // The expected value was provided by the issue report
    // The exact value also depends on the opencascade version
    // used so we are using a pretty large tolerance here
    EXPECT_NEAR(9.4031, wing.GetAspectRatio(), 1e-2);

    const auto& vtp = config.GetWing(3);

    // This value was roughly estimated
    EXPECT_NEAR(3.2, vtp.GetAspectRatio(), 1e-1);

}

TEST_F(TiglWing, spanDirection)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    auto& wing = config.GetWing(1);
    auto& vtp = config.GetWing(3);

    auto axis = tigl::winghelper::GetWingSpanAxis(wing);
    EXPECT_EQ(TIGL_Y_AXIS, axis);


    axis = tigl::winghelper::GetWingSpanAxis(vtp);
    EXPECT_EQ(TIGL_Z_AXIS, axis);

    auto wingSymAx = wing.GetSymmetryAxis();
    auto vtpSymAx = vtp.GetSymmetryAxis();

    wing.SetSymmetryAxis(TIGL_NO_SYMMETRY);
    vtp.SetSymmetryAxis(TIGL_X_Y_PLANE);

    axis = tigl::winghelper::GetWingSpanAxis(wing);
    EXPECT_EQ(TIGL_Y_AXIS, axis);

    axis = tigl::winghelper::GetWingSpanAxis(vtp);
    EXPECT_EQ(TIGL_Z_AXIS, axis);

    // we need to reset the axes, since the cpacs handle
    // is static for all tests
    wing.SetSymmetryAxis(wingSymAx);
    vtp.SetSymmetryAxis(vtpSymAx);
}

TEST_F(TiglWing, depthDirection)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    auto& wing = config.GetWing(1);
    auto& vtp = config.GetWing(3);

    auto axis = tigl::winghelper::GetWingDepthAxis(wing);
    EXPECT_EQ(TIGL_X_AXIS, axis);


    axis = tigl::winghelper::GetWingDepthAxis(vtp);
    EXPECT_EQ(TIGL_X_AXIS, axis);

    auto wingSymAx = wing.GetSymmetryAxis();
    auto vtpSymAx = vtp.GetSymmetryAxis();

    wing.SetSymmetryAxis(TIGL_NO_SYMMETRY);
    vtp.SetSymmetryAxis(TIGL_X_Y_PLANE);

    axis = tigl::winghelper::GetWingDepthAxis(wing);
    EXPECT_EQ(TIGL_X_AXIS, axis);

    axis = tigl::winghelper::GetWingDepthAxis(vtp);
    EXPECT_EQ(TIGL_X_AXIS, axis);

    // we need to reset the axes, since the cpacs handle
    // is static for all tests
    wing.SetSymmetryAxis(wingSymAx);
    vtp.SetSymmetryAxis(vtpSymAx);
}

TEST_F(TiglWing, testGetWingWithCutOuts)
{
    const char* filename = "TestData/simpletest.cpacs.xml";
    ReturnCode tixiRetX;
    TiglReturnCode tiglRetX;

    TixiDocumentHandle tiglHandleX = -1;
    TiglCPACSConfigurationHandle tixiHandleX = -1;

    tixiRetX = tixiOpenDocument(filename, &tixiHandleX);
    ASSERT_TRUE (tixiRetX == SUCCESS);
    tiglRetX = tiglOpenCPACSConfiguration(tixiHandleX, "", &tiglHandleX);
    ASSERT_TRUE(tiglRetX == TIGL_SUCCESS);

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandleX);
    tigl::CTiglUIDManager& uidmgr = config.GetUIDManager();
    auto& wing = uidmgr.ResolveObject<tigl::CCPACSWing>("Wing");

    // Check for warning if no control surfaces are defined
      { // Scope to destroy object of type CaptureTiGLLog and therefore reset console verbosity
          CaptureTiGLLog t{TILOG_WARNING};
          wing.GetLoftWithCutouts();
          auto logOutput = t.log();

          std::string comparisonString = "No control devices defined, GetLoftWithCutOuts() will return a clean shape.";
          ASSERT_TRUE((logOutput.find(comparisonString)) != std::string::npos);
      } // scope

    ASSERT_TRUE(BRepCheck_Analyzer(wing.GetLoftWithCutouts()).IsValid());

}

TEST_F(WingSimple, wingGetMAC_success)
{
    double  c_1, x_1, y_1, z_1;
    tiglWingGetMAC(tiglSimpleWingHandle, "Wing", &c_1, &x_1, &y_1, &z_1);
    ASSERT_NEAR(19./21., c_1, 1e-7);
}

TEST_F(WingSimple, wingGetReferenceArea_success)
{
    double ref = 0.;
    tiglWingGetReferenceArea(tiglSimpleWingHandle, 1, TIGL_X_Y_PLANE, &ref);
    ASSERT_NEAR(1.75, ref, 1e-7);

}

TEST_F(WingSimple, wingGetWettedArea_success)
{
    double ref = 0.;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetWettedArea(tiglSimpleWingHandle, "Wing", &ref));
}
