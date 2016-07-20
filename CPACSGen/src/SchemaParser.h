#pragma once


#include <string>
#include <vector>
#include <unordered_map>

#include "TixiDocument.h"
#include "Variant.hpp"

namespace tigl {
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

	struct Any : XSDElement {

	};

	struct Group : XSDElement {

	};

	struct All : XSDElement {
		std::vector<Element> elements;
	};

	struct Choice;
	struct Sequence : XSDElement {
		std::vector<Variant<Element, Group, Choice, Sequence, Any>> elements;
	};

	struct Choice : XSDElement {
		std::vector<Variant<Element, Group, Choice, Sequence, Any>> elements;
	};

	struct SimpleContent : XSDElement {
		std::string type;
	};

	struct Type : XSDElement {
		std::string name;
		std::string base;
	};

	struct ComplexType : Type {
		Variant<Group, All, Choice, Sequence, SimpleContent> content;
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

		Group    readGroup(const std::string& xpath, const std::string& containingTypeName);
		All      readAll(const std::string& xpath, const std::string& containingTypeName);
		Choice   readChoice(const std::string& xpath, const std::string& containingTypeName);
		Sequence readSequence(const std::string& xpath, const std::string& containingTypeName);
		Any      readAny(const std::string& xpath, const std::string& containingTypeName);

		void readExtension(const std::string& xpath, ComplexType& type);
		void readSimpleContent(const std::string& xpath, ComplexType& type);
		void readComplexContent(const std::string& xpath, ComplexType& type);
		void readComplexTypeElementConfiguration(const std::string& xpath, ComplexType& type);
		Attribute readAttribute(const std::string& xpath, const std::string& containingTypeName);
		void readRestriction(const std::string& xpath, SimpleType& type);
		std::string readComplexType(const std::string& xpath, const std::string& nameHint = "");
		std::string readSimpleType(const std::string& xpath, const std::string& nameHint = "");
		std::string readInlineType(const std::string& xpath, const std::string& nameHint);
		Element readElement(const std::string& xpath, const std::string& containingTypeName = "");

		std::string generateUniqueTypeName(const std::string& newNameSuggestion);
	};

	std::string stripTypeSuffix(std::string name);
}
