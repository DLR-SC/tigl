#include <boost/filesystem.hpp>
#include <vector>
#include <cctype>
#include <fstream>
#include <algorithm>

#include "NotImplementedException.h"
#include "codegen.h"

struct Scope;

class IndentingStreamWrapper {
public:
	IndentingStreamWrapper(std::ostream& os)
		: os(os){}

private:
	// indents on first use
	template<typename T>
	friend auto& operator<<(IndentingStreamWrapper& isw, T&& t) {
		for (unsigned int i = 0; i < isw.level; i++)
			isw.os << "\t";
		isw.os << std::forward<T>(t);
		return isw.os; // just return the unterlying stream
	}

	friend struct Scope;

	unsigned int level = 0;
	std::ostream& os;
};

struct Scope {
	Scope(IndentingStreamWrapper& isw)
		: isw(isw) { 
		isw.level++;
	}

	~Scope() {
		isw.level--;
	}

private:
	IndentingStreamWrapper& isw;
};

auto CapitalizeFirstLetter(std::string str) {
	if (str.empty())
		throw std::runtime_error("String is empty");

	str[0] = std::toupper(str[0]);

	return str;
}

std::string Enum::enumToStringFunc() const {
	return name + "ToString";
}

std::string Enum::stringToEnumFunc() const {
	return "stringTo" + CapitalizeFirstLetter(name);
}

std::string CodeGen::fieldType(const Field& field) const {
	switch (field.cardinality) {
		case Cardinality::Optional:
			return "Optional<" + getterSetterType(field) + ">";
		default:
			return getterSetterType(field);
	}
}

std::string CodeGen::getterSetterType(const Field& field) const {
	const auto it = m_customTypes.find(field.type);
	const auto typeName = (it != std::end(m_customTypes)) ? it->second : field.type;
	switch (field.cardinality) {
		case Cardinality::Optional:
		case Cardinality::Mandatory:
			return typeName;
		case Cardinality::Vector:
		{
			const bool makePointer = m_types.classes.find(field.type) != std::end(m_types.classes);
			return "std::vector<" + typeName + (makePointer ? "*" : "") + ">";
		}
		default:
			throw std::logic_error("Invalid cardinality");
	}
}

void CodeGen::writeFields(IndentingStreamWrapper& hpp, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		f.origin.visit([&](const auto* attOrElem) {
			hpp << "// generated from " << attOrElem->xpath << "\n";
		});
		hpp << fieldType(f) << " " << f.fieldName() << ";\n";

		if(&f != &fields.back())
			hpp << "\n";
	}
}

void CodeGen::writeAccessorDeclarations(IndentingStreamWrapper& hpp, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		if(f.cardinality == Cardinality::Optional)
			hpp << "TIGL_EXPORT bool Has" << CapitalizeFirstLetter(f.name) << "() const;\n";
		hpp << "TIGL_EXPORT const " << getterSetterType(f) << "& Get" << CapitalizeFirstLetter(f.name) << "() const;\n";
		hpp << "TIGL_EXPORT " << getterSetterType(f) << "& Get" << CapitalizeFirstLetter(f.name) << "();\n";
		if(m_types.classes.find(f.type) == std::end(m_types.classes)) // generate setter only for fundamental and enum types
			hpp << "TIGL_EXPORT void Set" << CapitalizeFirstLetter(f.name) << "(const " << getterSetterType(f) << "& value);\n";
		hpp << "\n";
	}
}

void CodeGen::writeAccessorImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		const auto op = f.cardinality == Cardinality::Optional;
		if (op)
			cpp << "TIGL_EXPORT bool " << className << "::Has" << CapitalizeFirstLetter(f.name) << "() const { return " << f.fieldName() << ".isValid(); }\n";
		cpp << "const " << getterSetterType(f) << "& " << className << "::Get" << CapitalizeFirstLetter(f.name) << "() const { return " << f.fieldName() << (op ? ".get()" : "") << "; }\n";
		cpp << getterSetterType(f) << "& " << className << "::Get" << CapitalizeFirstLetter(f.name) << "() { return " << f.fieldName() << (op ? ".get()" : "") << "; }\n";
		if (m_types.classes.find(f.type) == std::end(m_types.classes)) // generate setter only for fundamental and enum types
			cpp << "void " << className << "::Set" << CapitalizeFirstLetter(f.name) << "(const " << getterSetterType(f) << "& value) { " << f.fieldName() << " = value; }\n";
		cpp << "\n";
	}
}

