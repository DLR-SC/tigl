/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
*
* Created: 2015-06-15 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CBopCommon.h"

#include <cassert>

#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <Standard_Version.hxx>

#include "CBooleanOperTools.h"
#include "CNamedShape.h"

CBopCommon::CBopCommon(const PNamedShape shape, const PNamedShape cuttingTool)
    :  _resultshape(), _tool(cuttingTool), _source(shape), _dsfiller(nullptr)
{
    _fillerAllocated = false;
    _hasPerformed = false;
}

CBopCommon::CBopCommon(const PNamedShape shape, const PNamedShape cuttingTool, const BOPAlgo_PaveFiller & filler)
    :  _resultshape(), _tool(cuttingTool), _source(shape)
{
    _fillerAllocated = false;
    _hasPerformed = false;
    _dsfiller = const_cast<BOPAlgo_PaveFiller*>(&filler);
}

CBopCommon::~CBopCommon()
{
    if (_fillerAllocated && _dsfiller) {
        delete _dsfiller;
        _dsfiller = nullptr;
    }
}

CBopCommon::operator PNamedShape()
{
    return NamedShape();
}

void CBopCommon::PrepareFiller()
{
    if (!_tool || !_source) {
        return;
    }

    if (!_dsfiller) {
#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,3,0)
        BOPCol_ListOfShape aLS;
#else
        TopTools_ListOfShape aLS;
#endif
        aLS.Append(_tool->Shape());
        aLS.Append(_source->Shape());

        _dsfiller = new BOPAlgo_PaveFiller;
        _fillerAllocated = true;

        _dsfiller->SetArguments(aLS);
        _dsfiller->Perform();
    }
}

void CBopCommon::Perform()
{
    if (!_hasPerformed) {
        if (!_tool || !_source) {
           _resultshape.reset();
            return;
        }

        PrepareFiller();

        // use opencascade cutting routine (might be buggy)
        BRepAlgoAPI_Common commonTool(_source->Shape(), _tool->Shape(), *_dsfiller);

        TopoDS_Shape commonShape = commonTool.Shape();

        _resultshape = PNamedShape(new CNamedShape(commonShape, _source->Name()));
        _resultshape->SetName("blubb");
        CBooleanOperTools::MapFaceNamesAfterBOP(commonTool, _source, _resultshape);
        CBooleanOperTools::MapFaceNamesAfterBOP(commonTool, _tool,   _resultshape);

        _hasPerformed = true;
    }
}

const PNamedShape CBopCommon::NamedShape()
{
    Perform();
    return _resultshape;
}




