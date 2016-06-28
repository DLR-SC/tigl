#include <fstream>
#include <sstream>

#include "CustomTypesTable.h"

CustomTypesTable::CustomTypesTable()
	: Base({
		{ "CPACSWing",         "CCPACSWing" },
		{ "CPACSWings",        "CCPACSWings" },
		{ "CPACSWingSection",  "CCPACSWingSection" },
		{ "CPACSWingElement",  "CCPACSWingSectionElement" },
		{ "CPACSWingElements", "CCPACSWingSectionElements" },
		{ "CPACSWingSection",  "CCPACSWingSection" },
		{ "CPACSWingSections", "CCPACSWingSections" },
}) {}

void CustomTypesTable::read(const std::string& filename) {
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
