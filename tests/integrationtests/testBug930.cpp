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
#include "CTiglUIDManager.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"
#include "BRepTools.hxx"


TEST(Bug930, cell_getLoft)
{
   // https://github.com/DLR-SC/tigl/issues/930
   // cells on the lower shell span until the trailing edge, eventhough the contour coordinate is not 1. The same contour coordinates work
   // as expected on the upper shell. The cell span01_circ07 is one such offending cell.

   TiglHandleWrapper handle("TestData/bugs/930/IEA-25-310-UWT_ModGrid_final_red_mass_ver_2_loads_CPACS_cc_surface.xml", "aircraft");

   tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
   tigl::CCPACSConfiguration & config = manager.GetConfiguration(handle);
   tigl::CTiglUIDManager& uidMgr = config.GetUIDManager();

   auto span01_circ07 = uidMgr.GetGeometricComponent("span01_circ07").GetLoft()->Shape();

   double xmin, xmax, ymin, ymax, zmin, zmax;
   GetShapeExtension(span01_circ07, xmin, xmax, ymin, ymax, zmin, zmax);
   EXPECT_NEAR(xmin, -3.067, 0.01);
   EXPECT_NEAR(xmax, -0.372, 0.01);
   EXPECT_NEAR(ymin, 0., 0.01);
   EXPECT_NEAR(ymax, 30.225, 0.01);
   EXPECT_NEAR(zmin, -3.267, 0.01);
   EXPECT_NEAR(zmax, -0.443, 0.01);

   BRepTools::Write(span01_circ07, "TestData/export/bug930_span01_circ07.brep");

}
