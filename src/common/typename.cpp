#include "typename.h"

#include <boost/core/demangle.hpp>

#include <stdexcept>

namespace tigl {
    auto typeName(const std::type_info& ti) -> std::string {
        // name holds fully qualified class name
        // boost::demangle is required for GCC, just passes through string on VC++
        auto name = boost::core::demangle(ti.name()); // e.g. "[class] tigl::CCPACSWing"

        // strip leading "class" (VC++)
        const auto pos = name.find_last_of(' ');
        if (pos != std::string::npos)
            name = name.substr(pos + 1); // e.g. "tigl::CCPACSWing"

        return name;
    }
}
