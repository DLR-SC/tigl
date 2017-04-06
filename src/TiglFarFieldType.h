#pragma once

#include <string>
#include <cctype>
#include "to_string.h"
#include "tigl_internal.h"
#include "CTiglError.h"

namespace tigl
{
    enum TiglFarFieldType
    {
        NONE, // TODO(bgruber) not part of CPACS
        HALF_SPHERE,
        FULL_SPHERE,
        HALF_CUBE,
        FULL_CUBE
    };

    inline std::string TiglFarFieldTypeToString(const TiglFarFieldType& value)
    {
        switch (value) {
            case HALF_SPHERE: return "halfSphere";
            case FULL_SPHERE: return "fullSphere";
            case HALF_CUBE:   return "halfCube";
            case FULL_CUBE:   return "fullCube";
            case NONE:        return "none";
            default: throw CTiglError("Invalid enum value \"" + std_to_string(static_cast<int>(value)) + "\" for enum type TiglFarFieldType");
        }
    }

    inline TiglFarFieldType stringToTiglFarFieldType(const std::string& value)
    {
        struct ToLower { std::string operator()(std::string str) { for (std::size_t i = 0; i < str.length(); i++) {str[i] = std::tolower(str[i]); } return str; } } toLower;
        if (toLower(value) == "halfsphere") { return HALF_SPHERE; }
        if (toLower(value) == "fullsphere") { return FULL_SPHERE; }
        if (toLower(value) == "halfcube")   { return HALF_CUBE;   }
        if (toLower(value) == "fullcube")   { return FULL_CUBE;   }
        if (toLower(value) == "none")       { return NONE;        }
        throw CTiglError("Invalid string value \"" + value + "\" for enum type TiglFarFieldType");
    }
}
