/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
* @brief Tests for testing export functions.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#include "CTiglTriangularizer.h"
#include "CTiglExportCollada.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWing.h"


/******************************************************************************/

class tiglExport : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/CPACS_21_D150.xml";
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

    virtual void SetUp() {}
    virtual void TearDown() {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle tiglExport::tixiHandle = 0;
TiglCPACSConfigurationHandle tiglExport::tiglHandle = 0;

class tiglExportSimple : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglSimpleHandle = -1;
        tixiSimpleHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiSimpleHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiSimpleHandle, "", &tiglSimpleHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglSimpleHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiSimpleHandle) == SUCCESS);
        tiglSimpleHandle = -1;
        tixiSimpleHandle = -1;
    }

    virtual void SetUp() {}
    virtual void TearDown() {}


    static TixiDocumentHandle           tixiSimpleHandle;
    static TiglCPACSConfigurationHandle tiglSimpleHandle;
};

TixiDocumentHandle tiglExportSimple::tixiSimpleHandle = 0;
TiglCPACSConfigurationHandle tiglExportSimple::tiglSimpleHandle = 0;


class tiglExportRectangularWing : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simple_rectangle_compseg.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglRectangularWingHandle = -1;
        tixiRectangularWingHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiRectangularWingHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiRectangularWingHandle, "", &tiglRectangularWingHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglRectangularWingHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiRectangularWingHandle) == SUCCESS);
        tiglRectangularWingHandle = -1;
        tixiRectangularWingHandle = -1;
    }

    virtual void SetUp() {}
    virtual void TearDown() {}


    static TixiDocumentHandle           tixiRectangularWingHandle;
    static TiglCPACSConfigurationHandle tiglRectangularWingHandle;
};

TixiDocumentHandle tiglExportRectangularWing::tixiRectangularWingHandle = 0;
TiglCPACSConfigurationHandle tiglExportRectangularWing::tiglRectangularWingHandle = 0;



/******************************************************************************/

//void tiglxEportMeshedWingVTK_small_example()
//{
//    const BRepPrimAPI_MakeCylinder cone(/* radius */ 2.0, /* height */ 8.0);
//    const CTiglExportVtk writer(config);
//    writer.ExportMeshedWingVTK
//}

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
TEST_F(tiglExport, export_meshed_wing_success)
{
    const char* vtkWingFilename = "TestData/export/D150modelID_wing1.vtp";
    ASSERT_TRUE(tiglExportMeshedWingVTKByIndex(tiglHandle, 1, vtkWingFilename, 0.01) == TIGL_SUCCESS);
}

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
TEST_F(tiglExport, export_meshed_wing_simple_success)
{
    const char* vtkWingFilename = "TestData/export/D150modelID_wing1_simple.vtp";
    ASSERT_TRUE(tiglExportMeshedWingVTKSimpleByUID(tiglHandle, "D150_VAMP_W1", vtkWingFilename, 0.01) == TIGL_SUCCESS);
}

TEST_F(tiglExport, export_meshed_fuselage_success)
{
    const char* vtkFuselageFilename = "TestData/export/D150modelID_fuselage1.vtp";
    ASSERT_TRUE(tiglExportMeshedFuselageVTKSimpleByUID(tiglHandle, "D150_VAMP_FL1", vtkFuselageFilename, 0.03) == TIGL_SUCCESS);
}

TEST_F(tiglExport, export_fuselage_collada_success)
{
    const char* colladaFuselageFilename = "TestData/export/D150modelID_fuselage1.dae";
    ASSERT_TRUE(tiglExportFuselageColladaByUID(tiglHandle, "D150_VAMP_FL1", colladaFuselageFilename, 0.01) == TIGL_SUCCESS);
}

TEST_F(tiglExport, export_wing_collada_success)
{
    const char* colladaWing1Filename = "TestData/export/D150modelID_wing1.dae";
    ASSERT_TRUE(tiglExportWingColladaByUID(tiglHandle, "D150_VAMP_W1", colladaWing1Filename, 0.001) == TIGL_SUCCESS);
    const char* colladaWing2Filename = "TestData/export/D150modelID_wing2.dae";
    ASSERT_TRUE(tiglExportWingColladaByUID(tiglHandle, "D150_VAMP_HL1", colladaWing2Filename, 0.001) == TIGL_SUCCESS);
    const char* colladaWing3Filename = "TestData/export/D150modelID_wing3.dae";
    ASSERT_TRUE(tiglExportWingColladaByUID(tiglHandle, "D150_VAMP_SL1", colladaWing3Filename, 0.001) == TIGL_SUCCESS);
}

TEST_F(tiglExportSimple, export_wing_collada)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglSimpleHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    TiglReturnCode ret = tigl::CTiglExportCollada::write(wing.GetLoft(), "TestData/export/simpletest_wing.dae", 0.001);

    ASSERT_EQ(TIGL_SUCCESS, ret);
}


// check if face names were set correctly in the case with a trailing edge
TEST_F(tiglExportSimple, check_face_traits)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglExportIGES(tiglSimpleHandle,"TestData/export/simpletest.iges"));
    ASSERT_EQ(TIGL_SUCCESS, tiglExportFusedWingFuselageIGES(tiglSimpleHandle,"TestData/export/simpletest_fused.iges"));
}

// check if face names were set correctly in the case without a trailing edge
TEST_F(tiglExportRectangularWing, check_face_traits)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglExportIGES(tiglRectangularWingHandle,"TestData/export/rectangular_wing_test.iges"));
    ASSERT_EQ(TIGL_SUCCESS, tiglExportFusedWingFuselageIGES(tiglRectangularWingHandle,"TestData/export/rectangular_wing_test_fused.iges"));
}
