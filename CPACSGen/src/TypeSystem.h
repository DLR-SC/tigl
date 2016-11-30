#pragma once

#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <stdexcept>

#include "Variant.hpp"

namespace tigl {
    struct Attribute;
    struct Element;
    struct SimpleContent;
    struct ComplexType;
    struct SimpleType;
    class SchemaParser;
    class Tables;

    enum class Cardinality {
        Optional,
        Mandatory,
        Vector
    };

    inline auto toString(const Cardinality& c) -> std::string {
        switch (c) {
            case Cardinality::Optional:  return "Optional";
            case Cardinality::Mandatory: return "Mandatory";
            case Cardinality::Vector:    return "Vector";
            default: throw std::logic_error("No toString defined");
        }
    }

    enum class XMLConstruct {
        Element,
        Attribute,
        SimpleContent,
        FundamentalTypeBase
    };

    struct Class;
    struct Enum;

    struct Field {
        Variant<const Attribute*, const Element*, const SimpleContent*> origin;
        std::string cpacsName;
        std::string typeName;
        //Variant<Class*, Enum*> type;
        XMLConstruct xmlType;
        Cardinality cardinality;

        std::string customFieldName;

        auto name() const -> const std::string&{
            if (!customFieldName.empty())
            return customFieldName;
            else
                return cpacsName;
        }

        auto fieldName() const -> std::string {
            return "m_" + name();
        }
    };

    // non owning
    struct ClassDependencies {
        // in the context of inheritance
        std::vector<Class*> bases;
        std::vector<Class*> deriveds;

        // in the context of the XML tree
        std::vector<Class*> parents;
        std::vector<Class*> children;
        std::vector<Enum*> enumChildren;
    };

    struct Class {
        const ComplexType* origin;
        std::string name;
        std::string base;
        std::vector<Field> fields;
        bool pruned;

        ClassDependencies deps;
    };

    struct EnumValue {
        std::string name;

        EnumValue() = default;
        EnumValue(const std::string& name)
            : name(name) {}

        friend auto operator==(const EnumValue& a, const EnumValue& b) -> bool {
            return a.name == b.name;
        }
    };

    struct EnumDependencies {
        std::vector<Class*> parents;
    };

    struct Enum {
        const SimpleType* origin;
        std::string name;
        std::vector<EnumValue> values;
        bool pruned;

        EnumDependencies deps;
    };

    class TypeSystem {
    public:
        TypeSystem(SchemaParser& schema, Tables& tables);

        void buildDependencies();
        void collapseEnums();
        void runPruneList();

        Tables& tables;
        std::unordered_map<std::string, Class> classes;
        std::unordered_map<std::string, Enum> enums;
    };
}
