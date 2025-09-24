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
#include "CCPACSLeadingEdgeDevice.h"
#include "CCPACSWing.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"

#include "TopoDS_Face.hxx"
#include "TopExp_Explorer.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "BRepLProp.hxx"
#include "BRepLProp_SLProps.hxx"
#include "BRepTools.hxx"
#include "GeomLProp_SLProps.hxx"
#include "gp_Pln.hxx"
#include "Geom_Plane.hxx"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"

using namespace std;

/******************************************************************************/

class TiglTrailingEdgeDevice : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/CPACS_30_aircraft_flaps.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE(tixiRet == SUCCESS);
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

    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle TiglTrailingEdgeDevice::tixiHandle           = 0;
TiglCPACSConfigurationHandle TiglTrailingEdgeDevice::tiglHandle = 0;

class TiglTrailingEdgeDeviceSimple : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest-flaps.cpacs.xml";
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

    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle TiglTrailingEdgeDeviceSimple::tixiHandle           = 0;
TiglCPACSConfigurationHandle TiglTrailingEdgeDeviceSimple::tiglHandle = 0;
//#########################################################################################################

TEST_F(TiglTrailingEdgeDevice, tiglGetControlSurfaceCount)
{
    int numDevicesPtr = 0;

    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetControlSurfaceCount(tiglHandle, "D150_VAMP_W1_CompSeg1", NULL));
    ASSERT_EQ(TIGL_UID_ERROR, tiglGetControlSurfaceCount(tiglHandle, "rmplstlzchn", &numDevicesPtr));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglGetControlSurfaceCount(-1, "D150_VAMP_W1_CompSeg1", &numDevicesPtr));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetControlSurfaceCount(tiglHandle, "D150_VAMP_W1_CompSeg1", &numDevicesPtr));
    ASSERT_EQ(numDevicesPtr, 6);
}

TEST_F(TiglTrailingEdgeDevice, tiglGetTrailingEdgeDeviceUID)
{
    char* uid1 = nullptr;
    char* uid2 = nullptr;
    char* uid3 = nullptr;
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetTrailingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 1, NULL));
    ASSERT_EQ(TIGL_UID_ERROR, tiglGetTrailingEdgeDeviceUID(tiglHandle, "rmplstlzchn", 1, &uid1));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglGetTrailingEdgeDeviceUID(-1, "D150_VAMP_W1_CompSeg1", 1, &uid1));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglGetTrailingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 0, &uid1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetTrailingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 1, &uid1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetTrailingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 2, &uid2));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetTrailingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 3, &uid3));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglGetTrailingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 4, &uid3));
    ASSERT_STREQ(uid1, "D150_VAMP_W1_CompSeg1_innerFlap");
    ASSERT_STREQ(uid2, "D150_VAMP_W1_CompSeg1_outerFlap");
    ASSERT_STREQ(uid3, "D150_VAMP_W1_CompSeg1_aileron");
}

TEST_F(TiglTrailingEdgeDevice, tiglGetLeadingEdgeDeviceUID)
{
    char* uid1 = nullptr;
    char* uid2 = nullptr;
    char* uid3 = nullptr;
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetLeadingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 1, NULL));
    ASSERT_EQ(TIGL_UID_ERROR, tiglGetLeadingEdgeDeviceUID(tiglHandle, "rmplstlzchn", 1, &uid1));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglGetLeadingEdgeDeviceUID(-1, "D150_VAMP_W1_CompSeg1", 1, &uid1));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglGetLeadingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 0, &uid1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetLeadingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 1, &uid1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetLeadingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 2, &uid2));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetLeadingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 3, &uid3));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglGetLeadingEdgeDeviceUID(tiglHandle, "D150_VAMP_W1_CompSeg1", 4, &uid3));
    ASSERT_STREQ(uid1, "D150_VAMP_W1_CompSeg1_innerLED");
    ASSERT_STREQ(uid2, "D150_VAMP_W1_CompSeg1_outerLED1");
    ASSERT_STREQ(uid3, "D150_VAMP_W1_CompSeg1_outerLED2");
}

