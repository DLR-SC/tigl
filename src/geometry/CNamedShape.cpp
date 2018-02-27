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

CNamedShape::CNamedShape(const TopoDS_Shape &shape, const std::string& shapeName)
    : _myshape(shape), _myname(shapeName)
{
    SetShortName(shapeName);
    InitFaceTraits();
}

CNamedShape::CNamedShape(const TopoDS_Shape &shape, const std::string &shapeName, const std::string &shapeShortName)
    : _myshape(shape), _myname(shapeName), _myshortName(shapeShortName)
{
    InitFaceTraits();
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
    _myshortName = ns._myshortName;
    _myfaceTraits = ns._myfaceTraits;

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
    _myshortName = "UNKNOWN";
    _myfaceTraits.clear();
}

const TopoDS_Shape& CNamedShape::Shape() const
{
    return _myshape;
}

std::string CNamedShape::Name() const
{
    return _myname;
}

std::string CNamedShape::ShortName()  const
{
    return _myshortName;
}

unsigned int CNamedShape::GetFaceCount() const
{
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(_myshape, TopAbs_FACE, faceMap);
    return faceMap.Extent();
}

void CNamedShape::SetShape(const TopoDS_Shape& shape)
{
    _myshape = shape;
}

void CNamedShape::SetName(const std::string& name)
{
    _myname = name;
}


void CNamedShape::SetShortName(const std::string& name)
{
    std::string sname = name;
    if (sname.size() > 8) {
        sname = sname.substr(0,8);
    }
    _myshortName = sname;
}

void CNamedShape::InitFaceTraits()
{
    _myfaceTraits.clear();
    for (unsigned int i = 0; i < GetFaceCount(); ++i) {
        CFaceTraits traits;
        traits.SetName(Name());
        _myfaceTraits.push_back(traits);
    }
}

const CFaceTraits& CNamedShape::GetFaceTraits(int iFace) const
{
    return _myfaceTraits.at(iFace);
}

CFaceTraits& CNamedShape::FaceTraits(int iFace)
{
    return  _myfaceTraits.at(iFace);
}

void CNamedShape::SetFaceTraits(int iFace, const CFaceTraits &traits)
{
    _myfaceTraits.at(iFace) = traits;
}

CNamedShape::~CNamedShape()
{
}

CFaceTraits::CFaceTraits()
    : _origin(), _indexInOrigin(0), _faceName("")
{}

void CFaceTraits::SetName(const std::string& name)
{
    _faceName = name;
}

std::string CFaceTraits::Name() const
{
    return _faceName.c_str();
}

void CFaceTraits::SetIndex(unsigned int index)
{
    _indexInOrigin = index;
}

unsigned int CFaceTraits::Index() const
{
    return _indexInOrigin;
}

void CFaceTraits::SetOrigin(const PNamedShape origin)
{
    _origin = origin;
}

const PNamedShape CFaceTraits::Origin() const
{
    return _origin;
}

void CFaceTraits::SetDerivedFromShape(PNamedShape origin, unsigned int iface)
{
    *this = origin->GetFaceTraits(iface);
    if (!Origin()) {
        SetOrigin(origin);
    }
}


