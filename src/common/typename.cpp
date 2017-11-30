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

#include "typename.h"

#include <boost/core/demangle.hpp>

#include <stdexcept>

namespace tigl
{

std::string typeName(const std::type_info& ti)
{
    // name holds fully qualified class name
    // boost::demangle is required for GCC, just passes through string on VC++
    std::string name = boost::core::demangle(ti.name()); // e.g. "[class] tigl::CCPACSWing"

    // strip leading "class" (VC++)
    const size_t  pos = name.find_last_of(' ');
    if (pos != std::string::npos) {
        name = name.substr(pos + 1); // e.g. "tigl::CCPACSWing"
    }

    return name;
}

} // namespace tigl
