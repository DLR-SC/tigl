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

auto writeAttributeReadImplementation(IndentingStreamWrapper& cpp, const Field& f, const std::vector<Enum>& enums) {
	std::string readFunc;
	bool isEnum = false;
	if (f.type == "std::string")
		readFunc = "TixiGetTextAttribute";
	else if (f.type == "double")
		readFunc = "TixiGetDoubleAttribute";
	else if (f.type == "bool")
		readFunc = "TixiGetBoolAttribute";
	else {
		const auto it = std::find_if(std::begin(enums), std::end(enums), [&](const auto& e) {
			return e.name == f.type;
		});
		if (it != std::end(enums)) {
			isEnum = true;
			readFunc = it->stringToEnumFunc();
		} else
			throw std::logic_error("No attribute read function provided for type " + f.type);
	}
	// TODO: keep this list in sync with the shipped IOHelper

	if (!isEnum)
		cpp << f.fieldName() << " = " << readFunc << "(tixiHandle, xpath, \"" << f.name << "\");\n";
	else
		cpp << f.fieldName() << " = " << readFunc << "(TixiGetTextAttribute(tixiHandle, xpath, \"" << f.name << "\"));\n";
}

auto writeAttributeWriteImplementation(IndentingStreamWrapper& cpp, const Field& f, const std::vector<Enum>& enums) {
	std::string writeFunc;
	bool isEnum = false;
	if (f.type == "std::string")
		writeFunc = "TixiSaveTextAttribute";
	else if (f.type == "double")
		writeFunc = "TixiSaveDoubleAttribute";
	else if (f.type == "bool")
		writeFunc = "TixiSaveBoolAttribute";
	else {
		const auto it = std::find_if(std::begin(enums), std::end(enums), [&](const auto& e) {
			return e.name == f.type;
		});
		if (it != std::end(enums)) {
			isEnum = true;
			writeFunc = it->enumToStringFunc();
		} else
			throw std::logic_error("No attribute write function provided for type " + f.type);
	}
	// TODO: keep this list in sync with the shipped IOHelper

	if (!isEnum)
		cpp << writeFunc << "(tixiHandle, xpath, \"" << f.name << "\", " << f.fieldName() << ");\n";
	else
		cpp << "TixiSaveTextAttribute(tixiHandle, xpath, \"" << f.name << "\", " << writeFunc << "(" << f.fieldName() << "));\n";
}

auto writeElementReadImplementation(IndentingStreamWrapper& cpp, const Field& f, const std::vector<Class>& classes, const std::vector<Enum>& enums) {
	std::string readFunc;
	bool isEnum = false;
	bool isClass = false;
	if (f.type == "std::string")
		readFunc = "TixiGetTextElement";
	else if (f.type == "double")
		readFunc = "TixiGetDoubleElement";
	else if (f.type == "bool")
		readFunc = "TixiGetBoolElement";
	else if (f.type == "int")
		readFunc = "TixiGetIntElement";
	else {
		const auto it1 = std::find_if(std::begin(enums), std::end(enums), [&](const auto& e) {
			return e.name == f.type;
		});
		if (it1 != std::end(enums)) {
			isEnum = true;
			readFunc = it1->stringToEnumFunc();
		} else {
			const auto it2 = std::find_if(std::begin(classes), std::end(classes), [&](const auto& c) {
				return c.name == f.type;
			});
			if (it2 != std::end(classes))
				isClass = true;
			else
				throw std::logic_error("No element read function provided for type " + f.type);
		}
	}
	// TODO: keep this list in sync with the shipped IOHelper

	if (isEnum)
		cpp << f.fieldName() << " = " << readFunc << "(TixiGetTextElement(tixiHandle, xpath + \"/\" + " << f.name << "));\n";
	else if (isClass)
		cpp << f.fieldName() << ".ReadCPACS(tixiHandle, xpath + \"/\" + " << f.name << "));\n";
	else
		cpp << f.fieldName() << " = " << readFunc << "(tixiHandle, xpath + \"/\" + " << f.name << ");\n";
}

