#pragma once

#include <string>
#include <algorithm>
#include <unordered_map>

#include "Variant.hpp"
#include "SchemaParser.h"

enum class Cardinality {
	Optional,
	Mandatory,
	Vector
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

class IndentingStreamWrapper;

class CodeGen {
public:
	CodeGen(const std::string& outputLocation, const Types& types);
private:
	struct Includes {
		std::vector<std::string> hppIncludes;
		std::vector<std::string> hppForwards;
		std::vector<std::string> cppIncludes;
	};

	const Types& m_types;

	std::string fieldType(const Field& field) const;
	std::string getterSetterType(const Field& field) const;

	void writeFields(IndentingStreamWrapper& hpp, const std::vector<Field>& fields);
	void writeAccessorDeclarations(IndentingStreamWrapper& hpp, const std::vector<Field>& fields);
	void writeAccessorImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields);
	void writeIODeclarations(IndentingStreamWrapper& hpp, const std::string& className, const std::vector<Field>& fields);
	void writeReadAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f, bool attribute);
	void writeWriteAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f, bool attribute);
	void writeReadImplementation(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields);
	void writeWriteImplementation(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields);
	void writeLicenseHeader(IndentingStreamWrapper& f);
	Includes resolveIncludes(const Class& c);
	void writeHeader(IndentingStreamWrapper& hpp, const Class& c, const Includes& includes);
	void writeSource(IndentingStreamWrapper& cpp, const Class& c, const Includes& includes);
	void writeClass(IndentingStreamWrapper& hpp, IndentingStreamWrapper& cpp, const Class& c);
	void writeEnum(IndentingStreamWrapper& hpp, const Enum& e);
};
