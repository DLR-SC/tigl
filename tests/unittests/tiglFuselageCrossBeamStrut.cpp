/*
* Copyright (c) 2026 German Aerospace Center (DLR/SC)
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

#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Vec.hxx>

#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSCrossBeamStrutAssemblyPosition.h"

#include <cmath>

namespace
{
// direction of the line of a strut, from the cross beam to the frame, in the fuselage coordinate system
gp_Vec StrutDirection(const tigl::CCPACSCrossBeamStrutAssemblyPosition& strut)
{
    const TopoDS_Edge edge = TopoDS::Edge(strut.GetGeometry(true, FUSELAGE_COORDINATE_SYSTEM));
    const gp_Vec direction(BRep_Tool::Pnt(TopExp::FirstVertex(edge)), BRep_Tool::Pnt(TopExp::LastVertex(edge)));
    return direction.Normalized();
}
} // namespace

// angleX is measured like the referenceAngle of a stringer or frame position: the z-axis rotated about the x-axis,
// 180 degrees (the default) pointing straight down
TEST(TestFuselageCrossBeamStrut, angleXFromZAxis)
{
    TiglHandleWrapper tiglHandle("TestData/fuselage_structure-v3.xml", "");
    tigl::CCPACSConfiguration& config = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    tigl::CCPACSCrossBeamStrutAssemblyPosition& strut =
        config.GetUIDManager().ResolveObject<tigl::CCPACSCrossBeamStrutAssemblyPosition>("cargoCrossBeamStrut5");

    // angleX = 170: inclined by 10 degrees towards -y
    ASSERT_NEAR(*strut.GetAngleX(), 170., 1e-12);
    const double angle = 170. * M_PI / 180.;
    gp_Vec direction   = StrutDirection(strut);
    EXPECT_NEAR(direction.X(), 0., 1e-6);
    EXPECT_NEAR(direction.Y(), -std::sin(angle), 1e-6);
    EXPECT_NEAR(direction.Z(), std::cos(angle), 1e-6);

    // without angleX, the strut is vertical
    strut.SetAngleX(boost::none);
    direction = StrutDirection(strut);
    EXPECT_NEAR(direction.X(), 0., 1e-6);
    EXPECT_NEAR(direction.Y(), 0., 1e-6);
    EXPECT_NEAR(direction.Z(), -1., 1e-6);

    // 200 degrees: inclined by 20 degrees towards +y
    strut.SetAngleX(200.);
    direction = StrutDirection(strut);
    EXPECT_NEAR(direction.Y(), -std::sin(200. * M_PI / 180.), 1e-6);
    EXPECT_NEAR(direction.Z(), std::cos(200. * M_PI / 180.), 1e-6);
}
