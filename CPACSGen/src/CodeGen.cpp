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

void writeFields(IndentingStreamWrapper& hpp, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		f.origin.visit([&](const auto* attOrElem) {
			hpp << "// generated from " << attOrElem->xpath << "\n";
		});
		hpp << f.fieldType() << " " << f.fieldName() << ";\n";
	}
}

void writeAccessorDeclarations(IndentingStreamWrapper& hpp, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		hpp << "TIGL_EXPORT const " << f.fieldType() << "& Get" << CapitalizeFirstLetter(f.name) << "() const;\n";
		hpp << "TIGL_EXPORT void Set" << CapitalizeFirstLetter(f.name) << "(const " << f.fieldType() << "& value);\n";
		hpp << "\n";
	}
}

void writeAccessorImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		cpp << "const " << f.fieldType() << "& " << className << "::Get" << CapitalizeFirstLetter(f.name) << "() const { return " << f.fieldName() << "; }\n";
		cpp << "void " << className << "::Set" << CapitalizeFirstLetter(f.name) << "(const " << f.fieldType() << "& value) { " << f.fieldName() << " = value; }\n";
		cpp << "\n";
	}
}

void writeIODeclarations(IndentingStreamWrapper& hpp, const std::string& className, const std::vector<Field>& fields) {
	hpp << "TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
	hpp << "TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;\n";
	hpp << "\n";
}

namespace {
	std::unordered_map<std::string, std::string> fundamentalTypes = {
		{"std::string", "Text"},
		{"double", "Double"},
		{"bool", "Bool"},
		{"int", "Int"}
	};
}

auto writeReadAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f, const Types& types, bool attribute) {
	const std::string AttOrElem = attribute ? "Attribute" : "Element";

	// fundamental types
	const auto itF = fundamentalTypes.find(f.type);
	if (itF != std::end(fundamentalTypes)) {
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << f.fieldName() << " = " << "TixiGet" << itF->second << AttOrElem << "(tixiHandle, xpath, \"" << f.name << "\");\n";
				break;
			case Cardinality::Vector:
				if (attribute)
					throw std::runtime_error("Attributes cannot be vectors");
				cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {\n";
				{
					Scope s(cpp);
					cpp << "return TixiGet" << itF->second << AttOrElem << "(tixiHandle, childXPath, \"" << f.name << "\");\n";
				}
				cpp << "});\n";
				break;
		}

		return;
	}

	// enums
	const auto itE = types.enums.find(f.type);
	if (itE != std::end(types.enums)) {
		const auto& readFunc = itE->second.stringToEnumFunc();
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << f.fieldName() << " = " << readFunc << "(TixiGetText" << AttOrElem << "(tixiHandle, xpath + \"/" << f.name << "\"));\n";
				break;
			case Cardinality::Vector:
				throw NotImplementedException("Reading enum vectors is not implemented");
		}
		return;
	}

	// classes
	if (!attribute) {
		const auto itC = types.classes.find(f.type);
		if (itC != std::end(types.classes)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
				case Cardinality::Mandatory:
					cpp << f.fieldName() << ".ReadCPACS(tixiHandle, xpath + \"/" << f.name << "\");\n";
					break;
				case Cardinality::Vector:
					cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {\n";
					{
						Scope s(cpp);
						cpp << f.type << " child;\n";
						cpp << "child.ReadCPACS(tixiHandle, xpath + \"/" << f.name << "\");\n";
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


auto writeWriteAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f, const Types& types, bool attribute) {
	const std::string AttOrElem = attribute ? "Attribute" : "Element";

	// fundamental types
	const auto itF = fundamentalTypes.find(f.type);
	if (itF != std::end(fundamentalTypes)) {
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << "TixiSave" << itF->second << AttOrElem << "(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ");\n";
				break;
			case Cardinality::Vector:
				if (attribute)
					throw std::runtime_error("Attributes cannot be vectors");
				cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const " << f.type << "& child) {\n";
				{
					Scope s(cpp);
					cpp << "TixiSave" << itF->second << AttOrElem << "(tixiHandle, childXPath, \"" << f.name << "\", child);\n";
				}
				cpp << "});\n";
				break;
		}
		
		return;
	}

	// enums
	const auto itE = types.enums.find(f.type);
	if (itE != std::end(types.enums)) {
		const auto& writeFunc = itE->second.enumToStringFunc();
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << "TixiSaveText" << AttOrElem << "(tixiHandle, xpath, \"" << f.name << "\", " << writeFunc << "(" << f.fieldName() << "));\n";
				break;
			case Cardinality::Vector:
				throw NotImplementedException("Writing enum vectors is not implemented");
		}
		return;
	}

	// classes
	if (!attribute) {
		const auto itC = types.classes.find(f.type);
		if (itC != std::end(types.classes)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
				case Cardinality::Mandatory:
					cpp << f.fieldName() << ".WriteCPACS(tixiHandle, xpath + \"/" << f.name << "\");\n";
					break;
				case Cardinality::Vector:
					cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const " << f.type << "& child) {\n";
					{
						Scope s(cpp);
						cpp << "child.WriteCPACS(tixiHandle, childXPath);\n";
					}
					cpp << "});\n";
					break;
			}
			return;
		}
	}

	throw std::logic_error("No write function provided for type " + f.type);
}

