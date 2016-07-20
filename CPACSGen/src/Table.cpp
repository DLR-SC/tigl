#include <fstream>
#include <sstream>

#include "Table.h"

namespace tigl {
	MappingTable::MappingTable(std::initializer_list<std::pair<const std::string, std::string>> init)
		: m_map(init) {}

	void MappingTable::read(const std::string& filename) {
		std::ifstream f(filename);
		f.exceptions(std::ios::badbit | std::ios::failbit);

		std::string line;
		while (std::getline(f, line)) {
			// skip spaces and read two type names
			std::istringstream ss(line);
			std::string first;
			std::string second;
			ss >> first >> second;
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


	Table::Table(std::initializer_list<std::string> init)
		: m_set(init) {}

	void Table::read(const std::string& filename) {
		std::ifstream f(filename);
		f.exceptions(std::ios::badbit | std::ios::failbit);

		std::string line;
		while (std::getline(f, line))
			m_set.insert(line);
	}

	bool Table::contains(const std::string& key) const {
		return m_set.find(key) != std::end(m_set);
	}
}
