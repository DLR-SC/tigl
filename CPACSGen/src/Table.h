#pragma once

#include <string>
#include <unordered_map>
#include "optional.hpp"

class Table : public std::unordered_map<std::string, std::string> {
public:
	using Base = std::unordered_map<std::string, std::string>;

	using Base::Base;

	void read(const std::string& filename);
	void substituteIfExists(const std::string& key, std::string& value) const;
};
