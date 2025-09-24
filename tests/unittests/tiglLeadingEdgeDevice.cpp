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

class TiglLeadingEdgeDevice : public ::testing::Test
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

TixiDocumentHandle TiglLeadingEdgeDevice::tixiHandle           = 0;
TiglCPACSConfigurationHandle TiglLeadingEdgeDevice::tiglHandle = 0;

class TiglLeadingEdgeDeviceSimple : public ::testing::Test
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

TixiDocumentHandle TiglLeadingEdgeDeviceSimple::tixiHandle           = 0;
TiglCPACSConfigurationHandle TiglLeadingEdgeDeviceSimple::tiglHandle = 0;
//#########################################################################################################

TEST_F(TiglLeadingEdgeDevice, tiglGetControlSurfaceCount)
{
    int numDevicesPtr = 0;

    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetControlSurfaceCount(tiglHandle, "D150_VAMP_W1_CompSeg1", NULL));
    ASSERT_EQ(TIGL_UID_ERROR, tiglGetControlSurfaceCount(tiglHandle, "rmplstlzchn", &numDevicesPtr));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglGetControlSurfaceCount(-1, "D150_VAMP_W1_CompSeg1", &numDevicesPtr));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetControlSurfaceCount(tiglHandle, "D150_VAMP_W1_CompSeg1", &numDevicesPtr));
    ASSERT_EQ(numDevicesPtr, 6);
}

TEST_F(TiglLeadingEdgeDevice, tiglGetLeadingEdgeDeviceUID)
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

