/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-23 Philipp Kunze <Philipp.Kunze@dlr.de>
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
* @brief Tests for rotor functions.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include <cmath>
#include <string.h>

/******************************************************************************/

class RotorSimple : public ::testing::Test 
{
protected:
    virtual void SetUp()
    {
        const char* filename = "TestData/simple_test_rotors.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ(tixiRet, SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "RotorTestModel", &tiglHandle);
        ASSERT_EQ(tiglRet, TIGL_SUCCESS);
    }

    virtual void TearDown()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};


/******************************************************************************/

/**
* Tests for tiglGetRotorCount
*/
TEST_F(RotorSimple, tiglGetRotorCount)
{
    int rotorCount = -10;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglGetRotorCount(tiglHandle, 0), TIGL_NULL_POINTER);

    // Valid input:
    ASSERT_EQ(tiglGetRotorCount(tiglHandle, &rotorCount), TIGL_SUCCESS);
    ASSERT_EQ(rotorCount, 4);
}


/**
* Tests for tiglRotorGetUID
*/
TEST_F(RotorSimple, tiglRotorGetUID)
{
    char* rotorUID = 0;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetUID(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetUID(tiglHandle, 0, &rotorUID), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetUID(tiglHandle, 5, &rotorUID), TIGL_INDEX_ERROR);

    // Valid input:
    ASSERT_EQ(tiglRotorGetUID(tiglHandle, 1, &rotorUID), TIGL_SUCCESS);
    ASSERT_STREQ(rotorUID, "MainRotor1");
    ASSERT_EQ(tiglRotorGetUID(tiglHandle, 2, &rotorUID), TIGL_SUCCESS);
    ASSERT_STREQ(rotorUID, "MainRotor2");
    ASSERT_EQ(tiglRotorGetUID(tiglHandle, 3, &rotorUID), TIGL_SUCCESS);
    ASSERT_STREQ(rotorUID, "TailRotor");
    ASSERT_EQ(tiglRotorGetUID(tiglHandle, 4, &rotorUID), TIGL_SUCCESS);
    ASSERT_STREQ(rotorUID, "Propeller");
}


/**
* Tests for tiglRotorGetIndex
*/
TEST_F(RotorSimple, tiglRotorGetIndex)
{
    int rotorIndex = -10;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetIndex(tiglHandle, "MainRotor1", 0), TIGL_NULL_POINTER);

    // Invalid input (invalid UID):
    ASSERT_EQ(tiglRotorGetIndex(tiglHandle, "InvalidUID", &rotorIndex), TIGL_UID_ERROR);

    // Valid input:
    ASSERT_EQ(tiglRotorGetIndex(tiglHandle, "MainRotor1", &rotorIndex), TIGL_SUCCESS);
    ASSERT_EQ(rotorIndex, 1);
    ASSERT_EQ(tiglRotorGetIndex(tiglHandle, "MainRotor2", &rotorIndex), TIGL_SUCCESS);
    ASSERT_EQ(rotorIndex, 2);
    ASSERT_EQ(tiglRotorGetIndex(tiglHandle, "TailRotor", &rotorIndex), TIGL_SUCCESS);
    ASSERT_EQ(rotorIndex, 3);
    ASSERT_EQ(tiglRotorGetIndex(tiglHandle, "Propeller", &rotorIndex), TIGL_SUCCESS);
    ASSERT_EQ(rotorIndex, 4);
}


/**
* Tests for tiglRotorGetRadius
*/
TEST_F(RotorSimple, tiglRotorGetRadius)
{
    double radius = -1;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetRadius(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetRadius(tiglHandle, 0, &radius), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetRadius(tiglHandle, 5, &radius), TIGL_INDEX_ERROR);

    // Valid input:
    ASSERT_EQ(tiglRotorGetRadius(tiglHandle, 1, &radius), TIGL_SUCCESS);
    ASSERT_NEAR(radius, 5.0, 1E-10);
    ASSERT_EQ(tiglRotorGetRadius(tiglHandle, 2, &radius), TIGL_SUCCESS);
    ASSERT_NEAR(radius, 5.0, 1E-10);
    ASSERT_EQ(tiglRotorGetRadius(tiglHandle, 3, &radius), TIGL_SUCCESS);
    ASSERT_NEAR(radius, 1.0, 1E-10);
    ASSERT_EQ(tiglRotorGetRadius(tiglHandle, 4, &radius), TIGL_SUCCESS);
    ASSERT_NEAR(radius, 0.642482, 1E-6);
}


