/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-27 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef ECPACSTRANSLATIONTYPE
#define ECPACSTRANSLATIONTYPE

#include <stdexcept>
#include "to_string.h"

namespace tigl
{
#ifdef HAVE_CPP11
enum class ECPACSTranslationType
#else
enum ECPACSTranslationType
#endif
{
    ABS_LOCAL,
    ABS_GLOBAL
};

inline std::string ECPACSTranslationTypeToString(const ECPACSTranslationType& value) {
    switch (value) {
        case ENUM_VALUE(ECPACSTranslationType, ABS_LOCAL):  return "absLocal";
        case ENUM_VALUE(ECPACSTranslationType, ABS_GLOBAL): return "absGlobal";
        default: throw std::runtime_error("Invalid enum value \"" + std_to_string(static_cast<int>(value)) + "\" for enum type ECPACSTranslationType");
    }
}

inline ECPACSTranslationType stringToECPACSTranslationType(const std::string& value) {
    if (value == "absLocal")  { return ENUM_VALUE(ECPACSTranslationType, ABS_LOCAL);  }
    if (value == "absGlobal") { return ENUM_VALUE(ECPACSTranslationType, ABS_GLOBAL); }
    throw std::runtime_error("Invalid string value \"" + value + "\" for enum type ECPACSTranslationType");
}

}

#endif // ECPACSTRANSLATIONTYPE

