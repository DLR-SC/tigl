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


TEST(Bug829, cell_getLoft)
{
   // https://github.com/DLR-SC/tigl/issues/829
   // test the creation of cell geometries from contour coordinates, that span several segments in one component segment

   TiglHandleWrapper handle("TestData/bugs/829/IEA-10.0-198-RWT_CPACS_contour_coordinates.xml", "IEA-10.0-198-RWT");

   tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
   tigl::CCPACSConfiguration & config = manager.GetConfiguration(handle);
   tigl::CCPACSWing& wing = config.GetWing(1);
   tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));

   std::vector<double> xmins = {2.195, -0.811, -0.854, -0.908, -0.688, -1.025};
   std::vector<double> xmaxs = {2.288,  0.692,  0.346, -0.137, -0.306, -0.760};

   std::vector<double> ymins = {0.100, 1.776, 28.851, 69.474, 95.367, 95.367};
   std::vector<double> ymaxs = {1.834, 5.502, 31.178, 71.526, 96.700, 96.700};

   for (int i = 1; i<=6; ++i)
   {
       auto& cell_upper = componentSegment.GetStructure()->GetUpperShell().GetCell(i);

       TopoDS_Shape cellGeom = cell_upper.GetSkinGeometry();

       double xmin, xmax, ymin, ymax, zmin, zmax;
       GetShapeExtension(cellGeom, xmin, xmax, ymin, ymax, zmin, zmax);
       EXPECT_NEAR(xmin, xmins[i-1], 0.01) << i;
       EXPECT_NEAR(xmax, xmaxs[i-1], 0.01) << i;
       EXPECT_NEAR(ymin, ymins[i-1], 0.01) << i;
       EXPECT_NEAR(ymax, ymaxs[i-1], 0.01) << i;

       BRepTools::Write(cellGeom, ("TestData/export/bug829_cell_upper_" + std::to_string(i) + ".brep").c_str());
   }
}