/**
* Tests for tiglRotorGetReferenceArea
*/
TEST_F(RotorSimple, tiglRotorGetReferenceArea)
{
    double referenceArea = -1;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetReferenceArea(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetReferenceArea(tiglHandle, 0, &referenceArea), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetReferenceArea(tiglHandle, 5, &referenceArea), TIGL_INDEX_ERROR);

    // Valid input:
    ASSERT_EQ(tiglRotorGetReferenceArea(tiglHandle, 1, &referenceArea), TIGL_SUCCESS);
    ASSERT_NEAR(referenceArea, M_PI*5.0*5.0, 1E-10);
    ASSERT_EQ(tiglRotorGetReferenceArea(tiglHandle, 2, &referenceArea), TIGL_SUCCESS);
    ASSERT_NEAR(referenceArea, M_PI*5.0*5.0, 1E-10);
    ASSERT_EQ(tiglRotorGetReferenceArea(tiglHandle, 3, &referenceArea), TIGL_SUCCESS);
    ASSERT_NEAR(referenceArea, M_PI*1.0, 1E-10);
    ASSERT_EQ(tiglRotorGetReferenceArea(tiglHandle, 4, &referenceArea), TIGL_SUCCESS);
    ASSERT_NEAR(referenceArea, M_PI*0.642482*0.642482, 1E-3);
}


/**
* Tests for tiglRotorGetTotalBladePlanformArea
*/
TEST_F(RotorSimple, tiglRotorGetTotalBladePlanformArea)
{
    double totalBladePlanformArea = -1;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetTotalBladePlanformArea(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetTotalBladePlanformArea(tiglHandle, 0, &totalBladePlanformArea), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetTotalBladePlanformArea(tiglHandle, 5, &totalBladePlanformArea), TIGL_INDEX_ERROR);

    // Valid input:
    ASSERT_EQ(tiglRotorGetTotalBladePlanformArea(tiglHandle, 1, &totalBladePlanformArea), TIGL_SUCCESS);
    ASSERT_NEAR(totalBladePlanformArea, 4.7, 1E-2);
    ASSERT_EQ(tiglRotorGetTotalBladePlanformArea(tiglHandle, 2, &totalBladePlanformArea), TIGL_SUCCESS);
    ASSERT_NEAR(totalBladePlanformArea, 4.7, 1E-2);
    ASSERT_EQ(tiglRotorGetTotalBladePlanformArea(tiglHandle, 3, &totalBladePlanformArea), TIGL_SUCCESS);
    ASSERT_NEAR(totalBladePlanformArea, 0.54, 1E-2);
    ASSERT_EQ(tiglRotorGetTotalBladePlanformArea(tiglHandle, 4, &totalBladePlanformArea), TIGL_SUCCESS);
    ASSERT_NEAR(totalBladePlanformArea, 0.77, 1E-2);
}


/**
* Tests for tiglRotorGetSolidity
*/
TEST_F(RotorSimple, tiglRotorGetSolidity)
{
    double solidity = -1;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetSolidity(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetSolidity(tiglHandle, 0, &solidity), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetSolidity(tiglHandle, 5, &solidity), TIGL_INDEX_ERROR);

    // Valid input:
    ASSERT_EQ(tiglRotorGetSolidity(tiglHandle, 1, &solidity), TIGL_SUCCESS);
    ASSERT_NEAR(solidity, 4.7/(M_PI*5.0*5.0), 1E-2);
    ASSERT_EQ(tiglRotorGetSolidity(tiglHandle, 2, &solidity), TIGL_SUCCESS);
    ASSERT_NEAR(solidity, 4.7/(M_PI*5.0*5.0), 1E-2);
    ASSERT_EQ(tiglRotorGetSolidity(tiglHandle, 3, &solidity), TIGL_SUCCESS);
    ASSERT_NEAR(solidity, 0.54/(M_PI*1.0), 1E-2);
    ASSERT_EQ(tiglRotorGetSolidity(tiglHandle, 4, &solidity), TIGL_SUCCESS);
    ASSERT_NEAR(solidity, 0.77/(M_PI*0.642482*0.642482), 1E-2);
}


