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

#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Cut.hxx>


#include "CBooleanOperTools.h"

CCutShape::CCutShape(const CNamedShape &shape, const CNamedShape &cuttingTool)
    : _dsfiller(NULL), _source(shape), _tool(cuttingTool)
{
    _fillerAllocated = false;
    _hasPerformed = false;
}

CCutShape::CCutShape(const CNamedShape &shape, const CNamedShape &cuttingTool, const BOPAlgo_PaveFiller & filler)
    : _source(shape), _tool(cuttingTool)
{
    _fillerAllocated = false;
    _hasPerformed = false;
    _dsfiller = (BOPAlgo_PaveFiller*) &filler;
}

CCutShape::~CCutShape()
{
    if(_fillerAllocated && _dsfiller) {
        delete _dsfiller;
        _dsfiller = NULL;
    }
}

CCutShape::operator CNamedShape()
{
    return NamedShape();
}

void CCutShape::PrepareFiller(){
    if(!_dsfiller) {
        BOPCol_ListOfShape aLS;
        aLS.Append(_tool.Shape());
        aLS.Append(_source.Shape());

        _dsfiller = new BOPAlgo_PaveFiller;
        _fillerAllocated = true;

        _dsfiller->SetArguments(aLS);
        _dsfiller->Perform();
    }
}

void CCutShape::Perform()
{
    if(!_hasPerformed) {
        PrepareFiller();
        BRepAlgoAPI_Cut cutter(_source.Shape(), _tool.Shape(), *_dsfiller, Standard_True);

        TopoDS_Shape cuttedShape = cutter.Shape();

        _resultshape = CNamedShape(cuttedShape, _source.Name());
        CBooleanOperTools::MapFaceNamesAfterBOP(cutter, _source, _resultshape);
        CBooleanOperTools::MapFaceNamesAfterBOP(cutter, _tool,   _resultshape);

        _hasPerformed = true;
    }
}

const CNamedShape &CCutShape::NamedShape()
{
    Perform();
    return _resultshape;
}


