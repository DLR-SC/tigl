#include <boost/filesystem.hpp>
#include <vector>
#include <cctype>
#include <fstream>
#include <algorithm>

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
	for (const auto& f : fields)
		hpp << f.fieldType() << " " << f.fieldName() << ";\n";
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
		cpp << "" << f.fieldType() << " " << className << "::Get" << CapitalizeFirstLetter(f.name) << "() const { return " << f.fieldName() << "; }\n";
		cpp << "void " << className << "::Set" << CapitalizeFirstLetter(f.name) << "(const " << f.fieldType() << "& value) { " << f.fieldName() << " = value; }\n";
		cpp << "\n";
	}
}

void writeIODeclarations(IndentingStreamWrapper& hpp, const std::string& className, const std::vector<Field>& fields) {
	hpp << "TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
	hpp << "TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);\n";
	hpp << "\n";
}

auto writeAttributeReadImplementation(IndentingStreamWrapper& cpp, const Field& f, const Types& types) {
	// fundamental types
	if (f.type == "std::string") { cpp << f.fieldName() << " = " << "TixiGetTextAttribute"   << "(tixiHandle, xpath, \"" << f.name << "\");\n"; return; }
	if (f.type == "double")      { cpp << f.fieldName() << " = " << "TixiGetDoubleAttribute" << "(tixiHandle, xpath, \"" << f.name << "\");\n"; return; }
	if (f.type == "bool")        { cpp << f.fieldName() << " = " << "TixiGetBoolAttribute"   << "(tixiHandle, xpath, \"" << f.name << "\");\n"; return; }

	// enums
	const auto it = types.enums.find(f.type);
	if (it != std::end(types.enums)) {
		const auto& readFunc = it->second.stringToEnumFunc();
		cpp << f.fieldName() << " = " << readFunc << "(TixiGetTextAttribute(tixiHandle, xpath, \"" << f.name << "\"));\n";
		return;
	}

	throw std::logic_error("No attribute read function provided for type " + f.type);
}

auto writeAttributeWriteImplementation(IndentingStreamWrapper& cpp, const Field& f, const Types& types) {
	// fundamental types
	if (f.type == "std::string") { cpp << "TixiSaveTextAttribute"   << "(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ");\n"; return; }
	if (f.type == "double")      { cpp << "TixiSaveDoubleAttribute" << "(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ");\n"; return; }
	if (f.type == "bool")        { cpp << "TixiSaveBoolAttribute"   << "(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ");\n"; return; }

	// enums
	const auto it = types.enums.find(f.type);
	if (it != std::end(types.enums)) {
		const auto& writeFunc = it->second.enumToStringFunc();
		cpp << "TixiSaveTextAttribute(tixiHandle, xpath, \"" << f.name << "\", " << writeFunc << "(" << f.fieldName() << "));\n";
		return;
	}

	throw std::logic_error("No attribute write function provided for type " + f.type);
}

auto writeElementReadImplementation(IndentingStreamWrapper& cpp, const Field& f, const Types& types) {
	// fundamental types
	if (f.type == "std::string") { cpp << f.fieldName() << " = " << "TixiGetTextElement"   << "(tixiHandle, xpath + \"/\" + " << f.name << ");\n"; return; }
	if (f.type == "double")      { cpp << f.fieldName() << " = " << "TixiGetDoubleElement" << "(tixiHandle, xpath + \"/\" + " << f.name << ");\n"; return; }
	if (f.type == "bool")        { cpp << f.fieldName() << " = " << "TixiGetBoolElement"   << "(tixiHandle, xpath + \"/\" + " << f.name << ");\n"; return; }
	if (f.type == "int")         { cpp << f.fieldName() << " = " << "TixiGetIntElement"    << "(tixiHandle, xpath + \"/\" + " << f.name << ");\n"; return; }

	// enums
	const auto it = types.enums.find(f.type);
	if (it != std::end(types.enums)) {
		const auto& readFunc = it->second.stringToEnumFunc();
		cpp << f.fieldName() << " = " << readFunc << "(TixiGetTextElement(tixiHandle, xpath + \"/\" + " << f.name << "));\n";
		return;
	}

	// classes
	const auto it2 = types.classes.find(f.type);
	if (it2 != std::end(types.classes)) {
		cpp << f.fieldName() << ".ReadCPACS(tixiHandle, xpath + \"/\" + " << f.name << "));\n";
		return;
	}

	throw std::logic_error("No element read function provided for type " + f.type);
}