/**
* Tests for tiglRotorGetSurfaceArea
*/
TEST_F(RotorSimple, tiglRotorGetSurfaceArea)
{
    double surfaceArea = -1;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetSurfaceArea(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetSurfaceArea(tiglHandle, 0, &surfaceArea), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetSurfaceArea(tiglHandle, 5, &surfaceArea), TIGL_INDEX_ERROR);

    // Valid input: reference values computed from exported rotor blade STL meshes in MeshLab v1.3.2
    ASSERT_EQ(tiglRotorGetSurfaceArea(tiglHandle, 1, &surfaceArea), TIGL_SUCCESS);
    ASSERT_NEAR(surfaceArea, 4.*2.405028, 1E-2);
    ASSERT_EQ(tiglRotorGetSurfaceArea(tiglHandle, 2, &surfaceArea), TIGL_SUCCESS);
    ASSERT_NEAR(surfaceArea, 4.*2.405028, 1E-2);
    ASSERT_EQ(tiglRotorGetSurfaceArea(tiglHandle, 3, &surfaceArea), TIGL_SUCCESS);
    ASSERT_NEAR(surfaceArea, 4.*0.278578, 1E-2);
    ASSERT_EQ(tiglRotorGetSurfaceArea(tiglHandle, 4, &surfaceArea), TIGL_SUCCESS);
    ASSERT_NEAR(surfaceArea,10.*0.161529, 1E-2);
}


/**
* Tests for tiglRotorGetVolume
*/
TEST_F(RotorSimple, tiglRotorGetVolume)
{
    double volume = -1;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetVolume(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetVolume(tiglHandle, 0, &volume), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetVolume(tiglHandle, 5, &volume), TIGL_INDEX_ERROR);

    // Valid input: reference values computed from exported rotor blade STL meshes in MeshLab v1.3.2
    ASSERT_EQ(tiglRotorGetVolume(tiglHandle, 1, &volume), TIGL_SUCCESS);
    ASSERT_NEAR(volume, 4.*0.027428, 1E-2);
    ASSERT_EQ(tiglRotorGetVolume(tiglHandle, 2, &volume), TIGL_SUCCESS);
    ASSERT_NEAR(volume, 4.*0.027428, 1E-2);
    ASSERT_EQ(tiglRotorGetVolume(tiglHandle, 3, &volume), TIGL_SUCCESS);
    ASSERT_NEAR(volume, 4.*0.001660, 1E-2);
    ASSERT_EQ(tiglRotorGetVolume(tiglHandle, 4, &volume), TIGL_SUCCESS);
    ASSERT_NEAR(volume,10.*0.000492, 1E-2);
}


/**
* Tests for tiglRotorGetTipSpeed
*/
TEST_F(RotorSimple, tiglRotorGetTipSpeed)
{
    double tipSpeed = 0;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetTipSpeed(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotorIndex):
    ASSERT_EQ(tiglRotorGetTipSpeed(tiglHandle, 0, &tipSpeed), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetTipSpeed(tiglHandle, 5, &tipSpeed), TIGL_INDEX_ERROR);

    // Valid input:
    ASSERT_EQ(tiglRotorGetTipSpeed(tiglHandle, 1, &tipSpeed), TIGL_SUCCESS);
    ASSERT_NEAR(tipSpeed, 2.*M_PI*5.0*420./60., 1E-2);
    ASSERT_EQ(tiglRotorGetTipSpeed(tiglHandle, 2, &tipSpeed), TIGL_SUCCESS);
    ASSERT_NEAR(tipSpeed,-2.*M_PI*5.0*420./60, 1E-2);
    ASSERT_EQ(tiglRotorGetTipSpeed(tiglHandle, 3, &tipSpeed), TIGL_SUCCESS);
    ASSERT_NEAR(tipSpeed, 2.*M_PI*1.0*2220./60., 1E-2);
    ASSERT_EQ(tiglRotorGetTipSpeed(tiglHandle, 4, &tipSpeed), TIGL_SUCCESS);
    ASSERT_NEAR(tipSpeed, 2.*M_PI*0.642482*3300./60., 1E-2);
}


