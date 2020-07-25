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

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include "CNamedShape.h"
#include "PNamedShape.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSFrame.h"
#include "CCPACSPressureBulkheadAssemblyPosition.h"
#include "CCPACSFuselage.h"


TEST(TestFuselageFrame, bug694)
{
    const char* filename = "TestData/bugs/694/test_frame.xml";
    TiglHandleWrapper tiglHandle(filename, "");

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    // get the geometry of the frame and check for the correct position via bounding box
    const char* frameUid = "frame1";
    const tigl::CCPACSFrame& frame = config.GetUIDManager().ResolveObject<tigl::CCPACSFrame>(frameUid);
    const TopoDS_Shape frameShape = frame.GetGeometry(true);
    Bnd_Box frameBBox;
    BRepBndLib::Add(frameShape, frameBBox);

    const char* fuselageUid = "Fuselage";
    const tigl::CCPACSFuselage& fuselage = config.GetUIDManager().ResolveObject<tigl::CCPACSFuselage>(fuselageUid);
    const TopoDS_Shape fuselageShape = fuselage.GetLoft()->Shape();
    Bnd_Box fuselageBBox;
    BRepBndLib::Add(fuselageShape, fuselageBBox);

    // precision relative to Z extent of fuselage
    const double precision = 1.E-2 * (fuselageBBox.CornerMax().Z() - fuselageBBox.CornerMin().Z());

    // check that lower point of frame is above middle Z-coordinate of fuselage
    ASSERT_TRUE(frameBBox.CornerMin().Z() > (fuselageBBox.CornerMin().Z() + fuselageBBox.CornerMax().Z()) / 2.0);
    // check that upper point of frame is at top of fuselage
    ASSERT_NEAR(frameBBox.CornerMax().Z(), fuselageBBox.CornerMax().Z(), precision);
}

TEST(TestFuselageFrame, bug710)
{
    // The cpacs file is missing frame positionings. without them, neither the frame, nor the pressure bulkhead can be built.
    // TiGL should throw an error and not segfault when trying to build the pressure bulkhead

    const char* filename = "TestData/bugs/710/test_fuselage_walls_ex1.xml";
    TiglHandleWrapper tiglHandle(filename, "");

    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = manager.GetConfiguration(tiglHandle);

    // get the geometry of the frame and check for the correct position via bounding box
    const char* uid = "pressureBulkhead1";
    const tigl::CCPACSPressureBulkheadAssemblyPosition& bhp = config.GetUIDManager().ResolveObject<tigl::CCPACSPressureBulkheadAssemblyPosition>(uid);
    tigl::CCPACSFrame& frame = config.GetUIDManager().ResolveObject<tigl::CCPACSFrame>(bhp.GetFrameUID());
    EXPECT_EQ(frame.GetFramePositions().size(), 0);

    ASSERT_THROW(bhp.GetGeometry(), tigl::CTiglError);
}
