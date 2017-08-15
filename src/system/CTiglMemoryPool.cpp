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

#include "CTiglMemoryPool.h"

#include <list>
#include <string>

namespace tigl
{

class CTiglMemoryPoolImpl
{
public:
    std::list<std::string> strings;
};

CTiglMemoryPool::CTiglMemoryPool()
    : impl(new CTiglMemoryPoolImpl)
{
}

CTiglMemoryPool::~CTiglMemoryPool()
{
    if (impl) {
        delete impl;
        impl = NULL;
    }
}

const char* CTiglMemoryPool::MakeNontempString(const char *s)
{
    impl->strings.push_back(s);
    return impl->strings.back().c_str();
}

} // namespace tigl
