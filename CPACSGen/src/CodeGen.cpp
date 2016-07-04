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
		// finish last line
		isw.os << '\n';

		// indentation
		for (unsigned int i = 0; i < isw.level; i++)
			isw.os << '\t';

		// write
		isw.os << std::forward<T>(t);

		// just return the unterlying stream
		return isw.os;
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
			hpp << "// generated from " << attOrElem->xpath;
		});
		hpp << fieldType(f) << " " << f.fieldName() << ";";

		if(&f != &fields.back())
			hpp << "";
	}
}

void CodeGen::writeAccessorDeclarations(IndentingStreamWrapper& hpp, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		if(f.cardinality == Cardinality::Optional)
			hpp << "TIGL_EXPORT bool Has" << CapitalizeFirstLetter(f.name()) << "() const;";
		hpp << "TIGL_EXPORT const " << getterSetterType(f) << "& Get" << CapitalizeFirstLetter(f.name()) << "() const;";
		const bool isClassType = m_types.classes.find(f.type) == std::end(m_types.classes);
		if(isClassType) // generate setter only for fundamental and enum types
			hpp << "TIGL_EXPORT void Set" << CapitalizeFirstLetter(f.name()) << "(const " << getterSetterType(f) << "& value);";
		else
			hpp << "TIGL_EXPORT " << getterSetterType(f) << "& Get" << CapitalizeFirstLetter(f.name()) << "();";
		hpp << "";
	}
}

void CodeGen::writeAccessorImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields) {
	for (const auto& f : fields) {
		const auto op = f.cardinality == Cardinality::Optional;
		if (op)
			cpp << "TIGL_EXPORT bool " << className << "::Has" << CapitalizeFirstLetter(f.name()) << "() const { return " << f.fieldName() << ".isValid(); }";
		cpp << "const " << getterSetterType(f) << "& " << className << "::Get" << CapitalizeFirstLetter(f.name()) << "() const { return " << f.fieldName() << (op ? ".get()" : "") << "; }";
		cpp << getterSetterType(f) << "& " << className << "::Get" << CapitalizeFirstLetter(f.name()) << "() { return " << f.fieldName() << (op ? ".get()" : "") << "; }";
		if (m_types.classes.find(f.type) == std::end(m_types.classes)) // generate setter only for fundamental and enum types
			cpp << "void " << className << "::Set" << CapitalizeFirstLetter(f.name()) << "(const " << getterSetterType(f) << "& value) { " << f.fieldName() << " = value; }";
		cpp << "";
	}
}

void CodeGen::writeIODeclarations(IndentingStreamWrapper& hpp, const std::string& className, const std::vector<Field>& fields) {
	hpp << "TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);";
	hpp << "TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;";
	hpp << "";
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

	const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : ", \"" + f.cpacsName + "\"";

	// fundamental types
	const auto itF = fundamentalTypes.find(f.type);
	if (itF != std::end(fundamentalTypes)) {
		const auto& type = itF->second;
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << f.fieldName() << " = " << "TixiGet" << type << AttOrElem << "(tixiHandle, xpath" << subPathArg << ");";
				break;
			case Cardinality::Vector:
				if (f.xmlType == XMLConstruct::Attribute || f.xmlType == XMLConstruct::SimpleContent)
					throw std::runtime_error("Attributes or simpleContents cannot be vectors");
				cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {";
				{
					Scope s(cpp);
					cpp << "return TixiGet" << type << AttOrElem << "(tixiHandle, childXPath, \"" << f.cpacsName << "\");";
				}
				cpp << "});";
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
				cpp << f.fieldName() << " = " << readFunc << "(TixiGetText" << AttOrElem << "(tixiHandle, xpath" << subPathArg << "));";
				break;
			case Cardinality::Vector:
				throw NotImplementedException("Reading enum vectors is not implemented");
		}
		return;
	}

	// classes
	if (f.xmlType != XMLConstruct::Attribute) {
		const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : " + \"/" + f.cpacsName + "\"";
		const auto itC = m_types.classes.find(f.type);
		if (itC != std::end(m_types.classes)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
					cpp << f.fieldName() << ".construct();";
					cpp << f.fieldName() << "->ReadCPACS(tixiHandle, xpath + \"/" << f.cpacsName << "\");";
					break;
				case Cardinality::Mandatory:
					cpp << f.fieldName() << ".ReadCPACS(tixiHandle, xpath" << subPathArg << ");";
					break;
				case Cardinality::Vector:
					cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {";
					{
						Scope s(cpp);
						cpp << "using ChildType = std::remove_pointer_t<" << fieldType(f) << "::value_type>;";
						cpp << "ChildType* child = new ChildType;";
						cpp << "child->ReadCPACS(tixiHandle, xpath + \"/" << f.cpacsName << "\");";
						cpp << "return child;";
					}
					cpp << "});";
					break;
			}
			return;
		}
	}

	throw std::logic_error("No read function provided for type " + f.type);
}

