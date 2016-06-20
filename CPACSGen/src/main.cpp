#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cctype>

#include "TixiHelper.h"
#include "Variant.hpp"
#include "TypeSubstitutionTable.h"

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

enum class Cardinality {
	ZeroOrOne,
	One,
	ZeroOrMany,
	Many
};

struct Member {
	std::string type;
	std::string name;
	bool attribute;
	Cardinality cardinality;

	auto fieldName() const {
		return "m_" + name;
	}
};

auto buildMemberList(const ComplexType& type) {
	std::vector<Member> members;

	// attributes
	for (const auto& a : type.attributes) {
		Member m;
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
		Visitor(std::vector<Member>& members)
			: members(members) {}

		void operator()(const Element& e) const {
			Member m;
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
		std::vector<Member>& members;
	};

	type.elements.visit(Visitor(members));

	return members;
}

void writeFields(std::ofstream& hpp, const std::vector<Member>& members) {
	for (const auto& m : members)
		hpp << "\t\t" << m.type << " " << m.fieldName() << ";\n";
}

void writeAccessorDeclarations(std::ofstream& hpp, const std::vector<Member>& members) {
	for (const auto& m : members) {
		hpp << "\t\tTIGL_EXPORT " << m.type << " Get" << CapitalizeFirstLetter(m.name) << "() const;\n";
		hpp << "\t\tTIGL_EXPORT void Set" << CapitalizeFirstLetter(m.name) << "(const " << m.type << "& value);\n";
		hpp << "\n";
	}
}

void writeAccessorImplementations(std::ofstream& cpp, const std::string& className, const std::vector<Member>& members) {
	for (const auto& m : members) {
		cpp << "\t" << m.type << " " << className << "::Get" << CapitalizeFirstLetter(m.name) << "() const { return " << m.fieldName() << "; }\n";
		cpp << "\tvoid " << className << "::Set" << CapitalizeFirstLetter(m.name) << "(const " << m.type << "& value) { " << m.fieldName() << " = value; }\n";
		cpp << "\n";
	}
}

void writeIODeclarations(std::ofstream& hpp, const std::string& className, const std::vector<Member>& members) {
	hpp << "\t\tTIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
	hpp << "\t\tTIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
	hpp << "\n";
}

void writeIOImplementations(std::ofstream& cpp, const std::string& className, const std::vector<Member>& members) {
	// read
	cpp << "\tvoid " << className << "::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n";
	for (const auto& m : members) {
		cpp << "\t\t// read " << (m.attribute ? "attribute" : "element") << " " << m.name << "\n";
		if (m.attribute) {
			// we check for the attribute's existence anyway and report errors
			cpp << "\t\tif (TixiCheckAttribute(tixiHandle, xpath, " << m.name << ")) {\n";
			cpp << "\t\t\t" << m.fieldName() << " = TixiGetTextAttribute(tixiHandle, xpath, " << m.name << ");\n";
			if (m.cardinality == Cardinality::One) {
				// attribute must exist
				cpp << "\t\t} else {\n";
				cpp << "\t\t\tLOG(WARNING) << \"Required attribute " << m.name << " is missing\";\n";
				cpp << "\t\t}\n";
			} else
				cpp << "\t\t}\n";
		} else {
			cpp << "\t\tif (TixiCheckElement(tixiHandle, xpath + \"/\" + " << m.name << ")) {\n";
			cpp << "\t\t\t" << m.fieldName() << " = TixiGetTextElement(tixiHandle, xpath + \"/\" + " << m.name << ");\n";
			if (m.cardinality == Cardinality::One || m.cardinality == Cardinality::Many) {
				// element must exist
				cpp << "\t\t} else {\n";
				cpp << "\t\t\tLOG(WARNING) << \"Required element " << m.name << " is missing\";\n";
				cpp << "\t\t}\n";
			} else
				cpp << "\t\t}\n";
		}
		cpp << "\n";
	}
	cpp << "\t}\n";
	cpp << "\n";


	// write
	cpp << "\tvoid " << className << "::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n";
	for (const auto& m : members) {
		cpp << "\t\t// write " << (m.attribute ? "attribute" : "element") << " " << m.name << "\n";
		if(m.attribute)
			cpp << "\t\tTixiSaveTextAttribute(tixiHandle, xpath, \"" << m.name << "\", " << m.fieldName() << ");\n";
		else
			cpp << "\t\tTixiSaveTextElement(tixiHandle, xpath, \"" << m.name << "\", " << m.fieldName() << ");\n";
		cpp << "\n";
	}
	cpp << "\t}\n";
	cpp << "\n";
}

void writeLicenseHeader(std::ofstream& f) {
	f << "// This file was autogenerated by CPACSGen, do not edit\n";
	f << "//\n";
	f << "// Licensed under the Apache License, Version 2.0 (the \"License\")\n";
	f << "// you may not use this file except in compliance with the License.\n";
	f << "// You may obtain a copy of the License at\n";
	f << "//\n";
	f << "//     http://www.apache.org/licenses/LICENSE-2.0\n";
	f << "//\n";
	f << "// Unless required by applicable law or agreed to in writing, software\n";
	f << "// distributed under the License is distributed on an \"AS IS\" BASIS,\n";
	f << "// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n";
	f << "// See the License for the specific language governing permissions and\n";
	f << "// limitations under the License.\n";
	f << "\n";
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

		std::cout << "Generating classes" << std::endl;

		// generate code
		std::system(("mkdir " + outputLocation).c_str()); // TODO
		for (const auto& p : complexTypes) {
			const auto& type = p.second;

			const auto className = makeClassName(type.name);
			const auto baseName = makeClassName(type.base);

			const auto members = buildMemberList(type);

			// create header file
			std::ofstream hpp(outputLocation + "/" + className + ".h");
			hpp.exceptions(std::ios::failbit | std::ios::badbit);

			hpp << "#pragma once\n";
			hpp << "\n";

			// file header
			writeLicenseHeader(hpp);

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
			writeIODeclarations(hpp, className, members);

			// accessors
			writeAccessorDeclarations(hpp, members);

			hpp << "\tprivate:\n";

			// fields
			writeFields(hpp, members);
			hpp << "\n";

			hpp << "\t};\n";
			hpp << "}\n";
			hpp << "\n";

			// create source file
			std::ofstream cpp(outputLocation + "/" + className + ".cpp");
			cpp.exceptions(std::ios::failbit | std::ios::badbit);

			// file header
			writeLicenseHeader(cpp);

			// includes
			cpp << "#include \"IOHelper.h\"\n";
			cpp << "#include \"" << className << ".h\"\n";
			cpp << "\n";

			// namespace
			cpp << "namespace tigl {\n";

			// ctor
			cpp << "\t" << className << "::" << className << "() {}\n";
			cpp << "\n";

			// io
			writeIOImplementations(cpp, className, members);

			// accessors
			writeAccessorImplementations(cpp, className, members);

			cpp << "}\n";
			cpp << "\n";
		}

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