void CodeGen::writeIODeclarations(IndentingStreamWrapper& hpp, const std::string& className, const std::vector<Field>& fields) {
	hpp << "TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
	hpp << "TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;\n";
	hpp << "\n";
}

namespace {
	// TODO: we can add other integer types here (e.g. short, long, ...)
	std::unordered_map<std::string, std::string> fundamentalTypes = {
		{"std::string", "Text"},
		{"double", "Double"},
		{"bool", "Bool"},
		{"int", "Int"},
		{"time_t", "TimeT"}
	};

	std::string tixiFuncSuffix(const XMLConstruct& construct) {
		switch (construct) {
			case XMLConstruct::Attribute:
				return "Attribute";
			case XMLConstruct::Element:
			case XMLConstruct::SimpleContent:
				return "Element";
			default:
				throw std::logic_error("Cannot determine tixi function suffix for the given XML construct");
		}
	}

	std::string xmlConstructToString(const XMLConstruct& construct) {
		switch (construct) {
			case XMLConstruct::Attribute:     return "attribute";
			case XMLConstruct::Element:       return "element";
			case XMLConstruct::SimpleContent: return "simpleContent";
			default: throw std::logic_error("Unknown XML construct");
		}
	}
}

void CodeGen::writeReadAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f) {
	const std::string AttOrElem = tixiFuncSuffix(f.xmlType);

	const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : ", \"" + f.name + "\"";

	// fundamental types
	const auto itF = fundamentalTypes.find(f.type);
	if (itF != std::end(fundamentalTypes)) {
		const auto& type = itF->second;
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << f.fieldName() << " = " << "TixiGet" << type << AttOrElem << "(tixiHandle, xpath" << subPathArg << ");\n";
				break;
			case Cardinality::Vector:
				if (f.xmlType == XMLConstruct::Attribute || f.xmlType == XMLConstruct::SimpleContent)
					throw std::runtime_error("Attributes or simpleContents cannot be vectors");
				cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {\n";
				{
					Scope s(cpp);
					cpp << "return TixiGet" << type << AttOrElem << "(tixiHandle, childXPath, \"" << f.name << "\");\n";
				}
				cpp << "});\n";
				break;
		}

		return;
	}

	// enums
	const auto itE = m_types.enums.find(f.type);
	if (itE != std::end(m_types.enums)) {
		const auto& readFunc = itE->second.stringToEnumFunc();
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << f.fieldName() << " = " << readFunc << "(TixiGetText" << AttOrElem << "(tixiHandle, xpath" << subPathArg << "));\n";
				break;
			case Cardinality::Vector:
				throw NotImplementedException("Reading enum vectors is not implemented");
		}
		return;
	}

	// classes
	if (f.xmlType != XMLConstruct::Attribute) {
		const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : " + \"/" + f.name + "\"";
		const auto itC = m_types.classes.find(f.type);
		if (itC != std::end(m_types.classes)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
					cpp << f.fieldName() << ".construct();\n";
					cpp << f.fieldName() << "->ReadCPACS(tixiHandle, xpath + \"/" << f.name << "\");\n";
					break;
				case Cardinality::Mandatory:
					cpp << f.fieldName() << ".ReadCPACS(tixiHandle, xpath" << subPathArg << ");\n";
					break;
				case Cardinality::Vector:
					cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {\n";
					{
						Scope s(cpp);
						cpp << "using ChildType = std::remove_pointer_t<" << fieldType(f) << "::value_type>;\n";
						cpp << "ChildType* child = new ChildType;\n";
						cpp << "child->ReadCPACS(tixiHandle, xpath + \"/" << f.name << "\");\n";
						cpp << "return child;\n";
					}
					cpp << "});\n";
					break;
			}
			return;
		}
	}

	throw std::logic_error("No read function provided for type " + f.type);
}

