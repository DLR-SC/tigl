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

#include "PNamedShape.h"
#include "ListPNamedShape.h"
#include "tigl_internal.h"

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
    TIGL_EXPORT CFuseShapes(const PNamedShape parent, const ListPNamedShape& childs);
    TIGL_EXPORT virtual ~CFuseShapes();

    // returns result of fusing operation
    TIGL_EXPORT operator PNamedShape ();
    TIGL_EXPORT const    PNamedShape NamedShape();

    TIGL_EXPORT const ListPNamedShape& Intersections();
    TIGL_EXPORT const ListPNamedShape& TrimmedChilds();
    TIGL_EXPORT const PNamedShape   TrimmedParent();

    TIGL_EXPORT void Perform();


protected:
    void Clear();
    void DoFuse();

    bool _hasPerformed;

    PNamedShape _resultshape, _parent, _trimmedParent;
    ListPNamedShape _childs, _trimmedChilds, _intersections;
};
#endif // CFUSESHAPES_H
