#pragma once


#include <string>
#include <vector>
#include <unordered_map>

#include "TixiHelper.h"
#include "Variant.hpp"

struct XSDElement {
	std::string xpath;
};

struct Attribute : XSDElement {
	std::string name;
	std::string type;
	std::string default;
	std::string fixed;
	bool optional;
};

struct Element : XSDElement {
	std::string name;
	std::string type;
	int minOccurs;
	int maxOccurs;
};

struct All : XSDElement {
	std::vector<Element> elements;
};

struct Choice;
struct Sequence : XSDElement {
	std::vector<Variant<Element, Choice, Sequence>> elements;
};

struct Choice : XSDElement {
	std::vector<Variant<Element, Choice, Sequence>> elements;
};

struct Type : XSDElement {
	std::string name;
	std::string base;
};

struct ComplexType : Type {
	Variant<Element, Choice, Sequence, All> elements;
	std::vector<Attribute> attributes;
};

struct SimpleType : Type {
	std::vector<std::string> restrictionValues;
};

class SchemaParser {
public:
	SchemaParser(const std::string& cpacsLocation);

	const auto& types() const { return m_types; }

private:
	TixiDocument document;
	std::unordered_map<std::string, Variant<ComplexType, SimpleType>> m_types;

	All readAll(const std::string& xpath);
	Sequence readSequence(const std::string& xpath);
	Choice readChoice(const std::string& xpath);
	void readComplexTypeElementConfiguration(const std::string& xpath, ComplexType& type);
	Attribute readAttribute(const std::string& xpath);
	std::string readComplexType(const std::string& xpath);
	std::string readSimpleType(const std::string& xpath);
	std::string readType(const std::string& xpath);
	Element readElement(const std::string& xpath);

	std::string renameType(const std::string& oldName, std::string newNameSuggestion);
};