#pragma once

#include <string>
#include <cctype>
#include "to_string.h"
#include "tigl_internal.h"
#include "CTiglError.h"

namespace tigl
{
#ifdef HAVE_CPP11
    enum class TiglFarFieldType
#else
    enum TiglFarFieldType
#endif
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
            case ENUM_VALUE(TiglFarFieldType, HALF_SPHERE): return "halfSphere";
            case ENUM_VALUE(TiglFarFieldType, FULL_SPHERE): return "fullSphere";
            case ENUM_VALUE(TiglFarFieldType, HALF_CUBE):   return "halfCube";
            case ENUM_VALUE(TiglFarFieldType, FULL_CUBE):   return "fullCube";
            case ENUM_VALUE(TiglFarFieldType, NONE):        return "none";
            default: throw CTiglError("Invalid enum value \"" + std_to_string(static_cast<int>(value)) + "\" for enum type TiglFarFieldType");
        }
    }

    inline TiglFarFieldType stringToTiglFarFieldType(const std::string& value)
    {
        struct ToLower { std::string operator()(std::string str) { for (std::size_t i = 0; i < str.length(); i++) {str[i] = std::tolower(str[i]); } return str; } } toLower;
        if (toLower(value) == "halfsphere") { return ENUM_VALUE(TiglFarFieldType, HALF_SPHERE); }
        if (toLower(value) == "fullsphere") { return ENUM_VALUE(TiglFarFieldType, FULL_SPHERE); }
        if (toLower(value) == "halfcube")   { return ENUM_VALUE(TiglFarFieldType, HALF_CUBE);   }
        if (toLower(value) == "fullcube")   { return ENUM_VALUE(TiglFarFieldType, FULL_CUBE);   }
        if (toLower(value) == "none")       { return ENUM_VALUE(TiglFarFieldType, NONE);        }
        throw CTiglError("Invalid string value \"" + value + "\" for enum type TiglFarFieldType");
    }
}
