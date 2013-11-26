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

#ifndef CNAMEDSHAPE_H
#define CNAMEDSHAPE_H

#include <string>
#include <vector>
#include <TopoDS_Shape.hxx>

typedef std::vector<std::string> StringList;

class CNamedShape
{
public:
    CNamedShape();
    CNamedShape(const TopoDS_Shape& shape, const char* shapeName);
    CNamedShape(const CNamedShape&);
    CNamedShape &operator= (const CNamedShape&);

    // creates a true copy, i.e. it duplicates the underlying shape
    CNamedShape DeepCopy() const;

    void Clear();

    // getters
    const TopoDS_Shape& Shape() const;
    const char*         Name()  const;

    const StringList& FaceNames() const;


    const char* GetFaceName(int iFace) const;
    unsigned int GetFaceCount() const;

    // setters
    void SetFaceName(int iFace, const char* faceName);
    void SetFaceNames(const StringList&);

    void SetShape(const TopoDS_Shape&);
    void SetName(const char*);

protected:
    void MakeFaceNamesFromName();

    TopoDS_Shape  _myshape;
    std::string   _myname;
    StringList    _myfacenames;
};

#endif // CNAMEDSHAPE_H
