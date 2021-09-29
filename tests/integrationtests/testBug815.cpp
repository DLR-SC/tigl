/*
* Copyright (C) 2021 German Aerospace Center
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
#include "CCPACSWing.h"
#include "CCPACSWingCell.h"
#include "CCPACSWingComponentSegment.h"
#include "tiglcommonfunctions.h"

TEST(Bug815, cell_getLoft)
{
   // https://github.com/DLR-SC/tigl/issues/815
   TiglHandleWrapper handle("TestData/bugs/815/Beam_Composite_Export_Test_33_cell.xml", "RectBeam");

   tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
   tigl::CCPACSConfiguration & config = manager.GetConfiguration(handle);
   tigl::CCPACSWing& wing = config.GetWing(1);
   tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));

   auto& cell_upper = componentSegment.GetStructure()->GetUpperShell().GetCell(1);

   TopoDS_Shape cellGeom = cell_upper.GetSkinGeometry();

   double xmin, xmax, ymin, ymax, zmin, zmax;
   GetShapeExtension(cellGeom, xmin, xmax, ymin, ymax, zmin, zmax);
   EXPECT_NEAR(xmin, 0.2, 0.01);
   EXPECT_NEAR(xmax, 0.8, 0.01);

   BRepTools::Write(cellGeom, "TestData/export/bug815_cell_upper.brep");
}
