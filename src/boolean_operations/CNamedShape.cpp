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

#include "CNamedShape.h"

#include <cassert>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepBuilderAPI_Copy.hxx>

CNamedShape::CNamedShape()
{
    Clear();
}

CNamedShape::CNamedShape(const TopoDS_Shape &shape, const char *shapeName)
    : _myshape(shape), _myname(shapeName)
{
    MakeFaceNamesFromName();
}

CNamedShape::CNamedShape(const CNamedShape& ns)
    : _myshape(ns._myshape)
{
    *this = ns;
}

CNamedShape& CNamedShape::operator= (const CNamedShape& ns)
{
    _myshape = ns._myshape;
    _myname  = ns._myname;
    _myfacenames = ns._myfacenames;

    return *this;
}

PNamedShape CNamedShape::DeepCopy() const
{
    PNamedShape shape(new CNamedShape(*this));
    shape->_myshape = BRepBuilderAPI_Copy(_myshape);

    return shape;
}

void CNamedShape::Clear()
{
    _myshape.Nullify();
    _myname = "UNKNOWN";
    _myfacenames.clear();
}

const TopoDS_Shape& CNamedShape::Shape() const{
    return _myshape;
}

const char* CNamedShape::Name()  const {
    return _myname.c_str();
}

const StringList& CNamedShape::FaceNames() const
{
    return _myfacenames;
}

const char *CNamedShape::GetFaceName(int iFace) const
{
    return _myfacenames.at(iFace).c_str();
}

unsigned int CNamedShape::GetFaceCount() const
{
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(_myshape, TopAbs_FACE, faceMap);
    return faceMap.Extent();
}

void CNamedShape::SetFaceName(int iFace, const char *faceName)
{
    _myfacenames.at(iFace) = faceName;
}

void CNamedShape::SetFaceNames(const StringList & list)
{
    assert(list.size() == GetFaceCount());

    _myfacenames = list;
}

void CNamedShape::SetShape(const TopoDS_Shape& shape)
{
    _myshape = shape;
}

void CNamedShape::SetName(const char * name)
{
    _myname = name;
}

void CNamedShape::MakeFaceNamesFromName()
{
    _myfacenames.clear();
    for(unsigned int i = 0; i < GetFaceCount(); ++i) {
        _myfacenames.push_back(_myname);
    }
}

CNamedShape::~CNamedShape(){
}

