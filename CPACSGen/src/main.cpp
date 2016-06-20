#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <algorithm>
#include <iterator>

#include "TixiHelper.h"
#include "Variant.hpp"
#include "TypeSubstitutionTable.h"
#include "codegen.h"

class NotImplementedException : public std::exception {
public:
	NotImplementedException(const std::string& msg)
		: m_msg(msg) {}

	virtual const char* what() const override {
		return m_msg.c_str();
	}

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

std::unordered_map<std::string, ComplexType> complexTypes;
std::unordered_map<std::string, SimpleType> simpleTypes;
std::unordered_map<std::string, std::string> xsdTypes = {
	{ "xsd:boolean", "bool" },
	{ "xsd:double", "double" },
	{ "xsd:string", "std::string" }
};

Element readElement(const TixiDocument& document, const std::string& xpath);
std::string readType(const TixiDocument& document, const std::string& xpath);

All readAll(const TixiDocument& document, const std::string& xpath) {
	All all;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		all.elements.push_back(readElement(document, xpath));
	});
	return all;
}

Sequence readSequence(const TixiDocument& document, const std::string& xpath) {
	Sequence seq;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		seq.elements.push_back(readElement(document, xpath));
	});
	return seq;
}

Choice readChoice(const TixiDocument& document, const std::string& xpath) {
	Choice ch;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		ch.elements.push_back(readElement(document, xpath));
	});
	return ch;
}

void readComplexTypeElementConfiguration(const TixiDocument& document, const std::string& xpath, ComplexType& type) {
	if (document.checkElement(xpath + "/choice")) {
		document.forEachChild(xpath + "/choice", "element", [&](auto xpath) {
			readElement(document, xpath);
		});
	} else if (document.checkElement(xpath + "/all")) {
		type.elements = readAll(document, xpath + "/all");
	} else if (document.checkElement(xpath + "/sequence")) {
		type.elements = readSequence(document, xpath + "/sequence");
	} else if (document.checkElement(xpath + "/choice")) {
		type.elements = readChoice(document, xpath + "/choice");
	} else if (document.checkElement(xpath + "/complexContent")) {
		if (document.checkElement(xpath + "/complexContent/restriction")) {
			throw NotImplementedException("XSD complextype complexcontent restriction is not implemented");
		} else if (document.checkElement(xpath + "/complexContent/extension")) {
			type.base = document.textAttribute(xpath + "/complexContent/extension", "base");
			readComplexTypeElementConfiguration(document, xpath + "/complexContent/extension", type);
		}
	} else if (document.checkElement(xpath + "/simpleContent")) {

	}
}

Attribute readAttribute(const TixiDocument& document, const std::string& xpath) {
	Attribute att;
	att.name = document.textAttribute(xpath, "name");

	if (document.checkAttribute(xpath, "type"))
		// referencing other type
		att.type = document.textAttribute(xpath, "type");
	else
		// type defined inline
		att.type = readType(document, xpath);

	if (document.checkAttribute(xpath, "use")) {
		const auto use = document.textAttribute(xpath, "use");
		if (use == "optional")
			att.optional = true;
		else if (use == "required")
			att.optional = false;
		else
			throw std::runtime_error("Invalid value for optional attribute at xpath: " + xpath);
	} else {
		att.optional = false;
	}

	if (document.checkAttribute(xpath, "default"))
		att.default = document.textAttribute(xpath, "default");
	if (document.checkAttribute(xpath, "fixed"))
		att.fixed = document.textAttribute(xpath, "fixed");

	return att;
}

