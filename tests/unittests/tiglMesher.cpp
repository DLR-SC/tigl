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
#include <BRepPrimAPI_MakeBox.hxx>
#include <gmsh.h>
#include <vector>
#include <TopoDS_Shape.hxx>
#include <tigl.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSWing.h>


TEST(tiglMesher, cube)
{
    // create cube in gmsh to test if linking with gmsh works

    gmsh::initialize();
    gmsh::model::occ::addBox(0,0,0,5,5,5);
    gmsh::model::occ::synchronize();

    ASSERT_EQ(gmsh::model::getDimension(), 3);

    gmsh::clear();

    gmsh::finalize();
}

TEST(tiglMesher, cubeTopoDS)
{
    // creat cube as TopoDS-Shpae and mesh to test the Mesher functions

    tigl::Mesher m {};
    gmsh::clear();

    BRepPrimAPI_MakeBox testBox {5,5,5};
    TopoDS_Shape testShape = testBox.Solid();

    m.import(testShape);

    ASSERT_EQ(gmsh::model::getDimension(), 3);

    //set options for meshing the shape. dim = 2, min and max elementsize = 5 -> lenght of cube edges
    m.set_options(2, 5, 5);

    m.mesh();
    //mesh contains four triangular elements on every face -> 14 nodes and 36 elements

    //test if the mesh has 14 nodes ('nodeTags' contains all nodes in the mesh)
    std::vector<std::size_t>  nodeTags;
    std::vector<double>  coord;
    std::vector<double>  parametricCoord;
    gmsh::model::mesh::getNodes(nodeTags, coord, parametricCoord);

    ASSERT_EQ(nodeTags.size(), 14);

    //refine the mesh by splitting the elements and test if the new mesh has 50 nodes
    m.refine();
    std::vector<std::size_t>  nodeTags_r;
    std::vector<double>  coord_r;
    std::vector<double>  parametricCoord_r;
    gmsh::model::mesh::getNodes(nodeTags_r, coord_r, parametricCoord_r);

    ASSERT_EQ(nodeTags_r.size(), 50);


    m.write("test1.msh");

    gmsh::clear();
    }


