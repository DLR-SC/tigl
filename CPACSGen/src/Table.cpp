#include <fstream>
#include <sstream>

#include "Table.h"

void Table::read(const std::string& filename) {
	std::ifstream f(filename);
	f.exceptions(std::ios::badbit | std::ios::failbit);

	std::string line;
	while (std::getline(f, line)) {
		// skip spaces and read two type names
		std::istringstream ss(line);
		std::string first;
		std::string second;
		ss >> first >> second;
		insert(std::make_pair(first, second));
	}
}

void Table::substituteIfExists(const std::string& key, std::string& value) const {
	const auto it = find(key);
	if (it != std::end(*this))
		value = it->second;
}
