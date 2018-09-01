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

#include "CCutShape.h"

#include <cassert>

#include <Standard_Version.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Cut.hxx>

#include "CBooleanOperTools.h"
#include "CTrimShape.h"
#include "CMergeShapes.h"
#include "CNamedShape.h"

#define USE_OWN_ALGO

CCutShape::CCutShape(const PNamedShape shape, const PNamedShape cuttingTool)
    :  _resultshape(), _tool(cuttingTool), _source(shape), _dsfiller(NULL)
{
    _fillerAllocated = false;
    _hasPerformed = false;
}

CCutShape::CCutShape(const PNamedShape shape, const PNamedShape cuttingTool, const BOPAlgo_PaveFiller & filler)
    :  _resultshape(), _tool(cuttingTool), _source(shape)
{
    _fillerAllocated = false;
    _hasPerformed = false;
    _dsfiller = (BOPAlgo_PaveFiller*) &filler;
}

CCutShape::~CCutShape()
{
    if (_fillerAllocated && _dsfiller) {
        delete _dsfiller;
        _dsfiller = NULL;
    }
}

CCutShape::operator PNamedShape()
{
    return NamedShape();
}

void CCutShape::PrepareFiller()
{
    if (!_tool || !_source) {
        return;
    }

    if (!_dsfiller) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
        TopTools_ListOfShape aLS;
#else
        BOPCol_ListOfShape aLS;
#endif
        aLS.Append(_tool->Shape());
        aLS.Append(_source->Shape());

        _dsfiller = new BOPAlgo_PaveFiller;
        _fillerAllocated = true;

        _dsfiller->SetArguments(aLS);
        _dsfiller->Perform();
    }
}

void CCutShape::Perform()
{
    if (!_hasPerformed) {
        if (!_tool || !_source) {
           _resultshape.reset();
            return;
        }

        PrepareFiller();
#ifdef USE_OWN_ALGO
        CTrimShape trim1(_source, _tool, *_dsfiller, EXCLUDE);
        PNamedShape shape1 = trim1.NamedShape();

        CTrimShape trim2(_tool, _source, *_dsfiller, INCLUDE);
        PNamedShape shape2 = trim2.NamedShape();

        _resultshape = CMergeShapes(shape1, shape2);
#else
        // use opencascade cutting routine (might be buggy)
        BRepAlgoAPI_Cut cutter(_source->Shape(), _tool->Shape(), *_dsfiller, Standard_True);

        TopoDS_Shape cuttedShape = cutter.Shape();

        _resultshape = PNamedShape(new CNamedShape(cuttedShape, _source->Name()));
        CBooleanOperTools::MapFaceNamesAfterBOP(cutter, *_source, *_resultshape);
        CBooleanOperTools::MapFaceNamesAfterBOP(cutter, *_tool,   *_resultshape);
#endif

        _hasPerformed = true;
    }
}

const PNamedShape CCutShape::NamedShape()
{
    Perform();
    return _resultshape;
}


