/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSWing.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSConfigurationManager.h"


#include "TopoDS_Face.hxx"
#include "TopExp_Explorer.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "BRepLProp.hxx"
#include "BRepLProp_SLProps.hxx"

using namespace std;

/******************************************************************************/

class TiglTrailingEdgeDevice : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
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

  static void TearDownTestCase() {
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


TixiDocumentHandle TiglTrailingEdgeDevice::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglTrailingEdgeDevice::tiglHandle = 0;

//#########################################################################################################


TEST_F(TiglTrailingEdgeDevice, getFace)
{
    int compseg = 1;
    // now we have do use the internal interface as we currently have no public api for this
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

    for ( int j = 0; j < 3; j ++ ) {

        tigl::CCPACSWing& wing = config.GetWing(j+1);
        tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(compseg);

        int trailingEdgeCount = segment.getControlSurfaces().getTrailingEdgeDevices()->getTrailingEdgeDeviceCount();

        for ( int i = 1; i <= trailingEdgeCount; i++ ) {
            tigl::CCPACSTrailingEdgeDevice &trailingEdge = segment.getControlSurfaces().getTrailingEdgeDevices()->getTrailingEdgeDeviceByID(i);
            TopoDS_Face boundary_face = trailingEdge.getFace();

            BRepAdaptor_Surface surf_adapt_from_bound_face(boundary_face);
            BRepLProp_SLProps sl_props_from_bound_face(surf_adapt_from_bound_face, 1,1,1,1);
            gp_Dir normal_of_bound_face = sl_props_from_bound_face.Normal();

            gp_Vec direction(normal_of_bound_face.XYZ());
            direction.Normalize();

            if ( j == 0 ) {
                ASSERT_NEAR(1.0,fabs(direction.Z()),1e-7);
            } else if ( j == 1 ){
                ASSERT_NEAR(1.0,fabs(direction.Z()),1e-7);
            } else if ( j == 2 ){
                ASSERT_NEAR(1.0,fabs(direction.Y()),1e-7);
            }
        }
    }
}