std::string readComplexType(const TixiDocument& document, const std::string& xpath) {
	// read or generate type name
	const std::string name = [&] {
		if (document.checkAttribute(xpath, "name"))
			return document.textAttribute(xpath, "name");
		else {
			// generate unique type name
			static auto id = 0;
			return "inlineComplexType" + std::to_string(id++);
		}
	}();

	ComplexType& type = complexTypes[name];
	type.name = name;

	if (document.checkAttribute(xpath, "id"))
		throw NotImplementedException("XSD complextype id is not implemented");
	if (document.checkAttribute(xpath, "abstract"))
		throw NotImplementedException("XSD complextype abstract is not implemented");
	if (document.checkAttribute(xpath, "mixed"))
		throw NotImplementedException("XSD complextype mixed is not implemented");
	if (document.checkAttribute(xpath, "block"))
		throw NotImplementedException("XSD complextype block is not implemented");
	if (document.checkAttribute(xpath, "final"))
		throw NotImplementedException("XSD complextype final is not implemented");

	// read element configuration
	readComplexTypeElementConfiguration(document, xpath, type);

	// read attributes
	for (auto path : {
		xpath,
		xpath + "/complexContent/restriction",
		xpath + "/complexContent/extension",
		xpath + "/simpleContent/restriction",
		xpath + "/simpleContent/extension",
	}) {
		if (document.checkElement(path)) {
			document.forEachChild(path, "attribute", [&](auto xpath) {
				type.attributes.push_back(readAttribute(document, xpath));
			});
		}
	}

	return name;
}

std::string readSimpleType(const TixiDocument& document, const std::string& xpath) {
	// read or generate type name
	const std::string name = [&] {
		if (document.checkAttribute(xpath, "type"))
			return document.textAttribute(xpath, "name");
		else {
			// generate unique type name
			static auto id = 0;
			return "inlineSimpleType" + std::to_string(id++);
		}
	}();

	SimpleType& type = simpleTypes[name];
	type.name = name;

	if (document.checkElement(xpath + "/restriction")) {
		type.base = document.textAttribute(xpath + "/restriction", "base");

		// TODO: implement subelements
	}

	if (document.checkAttribute(xpath, "id"))
		throw NotImplementedException("XSD complextype id is not implemented");

	return name;
}

std::string readType(const TixiDocument& document, const std::string& xpath) {
	if (document.checkElement(xpath + "/complexType"))
		return readComplexType(document, xpath + "/complexType");
	else if (document.checkElement(xpath + "/simpleType"))
		return readSimpleType(document, xpath + "/simpleType");
	else
		return "ERROR_NO_TYPE";
	//throw std::runtime_error("Unexpected type at xpath: " + xpath);
}

Element readElement(const TixiDocument& document, const std::string& xpath) {
	Element element;
	element.name = document.textAttribute(xpath, "name");

	auto parseOccurs = [&](const std::string& att) {
		if (!document.checkAttribute(xpath, att))
			return -1;
		const auto value = document.textAttribute(xpath, att);
		if (value == "unbounded")
			return -1;
		else
			return std::stoi(value);
	};

	// minOccurs
	if (!document.checkAttribute(xpath, "minOccurs"))
		element.minOccurs = 1;
	else {
		const auto minOccurs = document.textAttribute(xpath, "minOccurs");
		element.minOccurs = std::stoi(minOccurs);
	}

	// maxOccurs
	if (!document.checkAttribute(xpath, "maxOccurs"))
		element.maxOccurs = 1;
	else {
		const auto maxOccurs = document.textAttribute(xpath, "maxOccurs");
		if (maxOccurs == "unbounded")
			element.maxOccurs = -1;
		else
			element.maxOccurs = std::stoi(maxOccurs);
	}

	if (document.checkAttribute(xpath, "type"))
		// referencing other type
		element.type = document.textAttribute(xpath, "type");
	else
		// type defined inline
		element.type = readType(document, xpath);

	return element;
}

auto makeClassName(std::string name) {
	if (!name.empty()) {
		// capitalize first letter
		name[0] = std::toupper(name[0]);

		// strip Type suffix if exists
		if (name.size() > 4 && name.compare(name.size() - 4, 4, "Type") == 0)
			name.erase(std::end(name) - 4, std::end(name));

		// prefix CPACS
		name = "CPACS" + name;
	}

	return name;
}

TypeSubstitutionTable typeSubstitutionTable;

