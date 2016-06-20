#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cctype>

#include "TixiHelper.h"
#include "Variant.hpp"

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

	Variant<Element, Choice, Sequence> elements;
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

	element.minOccurs = parseOccurs("minOccurs");
	element.maxOccurs = parseOccurs("maxOccurs");

	if (document.checkAttribute(xpath, "type"))
		// referencing other type
		element.type = document.textAttribute(xpath, "type");
	else
		// type defined inline
		element.type = readType(document, xpath);

	return element;
}

auto CapitalizeFirstLetter(std::string str) {
	if (str.empty())
		throw std::runtime_error("String is empty");

	str[0] = std::toupper(str[0]);

	return str;
}

auto makeClassName(std::string name) {
	if (!name.empty()) {
		// capitalize first letter
		name[0] = std::toupper(name[0]);

		// strip Type suffix if exists
		if (name.size() > 4 && name.compare(name.size() - 4, 4, "Type") == 0)
			name.erase(std::end(name) - 4, std::end(name));
	}

	return name;
}

auto resolveType(const std::string& name) {
	auto cit = complexTypes.find(name);
	if (cit != std::end(complexTypes))
		return makeClassName(name);
	auto sit = simpleTypes.find(name);
	if (sit != std::end(simpleTypes))
		return makeClassName(name);
	auto xit = xsdTypes.find(name);
	if (xit != std::end(xsdTypes))
		return xit->second;
	throw std::runtime_error("Unknown type: " + name);
}

struct Member {
	std::string type;
	std::string name;
};

auto buildMemberList(const ComplexType& type) {
	std::vector<Member> members;

	// attributes
	for (const auto& a : type.attributes) {
		Member m;
		m.name = a.name;
		m.type = resolveType(a.type);
		members.push_back(m);
	}

	// elements
	struct Visitor {
		void operator()(const Element& e) const {

		}

		void operator()(const Choice& e) const {

		}

		void operator()(const Sequence& e) const {

		}
	};

	type.elements.visit(Visitor());

	return members;
}

void writeFields(std::ofstream& hpp, const std::vector<Member>& members) {
	for (const auto& m : members)
		hpp << "\t\t" << m.type << " m_" << m.name << ";\n";
}

void writeAccessors(std::ofstream& hpp, const std::vector<Member>& members) {
	for (const auto& m : members) {
		hpp << "\t\tTIGL_EXPORT " << m.type << " Get" << CapitalizeFirstLetter(m.name) << "() const { return m_" << m.name << "; }\n";
		hpp << "\t\tTIGL_EXPORT void Set" << CapitalizeFirstLetter(m.name) << "(const " << m.type << "& value) { m_" << m.name << " = value; }\n";
		hpp << "\n";
	}
}

int main() {
	try {

		const std::string cpacsLocation = "../schema/cpacs_schema.xsd"; // TODO: inject this path by cmake
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

		std::cout << "Generating classes" << std::endl;

		// generate code
		std::system("mkdir generated"); // TODO
		for (const auto& p : complexTypes) {
			const auto& type = p.second;

			const auto className = makeClassName(type.name);
			const auto baseName = makeClassName(type.base);

			const auto members = buildMemberList(type);

			// create header file
			std::ofstream hpp("generated/" + className + ".h");
			hpp.exceptions(std::ios::failbit | std::ios::badbit);

			hpp << "#pragma once\n";
			hpp << "\n";

			// file header
			hpp << "// This file was autogenerated by CPACSGen, do not edit\n";
			hpp << "//\n";
			hpp << "// Licensed under the Apache License, Version 2.0 (the \"License\")\n";
			hpp << "// you may not use this file except in compliance with the License.\n";
			hpp << "// You may obtain a copy of the License at\n";
			hpp << "//\n";
			hpp << "//     http://www.apache.org/licenses/LICENSE-2.0\n";
			hpp << "//\n";
			hpp << "// Unless required by applicable law or agreed to in writing, software\n";
			hpp << "// distributed under the License is distributed on an \"AS IS\" BASIS,\n";
			hpp << "// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n";
			hpp << "// See the License for the specific language governing permissions and\n";
			hpp << "// limitations under the License.\n";
			hpp << "\n";

			// includes
			hpp << "#include <string>\n";
			hpp << "#include \"tixi.h\"\n";
			hpp << "\n";

			// namespace
			hpp << "namespace tigl {\n";

			// class name and base class
			hpp << "\tclass " << className << (baseName.empty() ? "" : " : public " + baseName) << " {\n";
			hpp << "\tpublic:\n";

			// ctor
			hpp << "\t\tTIGL_EXPORT " << className << "();\n";
			hpp << "\n";

			// io
			hpp << "\t\tTIGL_EXPORT void ReadCPACS (const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
			hpp << "\t\tTIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
			hpp << "\n";

			// accessors
			writeAccessors(hpp, members);
			hpp << "\n";

			hpp << "\tprivate:\n";

			// fields
			writeFields(hpp, members);
			hpp << "\n";

			hpp << "\t};\n";
			hpp << "}\n";
			hpp << "\n";

			// create source file
			std::ofstream cpp("generated/" + className + ".cpp");
			cpp.exceptions(std::ios::failbit | std::ios::badbit);

			cpp
				// file header
				<< "// This file was autogenerated by CPACSGen, do not edit\n"
				<< "//\n"
				<< "// Licensed under the Apache License, Version 2.0 (the \"License\")\n"
				<< "// you may not use this file except in compliance with the License.\n"
				<< "// You may obtain a copy of the License at\n"
				<< "//\n"
				<< "//     http://www.apache.org/licenses/LICENSE-2.0\n"
				<< "//\n"
				<< "// Unless required by applicable law or agreed to in writing, software\n"
				<< "// distributed under the License is distributed on an \"AS IS\" BASIS,\n"
				<< "// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
				<< "// See the License for the specific language governing permissions and\n"
				<< "// limitations under the License.\n"
				<< "\n"

				// includes
				<< "#include \"" << className << ".h\"\n"
				<< "\n"

				// namespace
				<< "namespace tigl {\n"

				// ctor
				<< "\t" << className << "::" << className << "() {}\n"
				<< "\n"

				// io
				<< "\tvoid " << className << "::ReadCPACS (const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n"
				<< "\t}\n"
				<< "\tvoid " << className << "::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n"
				<< "\t}\n"

				// fields
				<< "\t};\n"
				<< "}\n"
				<< "\n";
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}