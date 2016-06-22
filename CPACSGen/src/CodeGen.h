#pragma once

#include <string>
#include <algorithm>
#include <unordered_map>

#include "Variant.hpp"
#include "SchemaParser.h"

enum class Cardinality {
	ZeroOrOne,
	One,
	ZeroOrMany,
	Many
};

struct Field {
	Variant<const Attribute*, const Element*> origin;
	std::string name;
	std::string type;
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
				throw std::logic_error("Invalid cardinality");
		}
	}
};

struct Class {
	const ComplexType* origin;
	std::string name;
	std::string base;
	std::vector<Field> fields;
};

struct EnumValue {
	std::string name;
	std::string cppName;

	EnumValue() = default;
	EnumValue(const std::string& name)
		: name(name) {
		// replace some chars which are not allowed in C++ for cppName
		cppName = name;
		std::replace_if(std::begin(cppName), std::end(cppName), [](char c) {
			return c == '-' || c == ' ';
		}, '_');
	}
};

struct Enum {
	const SimpleType* origin;
	std::string name;
	std::vector<EnumValue> values;

	std::string enumToStringFunc() const;
	std::string stringToEnumFunc() const;
};

struct Types {
	std::unordered_map<std::string, Class> classes;
	std::unordered_map<std::string, Enum> enums;
};

class CodeGen {
public:
	CodeGen(const std::string& outputLocation, const Types& types);
private:

};
