#pragma once

#include <string>
#include <typeinfo>

namespace tigl {
    auto typeName(const std::type_info& ti) -> std::string;

    template <typename T>
    auto typeName() -> std::string {
        return typeName(typeid(T));
    }
}