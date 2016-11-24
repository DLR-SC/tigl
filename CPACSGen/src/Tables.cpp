#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>

#include "Tables.h"

namespace tigl {
    MappingTable::MappingTable(const std::string& filename) {
        read(filename);
    }

    void MappingTable::read(const std::string& filename) {
        std::ifstream f(filename);
        if (!f)
            throw std::ios::failure("Failed to open file " + filename + " for reading");

        std::cout << "Reading mapping table " << filename << std::endl;

        std::string line;
        while (std::getline(f, line)) {
            // skip empty lines and comments
            if (line.empty() || line.compare(0, 2, "//") == 0)
                continue;

            // skip spaces and read two type names
            std::istringstream ss(line);
            std::string first;
            std::string second;
            ss >> first >> second;

            std::cout << "\t" << first << " -> " << second << std::endl;
            m_map.insert(std::make_pair(first, second));
        }
    }

    bool MappingTable::contains(const std::string& key) const {
        return find(key) != nullptr;
    }

    const std::string* MappingTable::find(const std::string& key) const {
        const auto it = m_map.find(key);
        if (it != std::end(m_map))
            return &it->second;
        else
            return nullptr;
    }

    void MappingTable::substituteIfExists(const std::string& key, std::string& value) const {
        const auto p = find(key);
        if (p)
            value = *p;
    }

    Table::Table(const std::string& filename) {
        read(filename);
    }

    void Table::read(const std::string& filename) {
        std::ifstream f(filename);
        if (!f)
            throw std::ios::failure("Failed to open file " + filename + " for reading");

        std::cout << "Reading table " << filename << std::endl;

        std::string line;
        while (std::getline(f, line)) {
            // skip empty lines and comments
            if (line.empty() || line.compare(0, 2, "//") == 0)
                continue;

            // skip spaces and read one type name
            std::istringstream ss(line);
            std::string name;
            ss >> name;

            std::cout << "\t" << name << std::endl;
            m_set.insert(name);
        }
    }

    bool Table::contains(const std::string& key) const {
        return m_set.find(key) != std::end(m_set);
    }

    Tables::Tables(const std::string& inputDirectory) :
        m_customTypes      (inputDirectory + "/CustomTypes.txt"     ),
        m_fundamentalTypes (inputDirectory + "/FundamentalTypes.txt"),
        m_typeSubstitutions(inputDirectory + "/TypeSubstitution.txt"),
        m_parentPointers   (inputDirectory + "/ParentPointer.txt"   ),
        m_reservedNames    (inputDirectory + "/ReservedNames.txt"   ),
        m_pruneList        (inputDirectory + "/PruneList.txt"       ) {}
}
