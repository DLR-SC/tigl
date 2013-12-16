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

#include "CSharedPtr.h"

#include <string>
#include <vector>
#include <TopoDS_Shape.hxx>

class CNamedShape;
class CFaceTraits;

typedef std::vector<std::string> StringList;
typedef std::vector<CFaceTraits> FaceList;
typedef CSharedPtr<CNamedShape> PNamedShape;

/**
 * @brief The CFaceTraits class stores face metadata like
 * a reference to the shape where the face was originally created
 * in and its name.
 */
class CFaceTraits {
public:
    CFaceTraits();
    
    unsigned int Index() const;
    void SetIndex(unsigned int);
    
    const PNamedShape Origin() const;
    void SetOrigin(const PNamedShape);
    
    const char* Name() const;
    void SetName(const char* );
    
private:
    PNamedShape  _origin;        /** Pointer to the original shape where this face was created */
    unsigned int _indexInOrigin; /** Index of face in original shape */
    std::string   _faceName;     /** Name of the face */
};

/**
 * @brief The CNamedShape class stores a TopoDS_Shape and additional
 * metadata to origin of faces, names and history
 */
class CNamedShape
{
public:
    CNamedShape();
    CNamedShape(const TopoDS_Shape& shape, const char* shapeName);
    CNamedShape(const CNamedShape&);
    CNamedShape &operator= (const CNamedShape&);
    ~CNamedShape();

    // creates a true copy, i.e. it duplicates the underlying shape
    PNamedShape DeepCopy() const;

    void Clear();

    // getters
    const TopoDS_Shape& Shape() const;
    const char*         Name()  const;

    unsigned int GetFaceCount() const;
    const CFaceTraits& GetFaceTraits(int iFace) const;
    
    // setters
    void SetShape(const TopoDS_Shape&);
    void SetName(const char*);
    void SetFaceTraits(int iFace, const CFaceTraits& traits);

protected:
    void InitFaceTraits();

    TopoDS_Shape  _myshape;
    std::string   _myname;
    FaceList      _myfaceTraits;
};

#endif // CNAMEDSHAPE_H
