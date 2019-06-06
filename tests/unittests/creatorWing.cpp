/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 * Author: Malo Drougard
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
#include "tigl.h"
#include <tixi.h>
#include <tixicpp.h>
#include "CPACSWing.h"
#include "CPACSWings.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"

class creatorWing : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfigurationManager* manager = nullptr;
    tigl::CCPACSConfiguration* config         = nullptr;
    tigl::CCPACSWing* wing                    = nullptr;

    void setVariables(std::string inFilename, std::string wingUID)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager = &(tigl::CCPACSConfigurationManager::GetInstance());
        config  = &(manager->GetConfiguration(tiglHandle));
        wing    = &(config->GetWing(wingUID));
    }

    void setWing(std::string wingUID)
    {
        wing = &(config->GetWing(wingUID));
    }

    void unsetVariables()
    {
        filename = "";

        if (tiglHandle > -1) {
            tiglCloseCPACSConfiguration(tiglHandle);
        }
        if (tixiHandle > -1) {
            tixiCloseDocument(tixiHandle);
        }

        tiglHandle = -1;
        tiglHandle = -1;

        manager = nullptr;
        config  = nullptr;
        wing    = nullptr;
    }

    // Save the result in a new file (For visual check purpose)
    void saveInOutputFile()
    {
        // write the change in tixi memory
        config->WriteCPACS(config->GetUID());
        std::string newConfig = tixi::TixiExportDocumentAsString(tixiHandle);

        // Import-export to flat the xml // todo inform tixi developers about this "bug
        TixiDocumentHandle tixiHandle2 = tixi::TixiImportFromString(newConfig);
        newConfig                      = tixi::TixiExportDocumentAsString(tixiHandle2);

        std::ofstream myfile;
        myfile.open("TestData/Output/creatorWing-out.xml", std::ios::trunc);
        myfile << newConfig;
        myfile.close();
        tixiCloseDocument(tixiHandle2);
    }

    void TearDown()
    {
        unsetVariables();
    }
};

TEST_F(creatorWing, MultipleWings_GetWingHalfSpan)
{
    // Remark: The span is computed using bounding Box so, the result may no be accurate.

    double tolerance = 0.1;     // 10cm if we use meter metric
    // symmetry x-z case
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_NEAR(wing->GetWingHalfSpan(), 2, tolerance);

    setWing("W2_RX90");
    EXPECT_NEAR(wing->GetWingHalfSpan(), 2, tolerance);

    // symmetry x-z case
    setWing("W3_RX40");
    EXPECT_NEAR(wing->GetWingHalfSpan(), 1.532088, tolerance);

    // no symmetry case
    setWing("W4_RX40b");
    EXPECT_NEAR(wing->GetWingHalfSpan(), 1.532088, tolerance);

    // symmetry x-z
    // The symmetry prime on the heuristic (So, we get always the correct span)
    setWing("W5_RX60");
    EXPECT_NEAR(wing->GetWingHalfSpan(), 1, tolerance);

    // no symmetry case
    setWing("W6_RX60b");
    EXPECT_NEAR(wing->GetWingHalfSpan(), 1.73205, tolerance);

    // strange symmetry case (symmetry y-x plane)
    setWing("W7_SymX");
    EXPECT_NEAR(wing->GetWingHalfSpan(), 0, 2 * tolerance);

    // simple box wing
    setWing("W8_SBW");
    EXPECT_NEAR(wing->GetWingHalfSpan(),0.93969, tolerance);
    
    // empty wing
    setWing("W13_EmptyWing");
    EXPECT_NEAR(wing->GetWingHalfSpan(),0, tolerance);

}