/**
* Tests for tiglRotorGetRotorBladeCount
*/
TEST_F(RotorSimple, tiglRotorGetRotorBladeCount)
{
    int rotorBladeCount = -10;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorGetRotorBladeCount(tiglHandle, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid index):
    ASSERT_EQ(tiglRotorGetRotorBladeCount(tiglHandle, 0, &rotorBladeCount), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorGetRotorBladeCount(tiglHandle, 5, &rotorBladeCount), TIGL_INDEX_ERROR);

    // Valid input:
    rotorBladeCount = -10;
    ASSERT_EQ(tiglRotorGetRotorBladeCount(tiglHandle, 1, &rotorBladeCount), TIGL_SUCCESS);
    ASSERT_EQ(rotorBladeCount, 4);
    rotorBladeCount = -10;
    ASSERT_EQ(tiglRotorGetRotorBladeCount(tiglHandle, 2, &rotorBladeCount), TIGL_SUCCESS);
    ASSERT_EQ(rotorBladeCount, 4);
    rotorBladeCount = -10;
    ASSERT_EQ(tiglRotorGetRotorBladeCount(tiglHandle, 3, &rotorBladeCount), TIGL_SUCCESS);
    ASSERT_EQ(rotorBladeCount, 4);
    rotorBladeCount = -10;
    ASSERT_EQ(tiglRotorGetRotorBladeCount(tiglHandle, 4, &rotorBladeCount), TIGL_SUCCESS);
    ASSERT_EQ(rotorBladeCount, 10);
}


/**
* Tests for tiglRotorBladeGetWingIndex
*/
TEST_F(RotorSimple, tiglRotorBladeGetWingIndex)
{
    int wingIndex = -10;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 0, 1, &wingIndex), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 5, 1, &wingIndex), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 1, 0, &wingIndex), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 1, 5, &wingIndex), TIGL_INDEX_ERROR);

    // Valid input:
    for (int i=1; i<=4; i++) {
        wingIndex = -10;
        ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 1, i, &wingIndex), TIGL_SUCCESS);
        ASSERT_EQ(wingIndex, 1);
    }
    for (int i=1; i<=4; i++) {
        wingIndex = -10;
        ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 2, i, &wingIndex), TIGL_SUCCESS);
        ASSERT_EQ(wingIndex, 1);
    }
    for (int i=1; i<=4; i++) {
        wingIndex = -10;
        ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 3, i, &wingIndex), TIGL_SUCCESS);
        ASSERT_EQ(wingIndex, 2);
    }
    for (int i=1; i<=10; i++) {
        wingIndex = -10;
        ASSERT_EQ(tiglRotorBladeGetWingIndex(tiglHandle, 4, i, &wingIndex), TIGL_SUCCESS);
        ASSERT_EQ(wingIndex, 3);
    }
}


