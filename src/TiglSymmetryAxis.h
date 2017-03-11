#pragma once

#include "tigl.h"

#include <string>
#include "to_string.h"
#include "CTiglError.h"


namespace tigl
{
    using ::TiglSymmetryAxis;

    inline std::string TiglSymmetryAxisToString(const TiglSymmetryAxis& value)
    {
        switch (value) {
            case TIGL_NO_SYMMETRY: return "";
            case TIGL_X_Y_PLANE:   return "x-y-plane";
            case TIGL_X_Z_PLANE:   return "x-z-plane";
            case TIGL_Y_Z_PLANE:   return "y-z-plane";
            default: throw CTiglError("Invalid enum value \"" + std_to_string(static_cast<int>(value)) + "\" for enum type TiglSymmetryAxis");
        }
    }

    inline TiglSymmetryAxis stringToTiglSymmetryAxis(const std::string& value)
    {
        if (value == "x-y-plane") { return TIGL_X_Y_PLANE; }
        if (value == "x-z-plane") { return TIGL_X_Z_PLANE; }
        if (value == "y-z-plane") { return TIGL_Y_Z_PLANE; }
        return TIGL_NO_SYMMETRY;
    }
}