void CodeGen::writeWriteAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f) {
	const std::string AttOrElem = tixiFuncSuffix(f.xmlType);

	const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : ", \"" + f.name + "\"";

	// fundamental types
	const auto itF = fundamentalTypes.find(f.type);
	if (itF != std::end(fundamentalTypes)) {
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << "TixiSave" << AttOrElem << "(tixiHandle, xpath" << subPathArg << ", " << f.fieldName() << ");\n";
				break;
			case Cardinality::Vector:
				if (f.xmlType == XMLConstruct::Attribute || f.xmlType == XMLConstruct::SimpleContent)
					throw std::runtime_error("Attributes or simpleContents cannot be vectors");
				cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const " << f.type << "& child) {\n";
				{
					Scope s(cpp);
					cpp << "TixiSave" << AttOrElem << "(tixiHandle, childXPath, \"" << f.name << "\", child);\n";
				}
				cpp << "});\n";
				break;
		}
		
		return;
	}

	// enums
	const auto itE = m_types.enums.find(f.type);
	if (itE != std::end(m_types.enums)) {
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << "TixiSave" << AttOrElem << "(tixiHandle, xpath" << subPathArg << ", " << itE->second.enumToStringFunc() << "(" << f.fieldName() << (f.cardinality == Cardinality::Optional ? ".get()" : "") << "));\n";
				break;
			case Cardinality::Vector:
				throw NotImplementedException("Writing enum vectors is not implemented");
		}
		return;
	}

	// classes
	if (f.xmlType != XMLConstruct::Attribute) {
		const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : " + \"/" + f.name + "\"";
		const auto itC = m_types.classes.find(f.type);
		if (itC != std::end(m_types.classes)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
					cpp << "if (" << f.fieldName() << ".isValid()) {\n";
					{
						Scope s(cpp);
						cpp << f.fieldName() << "->WriteCPACS(tixiHandle, xpath + \"/" << f.name << "\");\n";
					}
					cpp << "}\n";
					break;
				case Cardinality::Mandatory:
					cpp << f.fieldName() << ".WriteCPACS(tixiHandle, xpath" << subPathArg << ");\n";
					break;
				case Cardinality::Vector:
					cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const " << f.type << "* child) {\n";
					{
						Scope s(cpp);
						cpp << "child->WriteCPACS(tixiHandle, childXPath);\n";
					}
					cpp << "});\n";
					break;
			}
			return;
		}
	}

	throw std::logic_error("No write function provided for type " + f.type);
}

void CodeGen::writeReadBaseImplementation(IndentingStreamWrapper& cpp, const std::string& type) {
	// fundamental types
	const auto itF = fundamentalTypes.find(type);
	if (itF != std::end(fundamentalTypes)) {
		cpp << "*this = " << "TixiGet" << itF->second << "Element(tixiHandle, xpath);\n";
		return;
	}

	// classes
	const auto itC = m_types.classes.find(type);
	if (itC != std::end(m_types.classes)) {
		cpp << type << "::ReadCPACS(tixiHandle, xpath);\n";
		return;
	}

	throw std::logic_error("No read function provided for type " + type);
}

void CodeGen::writeWriteBaseImplementation(IndentingStreamWrapper& cpp, const std::string& type) {
	// fundamental types
	const auto itF = fundamentalTypes.find(type);
	if (itF != std::end(fundamentalTypes)) {
		cpp << "TixiSaveElement(tixiHandle, xpath, *this);\n";
		return;
	}

	// classes
	const auto itC = m_types.classes.find(type);
	if (itC != std::end(m_types.classes)) {
		cpp << type << "::WriteCPACS(tixiHandle, xpath);\n";
		return;
	}

	throw std::logic_error("No write function provided for type " + type);
}

