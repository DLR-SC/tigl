#pragma once

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace tigl {
    class MappingTable {
    public:
        MappingTable(const std::string& filename);
        MappingTable(std::initializer_list<std::pair<const std::string, std::string>> init);

        void read(const std::string& filename);
        bool contains(const std::string& key) const;
        const std::string* find(const std::string& key) const;
        void substituteIfExists(const std::string& key, std::string& value) const;

    private:
        std::unordered_map<std::string, std::string> m_map;
    };

    class Table {
    public:
        Table(const std::string& filename);
        Table(std::initializer_list<std::string> init);

        void read(const std::string& filename);
        bool contains(const std::string& key) const;
    private:
        std::unordered_set<std::string> m_set;
    };
}
