/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-15 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLMEMORYPOOL_H
#define CTIGLMEMORYPOOL_H

#include "tigl_internal.h"

namespace tigl
{

class TIGL_EXPORT CTiglMemoryPool
{
public:
    CTiglMemoryPool();
    ~CTiglMemoryPool();

    /// This method inserst a copy of s into the memory
    /// manager, which can then me accessed by reference
    /// This can by usable, when you want to pass some
    /// string to the tigl c api and you want to make sure,
    /// that the created memory will be freed when closing
    /// the configuration.
    const char* MakeNontempString(const char * s);

private:
    class CTiglMemoryPoolImpl* impl;
};

}
#endif // CTIGLMEMORYPOOL_H
