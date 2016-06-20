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

	auto fieldType() const {
		switch (cardinality) {
			case Cardinality::ZeroOrOne:
				return "/* optional */ " + type; // TODO: use optional<T> here
			case Cardinality::One:
				return type;
			case Cardinality::ZeroOrMany:
			case Cardinality::Many:
				return "std::vector<" + type + ">";
			default:
				std::logic_error("Invalid cardinality");
		}
	}
};

struct Class {
	std::string name;
	std::string base;
	std::vector<Field> fields;
};

void generateCode(const std::string& outputLocation, const std::vector<Class>& classes);