auto resolveType(const std::string& name) {
	// search complex types
	const auto cit = complexTypes.find(name);
	if (cit != std::end(complexTypes)) {
		if (typeSubstitutionTable.hasSubstitution(name))
			return typeSubstitutionTable.substitute(name);
		else
			return makeClassName(name);
	}

	// search simple types
	const auto sit = simpleTypes.find(name);
	if (sit != std::end(simpleTypes)) {
		if (typeSubstitutionTable.hasSubstitution(name))
			return typeSubstitutionTable.substitute(name);
		else
			return makeClassName(name);
	}

	// search predefined xml schema types and replace them
	const auto xit = xsdTypes.find(name);
	if (xit != std::end(xsdTypes)) {
		if (typeSubstitutionTable.hasSubstitution(name))
			return typeSubstitutionTable.substitute(name);
		else
			return xit->second;
	}

	if (name == "ERROR_NO_TYPE") // TODO: remove after debugging
		return name;

	throw std::runtime_error("Unknown type: " + name);
}

auto buildFieldList(const ComplexType& type) {
	std::vector<Field> members;

	// attributes
	for (const auto& a : type.attributes) {
		Field m;
		m.name = a.name;
		m.type = resolveType(a.type);
		m.attribute = true;
		if (a.optional)
			m.cardinality = Cardinality::ZeroOrOne;
		else
			m.cardinality = Cardinality::One;
		members.push_back(m);
	}

	// elements
	struct Visitor {
		Visitor(std::vector<Field>& members)
			: members(members) {}

		void operator()(const Element& e) const {
			Field m;
			m.name = e.name;
			m.type = resolveType(e.type);
			m.attribute = false;
			if(e.minOccurs == 0 && e.maxOccurs == 1)
				m.cardinality = Cardinality::ZeroOrOne;
			if (e.minOccurs == 0 && e.maxOccurs > 1)
				m.cardinality = Cardinality::ZeroOrMany;
			else if (e.minOccurs == 1 && e.maxOccurs == 1)
				m.cardinality = Cardinality::One;
			else if(e.minOccurs == 1 && e.maxOccurs > 1)
				m.cardinality = Cardinality::Many;
			members.push_back(m);
		}

		void operator()(const Choice& c) const {
			for (const auto& v : c.elements)
				v.visit(Visitor(members));
		}

		void operator()(const Sequence& s) const {
			for (const auto& v : s.elements)
				v.visit(Visitor(members));
		}

		void operator()(const All& a) const {
			for (const auto& e : a.elements)
				operator()(e);
		}

	private:
		std::vector<Field>& members;
	};

	type.elements.visit(Visitor(members));

	return members;
}

int main() {
	try {
		const std::string cpacsLocation = "../schema/cpacs_schema.xsd"; // TODO: inject this path by cmake
		const std::vector<std::string> copyFiles = {
			"../src/IOHelper.h",
			"../src/IOHelper.cpp"
		};
		const std::string outputLocation = "generated";

		TixiDocument document(cpacsLocation);

		// read types and elements
		std::cout << "Parsing " << cpacsLocation << std::endl;

		document.forEachChild("/schema", "simpleType", [&](auto xpath) {
			readSimpleType(document, xpath);
		});

		document.forEachChild("/schema", "complexType", [&](auto xpath) {
			readComplexType(document, xpath);
		});

		document.forEachChild("/schema", "element", [&](auto xpath) {
			readElement(document, xpath);
		});

		// generate class model
		std::vector<Class> classes;
		classes.reserve(complexTypes.size());
		std::transform(std::begin(complexTypes), std::end(complexTypes), std::back_inserter(classes), [](const auto& p) {
			Class c;
			const auto& type = p.second;
			c.name = makeClassName(type.name);
			c.base = makeClassName(type.base);
			c.fields = buildFieldList(type);
			return c;
		});

		// generate code
		std::cout << "Generating classes" << std::endl;
		generateCode(outputLocation, classes);

		// copy IOHelper
		//for (const auto& filename : copyFiles) {
		//	const auto command = "copy " + filename + " " + outputLocation;
		//	system(command.c_str());
		//}
		
	} catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}