TEST_F(TiglLeadingEdgeDevice, tiglGetLeadingEdgeDeviceType)
{
    TiglControlSurfaceType type1, type2, type3;
    ASSERT_EQ(TIGL_UID_ERROR, tiglGetLeadingEdgeDeviceType(tiglHandle, "rmplstlzchn", &type1));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglGetLeadingEdgeDeviceType(-1, "D150_VAMP_W1_CompSeg1_innerLED", &type1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetLeadingEdgeDeviceType(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", &type1));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetLeadingEdgeDeviceType(tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED1", &type2));
    ASSERT_EQ(TIGL_SUCCESS, tiglGetLeadingEdgeDeviceType(tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED2", &type3));
    ASSERT_EQ(LEADING_EDGE_DEVICE, type1);
    ASSERT_EQ(LEADING_EDGE_DEVICE, type2);
    ASSERT_EQ(LEADING_EDGE_DEVICE, type3);
}

TEST_F(TiglLeadingEdgeDevice, tiglLeadingEdgeDeviceGetMinimumControlParameter)
{
    double controlParm1, controlParm2, controlParm3;
    ASSERT_EQ(TIGL_UID_ERROR,
              tiglLeadingEdgeDeviceGetMinimumControlParameter(tiglHandle, "rmplstlzchn", &controlParm1));
    ASSERT_EQ(TIGL_NOT_FOUND,
              tiglLeadingEdgeDeviceGetMinimumControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglLeadingEdgeDeviceGetMinimumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglLeadingEdgeDeviceGetMinimumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED1", &controlParm2));
    ASSERT_EQ(TIGL_SUCCESS, tiglLeadingEdgeDeviceGetMinimumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED2", &controlParm3));
    ASSERT_EQ(0, controlParm1);
    ASSERT_EQ(0, controlParm2);
    ASSERT_EQ(0, controlParm3);
}

TEST_F(TiglLeadingEdgeDevice, tiglLeadingEdgeDeviceGetMaximumControlParameter)
{
    double controlParm1, controlParm2, controlParm3;
    ASSERT_EQ(TIGL_UID_ERROR,
              tiglLeadingEdgeDeviceGetMaximumControlParameter(tiglHandle, "rmplstlzchn", &controlParm1));
    ASSERT_EQ(TIGL_NOT_FOUND,
              tiglLeadingEdgeDeviceGetMaximumControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglLeadingEdgeDeviceGetMaximumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS, tiglLeadingEdgeDeviceGetMaximumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED1", &controlParm2));
    ASSERT_EQ(TIGL_SUCCESS, tiglLeadingEdgeDeviceGetMaximumControlParameter(
                                tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED2", &controlParm3));
    ASSERT_EQ(1, controlParm1);
    ASSERT_EQ(1, controlParm2);
    ASSERT_EQ(1, controlParm3);
}

TEST_F(TiglLeadingEdgeDevice, tiglLeadingEdgeDeviceGetAndSetControlParameter)
{
    double controlParm1, controlParm2, controlParm3;
    ASSERT_EQ(TIGL_UID_ERROR, tiglLeadingEdgeDeviceGetControlParameter(tiglHandle, "rmplstlzchn", &controlParm1));
    ASSERT_EQ(TIGL_NOT_FOUND,
              tiglLeadingEdgeDeviceGetControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm1));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED1", &controlParm2));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_outerLED2", &controlParm3));
    ASSERT_EQ(0, controlParm1);
    ASSERT_EQ(0, controlParm2);
    ASSERT_EQ(0, controlParm3);

    double controlParm;

    // error codes for set control parameter
    ASSERT_EQ(TIGL_UID_ERROR, tiglLeadingEdgeDeviceSetControlParameter(tiglHandle, "rmplstlzchn", 5.0));
    ASSERT_EQ(TIGL_NOT_FOUND, tiglLeadingEdgeDeviceSetControlParameter(-1, "D150_VAMP_W1_CompSeg1_innerLED", 5.0));

    // clamp to max/min control parameter
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceSetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", 5.0));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm));
    ASSERT_EQ(1, controlParm);

    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceSetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", -5.0));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm));
    ASSERT_EQ(0, controlParm);

    // value between min and max
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceSetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", 0.33));
    ASSERT_EQ(TIGL_SUCCESS,
              tiglLeadingEdgeDeviceGetControlParameter(tiglHandle, "D150_VAMP_W1_CompSeg1_innerLED", &controlParm));
    ASSERT_NEAR(0.33, controlParm, 1e-10);
}

TEST_F(TiglLeadingEdgeDevice, CPACSLeadingEdgeDevices)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing                    = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment =
        static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::generated::CPACSLeadingEdgeDevices& devices = *componentSegment.GetControlSurfaces()->GetLeadingEdgeDevices();

    // trailing edge device count
    ASSERT_EQ(devices.GetLeadingEdgeDeviceCount(), 3);

    // get trailing edge device by index
    ASSERT_EQ(devices.GetLeadingEdgeDevice(1).GetUID(), "D150_VAMP_W1_CompSeg1_innerLED");
    ASSERT_EQ(devices.GetLeadingEdgeDevice(2).GetUID(), "D150_VAMP_W1_CompSeg1_outerLED1");
    ASSERT_EQ(devices.GetLeadingEdgeDevice(3).GetUID(), "D150_VAMP_W1_CompSeg1_outerLED2");
    ASSERT_THROW(devices.GetLeadingEdgeDevice(-1), tigl::CTiglError);
    ASSERT_THROW(devices.GetLeadingEdgeDevice(4), tigl::CTiglError);

    // get index of trailing edge device by uid
    ASSERT_EQ(devices.GetLeadingEdgeDeviceIndex("D150_VAMP_W1_CompSeg1_innerLED"), 1);
    ASSERT_EQ(devices.GetLeadingEdgeDeviceIndex("D150_VAMP_W1_CompSeg1_outerLED1"), 2);
    ASSERT_EQ(devices.GetLeadingEdgeDeviceIndex("D150_VAMP_W1_CompSeg1_outerLED2"), 3);
    ASSERT_THROW(devices.GetLeadingEdgeDeviceIndex("WrongUID"), tigl::CTiglError);

    // get trailing edge device by uid
    ASSERT_EQ(devices.GetLeadingEdgeDevice("D150_VAMP_W1_CompSeg1_innerLED").GetName(),
              devices.GetLeadingEdgeDevice(1).GetName());
    ASSERT_EQ(devices.GetLeadingEdgeDevice("D150_VAMP_W1_CompSeg1_outerLED1").GetName(),
              devices.GetLeadingEdgeDevice(2).GetName());
    ASSERT_EQ(devices.GetLeadingEdgeDevice("D150_VAMP_W1_CompSeg1_outerLED2").GetName(),
              devices.GetLeadingEdgeDevice(3).GetName());
    ASSERT_THROW(devices.GetLeadingEdgeDevice("WrongUID"), tigl::CTiglError);
}

TEST_F(TiglLeadingEdgeDeviceSimple, setControlParameterAndExport)
{
    ASSERT_EQ(TIGL_SUCCESS, tiglControlSurfaceSetControlParameter(tiglHandle, "FlapInner", 1.0));
    tiglExportConfiguration(tiglHandle, "TestData/export/simpletest-flaps.stp", TIGL_TRUE, 0.0);
}
