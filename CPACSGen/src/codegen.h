#pragma once

#include <string>

enum class Cardinality {
	ZeroOrOne,
	One,
	ZeroOrMany,
	Many
};

struct Field {
	std::string type;
	std::string name;
	bool attribute;
	Cardinality cardinality;

	auto fieldName() const {
		return "m_" + name;
	}
};

struct Class {
	std::string name;
	std::string base;
	std::vector<Field> fields;
};

void generateCode(const std::string& outputLocation, const std::vector<Class>& classes);
