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
#include <BRepPrimAPI_MakeBox.hxx>
#include <gmsh.h>

TEST(tiglMesher, cube)
{
    // create cube in gmsh to test if linking with gmsh works

    gmsh::initialize();
    gmsh::model::occ::addBox(0,0,0,5,5,5);
    gmsh::model::occ::synchronize();

    ASSERT_EQ(gmsh::model::getDimension(), 3);

    gmsh::finalize();
}

TEST(tiglMesher, cubeTopoDS)
{
    // creat cube as TopoDS-Shpae and mesh to test the Mesher functions

    Mesher m {};

    BRepPrimAPI_MakeBox testBox {5,5,5};
    TopoDS_Shape testShape = testBox.Solid();

    m.import(testShape);
    //gmsh::model::occ::synchronize();

    ASSERT_EQ(gmsh::model::getDimension(), 3);

    //set options for meshing the shape. dim = 3, name = "testcube.msh"
    m.set_options(3,"testcube.msh");
    m.mesh();

    //test if the maxNodeTag of the Mesh is 430 (Mesh should have 348 Nodes)
    std::size_t  maxTag;
    gmsh::model::mesh::getMaxNodeTag(maxTag);
    ASSERT_EQ(maxTag, 430);

    //test if the maxElementTag of the Mesh is 6483 (Mesh should have 1790 Elements)
    std::size_t  maxTag2;
    gmsh::model::mesh::getMaxElementTag(maxTag2);
    ASSERT_EQ(maxTag2, 6483);

    //refine the mesh and test if the new MaxElementTag is 19913
    m.refine();
    std::size_t  maxTag3;
    gmsh::model::mesh::getMaxElementTag(maxTag3);
    ASSERT_EQ(maxTag3, 19913);

}


TEST(tiglMesher, wing)
{

    // open cpacs configuration, read wing TopoDS_Shape and create mesh
}

TEST(tigLMesher, fused)
{

    // open cpacs configuration, fuse aircract and mesh far field
}