/**
* Tests for tiglRotorBladeGetWingUID
*/
TEST_F(RotorSimple, tiglRotorBladeGetWingUID)
{
    char* wingUID = 0;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 0, 1, &wingUID), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 5, 1, &wingUID), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 1, 0, &wingUID), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 1, 5, &wingUID), TIGL_INDEX_ERROR);

    // Valid input:
    for (int i=1; i<=4; i++) {
        wingUID = 0;
        ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 1, i, &wingUID), TIGL_SUCCESS);
        ASSERT_STREQ(wingUID, "MainRotor_blade");
    }
    for (int i=1; i<=4; i++) {
        wingUID = 0;
        ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 2, i, &wingUID), TIGL_SUCCESS);
        ASSERT_STREQ(wingUID, "MainRotor_blade");
    }
    for (int i=1; i<=4; i++) {
        wingUID = 0;
        ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 3, i, &wingUID), TIGL_SUCCESS);
        ASSERT_STREQ(wingUID, "TailRotor_blade");
    }
    for (int i=1; i<=10; i++) {
        wingUID = 0;
        ASSERT_EQ(tiglRotorBladeGetWingUID(tiglHandle, 4, i, &wingUID), TIGL_SUCCESS);
        ASSERT_STREQ(wingUID, "Propeller_blade");
    }
}


/**
* Tests for tiglRotorBladeGetAzimuthAngle
*/
TEST_F(RotorSimple, tiglRotorBladeGetAzimuthAngle)
{
    double azimuthAngle = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 0, 1, &azimuthAngle), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 5, 1, &azimuthAngle), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 1, 0, &azimuthAngle), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 1, 5, &azimuthAngle), TIGL_INDEX_ERROR);

    // Valid input:
    for (int i=1; i<=4; i++) {
        azimuthAngle = -10.;
        ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 1, i, &azimuthAngle), TIGL_SUCCESS);
        ASSERT_EQ(azimuthAngle, (i-1)*90.);
    }
    for (int i=1; i<=4; i++) {
        azimuthAngle = -10.;
        ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 2, i, &azimuthAngle), TIGL_SUCCESS);
        ASSERT_EQ(azimuthAngle, (i-1)*90.);
    }
    for (int i=1; i<=2; i++) {
        azimuthAngle = -10.;
        ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 3, i, &azimuthAngle), TIGL_SUCCESS);
        ASSERT_EQ(azimuthAngle, 15.+(i-1)*180.);
    }
    for (int i=3; i<=4; i++) {
        azimuthAngle = -10.;
        ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 3, i, &azimuthAngle), TIGL_SUCCESS);
        ASSERT_EQ(azimuthAngle, 135.+(i-3)*180.);
    }
    for (int i=1; i<=10; i++) {
        azimuthAngle = -10.;
        ASSERT_EQ(tiglRotorBladeGetAzimuthAngle(tiglHandle, 4, i, &azimuthAngle), TIGL_SUCCESS);
        ASSERT_EQ(azimuthAngle, (i-1)*36.);
    }
}


/**
* Tests for tiglRotorBladeGetRadius
*/
TEST_F(RotorSimple, tiglRotorBladeGetRadius)
{
    double radius = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 0, 1, &radius), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 5, 1, &radius), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 1, 0, &radius), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 1, 5, &radius), TIGL_INDEX_ERROR);

    // Valid input:
    for (int i=1; i<=4; i++) {
        radius = -10.;
        ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 1, i, &radius), TIGL_SUCCESS);
        ASSERT_NEAR(radius, 5.0, 1E-10);
    }
    for (int i=1; i<=4; i++) {
        radius = -10.;
        ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 2, i, &radius), TIGL_SUCCESS);
        ASSERT_NEAR(radius, 5.0, 1E-10);
    }
    for (int i=1; i<=4; i++) {
        radius = -10.;
        ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 3, i, &radius), TIGL_SUCCESS);
        ASSERT_NEAR(radius, 1.0, 1E-10);
    }
    for (int i=1; i<=10; i++) {
        radius = -10.;
        ASSERT_EQ(tiglRotorBladeGetRadius(tiglHandle, 4, i, &radius), TIGL_SUCCESS);
        ASSERT_NEAR(radius, 0.642482, 1E-6);
    }
}


