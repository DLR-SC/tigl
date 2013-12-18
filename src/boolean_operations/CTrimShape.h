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

#ifndef CTRIMSHAPE_H
#define CTRIMSHAPE_H

#include "PNamedShape.h"

class BOPAlgo_PaveFiller;

enum TrimOperation {
    EXCLUDE = 0,
    INCLUDE = 1
};

class CTrimShape
{
public:
    // the trimming tool must be a solid!
    CTrimShape(const PNamedShape shape, const PNamedShape trimmingTool, TrimOperation = EXCLUDE);
    CTrimShape(const PNamedShape shape, const PNamedShape trimmingTool, const BOPAlgo_PaveFiller&, TrimOperation = EXCLUDE);
    virtual ~CTrimShape();

    operator PNamedShape ();

    void Perform();
    const PNamedShape NamedShape();

protected:
    void PrepareFiller();

    bool _hasPerformed;
    TrimOperation _operation;

    PNamedShape _resultshape, _tool, _source;
    BOPAlgo_PaveFiller* _dsfiller;
    bool _fillerAllocated;

};

#endif // CTRIMSHAPE_H
