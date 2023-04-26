/* 
* Copyright (C) 2007-2023 German Aerospace Center (DLR/SC)
*
* Created: 2023-01-30 Ole Albers <Ole.Albers@dlr.de>
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
#include "Mesher.h"
//#include <TopoDS_Shape.hxx>
//#include <BRepPrimAPI_MakeBox.hxx>
#include <gmsh.h>

TEST(tiglMesher, cube)
{
    //gmsh::model::mesh::generate();
    gmsh::initialize();
    gmsh::model::occ::addBox(0,0,0,5,5,5);
    gmsh::model::occ::synchronize();
   // gmsh::model::getDimension();
    // create cube and mesh to test if linking with gmsh works
    ASSERT_EQ(gmsh::model::getDimension(), 3);
}

TEST(tiglMesher, wing)
{



    // open cpacs configuration, read wing TopoDS_Shape and create mesh
}

TEST(tigLMesher, fused)
{
    // open cpacs configuration, fuse aircract and mesh far field
}
