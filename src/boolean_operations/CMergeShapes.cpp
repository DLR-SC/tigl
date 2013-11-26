#include "CMergeShapes.h"

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


CMergeShapes::CMergeShapes(const CNamedShape &shape, const CNamedShape &tool)
    : _source(shape), _tool(tool)
{
    _hasPerformed = false;
}



CMergeShapes::~CMergeShapes()
{
}

CMergeShapes::operator CNamedShape()
{
    return NamedShape();
}


void CMergeShapes::Perform()
{
    if(!_hasPerformed) {
        CNamedShape& s1 = _source;
        CNamedShape& s2 = _tool;

        // check input shapes
        if(s1.Shape().IsNull() && !s2.Shape().IsNull()) {
            _resultshape = s2;
            _hasPerformed = true;
            return;
        }
        else if(!s1.Shape().IsNull() && s2.Shape().IsNull()) {
            _resultshape = s1;
            _hasPerformed = true;
            return;
        }
        else if(s1.Shape().IsNull() && s2.Shape().IsNull()) {
            _resultshape = CNamedShape();
            _hasPerformed = true;
            return;
        }

        // both shapes are valid
        BRepBuilderAPI_Sewing sewer;

        TopTools_IndexedMapOfShape m1, m2;
        TopExp::MapShapes(s1.Shape(), TopAbs_FACE, m1);
        TopExp::MapShapes(s2.Shape(), TopAbs_FACE, m2);

        assert(m1.Extent() == m2.Extent());

        // remove common faces
        for(int iface = 1; iface <= m1.Extent(); ++iface){
            const TopoDS_Face& f1 = TopoDS::Face(m1(iface));
            const TopoDS_Face& f2 = TopoDS::Face(m2(iface));

            gp_Pnt p1 = GetCentralFacePoint(f1);
            gp_Pnt p2 = GetCentralFacePoint(f2);

            if(p1.Distance(p2) > Precision::Confusion()){
                sewer.Add(f1);
                sewer.Add(f2);
            }
        }

        sewer.Perform();
        TopoDS_Shape shell = sewer.SewedShape();
        TopoDS_Shape solid = BRepBuilderAPI_MakeSolid(TopoDS::Shell(shell)).Solid();

        CNamedShape result(solid, "SEW_FUSE");
        BRepSewingToBRepBuilderShapeAdapter adapter(sewer);
        CBooleanOperTools::MapFaceNamesAfterBOP(adapter, s1, result);
        CBooleanOperTools::MapFaceNamesAfterBOP(adapter, s2, result);

        _resultshape = result;

        _hasPerformed = true;
    }
}

const CNamedShape &CMergeShapes::NamedShape()
{
    Perform();
    return _resultshape;
}