void writeReadImplementation(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields, const Types& types) {
	cpp << "void " << className << "::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n";
	{
		Scope s(cpp);
		for (const auto& f : fields) {
			const auto attOrElem = f.attribute ? "attribute" : "element";
			const auto AttOrElem = f.attribute ? "Attribute" : "Element";
			cpp << "// read " << attOrElem << " " << f.name << "\n";
			cpp << "if (TixiCheck" << AttOrElem << "(tixiHandle, xpath, \"" << f.name << "\")) {\n";
			{
				Scope s(cpp);
				writeReadAttributeOrElementImplementation(cpp, f, types, f.attribute);
			}
			cpp << "}\n";
			if (f.cardinality == Cardinality::Mandatory) {
				// attribute or element must exist
				cpp << "else {\n";
				{
					Scope s(cpp);
					cpp << "LOG(WARNING) << \"Required " << attOrElem << " " << f.name << " is missing\";\n";
				}
				cpp << "}\n";
			}
			cpp << "\n";
		}
	}
	cpp << "}\n";
}

void writeWriteImplementation(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields, const Types& types) {
	cpp << "void " << className << "::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {\n";
	{
		Scope s(cpp);
		for (const auto& f : fields) {
			const auto attOrElem = f.attribute ? "attribute" : "element";
			cpp << "// write " << attOrElem << " " << f.name << "\n";
			writeWriteAttributeOrElementImplementation(cpp, f, types, f.attribute);
			cpp << "\n";
		}
	}
	cpp << "}\n";
}

void writeLicenseHeader(IndentingStreamWrapper& f) {
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

void writeIncludes(IndentingStreamWrapper& hpp, const Class& c, const Types& types) {
	// standard headers
	bool stringHeader = false;
	bool vectorHeader = false;
	bool optionalHeader = false;
	for (const auto& f : c.fields) {
		if (f.type == "std::string")
			stringHeader = true;
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
	if (stringHeader) hpp << "#include <string>\n";
	if (vectorHeader) hpp << "#include <vector>\n";
	//if (optionalHeader) hpp << "#include <boost/optional.hpp>\n";

	// base class
	if (!c.base.empty())
		hpp << "#include \"" << c.base << ".h\"\n";

	// fields
	for (const auto& f : c.fields) {
		if (types.enums.find(f.type) != std::end(types.enums) ||
			types.classes.find(f.type) != std::end(types.classes)) {
			hpp << "#include \"" << f.type << ".h\"\n";
		}
	}
}

void writeClass(IndentingStreamWrapper& hpp, IndentingStreamWrapper& cpp, const Class& c, const Types& types) {
	//
	// create header file
	//

	hpp << "#pragma once\n";
	hpp << "\n";

	// file header
	writeLicenseHeader(hpp);

	// includes
	writeIncludes(hpp, c, types);
	hpp << "#include \"tigl_internal.h\"\n";
	hpp << "\n";

	// namespace
	hpp << "namespace tigl {\n";
	{
		Scope s(hpp);

		hpp << "namespace generated {\n";
		{
			Scope s(hpp);

			// class name and base class
			hpp << "// generated from " << c.origin->xpath << "\n";
			hpp << "class " << c.name << (c.base.empty() ? "" : " : public " + c.base) << " {\n";
			hpp << "public:\n";
			{
				Scope s(hpp);

				// ctor
				hpp << "TIGL_EXPORT " << c.name << "();\n";
				hpp << "\n";

				// io
				writeIODeclarations(hpp, c.name, c.fields);

				// accessors
				writeAccessorDeclarations(hpp, c.fields);

			}
			hpp << "private:\n";
			{
				Scope s(hpp);

				// fields
				writeFields(hpp, c.fields);
				hpp << "\n";
			}
			hpp << "};\n";
		}
		hpp << "}\n";
	}
	hpp << "}\n";
	hpp << "\n";

	//
	// create source file
	//

	// file header
	writeLicenseHeader(cpp);

	// includes
	cpp << "#include \"IOHelper.h\"\n";
	cpp << "#include \"CTiglLogging.h\"\n";
	cpp << "#include \"" << c.name << ".h\"\n";
	cpp << "\n";

	// namespace
	cpp << "namespace tigl {\n";
	{
		Scope s(cpp);

		cpp << "namespace generated {\n";
		{
			Scope s(cpp);

			// ctor
			cpp << "" << c.name << "::" << c.name << "() {}\n";
			cpp << "\n";

			// io
			writeReadImplementation(cpp, c.name, c.fields, types);
			cpp << "\n";
			writeWriteImplementation(cpp, c.name, c.fields, types);
			cpp << "\n";

			// accessors
			writeAccessorImplementations(cpp, c.name, c.fields);
		}
		cpp << "}\n";
	}
	cpp << "}\n";
	cpp << "\n";
}

void writeEnum(IndentingStreamWrapper& hpp, const Enum& e) {

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
			hpp << "inline " << e.name << " " << e.enumToStringFunc() << "(const std::string& value) {\n";
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

CodeGen::CodeGen(const std::string& outputLocation, const Types& types) {
	boost::filesystem::create_directories(outputLocation);

	for (const auto& p : types.classes) {
		const auto c = p.second;
		std::ofstream hppFile(outputLocation + "/" + c.name + ".h");
		hppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper hpp(hppFile);

		std::ofstream cppFile(outputLocation + "/" + c.name + ".cpp");
		cppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper cpp(cppFile);

		writeClass(hpp, cpp, c, types);
	}

	for (const auto& p : types.enums) {
		const auto e = p.second;
		std::ofstream hppFile(outputLocation + "/" + e.name + ".h");
		hppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper hpp(hppFile);

		writeEnum(hpp, e);
	}
}