void CodeGen::writeReadImplementation(IndentingStreamWrapper& cpp, const Class& c, const std::vector<Field>& fields) {
	cpp << "void " << c.name << "::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n";
	{
		Scope s(cpp);

		// base class
		if (!c.base.empty())
			writeReadBaseImplementation(cpp, c.base);

		// fields
		for (const auto& f : fields) {
			const auto construct = xmlConstructToString(f.xmlType);
			const auto AttOrElem = tixiFuncSuffix(f.xmlType);
			cpp << "// read " << construct << " " << f.name << "\n";
			cpp << "if (TixiCheck" << AttOrElem << "(tixiHandle, xpath, \"" << f.name << "\")) {\n";
			{
				Scope s(cpp);
				writeReadAttributeOrElementImplementation(cpp, f);
			}
			cpp << "}\n";
			if (f.cardinality == Cardinality::Mandatory) {
				// attribute or element must exist
				cpp << "else {\n";
				{
					Scope s(cpp);
					cpp << "LOG(WARNING) << \"Required " << construct << " " << f.name << " is missing\";\n";
				}
				cpp << "}\n";
			}
			cpp << "\n";
		}
	}
	cpp << "}\n";
}

void CodeGen::writeWriteImplementation(IndentingStreamWrapper& cpp, const Class& c, const std::vector<Field>& fields) {
	cpp << "void " << c.name << "::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {\n";
	{
		Scope s(cpp);

		// base class
		if (!c.base.empty())
			writeWriteBaseImplementation(cpp, c.base);

		// fields
		for (const auto& f : fields) {
			const auto construct = xmlConstructToString(f.xmlType);
			cpp << "// write " << construct << " " << f.name << "\n";
			if (f.cardinality == Cardinality::Optional) {
				cpp << "if (Has" << CapitalizeFirstLetter(f.name) << "()) {\n";
				{
					Scope s(cpp);
					writeWriteAttributeOrElementImplementation(cpp, f);
				}
				cpp << "}\n";
			} else
				writeWriteAttributeOrElementImplementation(cpp, f);
			cpp << "\n";
		}
	}
	cpp << "}\n";
}

