/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-12 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglPolyDataTools.h"

// OCCT includes
#include <BRep_Builder.hxx>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

namespace tigl {

CTiglPolyDataTools::CTiglPolyDataTools()
{
}

TopoDS_Shape CTiglPolyDataTools::MakeTopoDS(CTiglPolyData& mesh) {
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    tigl::CTiglPolyObject& co = mesh.currentObject();
    for(unsigned long ipoly = 0; ipoly < co.getNPolygons(); ++ipoly) {
        BRepBuilderAPI_MakePolygon poly;
        for (unsigned int ipoint = 0; ipoint < co.getNPointsOfPolygon(ipoly); ipoint++){
            long index =  co.getVertexIndexOfPolygon(ipoint, ipoly);
            tigl::CTiglPoint p = co.getVertexPoint(index);
            poly.Add(p.Get_gp_Pnt());
        }

        poly.Close();

        try {
            BRepBuilderAPI_MakeFace facemaker(poly.Wire(), Standard_False);
            if(facemaker.IsDone()){
                TopoDS_Face triangleFace = facemaker.Face();
                builder.Add(compound, triangleFace);
            }
        }
        catch(...) {
            // ignore error
        }
    }

    return compound;
}

} // namespace tigl
