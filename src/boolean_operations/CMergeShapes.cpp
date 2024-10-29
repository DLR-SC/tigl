/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-11-11 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CMergeShapes.h"

#include "CBooleanOperTools.h"
#include "BRepSewingToBRepBuilderShapeAdapter.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include <cassert>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <Precision.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <gp_Pnt.hxx>

#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>

#include <vector>

CMergeShapes::CMergeShapes(const PNamedShape shape, const PNamedShape tool)
    : _tool(tool), _source(shape)
{
    _hasPerformed = false;
}



CMergeShapes::~CMergeShapes()
{
}

CMergeShapes::operator PNamedShape()
{
    return NamedShape();
}


void CMergeShapes::Perform()
{
    if (!_hasPerformed) {
        PNamedShape s1 = _source;
        PNamedShape s2 = _tool;
        std::vector<TopoDS_Shape> v1, v2;


        // check input shapes
        if (!s1 && !s2) {
            _resultshape.reset();
            _hasPerformed = true;
            return;
        }

        if (!s1 && s2 && !s2->Shape().IsNull()) {
            _resultshape = s2;
            _hasPerformed = true;
            return;
        }

        if (!s2 && s1 && !s1->Shape().IsNull()) {
            _resultshape = s1;
            _hasPerformed = true;
            return;
        }

        if (s1->Shape().IsNull() && !s2->Shape().IsNull()) {
            _resultshape = s2;
            _hasPerformed = true;
            return;
        }
        else if (!s1->Shape().IsNull() && s2->Shape().IsNull()) {
            _resultshape = s1;
            _hasPerformed = true;
            return;
        }
        else if (s1->Shape().IsNull() && s2->Shape().IsNull()) {
            _resultshape.reset();
            _hasPerformed = true;
            return;
        }

        // both shapes are valid
        BRepBuilderAPI_Sewing sewer;

        TopTools_IndexedMapOfShape m1, m2;
        TopExp::MapShapes(s1->Shape(), TopAbs_FACE, m1);
        TopExp::MapShapes(s2->Shape(), TopAbs_FACE, m2);

        std::vector<std::pair<TopoDS_Shape, gp_Pnt>> facesOn1, facesOn2;


        // remove common faces
        for (int iface = 1; iface <= m1.Extent(); ++iface){
            const TopoDS_Face& face = TopoDS::Face(m1(iface));
            facesOn1.push_back(std::pair<TopoDS_Shape, gp_Pnt>(m1(iface),
                                                               GetCentralFacePoint(face)));

        }
        for (int iface = 1; iface <= m2.Extent(); ++iface){
            const TopoDS_Face& face = TopoDS::Face(m2(iface));
            facesOn2.push_back(std::pair<TopoDS_Shape, gp_Pnt>(m2(iface),
                                                               GetCentralFacePoint(face)));

        }

        std::vector<bool> v1_isSame(m1.Extent(), false);
        std::vector<bool> v2_isSame(m2.Extent(), false);

        for (size_t iface = 0; iface < facesOn1.size(); ++iface){
            gp_Pnt p1 = facesOn1[iface].second;
            for (size_t jface = 0; jface < facesOn2.size(); ++jface) {
                gp_Pnt p2 = facesOn2[jface].second;
                if (p1.SquareDistance(p2) < Precision::Confusion()) {
                    v1_isSame[iface] = true;
                    v2_isSame[jface] = true;
                }
            }
        }
        for (size_t iface = 0; iface < facesOn1.size(); ++iface) {
            if (!v1_isSame[iface]) {
                v1.push_back(facesOn1[iface].first);
            }
        }
        for (size_t iface = 0; iface < facesOn2.size(); ++iface) {
            if (!v2_isSame[iface]) {
                v2.push_back(facesOn2[iface].first);
            }
        }

        std::vector<TopoDS_Shape>::const_iterator it;
        for (it = v1.begin(); it != v1.end(); ++it){
            sewer.Add(*it);
        }
        for (it = v2.begin(); it != v2.end(); ++it){
            sewer.Add(*it);
        }

        sewer.Perform();
        // create solid of out sewed shape
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(sewer.SewedShape(), TopAbs_SHELL, map);
        BRepBuilderAPI_MakeSolid solidMaker;
        for (int ishell = 1; ishell <= map.Extent(); ++ishell) {
            solidMaker.Add(TopoDS::Shell(map(ishell)));
        }
        TopoDS_Shape solid = solidMaker.Solid();

        // map names to shape
        PNamedShape result(new CNamedShape(solid, "SEW_FUSE"));
        BRepSewingToBRepBuilderShapeAdapter adapter(sewer);
        CBooleanOperTools::MapFaceNamesAfterBOP(adapter, s1, result);
        CBooleanOperTools::MapFaceNamesAfterBOP(adapter, s2, result);

        _resultshape = result;

        _hasPerformed = true;
    }
}

const PNamedShape CMergeShapes::NamedShape()
{
    Perform();
    return _resultshape;
}