void CodeGen::writeLicenseHeader(IndentingStreamWrapper& f) {
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

CodeGen::Includes CodeGen::resolveIncludes(const Class& c) {
	Includes deps;

	deps.hppIncludes.push_back("<tixi.h>");
	deps.hppIncludes.push_back("<string>");

	// optional and vector
	bool vectorHeader = false;
	bool optionalHeader = false;
	for (const auto& f : c.fields) {
		switch (f.cardinality) {
			case Cardinality::Optional:
				optionalHeader = true;
				break;
			case Cardinality::Vector:
				vectorHeader = true;
				break;
			case Cardinality::Mandatory:
				break;
		}
	}
	if (vectorHeader)   deps.hppIncludes.push_back("<vector>");
	if (optionalHeader) deps.hppIncludes.push_back("\"optional.hpp\"");

	deps.hppIncludes.push_back("\"tigl_internal.h\"");

	// base class
	if (!c.base.empty())
		deps.hppIncludes.push_back("\"" + c.base + ".h\"");

	// fields
	for (const auto& f : c.fields) {
		if (m_types.enums.find(f.type) != std::end(m_types.enums) ||
			m_types.classes.find(f.type) != std::end(m_types.classes)) {
			// this is a class or enum type, include it

			auto it = m_customTypes.find(f.type);
			if (it == std::end(m_customTypes)) {
				switch (f.cardinality) {
					case Cardinality::Optional:
					case Cardinality::Mandatory:
						deps.hppIncludes.push_back("\"" + f.type + ".h\"");
						break;
					case Cardinality::Vector:
						deps.hppForwards.push_back(f.type);
						deps.cppIncludes.push_back("\"" + f.type + ".h\"");
						break;
				}
			} else {
				// custom types are Tigl types and resolved using include paths and require a different namespace
				switch (f.cardinality) {
					case Cardinality::Optional:
					case Cardinality::Mandatory:
						deps.hppIncludes.push_back("<" + it->second + ".h>");
						break;
					case Cardinality::Vector:
						deps.hppCustomForwards.push_back(it->second);
						deps.cppIncludes.push_back("<" + it->second + ".h>");
						break;
				}
			}
		}
	}

	// misc cpp includes
	deps.cppIncludes.push_back("\"IOHelper.h\"");
	deps.cppIncludes.push_back("\"CTiglLogging.h\"");
	deps.cppIncludes.push_back("\"" + c.name + ".h\"");

	return deps;
}

void CodeGen::writeHeader(IndentingStreamWrapper& hpp, const Class& c, const Includes& includes) {
	hpp << "#pragma once\n";
	hpp << "\n";

	// file header
	writeLicenseHeader(hpp);

	// includes
	for (const auto& inc : includes.hppIncludes)
		hpp << "#include " << inc << "\n";
	hpp << "\n";

	// namespace
	hpp << "namespace tigl {\n";
	{
		Scope s(hpp);

		// custom Tigl types declarations
		for (const auto& fwd : includes.hppCustomForwards)
			hpp << "class " << fwd << ";\n";
		hpp << "\n";

		hpp << "namespace generated {\n";
		{
			Scope s(hpp);

			// forward declarations
			for (const auto& fwd : includes.hppForwards)
				hpp << "class " << fwd << ";\n";
			hpp << "\n";

			// meta information from schema
			hpp << "// This class is used in: \n";
			for (const auto& c : c.deps.parents) {
				hpp << "// " << c->name << "\n";
			}
			hpp << "\n";
			hpp << "// generated from " << c.origin->xpath << "\n";

			// class name and base class
			hpp << "class " << c.name << (c.base.empty() ? "" : " : public " + c.base) << " {\n";
			hpp << "public:\n";
			{
				Scope s(hpp);

				// ctor
				hpp << "TIGL_EXPORT " << c.name << "();\n";
				hpp << "\n";

				// dtor
				hpp << "TIGL_EXPORT virtual ~" << c.name << "();\n";
				hpp << "\n";

				// io
				writeIODeclarations(hpp, c.name, c.fields);

				// accessors
				writeAccessorDeclarations(hpp, c.fields);

			}
			hpp << "protected:\n";
			{
				Scope s(hpp);

				// fields
				writeFields(hpp, c.fields);
				hpp << "\n";
			}
			hpp << "private:\n";
			{
				Scope s(hpp);

				// copy ctor and assign
				hpp << "TIGL_EXPORT " << c.name << "(const " << c.name << "&) = delete;\n";
				hpp << "TIGL_EXPORT " << c.name << "& operator=(const " << c.name << "&) = delete;\n";
				hpp << "\n";

				// move ctor and assign
				hpp << "TIGL_EXPORT " << c.name << "(" << c.name << "&&) = delete;\n";
				hpp << "TIGL_EXPORT " << c.name << "& operator=(" << c.name << "&&) = delete;\n";
			}
			hpp << "};\n";
		}
		hpp << "}\n";
		// export non-custom types into tigl namespace
		if (m_customTypes.find(c.name) != std::end(m_customTypes)) {
			hpp << "//using generated::" << c.name << ";\n";
		}
	}
	hpp << "}\n";
	hpp << "\n";
}

void CodeGen::writeSource(IndentingStreamWrapper& cpp, const Class& c, const Includes& includes) {
	// file header
	writeLicenseHeader(cpp);

	// includes
	for (const auto& inc : includes.cppIncludes)
		cpp << "#include " << inc << "\n";
	cpp << "\n";

	// namespace
	cpp << "namespace tigl {\n";
	{
		Scope s(cpp);

		cpp << "namespace generated {\n";
		{
			Scope s(cpp);

			// ctor
			cpp << c.name << "::" << c.name << "() {}\n";
			cpp << "\n";

			// dtor
			cpp << "" << c.name << "::~" << c.name << "() {}\n";
			cpp << "\n";

			// io
			writeReadImplementation(cpp, c, c.fields);
			cpp << "\n";
			writeWriteImplementation(cpp, c, c.fields);
			cpp << "\n";

			// accessors
			writeAccessorImplementations(cpp, c.name, c.fields);
		}
		cpp << "}\n";
	}
	cpp << "}\n";
	cpp << "\n";
}

void CodeGen::writeClass(IndentingStreamWrapper& hpp, IndentingStreamWrapper& cpp, const Class& c) {
	const auto includes = resolveIncludes(c);
	writeHeader(hpp, c, includes);
	writeSource(cpp, c, includes);
}

void CodeGen::writeEnum(IndentingStreamWrapper& hpp, const Enum& e) {

	hpp << "#pragma once\n";
	hpp << "\n";

	// file header
	writeLicenseHeader(hpp);

	// includes
	hpp << "#include <stdexcept>\n";
	hpp << "\n";

	// namespace
	hpp << "namespace tigl {\n";
	{
		Scope s(hpp);

		hpp << "namespace generated {\n";
		{
			Scope s(hpp);

			// meta information from schema
			hpp << "// This enum is used in: \n";
			for (const auto& c : e.deps.parents) {
				hpp << "// " << c->name << "\n";
			}
			hpp << "\n";
			hpp << "// generated from " << e.origin->xpath << "\n";

			// enum name
			hpp << "enum class " << e.name << " {\n";
			{
				Scope s(hpp);

				// values
				for (const auto& v : e.values)
					hpp << v.cppName << (&v != &e.values.back() ? "," : "") << "\n";
			}
			hpp << "};\n";
			hpp << "\n";

			// enum to string function
			hpp << "inline std::string " << e.enumToStringFunc() << "(const " << e.name << "& value) {\n";
			{
				Scope s(hpp);
				hpp << "switch(value) {\n";
				{
					Scope s(hpp);
					for (const auto& v : e.values)
						hpp << "case " << e.name << "::" << v.cppName << ": return \"" << v.name << "\";\n";
					hpp << "default: throw std::runtime_error(\"Invalid enum value \\\" + value + \\\" for enum type " << e.name << "\");\n";
				}
				hpp << "}\n";
			}
			hpp << "}\n";

			// string to enum function
			hpp << "inline " << e.name << " " << e.stringToEnumFunc() << "(const std::string& value) {\n";
			{
				Scope s(hpp);
				for (const auto& v : e.values)
					hpp << "if (value == \"" << v.name << "\") return " << e.name << "::" << v.cppName << ";\n";
				hpp << "throw std::runtime_error(\"Invalid enum value \\\" + value + \\\" for enum type " << e.name << "\");\n";
			}
			hpp << "}\n";
		}
		hpp << "}\n";
	}
	hpp << "}\n";
	hpp << "\n";
}

void CodeGen::buildDependencyTree() {
	auto& classes = m_types.classes;
	auto& enums = m_types.enums;

	for (auto& p : classes) {
		auto& c = p.second;

		// base
		if (!c.base.empty()) {
			const auto it = classes.find(c.base);
			if (it != std::end(classes)) {
				c.deps.bases.push_back(&it->second);
				it->second.deps.deriveds.push_back(&c);
			} else
				std::cerr << "Warning: class " << c.name << " has non-class base: " << c.base << std::endl;
		}

		// fields
		for (const auto& f : c.fields) {
			const auto eit = enums.find(f.type);
			if (eit != std::end(enums)) {
				c.deps.enumChildren.push_back(&eit->second);
				eit->second.deps.parents.push_back(&c);
			} else {
				const auto cit = classes.find(f.type);
				if (cit != std::end(classes)) {
					c.deps.children.push_back(&cit->second);
					cit->second.deps.parents.push_back(&c);
				}
			}
		}
	}
}

CodeGen::CodeGen(const std::string& outputLocation, Types types)
	: m_types(std::move(types)) {
	boost::filesystem::create_directories(outputLocation);

	buildDependencyTree();

	for (const auto& p : m_types.classes) {
		const auto c = p.second;
		std::ofstream hppFile(outputLocation + "/" + c.name + ".h");
		hppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper hpp(hppFile);

		std::ofstream cppFile(outputLocation + "/" + c.name + ".cpp");
		cppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper cpp(cppFile);

		writeClass(hpp, cpp, c);
	}

	for (const auto& p : m_types.enums) {
		const auto e = p.second;
		std::ofstream hppFile(outputLocation + "/" + e.name + ".h");
		hppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper hpp(hppFile);

		writeEnum(hpp, e);
	}
}
