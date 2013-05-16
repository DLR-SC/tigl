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
* @brief Tests for testing behavior of the TIGL intersection calculation routines.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#include "CTiglIntersectionCalculation.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWing.h"
#include "CCPACSFuselage.h"

class TiglIntersection : public ::testing::Test {
 protected:
  virtual void SetUp() {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Cpacs2Test", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
  }

  virtual void TearDown() {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
  }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};


/**
* Tests 
*/
TEST_F(TiglIntersection, tiglIntersection_FuselageWingIntersects)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSFuselage& fuselage = config.GetFuselage(1);

    TopoDS_Shape& wingShape = wing.GetLoft();
    TopoDS_Shape& fuselageShape = fuselage.GetLoft();

    tigl::CTiglIntersectionCalculation iCalc(fuselageShape, wingShape);
    
    ASSERT_EQ(1, iCalc.GetNumWires());
    
    // rough check of the leading edge
    gp_Pnt point = iCalc.GetPoint(0.5,1);
    ASSERT_NEAR(0.0, point.X(), 1e-3);
    ASSERT_NEAR(0.5, point.Y(), 1e-3);
    ASSERT_NEAR(0.0, point.Z(), 1e-2);
}


