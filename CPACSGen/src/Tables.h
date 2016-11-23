#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace tigl {
    class Table {
    public:
        Table(const std::string& filename);

        void read(const std::string& filename);
        bool contains(const std::string& key) const;
    private:
        std::unordered_set<std::string> m_set;
    };

    class MappingTable {
    public:
        MappingTable(const std::string& filename);

        void read(const std::string& filename);
        bool contains(const std::string& key) const;
        const std::string* find(const std::string& key) const;
        void substituteIfExists(const std::string& key, std::string& value) const;

    private:
        std::unordered_map<std::string, std::string> m_map;
    };

    class Tables {
    public:
        Tables(const std::string& inputDirectory);

        MappingTable m_customTypes;
        MappingTable m_fundamentalTypes;
        MappingTable m_typeSubstitutions;

        Table m_parentPointers;
        Table m_reservedNames;
    };
}
