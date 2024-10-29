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

#ifndef CBOPCOMMON_H
#define CBOPCOMMON_H


#include "PNamedShape.h"
#include "tigl_internal.h"

class BOPAlgo_PaveFiller;

class CBopCommon
{
public:
    // the trimming tool must be a solid!
    TIGL_EXPORT CBopCommon(const PNamedShape shape, const PNamedShape tool);
    TIGL_EXPORT CBopCommon(const PNamedShape shape, const PNamedShape tool, const BOPAlgo_PaveFiller&);
    TIGL_EXPORT virtual ~CBopCommon();

    TIGL_EXPORT operator PNamedShape ();

    TIGL_EXPORT void Perform();
    TIGL_EXPORT const PNamedShape NamedShape();

protected:
    void PrepareFiller();

    bool _hasPerformed;

    PNamedShape _resultshape, _tool, _source;
    BOPAlgo_PaveFiller* _dsfiller;
    bool _fillerAllocated;

};

#endif // CBOPCOMMON_H
