#pragma once


#include <string>
#include <vector>
#include <unordered_map>

#include "TixiHelper.h"
#include "Variant.hpp"

class NotImplementedException : public std::exception {
public:
	NotImplementedException(const std::string& msg);

	virtual const char* what() const override;

private:
	std::string m_msg;
};

struct Attribute {
	std::string name;
	std::string type;
	std::string default;
	std::string fixed;
	bool optional;
};

struct Element {
	std::string name;
	std::string type;
	int minOccurs;
	int maxOccurs;
};

struct All {
	std::vector<Element> elements;
};

struct Choice;
struct Sequence {
	std::vector<Variant<Element, Choice, Sequence>> elements;
};

struct Choice {
	std::vector<Variant<Element, Choice, Sequence>> elements;
};

struct ComplexType {
	std::string name;
	std::string base; // type name of base class or empty

	Variant<Element, Choice, Sequence, All> elements;
	std::vector<Attribute> attributes;
};

struct SimpleType {
	std::string name;
	std::string base;
};

class SchemaParser {
public:
	SchemaParser(const std::string& cpacsLocation);

	const auto& complexTypes() const { return m_complexTypes; }
	const auto& simpleTypes() const { return m_simpleTypes; }

private:
	TixiDocument document;
	std::unordered_map<std::string, ComplexType> m_complexTypes;
	std::unordered_map<std::string, SimpleType> m_simpleTypes;

	All readAll(const std::string& xpath);
	Sequence readSequence(const std::string& xpath);
	Choice readChoice(const std::string& xpath);
	void readComplexTypeElementConfiguration(const std::string& xpath, ComplexType& type);
	Attribute readAttribute(const std::string& xpath);
	std::string readComplexType(const std::string& xpath);
	std::string readSimpleType(const std::string& xpath);
	std::string readType(const std::string& xpath);
	Element readElement(const std::string& xpath);
};