/**
* Tests for tiglRotorBladeGetPlanformArea
*/
TEST_F(RotorSimple, tiglRotorBladeGetPlanformArea)
{
    double planformArea = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 0, 1, &planformArea), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 5, 1, &planformArea), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 1, 0, &planformArea), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 1, 5, &planformArea), TIGL_INDEX_ERROR);

    // Valid input:
    for (int i=1; i<=4; i++) {
        planformArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 1, i, &planformArea), TIGL_SUCCESS);
        ASSERT_NEAR(planformArea, 1.173, 1E-3);
    }
    for (int i=1; i<=4; i++) {
        planformArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 2, i, &planformArea), TIGL_SUCCESS);
        ASSERT_NEAR(planformArea, 1.173, 1E-3);
    }
    for (int i=1; i<=4; i++) {
        planformArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 3, i, &planformArea), TIGL_SUCCESS);
        ASSERT_NEAR(planformArea, 0.135, 1E-3);
    }
    for (int i=1; i<=10; i++) {
        planformArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetPlanformArea(tiglHandle, 4, i, &planformArea), TIGL_SUCCESS);
        ASSERT_NEAR(planformArea, 0.077, 1E-3);
    }
}


/**
* Tests for tiglRotorBladeGetSurfaceArea
*/
TEST_F(RotorSimple, tiglRotorBladeGetSurfaceArea)
{
    double surfaceArea = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 0, 1, &surfaceArea), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 5, 1, &surfaceArea), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 1, 0, &surfaceArea), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 1, 5, &surfaceArea), TIGL_INDEX_ERROR);

    // Valid input: reference values computed from exported rotor blade STL meshes in MeshLab v1.3.2
    for (int i=1; i<=4; i++) {
        surfaceArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 1, i, &surfaceArea), TIGL_SUCCESS);
        ASSERT_NEAR(surfaceArea, 2.405028, 1E-2);
    }
    for (int i=1; i<=4; i++) {
        surfaceArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 2, i, &surfaceArea), TIGL_SUCCESS);
        ASSERT_NEAR(surfaceArea, 2.405028, 1E-2);
    }
    for (int i=1; i<=4; i++) {
        surfaceArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 3, i, &surfaceArea), TIGL_SUCCESS);
        ASSERT_NEAR(surfaceArea, 0.278578, 1E-2);
    }
    for (int i=1; i<=10; i++) {
        surfaceArea = -10.;
        ASSERT_EQ(tiglRotorBladeGetSurfaceArea(tiglHandle, 4, i, &surfaceArea), TIGL_SUCCESS);
        ASSERT_NEAR(surfaceArea, 0.161529, 1E-2);
    }
}


/**
* Tests for tiglRotorBladeGetVolume
*/
TEST_F(RotorSimple, tiglRotorBladeGetVolume)
{
    double volume = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 0, 1, &volume), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 5, 1, &volume), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 1, 0, &volume), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 1, 5, &volume), TIGL_INDEX_ERROR);

    // Valid input: reference values computed from exported rotor blade STL meshes in MeshLab v1.3.2
    for (int i=1; i<=4; i++) {
        volume = -10.;
        ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 1, i, &volume), TIGL_SUCCESS);
        ASSERT_NEAR(volume, 0.027428, 1E-2);
    }
    for (int i=1; i<=4; i++) {
        volume = -10.;
        ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 2, i, &volume), TIGL_SUCCESS);
        ASSERT_NEAR(volume, 0.027428, 1E-2);
    }
    for (int i=1; i<=4; i++) {
        volume = -10.;
        ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 3, i, &volume), TIGL_SUCCESS);
        ASSERT_NEAR(volume, 0.001660, 1E-2);
    }
    for (int i=1; i<=10; i++) {
        volume = -10.;
        ASSERT_EQ(tiglRotorBladeGetVolume(tiglHandle, 4, i, &volume), TIGL_SUCCESS);
        ASSERT_NEAR(volume, 0.000492, 1E-2);
    }
}