TEST_F(creatorWing, MultipleWings_GetWingfSpanAndSetSymmetry)
{

    // Remark: using setSymmetry does not invalidate the wing,
    // but, at the moment,  all the functions use directly getSymmetry to perform their operation,
    // so this should not create trouble as long we do not cache some result that use the symmetry
    // This test make sure that we remain so.

    double tolerance = 0.1;
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_NEAR(wing->GetWingspan(), 4, tolerance);
    EXPECT_NEAR(wing->GetWingHalfSpan(), 2, tolerance);
    EXPECT_EQ(wing->GetSymmetryAxis(), TiglSymmetryAxis::TIGL_X_Z_PLANE);

    wing->SetSymmetryAxis(TiglSymmetryAxis::TIGL_NO_SYMMETRY);
    EXPECT_NEAR(wing->GetWingspan(), 2, tolerance);
    EXPECT_NEAR(wing->GetWingHalfSpan(), 2, tolerance);
    EXPECT_EQ(wing->GetSymmetryAxis(), TiglSymmetryAxis::TIGL_NO_SYMMETRY);

    wing->SetSymmetryAxis(TiglSymmetryAxis::TIGL_X_Z_PLANE);
    EXPECT_NEAR(wing->GetWingspan(), 4, tolerance);
    EXPECT_NEAR(wing->GetWingHalfSpan(), 2, tolerance);
    EXPECT_EQ(wing->GetSymmetryAxis(), TiglSymmetryAxis::TIGL_X_Z_PLANE);

}

TEST_F(creatorWing, MultipleWings_GetReferenceArea)
{
    // symmetry x-z case
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_NEAR(wing->GetReferenceArea(), 1.75, 0.0001);

    setWing("W2_RX90");
    EXPECT_NEAR(wing->GetReferenceArea(), 1.75, 0.0001);

    // symmetry x-z case
    setWing("W3_RX40");
    EXPECT_NEAR(wing->GetReferenceArea(), 0.40217333, 0.0001);

    // no symmetry case
    setWing("W4_RX40b");
    EXPECT_NEAR(wing->GetReferenceArea(), 0.40217333, 0.0001);

    // symmetry x-z
    // The symmetry prime on the heuristic (So, we get always the correct span)
    setWing("W5_RX60");
    EXPECT_NEAR(wing->GetReferenceArea(), 0.2625, 0.0001);

    // no symmetry case
    setWing("W6_RX60b");
    EXPECT_NEAR(wing->GetReferenceArea(), 0.454663, 0.0001);

    // strange symmetry case (symmetry y-x plane)
    // The area is 0  because we project against the XZ plane since the major axis is Y.
    setWing("W7_SymX");
    EXPECT_NEAR(wing->GetReferenceArea(),0, 0.0001);

    // simple box wing
    // Remark, since there is the area is projected two times on the plane (one for the lower wing, once for the upper wing)
    // the reference are is double // TODO verify that this behavior is not problematic
    setWing("W8_SBW");
    EXPECT_NEAR(wing->GetReferenceArea(),2*0.93969, 0.0001);
    
    // empty wing
    setWing("W13_EmptyWing");
    EXPECT_NEAR(wing->GetReferenceArea(), 0, 0.0001);

}

TEST_F(creatorWing, MultipleWings_GetSweep)
{
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_NEAR(wing->GetSweep(0), 14.0362434, 0.0001);
    EXPECT_NEAR(wing->GetSweep(1), 0, 0.0001);
    EXPECT_NEAR(wing->GetSweep(0.5), 7.125016, 0.0001 );


    setWing("W2_RX90");
    EXPECT_NEAR(wing->GetSweep(0), 14.0362434, 0.0001);

    // symmetry x-z case
    setWing("W3_RX40");
    EXPECT_NEAR(wing->GetSweep(0), 5.59175, 0.0001);

    // no symmetry case
    setWing("W4_RX40b");
    EXPECT_NEAR(wing->GetSweep(0), 5.59175, 0.0001);
    // symmetry x-z
    // The symmetry prime on the heuristic (So, we get always the correct span)
    setWing("W5_RX60");
    EXPECT_NEAR(wing->GetSweep(0), 8.530765, 0.0001);

    // no symmetry case
    setWing("W6_RX60b");
    EXPECT_NEAR(wing->GetSweep(0), 4.94961, 0.0001);

    // simple box wing
    setWing("W8_SBW");
    EXPECT_NEAR(wing->GetSweep(0), 0, 0.0001);

    setWing("W9_BWSweep"); // Remark the sweep and dihedral of positioning  can be view has extrinsic rotation Z then X
    EXPECT_NEAR(wing->GetSweep(0), 44.7803444, 0.0001);

    // the wing is describe in the negative Y direction
    // -> in the case to get the sweep angle, we need to take care of "mirroring the wing" prior to the computation
    setWing("W10_Reverse");
    EXPECT_NEAR(wing->GetSweep(0), 14.0362434, 0.0001);

    setWing("W12_NegSwp");
    EXPECT_NEAR(wing->GetSweep(0), -23, 0.0001);

    // empty wing
    setWing("W13_EmptyWing");
    EXPECT_NEAR(wing->GetSweep(0), 0, 0.0001);

    setWing("W15_ShiAir");
    EXPECT_NEAR(wing->GetSweep(0.5), 26.57, 0.01);


}


