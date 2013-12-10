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

#ifndef CFUSESHAPES_H
#define CFUSESHAPES_H

#include "CNamedShape.h"
#include <vector>

typedef std::vector<PNamedShape> ListCNamedShape;

/**
 * @brief CFuseShapes Implement a fuse, where the childs are fused with the parent
 *
 * This seems to be the most performant version so far as we can reuse a dsfiller
 * for trimming a parent and a child. It performs at the same speed as the original
 * occt fuse, but generates more reliable values.
 *
 * The function works only for solids!!!
 *
 */
class CFuseShapes
{
public:
    // all shapes must be solids!!!
    CFuseShapes(const PNamedShape parent, const ListCNamedShape& childs);
    virtual ~CFuseShapes();

    // returns result of fusing operation
    operator PNamedShape ();
    const    PNamedShape NamedShape();

    const ListCNamedShape& Intersections();
    const ListCNamedShape& TrimmedChilds();
    const PNamedShape   TrimmedParent();

    void Perform();


protected:
    void Clear();
    void DoFuse();

    bool _hasPerformed;

    PNamedShape _resultshape, _parent, _trimmedParent;
    ListCNamedShape _childs, _trimmedChilds, _intersections;
};
#endif // CFUSESHAPES_H
