/*
* Copyright (c) 2020 RISC Software GmbH
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

#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingSparPosition.h"
#include "CCPACSEtaXsiPoint.h"


TEST(TestInvalidation, invalidation)
{
    const char* filename = "TestData/cell_rib_spar_test.xml";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    ASSERT_EQ(SUCCESS, tixiOpenDocument(filename, &tixiHandle));
    ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    // Wing_CS_RibDef2 is defined as starting from spar position Wing_CS_spar1_Pos2,
    // so when moving the spar position, the rib should get invalidated automatically.
    // We check the start point of the rib before and after moving the spar position,
    // when the rib was invalidated correctly the start point must have moved.
    const char* ribUid = "Wing_CS_RibDef2";
    const tigl::CCPACSWingRibsDefinition& rib = config.GetUIDManager().ResolveObject<tigl::CCPACSWingRibsDefinition>(ribUid);
    
    // get inital start point of rib
    gp_Pnt initialStartPnt, finalStartPnt, dummy;
    rib.GetRibMidplanePoints(1, initialStartPnt, dummy);

    // move spar position
    const char* sparPosUid = "Wing_CS_spar1_Pos2";
    tigl::CCPACSWingSparPosition& sparPos = config.GetUIDManager().ResolveObject<tigl::CCPACSWingSparPosition>(sparPosUid);
    sparPos.GetSparPositionEtaXsi_choice2()->SetEta(0.3);

    // get updated start point of rib (in case it was invalidated)
    rib.GetRibMidplanePoints(1, finalStartPnt, dummy);

    const double precision = 1e-7;
    const gp_Vec offset(0, -0.1, 0);
    ASSERT_NEAR(initialStartPnt.Translated(offset).Distance(finalStartPnt), 0, precision);
}
