#include <fstream>
#include <sstream>
#include <regex>

#include "Table.h"

namespace tigl {
	MappingTable::MappingTable(const std::string& filename) {
		read(filename);
	}

	MappingTable::MappingTable(std::initializer_list<std::pair<const std::string, std::string>> init)
		: m_map(init) {}

	void MappingTable::read(const std::string& filename) {
		std::ifstream f(filename);
		if (!f)
			throw std::ios::failure("Failed to open file " + filename + " for reading");

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

			if (first.empty() || second.empty())
				continue;

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

	Table::Table(std::initializer_list<std::string> init)
		: m_set(init) {}

	void Table::read(const std::string& filename) {
		std::ifstream f(filename);
		if (!f)
			throw std::ios::failure("Failed to open file " + filename + " for reading");

		std::string line;
		while (std::getline(f, line)) {
			// skip empty lines and comments
			if (line.empty() || line.compare(0, 2, "//") == 0)
				continue;

			// skip spaces and read one type name
			std::istringstream ss(line);
			std::string name;
			ss >> name;

			m_set.insert(name);
		}
	}

	bool Table::contains(const std::string& key) const {
		return m_set.find(key) != std::end(m_set);
	}
}
