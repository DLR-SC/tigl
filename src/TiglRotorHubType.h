/*
* Copyright (c) 2018 RISC Software GmbH
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


#pragma once

#include <string>
#include <cctype>
#include "to_string.h"
#include "tigl_internal.h"
#include "CTiglError.h"

namespace tigl
{

enum TiglRotorHubType
{
    TIGLROTORHUB_UNDEFINED,
    TIGLROTORHUB_SEMI_RIGID,
    TIGLROTORHUB_RIGID,
    TIGLROTORHUB_ARTICULATED,
    TIGLROTORHUB_HINGELESS
};

inline std::string TiglRotorHubTypeToString(const TiglRotorHubType& value)
{
    switch (value) {
    case TIGLROTORHUB_SEMI_RIGID:
        return "semiRigid";
    case TIGLROTORHUB_RIGID:
        return "rigid";
    case TIGLROTORHUB_ARTICULATED:
        return "articulated";
    case TIGLROTORHUB_HINGELESS:
        return "hingeless";
    case TIGLROTORHUB_UNDEFINED:
        return "undefined";
    default:
        throw CTiglError("Invalid enum value \"" + std_to_string(static_cast<int>(value)) +
                         "\" for enum type TiglRotorHubType");
    }
}

inline TiglRotorHubType stringToTiglRotorHubType(const std::string& value)
{
    struct ToLower {
        std::string operator()(std::string str)
        {
            for (std::size_t i = 0; i < str.length(); i++) {
                str[i] = std::tolower(str[i]);
            }
            return str;
        }
    } toLower;
    if (toLower(value) == "semiRigid") {
        return TIGLROTORHUB_SEMI_RIGID;
    }
    if (toLower(value) == "rigid") {
        return TIGLROTORHUB_RIGID;
    }
    if (toLower(value) == "articulated") {
        return TIGLROTORHUB_ARTICULATED;
    }
    if (toLower(value) == "hingeless") {
        return TIGLROTORHUB_HINGELESS;
    }
    if (toLower(value) == "undefined") {
        return TIGLROTORHUB_UNDEFINED;
    }
    throw CTiglError("Invalid string value \"" + value + "\" for enum type TiglRotorHubType");
}

} // namespace tigl
