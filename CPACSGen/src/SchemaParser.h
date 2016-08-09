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
		std::string defaultValue;
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
		using Types = std::unordered_map<std::string, Variant<ComplexType, SimpleType>>;

		SchemaParser(const std::string& cpacsLocation);

		auto types() const -> const Types& {
			return m_types;
		}

	private:
		tixihelper::TixiDocument document;
		Types m_types;

		auto readGroup(const std::string& xpath, const std::string& containingTypeName) -> Group;
		auto readAll(const std::string& xpath, const std::string& containingTypeName) -> All;
		auto readChoice(const std::string& xpath, const std::string& containingTypeName) -> Choice;
		auto readSequence(const std::string& xpath, const std::string& containingTypeName) -> Sequence;
		auto readAny(const std::string& xpath, const std::string& containingTypeName) -> Any;

		void readExtension(const std::string& xpath, ComplexType& type);
		void readSimpleContent(const std::string& xpath, ComplexType& type);
		void readComplexContent(const std::string& xpath, ComplexType& type);
		void readComplexTypeElementConfiguration(const std::string& xpath, ComplexType& type);
		auto readAttribute(const std::string& xpath, const std::string& containingTypeName) -> Attribute;
		void readRestriction(const std::string& xpath, SimpleType& type);
		auto readComplexType(const std::string& xpath, const std::string& nameHint = "") -> std::string;
		auto readSimpleType(const std::string& xpath, const std::string& nameHint = "") -> std::string;
		auto readInlineType(const std::string& xpath, const std::string& nameHint) -> std::string;
		auto readElement(const std::string& xpath, const std::string& containingTypeName = "") -> Element;

		auto generateUniqueTypeName(const std::string& newNameSuggestion) -> std::string;
	};

	auto stripTypeSuffix(std::string name) -> std::string;
}
