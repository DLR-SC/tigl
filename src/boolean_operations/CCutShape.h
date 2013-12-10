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

#ifndef CCUTSHAPE_H
#define CCUTSHAPE_H

#include "CNamedShape.h"

class BOPAlgo_PaveFiller;

class CCutShape
{
public:
    // the trimming tool must be a solid!
    CCutShape(const PNamedShape shape, const PNamedShape cuttingTool);
    CCutShape(const PNamedShape shape, const PNamedShape cuttingTool, const BOPAlgo_PaveFiller&);
    virtual ~CCutShape();

    operator PNamedShape ();

    void Perform();
    const PNamedShape NamedShape();

protected:
    void PrepareFiller();

    bool _hasPerformed;

    PNamedShape _resultshape, _tool, _source;
    BOPAlgo_PaveFiller* _dsfiller;
    bool _fillerAllocated;

};

#endif // CCUTSHAPE_H