TEST_F(creatorWing, MultipleWings_GetDihedral)
{
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_NEAR(wing->GetDihedral(0), 0, 0.0001);
    EXPECT_NEAR(wing->GetDihedral(0.5), 0, 0.0001);
    EXPECT_NEAR(wing->GetDihedral(1), 0, 0.0001);

    setWing("W2_RX90");
    EXPECT_NEAR(wing->GetDihedral(0), 0, 0.0001);

    // symmetry x-z case
    setWing("W3_RX40");
    EXPECT_NEAR(wing->GetDihedral(0), 40, 0.0001);
    EXPECT_NEAR(wing->GetDihedral(0.7), 40, 0.0001);

    // no symmetry case
    setWing("W4_RX40b");
    EXPECT_NEAR(wing->GetDihedral(0), 40, 0.0001);

    // symmetry x-z
    // The symmetry prime on the heuristic (So, we get always the correct span)
    setWing("W5_RX60");
    EXPECT_NEAR(wing->GetDihedral(0), 60, 0.0001);

    // no symmetry case
    setWing("W6_RX60b");
    EXPECT_NEAR(wing->GetDihedral(0), 30, 0.0001);

    // simple box wing
    setWing("W8_SBW");
    EXPECT_NEAR(wing->GetDihedral(0), 20, 0.0001);

    setWing("W9_BWSweep"); // Remark the sweep and dihedral of positioning  can be view has extrinsic rotation Z then X
    EXPECT_NEAR(wing->GetDihedral(0), 20, 0.0001);

    // the wing is describe in the negative Y direction
    // -> in the case to get the sweep angle, we need to take care of "mirroring the wing" prior to the computation
    setWing("W10_Reverse");
    EXPECT_NEAR(wing->GetDihedral(0), 0, 0.0001);

    setWing("W11_NegDih");
    EXPECT_NEAR(wing->GetDihedral(0), -10, 0.0001);
    EXPECT_NEAR(wing->GetDihedral(1), -10, 0.0001);

    setWing("W12_NegSwp");
    EXPECT_NEAR(wing->GetDihedral(0.5), 0, 0.0001);


    // empty wing
    setWing("W13_EmptyWing");
    EXPECT_NEAR(wing->GetDihedral(0.5), 0, 0.0001);

    // wing with a dihedral that change depending the input chord value
    setWing("W14_ChaDih");
    EXPECT_NEAR(wing->GetDihedral(0), 45, 0.0001);
    EXPECT_NEAR(wing->GetDihedral(1), 36.87, 0.01);

    setWing("W15_ShiAir");
    EXPECT_NEAR(wing->GetDihedral(0), 26.57, 0.01);


}


TEST_F(creatorWing, MultipleWings_GetRootLEPosition)
{
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_EQ(wing->GetRootLEPosition(), tigl::CTiglPoint(0,0,0));

    setWing("W7_SymX");
    EXPECT_EQ(wing->GetRootLEPosition(), tigl::CTiglPoint(0,4,0));

    setWing("W3_RX40");
    EXPECT_EQ(wing->GetRootLEPosition(), tigl::CTiglPoint(10,0,0));

    setWing("W10_Reverse");
    EXPECT_EQ(wing->GetRootLEPosition(), tigl::CTiglPoint(0,-4,0));

    setWing("W13_EmptyWing");
    EXPECT_EQ(wing->GetRootLEPosition(), tigl::CTiglPoint(0,0,0));

    setWing("W14_ChaDih");
    EXPECT_EQ(wing->GetRootLEPosition(), tigl::CTiglPoint(-10,8,0));

    // case of shifted airfoils
    setWing("W15_ShiAir");
    EXPECT_EQ(wing->GetRootLEPosition(), tigl::CTiglPoint(-9,-10,0));

}