void CodeGen::writeWriteAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f) {
	const std::string AttOrElem = tixiFuncSuffix(f.xmlType);

	const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : ", \"" + f.cpacsName + "\"";

	// fundamental types
	const auto itF = fundamentalTypes.find(f.type);
	if (itF != std::end(fundamentalTypes)) {
		switch (f.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				cpp << "TixiSave" << AttOrElem << "(tixiHandle, xpath" << subPathArg << ", " << f.fieldName() << ");";
				break;
			case Cardinality::Vector:
				if (f.xmlType == XMLConstruct::Attribute || f.xmlType == XMLConstruct::SimpleContent)
					throw std::runtime_error("Attributes or simpleContents cannot be vectors");
				cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const " << f.type << "& child) {";
				{
					Scope s(cpp);
					cpp << "TixiSave" << AttOrElem << "(tixiHandle, childXPath, \"" << f.cpacsName << "\", child);";
				}
				cpp << "});";
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
				cpp << "TixiSave" << AttOrElem << "(tixiHandle, xpath" << subPathArg << ", " << itE->second.enumToStringFunc() << "(" << f.fieldName() << (f.cardinality == Cardinality::Optional ? ".get()" : "") << "));";
				break;
			case Cardinality::Vector:
				throw NotImplementedException("Writing enum vectors is not implemented");
		}
		return;
	}

	// classes
	if (f.xmlType != XMLConstruct::Attribute) {
		const auto subPathArg = f.xmlType == XMLConstruct::SimpleContent ? "" : " + \"/" + f.cpacsName + "\"";
		const auto itC = m_types.classes.find(f.type);
		if (itC != std::end(m_types.classes)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
					cpp << "if (" << f.fieldName() << ".isValid()) {";
					{
						Scope s(cpp);
						cpp << f.fieldName() << "->WriteCPACS(tixiHandle, xpath + \"/" << f.cpacsName << "\");";
					}
					cpp << "}";
					break;
				case Cardinality::Mandatory:
					cpp << f.fieldName() << ".WriteCPACS(tixiHandle, xpath" << subPathArg << ");";
					break;
				case Cardinality::Vector:
					cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const " << f.type << "* child) {";
					{
						Scope s(cpp);
						cpp << "child->WriteCPACS(tixiHandle, childXPath);";
					}
					cpp << "});";
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
		cpp << "*this = " << "TixiGet" << itF->second << "Element(tixiHandle, xpath);";
		return;
	}

	// classes
	const auto itC = m_types.classes.find(type);
	if (itC != std::end(m_types.classes)) {
		cpp << type << "::ReadCPACS(tixiHandle, xpath);";
		return;
	}

	throw std::logic_error("No read function provided for type " + type);
}

void CodeGen::writeWriteBaseImplementation(IndentingStreamWrapper& cpp, const std::string& type) {
	// fundamental types
	const auto itF = fundamentalTypes.find(type);
	if (itF != std::end(fundamentalTypes)) {
		cpp << "TixiSaveElement(tixiHandle, xpath, *this);";
		return;
	}

	// classes
	const auto itC = m_types.classes.find(type);
	if (itC != std::end(m_types.classes)) {
		cpp << type << "::WriteCPACS(tixiHandle, xpath);";
		return;
	}

	throw std::logic_error("No write function provided for type " + type);
}

void CodeGen::writeReadImplementation(IndentingStreamWrapper& cpp, const Class& c, const std::vector<Field>& fields) {
	cpp << "void " << c.name << "::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {";
	{
		Scope s(cpp);

		// base class
		if (!c.base.empty())
			writeReadBaseImplementation(cpp, c.base);

		// fields
		for (const auto& f : fields) {
			const auto construct = xmlConstructToString(f.xmlType);
			const auto AttOrElem = tixiFuncSuffix(f.xmlType);
			cpp << "// read " << construct << " " << f.cpacsName << "";
			cpp << "if (TixiCheck" << AttOrElem << "(tixiHandle, xpath, \"" << f.cpacsName << "\")) {";
			{
				Scope s(cpp);
				writeReadAttributeOrElementImplementation(cpp, f);
			}
			cpp << "}";
			if (f.cardinality == Cardinality::Mandatory) {
				// attribute or element must exist
				cpp << "else {";
				{
					Scope s(cpp);
					cpp << "LOG(WARNING) << \"Required " << construct << " " << f.cpacsName << " is missing\";";
				}
				cpp << "}";
			}
			cpp << "";
		}
	}
	cpp << "}";
}

void CodeGen::writeWriteImplementation(IndentingStreamWrapper& cpp, const Class& c, const std::vector<Field>& fields) {
	cpp << "void " << c.name << "::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {";
	{
		Scope s(cpp);

		// base class
		if (!c.base.empty())
			writeWriteBaseImplementation(cpp, c.base);

		// fields
		for (const auto& f : fields) {
			const auto construct = xmlConstructToString(f.xmlType);
			cpp << "// write " << construct << " " << f.cpacsName;
			if (f.cardinality == Cardinality::Optional) {
				cpp << "if (Has" << CapitalizeFirstLetter(f.name()) << "()) {";
				{
					Scope s(cpp);
					writeWriteAttributeOrElementImplementation(cpp, f);
				}
				cpp << "}";
			} else
				writeWriteAttributeOrElementImplementation(cpp, f);
			cpp << "";
		}
	}
	cpp << "}";
}

void CodeGen::writeLicenseHeader(IndentingStreamWrapper& f) {
	f << "// This file was autogenerated by CPACSGen, do not edit";
	f << "//";
	f << "// Licensed under the Apache License, Version 2.0 (the \"License\")";
	f << "// you may not use this file except in compliance with the License.";
	f << "// You may obtain a copy of the License at";
	f << "//";
	f << "//     http://www.apache.org/licenses/LICENSE-2.0";
	f << "//";
	f << "// Unless required by applicable law or agreed to in writing, software";
	f << "// distributed under the License is distributed on an \"AS IS\" BASIS,";
	f << "// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.";
	f << "// See the License for the specific language governing permissions and";
	f << "// limitations under the License.";
	f << "";
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

	// parent pointers
	if (m_parentPointers.find(c.name) != std::end(m_parentPointers)) {
		for (const auto& dep : c.deps.parents)
			deps.hppForwards.push_back(dep->name);
	}

	// misc cpp includes
	deps.cppIncludes.push_back("\"IOHelper.h\"");
	deps.cppIncludes.push_back("\"CTiglLogging.h\"");
	deps.cppIncludes.push_back("\"" + c.name + ".h\"");

	return deps;
}

void CodeGen::writeParentPointerCtors(IndentingStreamWrapper& hpp, const Class& c) {
	if (m_parentPointers.find(c.name) != std::end(m_parentPointers))
		for (const auto& dep : c.deps.parents)
			hpp << "TIGL_EXPORT " << c.name << "(" << dep->name << "* parent);";
}

void CodeGen::writeParentPointerFields(IndentingStreamWrapper& hpp, const Class& c) {
	if (m_parentPointers.find(c.name) != std::end(m_parentPointers)) {
		if (c.deps.parents.size() > 1) {
			throw NotImplementedException("Multiple parent classes are not implemented");
		}

		for (const auto& dep : c.deps.parents)
			hpp << dep->name << "* m_parent_" << dep->name << ";";
	}
}

void CodeGen::writeParentPointerCtorImplementations(IndentingStreamWrapper& cpp, const Class& c) {
	if (m_parentPointers.find(c.name) != std::end(m_parentPointers)) {
		for (const auto& dep : c.deps.parents) {
			cpp << c.name << "::" << c.name << "(" << dep->name << "* parent) {";
			{
				Scope s(cpp);
				cpp << "m_parent_" << dep->name << " = parent;";
			}
			cpp << "}";
			cpp << "";
		}
	}
}

void CodeGen::writeHeader(IndentingStreamWrapper& hpp, const Class& c, const Includes& includes) {
	hpp << "#pragma once";
	hpp << "";

	// file header
	writeLicenseHeader(hpp);

	// includes
	for (const auto& inc : includes.hppIncludes)
		hpp << "#include " << inc << "";
	hpp << "";

	// namespace
	hpp << "namespace tigl {";
	{
		Scope s(hpp);

		// custom Tigl types declarations
		for (const auto& fwd : includes.hppCustomForwards)
			hpp << "class " << fwd << ";";
		hpp << "";

		hpp << "namespace generated {";
		{
			Scope s(hpp);

			// forward declarations
			for (const auto& fwd : includes.hppForwards)
				hpp << "class " << fwd << ";";
			hpp << "";

			// meta information from schema
			hpp << "// This class is used in:";
			for (const auto& c : c.deps.parents) {
				hpp << "// " << c->name << "";
			}
			hpp << "";
			hpp << "// generated from " << c.origin->xpath << "";

			// class name and base class
			hpp << "class " << c.name << (c.base.empty() ? "" : " : public " + c.base) << " {";
			hpp << "public:";
			{
				Scope s(hpp);

				// ctor
				hpp << "TIGL_EXPORT " << c.name << "();";
				writeParentPointerCtors(hpp, c);
				hpp << "";

				// dtor
				hpp << "TIGL_EXPORT virtual ~" << c.name << "();";
				hpp << "";

				// io
				writeIODeclarations(hpp, c.name, c.fields);

				// accessors
				writeAccessorDeclarations(hpp, c.fields);

			}
			hpp << "protected:";
			{
				Scope s(hpp);

				// parent pointers
				writeParentPointerFields(hpp, c);
				hpp << "";

				// fields
				writeFields(hpp, c.fields);
				hpp << "";
			}
			hpp << "private:";
			{
				Scope s(hpp);

				// copy ctor and assign
				hpp << "TIGL_EXPORT " << c.name << "(const " << c.name << "&) = delete;";
				hpp << "TIGL_EXPORT " << c.name << "& operator=(const " << c.name << "&) = delete;";
				hpp << "";

				// move ctor and assign
				hpp << "TIGL_EXPORT " << c.name << "(" << c.name << "&&) = delete;";
				hpp << "TIGL_EXPORT " << c.name << "& operator=(" << c.name << "&&) = delete;";
			}
			hpp << "};";
		}
		hpp << "}";
		// export non-custom types into tigl namespace
		if (m_customTypes.find(c.name) != std::end(m_customTypes)) {
			hpp << "//using generated::" << c.name << ";";
		}
	}
	hpp << "}";
	hpp << "";
}

void CodeGen::writeSource(IndentingStreamWrapper& cpp, const Class& c, const Includes& includes) {
	// file header
	writeLicenseHeader(cpp);

	// includes
	for (const auto& inc : includes.cppIncludes)
		cpp << "#include " << inc << "";
	cpp << "";

	// namespace
	cpp << "namespace tigl {";
	{
		Scope s(cpp);

		cpp << "namespace generated {";
		{
			Scope s(cpp);

			// ctor
			cpp << c.name << "::" << c.name << "() {}";
			cpp << "";
			writeParentPointerCtorImplementations(cpp, c);

			// dtor
			cpp << c.name << "::~" << c.name << "() {}";
			cpp << "";

			// io
			writeReadImplementation(cpp, c, c.fields);
			cpp << "";
			writeWriteImplementation(cpp, c, c.fields);
			cpp << "";

			// accessors
			writeAccessorImplementations(cpp, c.name, c.fields);
		}
		cpp << "}";
	}
	cpp << "}";
	cpp << "";
}

void CodeGen::writeClass(IndentingStreamWrapper& hpp, IndentingStreamWrapper& cpp, const Class& c) {
	const auto includes = resolveIncludes(c);
	writeHeader(hpp, c, includes);
	writeSource(cpp, c, includes);
}

void CodeGen::writeEnum(IndentingStreamWrapper& hpp, const Enum& e) {

	hpp << "#pragma once";
	hpp << "";

	// file header
	writeLicenseHeader(hpp);

	// includes
	hpp << "#include <stdexcept>";
	hpp << "";

	// namespace
	hpp << "namespace tigl {";
	{
		Scope s(hpp);

		hpp << "namespace generated {";
		{
			Scope s(hpp);

			// meta information from schema
			hpp << "// This enum is used in:";
			for (const auto& c : e.deps.parents) {
				hpp << "// " << c->name << "";
			}
			hpp << "";
			hpp << "// generated from " << e.origin->xpath;

			// enum name
			hpp << "enum class " << e.name << " {";
			{
				Scope s(hpp);

				// values
				for (const auto& v : e.values)
					hpp << v.cppName << (&v != &e.values.back() ? "," : "") << "";
			}
			hpp << "};";
			hpp << "";

			// enum to string function
			hpp << "inline std::string " << e.enumToStringFunc() << "(const " << e.name << "& value) {";
			{
				Scope s(hpp);
				hpp << "switch(value) {";
				{
					Scope s(hpp);
					for (const auto& v : e.values)
						hpp << "case " << e.name << "::" << v.cppName << ": return \"" << v.name << "\";";
					hpp << "default: throw std::runtime_error(\"Invalid enum value \\\" + value + \\\" for enum type " << e.name << "\");";
				}
				hpp << "}";
			}
			hpp << "}";

			// string to enum function
			hpp << "inline " << e.name << " " << e.stringToEnumFunc() << "(const std::string& value) {";
			{
				Scope s(hpp);
				for (const auto& v : e.values)
					hpp << "if (value == \"" << v.name << "\") return " << e.name << "::" << v.cppName << ";";
				hpp << "throw std::runtime_error(\"Invalid enum value \\\" + value + \\\" for enum type " << e.name << "\");";
			}
			hpp << "}";
		}
		hpp << "}";
	}
	hpp << "}";
	hpp << "";
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

	auto sortAndUnique = [](auto& con) {
		std::sort(std::begin(con), std::end(con));
		con.erase(std::unique(std::begin(con), std::end(con)), std::end(con));
	};

	// sort and unique
	for (auto& p : classes) {
		auto& c = p.second;
		sortAndUnique(c.deps.bases);
		sortAndUnique(c.deps.children);
		sortAndUnique(c.deps.deriveds);
		sortAndUnique(c.deps.enumChildren);
		sortAndUnique(c.deps.parents);
	}

	for (auto& p : enums) {
		auto& e = p.second;
		sortAndUnique(e.deps.parents);
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