TEST_F(TiglTrailingEdgeDevice, tiglGetTrailingEdgeDeviceType)
{
    TiglControlSurfaceType type1, type2, type3;
    ASSERT_EQ(TIGL_UID_ERROR, tiglGetTrailingEdgeDeviceType(tiglHandle, "rmplstlzchn", &type1));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglGetTrailingEdgeDeviceType(-1, "D150_VAMP_W1_CompSeg1_innerFlap", &type1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetTrailingEdgeDeviceType(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", &type1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetTrailingEdgeDeviceType(tiglHandle, "D150_VAMP_W1_CompSeg1_outerFlap", &type2));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetTrailingEdgeDeviceType(tiglHandle, "D150_VAMP_W1_CompSeg1_aileron", &type3));
    ASSERT_EQ(TRAILING_EDGE_DEVICE, type1);
    ASSERT_EQ(TRAILING_EDGE_DEVICE, type2);
    ASSERT_EQ(TRAILING_EDGE_DEVICE, type3);
}

TEST_F(TiglTrailingEdgeDevice, tiglTrailingEdgeDeviceGetMinimumControlParameter)
{
    double controlParm1, controlParm2, controlParm3;
    ASSERT_EQ(TIGL_UID_ERROR,
              tiglTrailingEdgeDeviceGetMinimumControlParameter(tiglHandle, "rmplstlzchn", &controlParm1));
    ASSERT_EQ(TIGL_NOT_FOUND,
              tiglTrailingEdgeDeviceGetMinimumControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglTrailingEdgeDeviceGetMinimumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglTrailingEdgeDeviceGetMinimumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_outerFlap", &controlParm2));
    ASSERT_EQ(TIGL_SUCCESS, tiglTrailingEdgeDeviceGetMinimumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_aileron", &controlParm3));
    ASSERT_EQ(0, controlParm1);
    ASSERT_EQ(0, controlParm2);
    ASSERT_EQ(-1, controlParm3);
}

TEST_F(TiglTrailingEdgeDevice, tiglTrailingEdgeDeviceGetMaximumControlParameter)
{
    double controlParm1, controlParm2, controlParm3;
    ASSERT_EQ(TIGL_UID_ERROR,
              tiglTrailingEdgeDeviceGetMaximumControlParameter(tiglHandle, "rmplstlzchn", &controlParm1));
    ASSERT_EQ(TIGL_NOT_FOUND,
              tiglTrailingEdgeDeviceGetMaximumControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglTrailingEdgeDeviceGetMaximumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglTrailingEdgeDeviceGetMaximumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_outerFlap", &controlParm2));
    ASSERT_EQ(TIGL_SUCCESS, tiglTrailingEdgeDeviceGetMaximumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_aileron", &controlParm3));
    ASSERT_EQ(1, controlParm1);
    ASSERT_EQ(1, controlParm2);
    ASSERT_EQ(1, controlParm3);
}

TEST_F(TiglTrailingEdgeDevice, tiglTrailingEdgeDeviceGetAndSetControlParameter)
{
    double controlParm1, controlParm2, controlParm3;
    ASSERT_EQ(TIGL_UID_ERROR, tiglTrailingEdgeDeviceGetControlParameter(tiglHandle, "rmplstlzchn", &controlParm1));
    ASSERT_EQ(TIGL_NOT_FOUND,
              tiglTrailingEdgeDeviceGetControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_outerFlap", &controlParm2));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_aileron", &controlParm3));
    ASSERT_EQ(0, controlParm1);
    ASSERT_EQ(0, controlParm2);
    ASSERT_EQ(0, controlParm3);

    double controlParm;

    // error codes for set control parameter
    ASSERT_EQ(TIGL_UID_ERROR, tiglTrailingEdgeDeviceSetControlParameter(tiglHandle, "rmplstlzchn", 5.0));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglTrailingEdgeDeviceSetControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerFlap", 5.0));

    // clamp to max/min control parameter
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceSetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", 5.0));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm));
    ASSERT_EQ(1, controlParm);

    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceSetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", -5.0));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm));
    ASSERT_EQ(0, controlParm);

    // value between min and max
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceSetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", 0.33));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglTrailingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerFlap", &controlParm));
    ASSERT_NEAR(0.33, controlParm, 1e-10);
}

TEST_F(TiglTrailingEdgeDevice, CPACSTrailingEdgeDevices)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing                    = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment =
        static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::generated::CPACSTrailingEdgeDevices& devices =
        *componentSegment.GetControlSurfaces()->GetTrailingEdgeDevices();

    // trailing edge device count
    ASSERT_EQ(devices.GetTrailingEdgeDeviceCount(), 3);

    // get trailing edge device by index
    ASSERT_EQ(devices.GetTrailingEdgeDevice(1).GetUID(), "D150_VAMP_W1_CompSeg1_innerFlap");
    ASSERT_EQ(devices.GetTrailingEdgeDevice(2).GetUID(), "D150_VAMP_W1_CompSeg1_outerFlap");
    ASSERT_EQ(devices.GetTrailingEdgeDevice(3).GetUID(), "D150_VAMP_W1_CompSeg1_aileron");
    ASSERT_THROW(devices.GetTrailingEdgeDevice(-1), tigl::CTiglError);
    ASSERT_THROW(devices.GetTrailingEdgeDevice(4), tigl::CTiglError);

    // get index of trailing edge device by uid
    ASSERT_EQ(devices.GetTrailingEdgeDeviceIndex("D150_VAMP_W1_CompSeg1_innerFlap"), 1);
    ASSERT_EQ(devices.GetTrailingEdgeDeviceIndex("D150_VAMP_W1_CompSeg1_outerFlap"), 2);
    ASSERT_EQ(devices.GetTrailingEdgeDeviceIndex("D150_VAMP_W1_CompSeg1_aileron"), 3);
    ASSERT_THROW(devices.GetTrailingEdgeDeviceIndex("WrongUID"), tigl::CTiglError);

    // get trailing edge device by uid
    ASSERT_EQ(devices.GetTrailingEdgeDevice("D150_VAMP_W1_CompSeg1_innerFlap").GetName(),
              devices.GetTrailingEdgeDevice(1).GetName());
    ASSERT_EQ(devices.GetTrailingEdgeDevice("D150_VAMP_W1_CompSeg1_outerFlap").GetName(),
              devices.GetTrailingEdgeDevice(2).GetName());
    ASSERT_EQ(devices.GetTrailingEdgeDevice("D150_VAMP_W1_CompSeg1_aileron").GetName(),
              devices.GetTrailingEdgeDevice(3).GetName());
    ASSERT_THROW(devices.GetTrailingEdgeDevice("WrongUID"), tigl::CTiglError);
}

TEST_F(TiglTrailingEdgeDeviceSimple, setControlParameterAndExport)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglTrailingEdgeDeviceSetControlParameter(tiglHandle, "FlapInner", 1.0));
    tiglExportConfiguration(tiglHandle, "TestData/export/simpletest-flaps.stp", TIGL_TRUE, 0.0);
}

TEST_F(TiglTrailingEdgeDeviceSimple, bug_780_reference_segment)
{
    // a test for Github issue #780, the flap should extend from eta 0.25
    // to eta 0.75 relative to the outer segment, not the component segment
    auto& manager          = tigl::CCPACSConfigurationManager::GetInstance();
    auto& config           = manager.GetConfiguration(tiglHandle);
    auto& wing             = config.GetWing(1);
    auto& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    auto& devices          = *componentSegment.GetControlSurfaces()->GetTrailingEdgeDevices();
    auto& outer_flap       = devices.GetTrailingEdgeDevice(2);
    auto flap_shape        = outer_flap.GetFlapShape()->Shape();

    //To Do: With OpenCascade 7.x we can calculate a tight bounding box, instead of checking for
    //the extremal points along the y-axis
    gp_Pnt min, max;
    GetMinMaxPoint(flap_shape, gp_Vec(0., 1., 0.), min, max);
    EXPECT_NEAR(min.Y(), 1.25, 1e-2);
    EXPECT_NEAR(max.Y(), 1.75, 1e-2);
}