/**
* Tests for tiglRotorBladeGetTipSpeed
*/
TEST_F(RotorSimple, tiglRotorBladeGetTipSpeed)
{
    double tipSpeed = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 1, 1, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 0, 1, &tipSpeed), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 5, 1, &tipSpeed), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 1, 0, &tipSpeed), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 1, 5, &tipSpeed), TIGL_INDEX_ERROR);

    // Valid input:
    for (int i=1; i<=4; i++) {
        tipSpeed = -10.;
        ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 1, i, &tipSpeed), TIGL_SUCCESS);
        ASSERT_NEAR(tipSpeed, 2.*M_PI*5.0*420./60., 1E-2);
    }
    for (int i=1; i<=4; i++) {
        tipSpeed = -10.;
        ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 2, i, &tipSpeed), TIGL_SUCCESS);
        ASSERT_NEAR(tipSpeed,-2.*M_PI*5.0*420./60, 1E-2);
    }
    for (int i=1; i<=4; i++) {
        tipSpeed = -10.;
        ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 3, i, &tipSpeed), TIGL_SUCCESS);
        ASSERT_NEAR(tipSpeed, 2.*M_PI*1.0*2220./60., 1E-2);
    }
    for (int i=1; i<=10; i++) {
        tipSpeed = -10.;
        ASSERT_EQ(tiglRotorBladeGetTipSpeed(tiglHandle, 4, i, &tipSpeed), TIGL_SUCCESS);
        ASSERT_NEAR(tipSpeed, 2.*M_PI*0.642482*3300./60., 1E-2);
    }
}


/**
* Tests for tiglRotorBladeGetLocalRadius
*/
TEST_F(RotorSimple, tiglRotorBladeGetLocalRadius)
{
    double localRadius = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 1, 1, 1, 0.0, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 0, 1, 1, 0.0, &localRadius), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 5, 1, 1, 0.0, &localRadius), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 1, 0, 1, 0.0, &localRadius), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 1, 5, 1, 0.0, &localRadius), TIGL_INDEX_ERROR);

    // Invalid input (invalid segment index):
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 1, 1, 0, 0.0, &localRadius), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 1, 1, 4, 0.0, &localRadius), TIGL_INDEX_ERROR);

    // Invalid input (invalid eta outside range [0,1]):
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 1, 1, 1,-0.1, &localRadius), TIGL_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 1, 1, 1, 1.1, &localRadius), TIGL_ERROR);

    // Valid input:
    for (int iRotor=1; iRotor<=2; iRotor++) {
        for (int iBlade=1; iBlade<=4; iBlade++) {
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, iRotor, iBlade, 1, 0.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 1.00, 1E-6);
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, iRotor, iBlade, 1, 0.5, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 2.375, 1E-6);
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, iRotor, iBlade, 1, 1.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 3.75, 1E-6);
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, iRotor, iBlade, 3, 0.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 4.50, 1E-6);
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, iRotor, iBlade, 3, 1.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 5.00, 1E-6);
        }
    }
    for (int iBlade=1; iBlade<=4; iBlade++) {
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 3, iBlade, 1, 0.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 0.25, 1E-6);
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 3, iBlade, 1, 0.5, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 0.625, 1E-6);
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 3, iBlade, 1, 1.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 1.00, 1E-6);
    }
    for (int iBlade=1; iBlade<=10; iBlade++) {
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 4, iBlade, 1, 0.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 0.139064, 1E-5);
            localRadius = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalRadius(tiglHandle, 4, iBlade, 9, 1.0, &localRadius), TIGL_SUCCESS);
            ASSERT_NEAR(localRadius, 0.642482, 1E-5);
    }
}


