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
#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWing.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSConfigurationManager.h"


#include "TopoDS_Face.hxx"
#include "TopExp_Explorer.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "BRepLProp.hxx"
#include "BRepLProp_SLProps.hxx"
#include "BRepTools.hxx"
#include "GeomLProp_SLProps.hxx"
#include "gp_Pln.hxx"
#include "Geom_Plane.hxx"
#include "Handle_Geom_Plane.hxx"

using namespace std;

/******************************************************************************/

class TiglControlSurfaceDevice : public ::testing::Test {
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


TixiDocumentHandle TiglControlSurfaceDevice::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglControlSurfaceDevice::tiglHandle = 0;

//#########################################################################################################


TEST_F(TiglControlSurfaceDevice, getFaceAndWCSNormal)
{
    int compseg = 1;
    // now we have do use the internal interface as we currently have no public api for this
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

    for ( int j = 1; j < config.GetWingCount(); j ++ ) {

        tigl::CCPACSWing& wing = config.GetWing(j);
        tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(compseg);

        int controlSurfaceCount = segment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount();

        for ( int i = 1; i <= controlSurfaceCount; i++ ) {
            tigl::CCPACSControlSurfaceDevice &controlSurface = segment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceByID(i);
            TopoDS_Face face = controlSurface.getFace();

            // check if each TED-Face has the same normalvector as the WingComponentSegment
            TopoDS_Face aCurrentFace = face;
            Standard_Real umin, umax, vmin, vmax;
            BRepTools::UVBounds(aCurrentFace,umin, umax, vmin, vmax);
            Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aCurrentFace);
            GeomLProp_SLProps props(aSurface, umin, vmin,1, 0.01);

            gp_Vec normalTED = gp_Vec(props.Normal().XYZ());
            gp_Vec normalWCS = controlSurface.getNormalOfControlSurfaceDevice();

            ASSERT_NEAR(std::fabs(normalTED.X()),std::fabs(normalWCS.X()),1e-4);
            ASSERT_NEAR(std::fabs(normalTED.Y()),std::fabs(normalWCS.Y()),1e-4);
            ASSERT_NEAR(std::fabs(normalTED.Z()),std::fabs(normalWCS.Z()),1e-4);
        }
    }
}

TEST_F(TiglControlSurfaceDevice, getProjectedPoints)
{
    // check if all projected points are aligned in a plane.

    int compseg = 1;
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

    for ( int j = 1; j < config.GetWingCount(); j ++ ) {

        tigl::CCPACSWing& wing = config.GetWing(j);
        tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(compseg);

        int controlSurfaceCount = segment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount();

        for ( int i = 1; i <= controlSurfaceCount; i++ ) {
            tigl::CCPACSControlSurfaceDevice &controlSurface = segment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceByID(i);

            tigl::CCPACSControlSurfaceBorder outerBorder = controlSurface.getOuterShape().getOuterBorder();
            gp_Pnt point1 = controlSurface.getSegment()->GetPoint(outerBorder.getEtaLE(),outerBorder.getXsiLE());
            gp_Pnt point2 = controlSurface.getSegment()->GetPoint(outerBorder.getEtaTE(),outerBorder.getXsiTE());

            tigl::CCPACSControlSurfaceBorder innerBorder = controlSurface.getOuterShape().getInnerBorder();
            gp_Pnt point3 = controlSurface.getSegment()->GetPoint(innerBorder.getEtaLE(),innerBorder.getXsiLE());
            gp_Pnt point4 = controlSurface.getSegment()->GetPoint(innerBorder.getEtaTE(), innerBorder.getXsiTE());

            gp_Vec pp1,pp2,pp3,pp4;
            controlSurface.getProjectedPoints(point1,point2,point3,point4,pp1,pp2,pp3,pp4);

            gp_Pnt sv = gp_Pnt(pp1.XYZ());
            gp_Vec normal = (pp2 - pp1)^(pp3 - pp1);
            gp_Pln plane(sv,normal);
            ASSERT_EQ(plane.Contains(gp_Pnt(pp4.XYZ()),1e-5),Standard_Boolean(true));
        }
    }
}

