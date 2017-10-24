/*
* Copyright (c) 2016 RISC Software GmbH
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

#ifndef TYPENAME_H
#define TYPENAME_H

#include <string>
#include <typeinfo>

namespace tigl
{

std::string typeName(const std::type_info& ti);

template <typename T>
std::string typeName()
{
    return typeName(typeid(T));
}

}

#endif // TYPENAME_H