/**
* Tests for tiglRotorBladeGetLocalChord
*/
TEST_F(RotorSimple, tiglRotorBladeGetLocalChord)
{
    double localChord = -10.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 1, 1, 1, 0.0, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 0, 1, 1, 0.0, &localChord), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 5, 1, 1, 0.0, &localChord), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 1, 0, 1, 0.0, &localChord), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 1, 5, 1, 0.0, &localChord), TIGL_INDEX_ERROR);

    // Invalid input (invalid segment index):
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 1, 1, 0, 0.0, &localChord), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 1, 1, 4, 0.0, &localChord), TIGL_INDEX_ERROR);

    // Invalid input (invalid eta outside range [0,1]):
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 1, 1, 1,-0.1, &localChord), TIGL_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 1, 1, 1, 1.1, &localChord), TIGL_ERROR);

    // Valid input:
    for (int iRotor=1; iRotor<=2; iRotor++) {
        for (int iBlade=1; iBlade<=4; iBlade++) {
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, iRotor, iBlade, 1, 0.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.30, 1E-6);
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, iRotor, iBlade, 1, 1.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.30, 1E-6);
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, iRotor, iBlade, 3, 0.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.30, 1E-6);
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, iRotor, iBlade, 3, 0.5, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.25, 1E-5);
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, iRotor, iBlade, 3, 1.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.20, 1E-6);
        }
    }
    for (int iBlade=1; iBlade<=4; iBlade++) {
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 3, iBlade, 1, 0.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.18, 1E-6);
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 3, iBlade, 1, 0.5, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.18, 1E-6);
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 3, iBlade, 1, 1.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.18, 1E-6);
    }
    for (int iBlade=1; iBlade<=10; iBlade++) {
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 4, iBlade, 1, 0.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.18125, 1E-6);
            localChord = -10.;
            ASSERT_EQ(tiglRotorBladeGetLocalChord(tiglHandle, 4, iBlade, 9, 1.0, &localChord), TIGL_SUCCESS);
            ASSERT_NEAR(localChord, 0.1375, 1E-6);
    }
}


/**
* Tests for tiglRotorBladeGetLocalTwistAngle
*/
TEST_F(RotorSimple, tiglRotorBladeGetLocalTwistAngle)
{
    double localTwistAngle = -999.;

    // Invalid input (null pointer):
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 1, 1, 1, 0.0, 0), TIGL_NULL_POINTER);

    // Invalid input (invalid rotor index):
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 0, 1, 1, 0.0, &localTwistAngle), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 5, 1, 1, 0.0, &localTwistAngle), TIGL_INDEX_ERROR);

    // Invalid input (invalid rotor blade index):
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 1, 0, 1, 0.0, &localTwistAngle), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 1, 5, 1, 0.0, &localTwistAngle), TIGL_INDEX_ERROR);

    // Invalid input (invalid segment index):
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 1, 1, 0, 0.0, &localTwistAngle), TIGL_INDEX_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 1, 1, 4, 0.0, &localTwistAngle), TIGL_INDEX_ERROR);

    // Invalid input (invalid eta outside range [0,1]):
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 1, 1, 1,-0.1, &localTwistAngle), TIGL_ERROR);
    ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 1, 1, 1, 1.1, &localTwistAngle), TIGL_ERROR);

    // Valid input:
    for (int iRotor=1; iRotor<=2; iRotor++) {
        for (int iBlade=1; iBlade<=4; iBlade++) {
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, iRotor, iBlade, 1, 0.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle, 0.000, 1E-6);
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, iRotor, iBlade, 1, 0.5, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle,-2.2725, 1E-6);
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, iRotor, iBlade, 1, 1.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle,-4.545, 1E-6);
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, iRotor, iBlade, 3, 0.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle,-3.490, 1E-6);
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, iRotor, iBlade, 3, 1.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle,-4.320, 1E-6);
        }
    }
    for (int iBlade=1; iBlade<=4; iBlade++) {
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 3, iBlade, 1, 0.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle, 0.0, 1E-6);
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 3, iBlade, 1, 0.5, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle, 0.0, 1E-6);
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 3, iBlade, 1, 1.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle, 0.0, 1E-6);
    }
    for (int iBlade=1; iBlade<=10; iBlade++) {
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 4, iBlade, 1, 0.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle, 13.0162, 1E-3);
            localTwistAngle = -999.;
            ASSERT_EQ(tiglRotorBladeGetLocalTwistAngle(tiglHandle, 4, iBlade, 9, 1.0, &localTwistAngle), TIGL_SUCCESS);
            ASSERT_NEAR(localTwistAngle,-9.52576, 1E-3);
    }
}