auto writeElementWriteImplementation(IndentingStreamWrapper& cpp, const Field& f, const std::vector<Class>& classes, const std::vector<Enum>& enums) {
	std::string writeFunc;
	bool isEnum = false;
	bool isClass = false;
	if (f.type == "std::string")
		writeFunc = "TixiSaveTextElement";
	else if (f.type == "double")
		writeFunc = "TixiSaveDoubleElement";
	else if (f.type == "bool")
		writeFunc = "TixiSaveBoolElement";
	else if (f.type == "int")
		writeFunc = "TixiSaveIntElement";
	else {
		const auto it1 = std::find_if(std::begin(enums), std::end(enums), [&](const auto& e) {
			return e.name == f.type;
		});
		if (it1 != std::end(enums)) {
			isEnum = true;
			writeFunc = it1->enumToStringFunc();
		} else {
			const auto it2 = std::find_if(std::begin(classes), std::end(classes), [&](const auto& c) {
				return c.name == f.type;
			});
			if (it2 != std::end(classes))
				isClass = true;
			else
				throw std::logic_error("No element write function provided for type " + f.type);
		}
	}
	// TODO: keep this list in sync with the shipped IOHelper

	if (isEnum)
		cpp << "TixiSaveTextElement(tixiHandle, xpath, " << f.name << "\", " << writeFunc << "(" << f.fieldName() << "));\n";
	else if (isClass)
		cpp << f.fieldName() << ".WriteCPACS(tixiHandle, xpath + \"/\" + " << f.name << "));\n";
	else
		cpp << writeFunc << "(tixiHandle, xpath, " << f.name << "\", " << f.fieldName() << ");\n";
}

void writeIOImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields, const std::vector<Class>& classes, const std::vector<Enum>& enums) {
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
					writeAttributeReadImplementation(cpp, f, enums);
				}
				if (f.cardinality == Cardinality::One) {
					// attribute must exist
					cpp << "} else {\n";
					{
						Scope s(cpp);
						cpp << "LOG(WARNING) << \"Required attribute " << f.name << " is missing\";\n";
					}
					cpp << "}\n";
				} else
					cpp << "}\n";
			} else {
				cpp << "if (TixiCheckElement(tixiHandle, xpath + \"/\" + " << f.name << ")) {\n";
				{
					Scope s(cpp);
					writeElementReadImplementation(cpp, f, classes, enums);
				}
				if (f.cardinality == Cardinality::One || f.cardinality == Cardinality::Many) {
					// element must exist
					cpp << "} else {\n";
					{
						Scope s(cpp);
						cpp << "LOG(WARNING) << \"Required element " << f.name << " is missing\";\n";
					}
					cpp << "}\n";
				} else
					cpp << "}\n";
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
				writeAttributeWriteImplementation(cpp, f, enums);
			else
				writeElementWriteImplementation(cpp, f, classes, enums);
			cpp << "\n";
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

void writeClass(IndentingStreamWrapper& hpp, IndentingStreamWrapper& cpp, const Class& c, const std::vector<Class>& classes, const std::vector<Enum>& enums) {
	//
	// create header file
	//

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

		// ctor
		cpp << "" << c.name << "::" << c.name << "() {}\n";
		cpp << "\n";

		// io
		writeIOImplementations(cpp, c.name, c.fields, classes, enums);

		// accessors
		writeAccessorImplementations(cpp, c.name, c.fields);

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

	// namespace
	hpp << "namespace tigl {\n";
	{
		Scope s(hpp);

		// enum name
		hpp << "enum class " << e.name << " {\n";
		{
			Scope s(hpp);

			// values
			for (const auto& v : e.values)
				hpp << v << (&v != &e.values.back() ? "," : "") << "\n";
		}
		hpp << "};\n";

		// enum to string function
		hpp << "inline std::string " << e.enumToStringFunc() << "(const " << e.name << "& value) {\n";
		{
			Scope s(hpp);
			hpp << "switch(value) {\n";
			{
				Scope s(hpp);
				for (const auto& v : e.values)
					hpp << "case " << e.name << "::" << v << ": return \"" << v << "\";\n";
			}
			hpp << "}\n";
		}
		hpp << "}\n";

		// string to enum function
		hpp << "inline " << e.name << " " << e.enumToStringFunc() << "(const std::string& value) {\n";
		{
			Scope s(hpp);
			for (const auto& v : e.values) {
				hpp << "if (value == \"" << "\") {\n";
				{
					Scope s(hpp);
					hpp << "return " << e.name << "::" << v << ";\n";
				}
				hpp << "}\n";
			}
			hpp << "throw std::runtime_error(\"Invalid enum value \\\" + value + \\\" for enum type " << e.name << " \");";
		}
		hpp << "}\n";
	}
	hpp << "}\n";
	hpp << "\n";
}

void generateCode(const std::string& outputLocation, const std::vector<Class>& classes, const std::vector<Enum>& enums) {
	std::system(("mkdir " + outputLocation).c_str()); // TODO

	for (const auto& c : classes) {
		std::ofstream hppFile(outputLocation + "/" + c.name + ".h");
		hppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper hpp(hppFile);

		std::ofstream cppFile(outputLocation + "/" + c.name + ".cpp");
		cppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper cpp(cppFile);

		writeClass(hpp, cpp, c, classes, enums);
	}

	for (const auto& e : enums) {
		std::ofstream hppFile(outputLocation + "/" + e.name + ".h");
		hppFile.exceptions(std::ios::failbit | std::ios::badbit);
		IndentingStreamWrapper hpp(hppFile);

		writeEnum(hpp, e);
	}
}