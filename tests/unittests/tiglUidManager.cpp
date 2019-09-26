/* 
* Copyright (C) RISC Software GmbH
*
* Created: 2017-05-08 Bernhard Manfred Gruber
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
* @brief Tests for the UidManager and automatic uid registration
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CTiglUIDManager.h"
#include "CCPACSWingSectionElement.h"

namespace {
    class tiglUidManagerTest : public ::testing::Test {
    protected:
        void SetUp() OVERRIDE {
            tixiHandle = -1;
            ASSERT_TRUE(tixiOpenDocument("TestData/CPACS_30_D150.xml", &tixiHandle) == SUCCESS);

            tiglHandle = -1;
            ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle) == TIGL_SUCCESS);

            uidMgr = &tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle).GetUIDManager();
        }

        void TearDown() OVERRIDE {
            ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
            ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
            tiglHandle = -1;
            tixiHandle = -1;
        }

        TixiDocumentHandle           tixiHandle;
        TiglCPACSConfigurationHandle tiglHandle;
        tigl::CTiglUIDManager* uidMgr;
    };

    /**
    * Tests wheter all uids can be resolved. Commented uids are currently pruned.
    */
    TEST_F(tiglUidManagerTest, allUids) {
        std::vector<std::string> uids;
        uids.push_back("D150_VAMP");
        uids.push_back("D150_VAMP_FL1");
        uids.push_back("D150_VAMP_FL1_Sec1");
        uids.push_back("D150_VAMP_FL1_Sec1_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec2");
        uids.push_back("D150_VAMP_FL1_Sec2_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec3");
        uids.push_back("D150_VAMP_FL1_Sec3_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec4");
        uids.push_back("D150_VAMP_FL1_Sec4_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec5");
        uids.push_back("D150_VAMP_FL1_Sec5_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec6");
        uids.push_back("D150_VAMP_FL1_Sec6_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec7");
        uids.push_back("D150_VAMP_FL1_Sec7_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec8");
        uids.push_back("D150_VAMP_FL1_Sec8_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec9");
        uids.push_back("D150_VAMP_FL1_Sec9_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec10");
        uids.push_back("D150_VAMP_FL1_Sec10_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec11");
        uids.push_back("D150_VAMP_FL1_Sec11_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec12");
        uids.push_back("D150_VAMP_FL1_Sec12_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec13");
        uids.push_back("D150_VAMP_FL1_Sec13_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec14");
        uids.push_back("D150_VAMP_FL1_Sec14_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec15");
        uids.push_back("D150_VAMP_FL1_Sec15_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec16");
        uids.push_back("D150_VAMP_FL1_Sec16_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec17");
        uids.push_back("D150_VAMP_FL1_Sec17_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec18");
        uids.push_back("D150_VAMP_FL1_Sec18_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec19");
        uids.push_back("D150_VAMP_FL1_Sec19_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec20");
        uids.push_back("D150_VAMP_FL1_Sec20_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec21");
        uids.push_back("D150_VAMP_FL1_Sec21_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec22");
        uids.push_back("D150_VAMP_FL1_Sec22_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec23");
        uids.push_back("D150_VAMP_FL1_Sec23_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec24");
        uids.push_back("D150_VAMP_FL1_Sec24_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec25");
        uids.push_back("D150_VAMP_FL1_Sec25_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec26");
        uids.push_back("D150_VAMP_FL1_Sec26_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec27");
        uids.push_back("D150_VAMP_FL1_Sec27_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec28");
        uids.push_back("D150_VAMP_FL1_Sec28_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec29");
        uids.push_back("D150_VAMP_FL1_Sec29_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec30");
        uids.push_back("D150_VAMP_FL1_Sec30_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec31");
        uids.push_back("D150_VAMP_FL1_Sec31_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec32");
        uids.push_back("D150_VAMP_FL1_Sec32_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec33");
        uids.push_back("D150_VAMP_FL1_Sec33_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec34");
        uids.push_back("D150_VAMP_FL1_Sec34_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec35");
        uids.push_back("D150_VAMP_FL1_Sec35_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec36");
        uids.push_back("D150_VAMP_FL1_Sec36_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec37");
        uids.push_back("D150_VAMP_FL1_Sec37_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec38");
        uids.push_back("D150_VAMP_FL1_Sec38_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec39");
        uids.push_back("D150_VAMP_FL1_Sec39_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec40");
        uids.push_back("D150_VAMP_FL1_Sec40_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec41");
        uids.push_back("D150_VAMP_FL1_Sec41_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec42");
        uids.push_back("D150_VAMP_FL1_Sec42_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec43");
        uids.push_back("D150_VAMP_FL1_Sec43_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec44");
        uids.push_back("D150_VAMP_FL1_Sec44_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec45");
        uids.push_back("D150_VAMP_FL1_Sec45_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec46");
        uids.push_back("D150_VAMP_FL1_Sec46_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec47");
        uids.push_back("D150_VAMP_FL1_Sec47_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec48");
        uids.push_back("D150_VAMP_FL1_Sec48_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec49");
        uids.push_back("D150_VAMP_FL1_Sec49_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec50");
        uids.push_back("D150_VAMP_FL1_Sec50_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec51");
        uids.push_back("D150_VAMP_FL1_Sec51_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec52");
        uids.push_back("D150_VAMP_FL1_Sec52_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec53");
        uids.push_back("D150_VAMP_FL1_Sec53_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec54");
        uids.push_back("D150_VAMP_FL1_Sec54_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec55");
        uids.push_back("D150_VAMP_FL1_Sec55_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec56");
        uids.push_back("D150_VAMP_FL1_Sec56_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec57");
        uids.push_back("D150_VAMP_FL1_Sec57_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec58");
        uids.push_back("D150_VAMP_FL1_Sec58_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec59");
        uids.push_back("D150_VAMP_FL1_Sec59_Elem1");
        uids.push_back("D150_VAMP_FL1_Sec60");
        uids.push_back("D150_VAMP_FL1_Sec60_Elem1");
        uids.push_back("D150_VAMP_FL1_Pos1");
        uids.push_back("D150_VAMP_FL1_Pos2");
        uids.push_back("D150_VAMP_FL1_Pos3");
        uids.push_back("D150_VAMP_FL1_Pos4");
        uids.push_back("D150_VAMP_FL1_Pos5");
        uids.push_back("D150_VAMP_FL1_Pos6");
        uids.push_back("D150_VAMP_FL1_Pos7");
        uids.push_back("D150_VAMP_FL1_Pos8");
        uids.push_back("D150_VAMP_FL1_Pos9");
        uids.push_back("D150_VAMP_FL1_Pos10");
        uids.push_back("D150_VAMP_FL1_Pos11");
        uids.push_back("D150_VAMP_FL1_Pos12");
        uids.push_back("D150_VAMP_FL1_Pos13");
        uids.push_back("D150_VAMP_FL1_Pos14");
        uids.push_back("D150_VAMP_FL1_Pos15");
        uids.push_back("D150_VAMP_FL1_Pos16");
        uids.push_back("D150_VAMP_FL1_Pos17");
        uids.push_back("D150_VAMP_FL1_Pos18");
        uids.push_back("D150_VAMP_FL1_Pos19");
        uids.push_back("D150_VAMP_FL1_Pos20");
        uids.push_back("D150_VAMP_FL1_Pos21");
        uids.push_back("D150_VAMP_FL1_Pos22");
        uids.push_back("D150_VAMP_FL1_Pos23");
        uids.push_back("D150_VAMP_FL1_Pos24");
        uids.push_back("D150_VAMP_FL1_Pos25");
        uids.push_back("D150_VAMP_FL1_Pos26");
        uids.push_back("D150_VAMP_FL1_Pos27");
        uids.push_back("D150_VAMP_FL1_Pos28");
        uids.push_back("D150_VAMP_FL1_Pos29");
        uids.push_back("D150_VAMP_FL1_Pos30");
        uids.push_back("D150_VAMP_FL1_Pos31");
        uids.push_back("D150_VAMP_FL1_Pos32");
        uids.push_back("D150_VAMP_FL1_Pos33");
        uids.push_back("D150_VAMP_FL1_Pos34");
        uids.push_back("D150_VAMP_FL1_Pos35");
        uids.push_back("D150_VAMP_FL1_Pos36");
        uids.push_back("D150_VAMP_FL1_Pos37");
        uids.push_back("D150_VAMP_FL1_Pos38");
        uids.push_back("D150_VAMP_FL1_Pos39");
        uids.push_back("D150_VAMP_FL1_Pos40");
        uids.push_back("D150_VAMP_FL1_Pos41");
        uids.push_back("D150_VAMP_FL1_Pos42");
        uids.push_back("D150_VAMP_FL1_Pos43");
        uids.push_back("D150_VAMP_FL1_Pos44");
        uids.push_back("D150_VAMP_FL1_Pos45");
        uids.push_back("D150_VAMP_FL1_Pos46");
        uids.push_back("D150_VAMP_FL1_Pos47");
        uids.push_back("D150_VAMP_FL1_Pos48");
        uids.push_back("D150_VAMP_FL1_Pos49");
        uids.push_back("D150_VAMP_FL1_Pos50");
        uids.push_back("D150_VAMP_FL1_Pos51");
        uids.push_back("D150_VAMP_FL1_Pos52");
        uids.push_back("D150_VAMP_FL1_Pos53");
        uids.push_back("D150_VAMP_FL1_Pos54");
        uids.push_back("D150_VAMP_FL1_Pos55");
        uids.push_back("D150_VAMP_FL1_Pos56");
        uids.push_back("D150_VAMP_FL1_Pos57");
        uids.push_back("D150_VAMP_FL1_Pos58");
        uids.push_back("D150_VAMP_FL1_Pos59");
        uids.push_back("D150_VAMP_FL1_Pos60");
        uids.push_back("D150_VAMP_FL1_Seg1");
        uids.push_back("D150_VAMP_FL1_Seg2");
        uids.push_back("D150_VAMP_FL1_Seg3");
        uids.push_back("D150_VAMP_FL1_Seg4");
        uids.push_back("D150_VAMP_FL1_Seg5");
        uids.push_back("D150_VAMP_FL1_Seg6");
        uids.push_back("D150_VAMP_FL1_Seg7");
        uids.push_back("D150_VAMP_FL1_Seg8");
        uids.push_back("D150_VAMP_FL1_Seg9");
        uids.push_back("D150_VAMP_FL1_Seg10");
        uids.push_back("D150_VAMP_FL1_Seg11");
        uids.push_back("D150_VAMP_FL1_Seg12");
        uids.push_back("D150_VAMP_FL1_Seg13");
        uids.push_back("D150_VAMP_FL1_Seg14");
        uids.push_back("D150_VAMP_FL1_Seg15");
        uids.push_back("D150_VAMP_FL1_Seg16");
        uids.push_back("D150_VAMP_FL1_Seg17");
        uids.push_back("D150_VAMP_FL1_Seg18");
        uids.push_back("D150_VAMP_FL1_Seg19");
        uids.push_back("D150_VAMP_FL1_Seg20");
        uids.push_back("D150_VAMP_FL1_Seg21");
        uids.push_back("D150_VAMP_FL1_Seg22");
        uids.push_back("D150_VAMP_FL1_Seg23");
        uids.push_back("D150_VAMP_FL1_Seg24");
        uids.push_back("D150_VAMP_FL1_Seg25");
        uids.push_back("D150_VAMP_FL1_Seg26");
        uids.push_back("D150_VAMP_FL1_Seg27");
        uids.push_back("D150_VAMP_FL1_Seg28");
        uids.push_back("D150_VAMP_FL1_Seg29");
        uids.push_back("D150_VAMP_FL1_Seg30");
        uids.push_back("D150_VAMP_FL1_Seg31");
        uids.push_back("D150_VAMP_FL1_Seg32");
        uids.push_back("D150_VAMP_FL1_Seg33");
        uids.push_back("D150_VAMP_FL1_Seg34");
        uids.push_back("D150_VAMP_FL1_Seg35");
        uids.push_back("D150_VAMP_FL1_Seg36");
        uids.push_back("D150_VAMP_FL1_Seg37");
        uids.push_back("D150_VAMP_FL1_Seg38");
        uids.push_back("D150_VAMP_FL1_Seg39");
        uids.push_back("D150_VAMP_FL1_Seg40");
        uids.push_back("D150_VAMP_FL1_Seg41");
        uids.push_back("D150_VAMP_FL1_Seg42");
        uids.push_back("D150_VAMP_FL1_Seg43");
        uids.push_back("D150_VAMP_FL1_Seg44");
        uids.push_back("D150_VAMP_FL1_Seg45");
        uids.push_back("D150_VAMP_FL1_Seg46");
        uids.push_back("D150_VAMP_FL1_Seg47");
        uids.push_back("D150_VAMP_FL1_Seg48");
        uids.push_back("D150_VAMP_FL1_Seg49");
        uids.push_back("D150_VAMP_FL1_Seg50");
        uids.push_back("D150_VAMP_FL1_Seg51");
        uids.push_back("D150_VAMP_FL1_Seg52");
        uids.push_back("D150_VAMP_FL1_Seg53");
        uids.push_back("D150_VAMP_FL1_Seg54");
        uids.push_back("D150_VAMP_FL1_Seg55");
        uids.push_back("D150_VAMP_FL1_Seg56");
        uids.push_back("D150_VAMP_FL1_Seg57");
        uids.push_back("D150_VAMP_FL1_Seg58");
        uids.push_back("D150_VAMP_FL1_Seg59");
        uids.push_back("D150_VAMP_W1");
        uids.push_back("D150_VAMP_W1_Sec1");
        uids.push_back("D150_VAMP_W1_Sec1_Elem1");
        uids.push_back("D150_VAMP_W1_Sec2");
        uids.push_back("D150_VAMP_W1_Sec2_Elem1");
        uids.push_back("D150_VAMP_W1_Sec3");
        uids.push_back("D150_VAMP_W1_Sec3_Elem1");
        uids.push_back("D150_VAMP_W1_Sec4");
        uids.push_back("D150_VAMP_W1_Sec4_Elem1");
        uids.push_back("D150_VAMP_W1_Pos1");
        uids.push_back("D150_VAMP_W1_Pos2");
        uids.push_back("D150_VAMP_W1_Pos3");
        uids.push_back("D150_VAMP_W1_Pos4");
        uids.push_back("D150_VAMP_W1_Seg1");
        uids.push_back("D150_VAMP_W1_Seg2");
        uids.push_back("D150_VAMP_W1_Seg3");
        uids.push_back("D150_VAMP_W1_CompSeg1");
        uids.push_back("ribs_CWB");
        uids.push_back("ribs_IW");
        uids.push_back("ribs_EM");
        uids.push_back("ribs_12");
        uids.push_back("ribs_13");
        uids.push_back("ribs_OW");
        uids.push_back("ribs_Tip");
        uids.push_back("FS_P1");
        uids.push_back("FS_P2");
        uids.push_back("FS_P3");
        uids.push_back("RS_P1");
        uids.push_back("RS_P2");
        uids.push_back("RS_P3");
        uids.push_back("RS_P4");
        uids.push_back("RS_P5");
        uids.push_back("frontSpar");
        uids.push_back("rearSpar");
        //uids.push_back("D150_VAMP_W1_CompSeg1_innerFlap");
        //uids.push_back("D150_VAMP_W1_CompSeg1_outerFlap");
        //uids.push_back("D150_VAMP_W1_CompSeg1_aileron");
        //uids.push_back("D150_VAMP_W1_CompSeg1_Spoiler4");
        //uids.push_back("D150_VAMP_W1_CompSeg1_Spoiler5");
        //uids.push_back("D150_VAMP_W1_CompSeg1_Spoiler6");
        //uids.push_back("D150_VAMP_W1_CompSeg1_Spoiler7");
        //uids.push_back("D150_VAMP_W1_CompSeg1_Spoiler8");
        uids.push_back("D150_VAMP_HL1");
        uids.push_back("D150_VAMP_HL1_Sec1");
        uids.push_back("D150_VAMP_HL1_Sec1_Elem1");
        uids.push_back("D150_VAMP_HL1_Sec2");
        uids.push_back("D150_VAMP_HL1_Sec2_Elem1");
        uids.push_back("D150_VAMP_HL1_Pos1");
        uids.push_back("D150_VAMP_HL1_Pos2");
        uids.push_back("D150_VAMP_HL1_Seg1");
        uids.push_back("D150_VAMP_HL1_CompSeg1");
        //uids.push_back("D150_VAMP_HL1_CompSeg1_elevator");
        uids.push_back("D150_VAMP_SL1");
        uids.push_back("D150_VAMP_SL1_Sec1");
        uids.push_back("D150_VAMP_SL1_Sec1_Elem1");
        uids.push_back("D150_VAMP_SL1_Sec2");
        uids.push_back("D150_VAMP_SL1_Sec2_Elem1");
        uids.push_back("D150_VAMP_SL1_Pos1");
        uids.push_back("D150_VAMP_SL1_Pos2");
        uids.push_back("D150_VAMP_SL1_Seg1");
        uids.push_back("D150_VAMP_SL1_CompSeg1");
        //uids.push_back("D150_VAMP_SL1_CompSeg1_rudder");
        //uids.push_back("4CM036_D150");
        uids.push_back("D150_VAMP_FL1_ProfCirc");
        uids.push_back("D150_VAMP_FL1_ProfSupEl1");
        uids.push_back("D150_VAMP_FL1_ProfSupEl2");
        uids.push_back("D150_VAMP_FL1_ProfSupEl3");
        uids.push_back("D150_VAMP_FL1_ProfSupEl4");
        uids.push_back("D150_VAMP_FL1_ProfSupEl5");
        uids.push_back("D150_VAMP_FL1_ProfSupEl6");
        uids.push_back("D150_VAMP_FL1_ProfSupEl7");
        uids.push_back("D150_VAMP_FL1_ProfSupEl8");
        uids.push_back("D150_VAMP_FL1_ProfSupEl9");
        uids.push_back("D150_VAMP_FL1_ProfSupEl10");
        uids.push_back("NACA0012");
        uids.push_back("D150_VAMP_W_SupCritProf1");
        uids.push_back("D150_VAMP_HL_NACA0000Prof1");
        uids.push_back("D150_VAMP_SL_NACA0000Prof1");

        for (std::size_t i = 0; i < uids.size(); i++) {
            tigl::CTiglUIDManager::TypedPtr ptr = uidMgr->ResolveObject(uids[i]);
            ASSERT_TRUE(ptr.ptr != NULL);
            ASSERT_TRUE(ptr.type != NULL);
        }
    }

    TEST_F(tiglUidManagerTest, aircraftUid) {
        tigl::CCPACSAircraftModel& model = uidMgr->ResolveObject<tigl::CCPACSAircraftModel>("D150_VAMP");
        ASSERT_TRUE(model.GetUID() == "D150_VAMP");
    }

    TEST_F(tiglUidManagerTest, wingUid) {
        tigl::CCPACSWing& wing = uidMgr->ResolveObject<tigl::CCPACSWing>("D150_VAMP_HL1");
        ASSERT_TRUE(wing.GetUID() == "D150_VAMP_HL1");
    }

    TEST_F(tiglUidManagerTest, allWings) {
        std::vector<tigl::CCPACSWing*> wings = uidMgr->ResolveObjects<tigl::CCPACSWing>();
        ASSERT_TRUE(wings.size() == 3);
    }

    TEST_F(tiglUidManagerTest, makeUIDUnique)
    {
        EXPECT_EQ("AlreadyUniqueUID", uidMgr->MakeUIDUnique("AlreadyUniqueUID"));
        EXPECT_EQ("D150_VAMP_W1_Sec3_Elem1U1", uidMgr->MakeUIDUnique("D150_VAMP_W1_Sec3_Elem1"));
        // create a existing prefixed UID
        uidMgr->ResolveObject<tigl::CCPACSWingSectionElement>("D150_VAMP_W1_Sec2_Elem1")
            .SetUID("D150_VAMP_W1_Sec3_Elem1U1");
        EXPECT_EQ("D150_VAMP_W1_Sec3_Elem1U2", uidMgr->MakeUIDUnique("D150_VAMP_W1_Sec3_Elem1"));
    }

}