TEST_F(creatorWing, MultipleWings_SetRootLEPosition)
{
    tigl::CTiglPoint newRootPosition;

    setVariables("TestData/multiple_wings.xml", "Wing");
    newRootPosition = tigl::CTiglPoint(3, 4, 5);
    wing->SetRootLEPosition(newRootPosition);
    EXPECT_EQ(wing->GetRootLEPosition(), newRootPosition);
    newRootPosition = tigl::CTiglPoint(-1, 0, 0);
    wing->SetRootLEPosition(newRootPosition);
    EXPECT_EQ(wing->GetRootLEPosition(), newRootPosition);

    setWing("W3_RX40");
    newRootPosition = tigl::CTiglPoint(3.5, -10, 55);
    wing->SetRootLEPosition(newRootPosition);
    EXPECT_EQ(wing->GetRootLEPosition(), newRootPosition);
    newRootPosition = tigl::CTiglPoint(-1, -4, 11);
    wing->SetRootLEPosition(newRootPosition);
    EXPECT_EQ(wing->GetRootLEPosition(), newRootPosition);

    setWing("W13_EmptyWing");
    newRootPosition = tigl::CTiglPoint(3, 4, 5);
    wing->SetRootLEPosition(newRootPosition); // just check if no excpetion is throw
    // In the case of a empty wing, the GetRootLEPosition always return (0,0,0)

    setWing("W15_ShiAir");
    newRootPosition = tigl::CTiglPoint(3, 4, 5);
    wing->SetRootLEPosition(newRootPosition);
    EXPECT_EQ(wing->GetRootLEPosition(), newRootPosition);
    newRootPosition = tigl::CTiglPoint(-1, 0, 0);
    wing->SetRootLEPosition(newRootPosition);
    EXPECT_EQ(wing->GetRootLEPosition(), newRootPosition);

    setWing("W9_BWSweep");
    newRootPosition = tigl::CTiglPoint(13, 4.4, 5);
    wing->SetRootLEPosition(newRootPosition);
    EXPECT_EQ(wing->GetRootLEPosition(), newRootPosition);

    saveInOutputFile();
}




TEST_F(creatorWing, MultipleWings_SetRotation)
{
    setVariables("TestData/multiple_wings.xml", "Wing");
    tigl::CTiglPoint newRot, resRot;

    newRot = tigl::CTiglPoint(0,10,0);
    wing->SetRotation(newRot);
    resRot = wing->GetRotation();
    EXPECT_TRUE(resRot.isNear(newRot, 0.0001));

    setWing("W9_BWSweep");
    newRot = tigl::CTiglPoint(-20,10,80);
    wing->SetRotation(newRot);
    resRot = wing->GetRotation();
    EXPECT_TRUE(resRot.isNear(newRot, 0.0001));

    saveInOutputFile();
}

TEST_F(creatorWing, MultipleWings_SetSweep)
{
    setVariables("TestData/multiple_wings.xml", "Wing");

    double newSweep, resSweep, chordPercent;
    double oldDihedral;  // for checking purpose, dihedral should not move when sweep change
    tigl::CTiglPoint oldRootLE;  // for checking purpose
    double tolerance = 0.0001;

    setWing("Wing");
    newSweep = 30;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));

    newSweep = 30;
    chordPercent = 0.0;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    newSweep = 30;
    chordPercent = 1;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));

    newSweep = -30;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    setWing("W2_RX90");
    newSweep = 40;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    setWing("W10_Reverse");
    newSweep = 40;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    setWing("W8_SBW");
    newSweep = 40;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    setWing("W5_RX60");
    newSweep = -20;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    setWing("W6_RX60b");
    newSweep = -20;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    setWing("W14_ChaDih");
    newSweep = 60;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));


    setWing("W15_ShiAir");
    newSweep = 60;
    chordPercent = 0.25;
    oldDihedral = wing->GetDihedral(chordPercent);
    oldRootLE = wing->GetRootLEPosition();
    wing->SetSweep(newSweep, chordPercent );
    resSweep = wing->GetSweep(chordPercent);
    EXPECT_NEAR(resSweep, newSweep, tolerance);
    EXPECT_NEAR(oldDihedral, wing->GetDihedral(chordPercent), tolerance);
    EXPECT_TRUE(oldRootLE.isNear(wing->GetRootLEPosition(), tolerance));



    saveInOutputFile();

}