auto writeElementWriteImplementation(IndentingStreamWrapper& cpp, const Field& f, const Types& types) {
	// fundamental types
	if (f.type == "std::string") { cpp << "TixiSaveTextElement"   << "(tixiHandle, xpath, " << f.name << "\", " << f.fieldName() << ");\n"; return; }
	if (f.type == "double")      { cpp << "TixiSaveDoubleElement" << "(tixiHandle, xpath, " << f.name << "\", " << f.fieldName() << ");\n"; return; }
	if (f.type == "bool")        { cpp << "TixiSaveBoolElement"   << "(tixiHandle, xpath, " << f.name << "\", " << f.fieldName() << ");\n"; return; }
	if (f.type == "int")         { cpp << "TixiSaveIntElement"    << "(tixiHandle, xpath, " << f.name << "\", " << f.fieldName() << ");\n"; return; }

	// enums
	const auto it = types.enums.find(f.type);
	if (it != std::end(types.enums)) {
		const auto& writeFunc = it->second.enumToStringFunc();
		cpp << "TixiSaveTextElement(tixiHandle, xpath, " << f.name << "\", " << writeFunc << "(" << f.fieldName() << "));\n";
		return;
	}

	// classes
	const auto it2 = types.classes.find(f.type);
	if (it2 != std::end(types.classes)) {
		cpp << f.fieldName() << ".WriteCPACS(tixiHandle, xpath + \"/\" + " << f.name << "));\n";
		return;
	}

	throw std::logic_error("No element write function provided for type " + f.type);
}

void writeIOImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields, const Types& types) {
	// read
	cpp << "void " << className << "::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n";
	{
		Scope s(cpp);
		for (const auto& f : fields) {
			cpp << "// read " << (f.attribute ? "attribute" : "element") << " " << f.name << "\n";
			if (f.attribute) {
				// we check for the attribute's existence anyway and report errors
				cpp << "if (TixiCheckAttribute(tixiHandle, xpath, \"" << f.name << "\")) {\n";
				{
					Scope s(cpp);
					writeAttributeReadImplementation(cpp, f, types);
				}
				cpp << "}\n";
				if (f.cardinality == Cardinality::One) {
					// attribute must exist
					cpp << "else {\n";
					{
						Scope s(cpp);
						cpp << "LOG(WARNING) << \"Required attribute " << f.name << " is missing\";\n";
					}
					cpp << "}\n";
				}
			} else {
				cpp << "if (TixiCheckElement(tixiHandle, xpath + \"/\" + " << f.name << ")) {\n";
				{
					Scope s(cpp);
					writeElementReadImplementation(cpp, f, types);
				}
				cpp << "}\n";
				if (f.cardinality == Cardinality::One || f.cardinality == Cardinality::Many) {
					// element must exist
					cpp << "else {\n";
					{
						Scope s(cpp);
						cpp << "LOG(WARNING) << \"Required element " << f.name << " is missing\";\n";
					}
					cpp << "}\n";
				}
			}
			cpp << "\n";
		}
	}
	cpp << "}\n";
	cpp << "\n";


	// write
	cpp << "void " << className << "::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {\n";
	{
		Scope s(cpp);
		for (const auto& f : fields) {
			cpp << "// write " << (f.attribute ? "attribute" : "element") << " " << f.name << "\n";
			if (f.attribute)
				writeAttributeWriteImplementation(cpp, f, types);
			else
				writeElementWriteImplementation(cpp, f, types);
		}
	}
	cpp << "}\n";
	cpp << "\n";
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
	bool stringHeader = false;
	bool vectorHeader = false;
	bool optionalHeader = false;
	for (const auto& f : c.fields) {
		if (f.type == "std::string")
			stringHeader = true;
		switch (f.cardinality) {
			case Cardinality::ZeroOrOne:
				optionalHeader = true;
				break;
			case Cardinality::Many:
			case Cardinality::ZeroOrMany:
				vectorHeader = true;
				break;
			case Cardinality::One:
				break;
		}
	}
	if (stringHeader) hpp << "#include <string>";
	if (vectorHeader) hpp << "#include <vector>";
	//if (optionalHeader) hpp << "#include <boost/optional.hpp>";

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
	hpp << "#include \"tixi.h\"\n";
	hpp << "\n";

	// namespace
	hpp << "namespace tigl {\n";
	{
		Scope s(hpp);

		hpp << "namespace generated {\n";
		{
			Scope s(hpp);

			// class name and base class
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
			writeIOImplementations(cpp, c.name, c.fields, types);

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
				}
				hpp << "}\n";
			}
			hpp << "}\n";

			// string to enum function
			hpp << "inline " << e.name << " " << e.enumToStringFunc() << "(const std::string& value) {\n";
			{
				Scope s(hpp);
				for (const auto& v : e.values) {
					hpp << "if (value == \"" << v.name << "\") return " << e.name << "::" << v.cppName << ";\n";
				}
				hpp << "throw std::runtime_error(\"Invalid enum value \\\" + value + \\\" for enum type " << e.name << "\");\n";
			}
			hpp << "}\n";
		}
		hpp << "}\n";
	}
	hpp << "}\n";
	hpp << "\n";
}

void generateCode(const std::string& outputLocation, const Types& types) {
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