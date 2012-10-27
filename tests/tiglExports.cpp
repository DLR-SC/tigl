/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

class tiglExport : public ::testing::Test {
 protected:
  virtual void SetUp() {
    char* filename = "TestData/CPACS_21_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE( tixiRet == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE (tiglRet == TIGL_SUCCESS);

  }

  virtual void TearDown() {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_TRUE(tixiCloseDocument(tixiHandle));
        tiglHandle = -1;
        tixiHandle = -1;
  }
};



/******************************************************************************/

//void tiglxEportMeshedWingVTK_small_example(void)
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
    char* vtkWingFilename = "TestData/export/D150modelID_wing1.vtp";
    char* vtkGeometryFilename = "TestData/export/D150modelID_geometry.vtp";
    ASSERT_TRUE(tiglExportMeshedWingVTKByIndex(tiglHandle, 1, vtkWingFilename, 0.01) == TIGL_SUCCESS);
}

TEST_F(tiglExport, export_meshed_fuselage_success)
{
    char* vtkFuselageFilename = "TestData/export/D150modelID_fuselage1.vtp";
    ASSERT_TRUE(tiglExportMeshedFuselageVTKSimpleByUID(tiglHandle, "D150_VAMP_FL1", vtkFuselageFilename, 0.1) == TIGL_SUCCESS);
}
