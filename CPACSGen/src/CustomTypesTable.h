#pragma once

#include <string>
#include <unordered_map>

// TODO: read from file
// custom types in Tigl which inherit from the generated ones
class CustomTypesTable : public std::unordered_map<std::string, std::string> {
public:
	using Base = std::unordered_map<std::string, std::string>;

	CustomTypesTable()
		: Base({
		{ "CPACSWing", "CCPACSWing" },
	}) {}
};
