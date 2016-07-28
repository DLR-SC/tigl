#include <vector>
#include <cctype>
#include <fstream>
#include <algorithm>

#include "NotImplementedException.h"
#include "CodeGen.h"

namespace tigl {
	// some options
	namespace {
		bool c_generateDefaultCtorsForParentPointerTypes = false;
		bool c_generateCaseSensitiveStringToEnumConversion = false; // true would be more strict, but some test data has troubles with this
		bool c_generateTryCatchAroundOptionalClassReads = true;
	}

	struct Scope;

	class IndentingStreamWrapper {
	public:
		IndentingStreamWrapper(std::ostream& os)
			: os(os) {}

		// indents on first use
		template<typename T>
		friend auto operator<<(IndentingStreamWrapper& isw, T&& t) -> std::ostream& {
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

		auto raw() -> std::ostream& {
			return os;
		}

	private:
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

	namespace {
		auto customReplacedType(const std::string& type) -> const std::string& {
			const auto p = s_customTypes.find(type);
			return p ? *p : type;
		}

		auto CapitalizeFirstLetter(std::string str) -> std::string {
			if (str.empty())
				return str;

			str[0] = std::toupper(str[0]);

			return str;
		}

		auto enumToStringFunc(const Enum& e) -> std::string {
			return customReplacedType(e.name) + "ToString";
		}

		auto stringToEnumFunc(const Enum& e) -> std::string {
			return "stringTo" + CapitalizeFirstLetter(customReplacedType(e.name));
		}
	}

	auto CodeGen::getterSetterType(const Field& field) const -> std::string {
		const auto typeName = customReplacedType(field.typeName);
		switch (field.cardinality) {
			case Cardinality::Optional:
			case Cardinality::Mandatory:
				return typeName;
			case Cardinality::Vector:
			{
				const bool makePointer = m_types.classes.find(field.typeName) != std::end(m_types.classes);
				if(makePointer)
					return "std::vector<std::unique_ptr<" + typeName + ">>";
				else
					return "std::vector<" + typeName + ">";
			}
			default:
				throw std::logic_error("Invalid cardinality");
		}
	}

	auto CodeGen::fieldType(const Field& field) const -> std::string {
		switch (field.cardinality) {
			case Cardinality::Optional:
				return "Optional<" + getterSetterType(field) + ">";
			default:
				return getterSetterType(field);
		}
	}

	namespace {
		// TODO: create polymorphic lambda when C++14 is available
		struct WriteGeneratedFromVisitor {
			WriteGeneratedFromVisitor(IndentingStreamWrapper& hpp)
				: hpp(hpp) {}

			template <typename T>
			void operator()(const T* attOrElem) {
				hpp << "// generated from " << attOrElem->xpath;
			}

		private:
			IndentingStreamWrapper& hpp;
		};
	}

	void CodeGen::writeFields(IndentingStreamWrapper& hpp, const std::vector<Field>& fields) {
		for (const auto& f : fields) {
			f.origin.visit(WriteGeneratedFromVisitor(hpp));
			//f.origin.visit([&](const auto* attOrElem) {
			//	hpp << "// generated from " << attOrElem->xpath;
			//});
			hpp << fieldType(f) << " " << f.fieldName() << ";";

			if (&f != &fields.back())
				hpp << "";
		}
		if (fields.size() > 0)
			hpp << "";
	}

	void CodeGen::writeAccessorDeclarations(IndentingStreamWrapper& hpp, const std::vector<Field>& fields) {
		for (const auto& f : fields) {
			if (f.cardinality == Cardinality::Optional)
				hpp << "TIGL_EXPORT bool Has" << CapitalizeFirstLetter(f.name()) << "() const;";
			hpp << "TIGL_EXPORT const " << getterSetterType(f) << "& Get" << CapitalizeFirstLetter(f.name()) << "() const;";
			const bool isClassType = m_types.classes.find(f.typeName) == std::end(m_types.classes);
			if (isClassType) // generate setter only for fundamental and enum types
				hpp << "TIGL_EXPORT void Set" << CapitalizeFirstLetter(f.name()) << "(const " << getterSetterType(f) << "& value);";
			else
				hpp << "TIGL_EXPORT " << getterSetterType(f) << "& Get" << CapitalizeFirstLetter(f.name()) << "();";
			hpp << "";
		}
	}

	void CodeGen::writeAccessorImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields) {
		for (const auto& f : fields) {
			const auto op = f.cardinality == Cardinality::Optional;
			if (op) {
				cpp << "bool " << className << "::Has" << CapitalizeFirstLetter(f.name()) << "() const {";
				{
					Scope s(cpp);
					cpp << "return static_cast<bool>(" << f.fieldName() << ");";
				}
				cpp << "}";
				cpp << "";
			}

			cpp << "const " << getterSetterType(f) << "& " << className << "::Get" << CapitalizeFirstLetter(f.name()) << "() const {";
			{
				Scope s(cpp);
				cpp << "return " << (op ? "*" : "") << f.fieldName() << ";";
			}
			cpp << "}";
			cpp << "";

			const bool isClassType = m_types.classes.find(f.typeName) == std::end(m_types.classes);
			// generate setter only for fundamental and enum types
			if (isClassType) {
				cpp << "void " << className << "::Set" << CapitalizeFirstLetter(f.name()) << "(const " << getterSetterType(f) << "& value) {";
				{
					Scope s(cpp);
					cpp << f.fieldName() << " = value;";
				}
				cpp << "}";
			} else {
				cpp << getterSetterType(f) << "& " << className << "::Get" << CapitalizeFirstLetter(f.name()) << "() {";
				{
					Scope s(cpp);
					cpp << "return " << (op ? "*" : "") << f.fieldName() << ";";
				}
				cpp << "}";
			}
			cpp << "";
		}
	}

	void CodeGen::writeParentPointerGetters(IndentingStreamWrapper& hpp, const Class& c) {
		if (s_parentPointers.contains(c.name)) {
			if (c.deps.parents.size() > 1) {
				hpp << "// getter for parent classes";
				hpp << "template<typename P>";
				hpp << "bool IsParent() const {";
				{
					Scope s(hpp);
					hpp << "return m_parentType != nullptr && *m_parentType == typeid(P);";
				}
				hpp << "}";
				hpp << "";
				hpp << "template<typename P>";
				hpp << "P* GetParent() const {";
				{
					Scope s(hpp);
					hpp << "static_assert(";
					for (const auto& dep : c.deps.parents) {
						if (&dep != &c.deps.parents[0])
							hpp.raw() << " || ";
						hpp.raw() << "std::is_same<P, " << customReplacedType(dep->name) << ">::value";
					}
					hpp.raw() << ", \"template argument for P is not a parent class of " << c.name << "\");";
					hpp << "if (m_parent == nullptr) {";
					{
						Scope s(hpp);
						hpp << "return nullptr;";
					}
					hpp << "}";
					hpp << "if (!IsParent<P>()) {";
					{
						Scope s(hpp);
						hpp << "throw std::runtime_error(\"bad parent\");";
					}
					hpp << "}";
					hpp << "return static_cast<P*>(m_parent);";
				}
				hpp << "}";
			} else if (c.deps.parents.size() == 1) {
				hpp << "// getter for parent class";
				hpp << customReplacedType(c.deps.parents[0]->name) << "* GetParent() const;";
			}
			hpp << "";
		}
	}

	void CodeGen::writeParentPointerGetterImplementation(IndentingStreamWrapper& cpp, const Class& c) {
		if (s_parentPointers.contains(c.name)) {
			if (c.deps.parents.size() == 1) {
				cpp << customReplacedType(c.deps.parents[0]->name) << "* " << c.name << "::GetParent() const {";
				{
					Scope s(cpp);
					cpp << "return m_parent;";
				}
				cpp << "}";
				cpp << "";
			}
		}
	}

	void CodeGen::writeIODeclarations(IndentingStreamWrapper& hpp, const std::string& className, const std::vector<Field>& fields) {
		hpp << "TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);";
		hpp << "TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;";
		hpp << "";
	}

	namespace {
		auto tixiFuncSuffix(const XMLConstruct& construct) -> std::string {
			switch (construct) {
				case XMLConstruct::Attribute:
					return "Attribute";
				case XMLConstruct::Element:
				case XMLConstruct::SimpleContent:
				case XMLConstruct::FundamentalTypeBase:
					return "Element";
				default:
					throw std::logic_error("Cannot determine tixi function suffix for the given XML construct");
			}
		}

		auto xmlConstructToString(const XMLConstruct& construct) -> std::string {
			switch (construct) {
				case XMLConstruct::Attribute:           return "attribute";
				case XMLConstruct::Element:             return "element";
				case XMLConstruct::SimpleContent:       return "simpleContent";
				case XMLConstruct::FundamentalTypeBase: return "fundamental type base class";
				default: throw std::logic_error("Unknown XML construct");
			}
		}
	}

	void CodeGen::writeReadAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Class& c, const Field& f) {
		const std::string AttOrElem = tixiFuncSuffix(f.xmlType);

		// fundamental types
		const auto pf = s_fundamentalTypes.find(f.typeName);
		if (pf) {
			const auto& type = *pf;
			switch (f.cardinality) {
				case Cardinality::Optional:
				case Cardinality::Mandatory:
					cpp << f.fieldName() << " = TixiGet" << type << AttOrElem << "(tixiHandle, xpath, \"" + f.cpacsName + "\");";
					break;
				case Cardinality::Vector:
					if (f.xmlType == XMLConstruct::Attribute || f.xmlType == XMLConstruct::SimpleContent || f.xmlType == XMLConstruct::FundamentalTypeBase)
						throw std::runtime_error("Attributes, simpleContents and bases cannot be vectors");
					cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {";
					{
						Scope s(cpp);
						cpp << "return TixiGet" << type << AttOrElem << "(tixiHandle, childXPath, \"\");";
					}
					cpp << "});";
					break;
			}

			return;
		}

		// enums
		const auto itE = m_types.enums.find(f.typeName);
		if (itE != std::end(m_types.enums)) {
			const auto& readFunc = stringToEnumFunc(itE->second);
			switch (f.cardinality) {
				case Cardinality::Optional:
				case Cardinality::Mandatory:
					cpp << f.fieldName() << " = " << readFunc << "(TixiGetText" << AttOrElem << "(tixiHandle, xpath, \"" + f.cpacsName + "\"));";
					break;
				case Cardinality::Vector:
					throw NotImplementedException("Reading enum vectors is not implemented");
			}
			return;
		}

		// classes
		if (f.xmlType != XMLConstruct::Attribute && f.xmlType != XMLConstruct::FundamentalTypeBase) {
			const auto itC = m_types.classes.find(f.typeName);
			const bool requiresParentPointer = s_parentPointers.contains(f.typeName);
			if (itC != std::end(m_types.classes)) {
				switch (f.cardinality) {
					case Cardinality::Optional:
						if (requiresParentPointer)
							cpp << f.fieldName() << ".construct(" << parentPointerThis(c) << ");";
						else
							cpp << f.fieldName() << ".construct();";
						if (c_generateTryCatchAroundOptionalClassReads) {
							cpp << "try {";
							{
								Scope s(cpp);
								cpp << f.fieldName() << "->ReadCPACS(tixiHandle, xpath + \"/" << f.cpacsName << "\");";
							}
							cpp << "} catch(const std::exception& e) {";
							{
								Scope s(cpp);
								cpp << "LOG(ERROR) << \"Failed to read " << f.cpacsName << " at xpath << \" << xpath << \": \" << e.what();";
								cpp << f.fieldName() << ".destroy();";
							}
							cpp << "}";
						} else
							cpp << f.fieldName() << "->ReadCPACS(tixiHandle, xpath + \"/" << f.cpacsName << "\");";
						break;
					case Cardinality::Mandatory:
						cpp << f.fieldName() << ".ReadCPACS(tixiHandle, xpath + \"/" + f.cpacsName + "\");";
						break;
					case Cardinality::Vector:
						cpp << "TixiReadElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath) {";
						{
							Scope s(cpp);
							cpp << "auto child = std::make_unique<" << customReplacedType(f.typeName) << ">(" << (requiresParentPointer ? parentPointerThis(c) : "") << ");";
							cpp << "child->ReadCPACS(tixiHandle, childXPath);";
							cpp << "return child;";
						}
						cpp << "});";
						break;
				}
				return;
			}
		}

		throw std::logic_error("No read function provided for type " + f.typeName);
	}

	void CodeGen::writeWriteAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f) {
		const std::string AttOrElem = tixiFuncSuffix(f.xmlType);

		// fundamental types
		if (s_fundamentalTypes.contains(f.typeName)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
				case Cardinality::Mandatory:
					cpp << "TixiSave" << AttOrElem << "(tixiHandle, xpath, \"" + f.cpacsName + "\", " << f.fieldName() << ");";
					break;
				case Cardinality::Vector:
					if (f.xmlType == XMLConstruct::Attribute || f.xmlType == XMLConstruct::SimpleContent || f.xmlType == XMLConstruct::FundamentalTypeBase)
						throw std::runtime_error("Attributes, simpleContents and bases cannot be vectors");
					cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const " << customReplacedType(f.typeName) << "& child) {";
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
		const auto itE = m_types.enums.find(f.typeName);
		if (itE != std::end(m_types.enums)) {
			switch (f.cardinality) {
				case Cardinality::Optional:
				case Cardinality::Mandatory:
					cpp << "TixiSave" << AttOrElem << "(tixiHandle, xpath, \"" + f.cpacsName + "\", " << enumToStringFunc(itE->second) << "(" << (f.cardinality == Cardinality::Optional ? "*" : "") << f.fieldName() << "));";
					break;
				case Cardinality::Vector:
					throw NotImplementedException("Writing enum vectors is not implemented");
			}
			return;
		}

		// classes
		if (f.xmlType != XMLConstruct::Attribute && f.xmlType != XMLConstruct::FundamentalTypeBase) {
			const auto itC = m_types.classes.find(f.typeName);
			if (itC != std::end(m_types.classes)) {
				switch (f.cardinality) {
					case Cardinality::Optional:
						cpp << "if (" << f.fieldName() << ") {";
						{
							Scope s(cpp);
							cpp << f.fieldName() << "->WriteCPACS(tixiHandle, xpath + \"/" << f.cpacsName << "\");";
						}
						cpp << "}";
						break;
					case Cardinality::Mandatory:
						cpp << f.fieldName() << ".WriteCPACS(tixiHandle, xpath + \"/" + f.cpacsName + "\");";
						break;
					case Cardinality::Vector:
						cpp << "TixiSaveElements(tixiHandle, xpath, \"" << f.cpacsName << "\", " << f.fieldName() << ", [&](const std::string& childXPath, const std::unique_ptr<" << customReplacedType(f.typeName) << ">& child) {";
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

		throw std::logic_error("No write function provided for type " + f.typeName);
	}

	void CodeGen::writeReadBaseImplementation(IndentingStreamWrapper& cpp, const std::string& type) {
		// fundamental types
		const auto pf = s_fundamentalTypes.find(type);
		if (pf) {
			cpp << "*this = TixiGet" << *pf << "Element(tixiHandle, xpath);";
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
		if (s_fundamentalTypes.contains(type)) {
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
			if (!c.base.empty()) {
				cpp << "// read base";
				writeReadBaseImplementation(cpp, c.base);
				cpp << "";
			}

			// fields
			for (const auto& f : fields) {
				const auto construct = xmlConstructToString(f.xmlType);
				const auto AttOrElem = tixiFuncSuffix(f.xmlType);
				cpp << "// read " << construct << " " << f.cpacsName << "";
				cpp << "if (TixiCheck" << AttOrElem << "(tixiHandle, xpath, \"" << f.cpacsName << "\")) {";
				{
					Scope s(cpp);
					writeReadAttributeOrElementImplementation(cpp, c, f);
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
		cpp << "";
	}

	void CodeGen::writeWriteImplementation(IndentingStreamWrapper& cpp, const Class& c, const std::vector<Field>& fields) {
		cpp << "void " << c.name << "::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {";
		{
			Scope s(cpp);

			// base class
			if (!c.base.empty()) {
				cpp << "// write base";
				writeWriteBaseImplementation(cpp, c.base);
				cpp << "";
			}

			// fields
			for (const auto& f : fields) {
				const auto construct = xmlConstructToString(f.xmlType);
				cpp << "// write " << construct << " " << f.cpacsName;
				writeWriteAttributeOrElementImplementation(cpp, f);
				cpp << "";
			}
		}
		cpp << "}";
		cpp << "";
	}

	void CodeGen::writeLicenseHeader(IndentingStreamWrapper& f) {
		f << "// Copyright (c) 2016 RISC Software GmbH";
		f << "//";
		f << "// This file was generated by CPACSGen from CPACS XML Schema (c) German Aerospace Center (DLR/SC).";
		f << "// Do not edit, all changes are lost when files are re-generated.";
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

	auto CodeGen::resolveIncludes(const Class& c) -> Includes {
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
		if (vectorHeader) {
			deps.hppIncludes.push_back("<vector>");
			deps.hppIncludes.push_back("<memory>"); // for unique_ptr
		}
		if (optionalHeader)
			deps.hppIncludes.push_back("\"Optional.hpp\"");

		deps.hppIncludes.push_back("\"tigl_internal.h\"");

		// base class
		if (!c.base.empty() && m_types.classes.find(c.base) != std::end(m_types.classes)) {
			deps.hppIncludes.push_back("\"" + c.base + ".h\"");
		}

		// fields
		for (const auto& f : c.fields) {
			if (m_types.enums.find(f.typeName) != std::end(m_types.enums) ||
				m_types.classes.find(f.typeName) != std::end(m_types.classes)) {
				// this is a class or enum type, include it

				const auto p = s_customTypes.find(f.typeName);
				if (!p) {
					switch (f.cardinality) {
						case Cardinality::Optional:
						case Cardinality::Mandatory:
							deps.hppIncludes.push_back("\"" + f.typeName + ".h\"");
							break;
						case Cardinality::Vector:
							deps.hppForwards.push_back(f.typeName);
							deps.cppIncludes.push_back("\"" + f.typeName + ".h\"");
							break;
					}
				} else {
					// custom types are Tigl types and resolved using include paths and require a different namespace
					switch (f.cardinality) {
						case Cardinality::Optional:
						case Cardinality::Mandatory:
							deps.hppIncludes.push_back("<" + *p + ".h>");
							break;
						case Cardinality::Vector:
							deps.hppCustomForwards.push_back(*p);
							deps.cppIncludes.push_back("<" + *p + ".h>");
							break;
					}
				}
			}
		}

		// parent pointers
		if (s_parentPointers.contains(c.name)) {
			deps.cppIncludes.push_back("<cassert>");
			for (const auto& dep : c.deps.parents) {
				const auto p = s_customTypes.find(dep->name);
				if (p)
					deps.hppCustomForwards.push_back(*p);
				else
					deps.hppForwards.push_back(dep->name);
			}
		}

		// misc cpp includes
		deps.cppIncludes.push_back("\"IOHelper.h\"");
		deps.cppIncludes.push_back("\"CTiglLogging.h\"");
		deps.cppIncludes.push_back("\"" + c.name + ".h\"");

		return deps;
	}

	void CodeGen::writeCtors(IndentingStreamWrapper& hpp, const Class& c) {
		if (s_parentPointers.contains(c.name)) {
			if (c_generateDefaultCtorsForParentPointerTypes)
				hpp << "TIGL_EXPORT " << c.name << "();";
			for (const auto& dep : c.deps.parents)
				hpp << "TIGL_EXPORT " << c.name << "(" << customReplacedType(dep->name) << "* parent);";
			hpp << "";
		} else {
			hpp << "TIGL_EXPORT " << c.name << "();";
		}
	}

	void CodeGen::writeParentPointerFields(IndentingStreamWrapper& hpp, const Class& c) {
		if (s_parentPointers.contains(c.name)) {
			if (c.deps.parents.size() > 1) {
				hpp << "// pointer to parent classes";
				hpp << "void* m_parent;";
				hpp << "const std::type_info* m_parentType;";
			} else if (c.deps.parents.size() == 1) {
				hpp << "// pointer to parent class";
				hpp << customReplacedType(c.deps.parents[0]->name) << "* m_parent;";
			}
			hpp << "";
		}
	}

	auto CodeGen::parentPointerThis(const Class& c) const -> std::string {
		const auto cust = s_customTypes.find(c.name);
		if (cust)
			return "reinterpret_cast<" + *cust + "*>(this)";
		else
			return "this";
	}

	void CodeGen::writeCtorImplementations(IndentingStreamWrapper& cpp, const Class& c) {
		auto writeParentPointerFieldInitializers = [&] {
			Scope s(cpp);
			bool first = true;
			for (const auto& f : c.fields) {
				if (f.cardinality == Cardinality::Mandatory && s_parentPointers.contains(f.typeName)) {
					if (first) {
						cpp.raw() << " :";
						first = false;
					} else
						cpp.raw() << ", ";
					cpp << f.fieldName() << "(" << parentPointerThis(c) << ")";
				}
			}
		};

		// if this class holds parent pointers, we have to provide corresponding ctor overloads
		if (s_parentPointers.contains(c.name)) {
			if (c_generateDefaultCtorsForParentPointerTypes) {
				cpp << c.name << "::" << c.name << "()";
				writeParentPointerFieldInitializers();
				cpp.raw() << " {";
				{
					Scope s(cpp);
					cpp << "m_parent = nullptr;";
					if (c.deps.parents.size() > 1)
						cpp << "m_parentType = nullptr;";
				}
				cpp << "}";
				cpp << "";
			}
			if (c.deps.parents.size() == 1) {
				cpp << c.name << "::" << c.name << "(" << customReplacedType(c.deps.parents[0]->name) << "* parent)";
				writeParentPointerFieldInitializers();
				cpp.raw() << " {";
				{
					Scope s(cpp);
					cpp << "assert(parent != nullptr);";
					cpp << "m_parent = parent;";
				}
				cpp << "}";
				cpp << "";
			} else {
				for (const auto& dep : c.deps.parents) {
					const auto rn = customReplacedType(dep->name);
					cpp << c.name << "::" << c.name << "(" << rn << "* parent)";
					writeParentPointerFieldInitializers();
					cpp.raw() << " {";
					{
						Scope s(cpp);
						cpp << "assert(parent != nullptr);";
						cpp << "m_parent = parent;";
						cpp << "m_parentType = &typeid(" << rn << ");";
					}
					cpp << "}";
					cpp << "";
				}
			}
		} else {
			cpp << c.name << "::" << c.name << "()";
			writeParentPointerFieldInitializers();
			cpp.raw() << "{}";
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
		if (includes.hppIncludes.size() > 0)
			hpp << "";

		// namespace
		hpp << "namespace tigl {";
		{
			Scope s(hpp);

			// custom Tigl types declarations
			for (const auto& fwd : includes.hppCustomForwards)
				hpp << "class " << fwd << ";";
			if(includes.hppCustomForwards.size() > 0)
				hpp << "";

			hpp << "namespace generated {";
			{
				Scope s(hpp);

				// forward declarations
				for (const auto& fwd : includes.hppForwards)
					hpp << "class " << fwd << ";";
				if (includes.hppForwards.size() > 0)
					hpp << "";

				// meta information from schema
				hpp << "// This class is used in:";
				for (const auto& c : c.deps.parents)
					hpp << "// " << c->name << "";
				if (c.deps.parents.size() > 0)
					hpp << "";

				hpp << "// generated from " << c.origin->xpath << "";

				// class name and base class
				hpp << "class " << c.name << (c.base.empty() ? "" : " : public " + c.base) << " {";
				hpp << "public:";
				{
					Scope s(hpp);

					// ctor
					writeCtors(hpp, c);

					// dtor
					hpp << "TIGL_EXPORT virtual ~" << c.name << "();";
					hpp << "";

					// parent pointers
					writeParentPointerGetters(hpp, c);

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

					// fields
					writeFields(hpp, c.fields);
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
			if (!s_customTypes.contains(c.name)) {
				hpp << "";
				hpp << "// This type is not customized, export it into tigl namespace";
				hpp << "using generated::" << c.name << ";";
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
		if (includes.cppIncludes.size() > 0)
			cpp << "";

		// namespace
		cpp << "namespace tigl {";
		{
			Scope s(cpp);

			cpp << "namespace generated {";
			{
				Scope s(cpp);

				// ctor
				writeCtorImplementations(cpp, c);

				// dtor
				cpp << c.name << "::~" << c.name << "() {}";
				cpp << "";

				// parent pointers
				writeParentPointerGetterImplementation(cpp, c);

				// io
				writeReadImplementation(cpp, c, c.fields);
				writeWriteImplementation(cpp, c, c.fields);

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
		hpp << "#include <string>";
		if (!c_generateCaseSensitiveStringToEnumConversion)
			hpp << "#include <cctype>";
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
				hpp << "inline std::string " << enumToStringFunc(e) << "(const " << e.name << "& value) {";
				{
					Scope s(hpp);
					hpp << "switch(value) {";
					{
						Scope s(hpp);
						for (const auto& v : e.values)
							hpp << "case " << e.name << "::" << v.cppName << ": return \"" << v.name << "\";";
						hpp << "default: throw std::runtime_error(\"Invalid enum value \\\"\" + std::to_string(static_cast<int>(value)) + \"\\\" for enum type " << e.name << "\");";
					}
					hpp << "}";
				}
				hpp << "}";

				// string to enum function
				hpp << "inline " << e.name << " " << stringToEnumFunc(e) << "(const std::string& value) {";
				{
					Scope s(hpp);
					if (c_generateCaseSensitiveStringToEnumConversion) {
						for (const auto& v : e.values)
							hpp << "if (value == \"" << v.name << "\") return " << e.name << "::" << v.cppName << ";";
					} else {
						auto toLower = [](std::string str) { for (char& c : str) c = std::tolower(c); return str; };
						hpp << "auto toLower = [](std::string str) { for (char& c : str) c = std::tolower(c); return str; };";
						for (const auto& v : e.values)
							hpp << "if (toLower(value) == \"" << toLower(v.name) << "\") return " << e.name << "::" << v.cppName << ";";
					}

					hpp << "throw std::runtime_error(\"Invalid string value \\\"\" + value + \"\\\" for enum type " << e.name << "\");";
				}
				hpp << "}";
			}
			hpp << "}";
		}
		hpp << "}";
		hpp << "";
	}

	namespace {
		// TODO: replace by lambda when C++14 is available
		struct SortAndUnique {
			template <typename T>
			void operator()(T& con) {
				std::sort(std::begin(con), std::end(con));
				con.erase(std::unique(std::begin(con), std::end(con)), std::end(con));
			}
		};
	}

	void Types::buildTypeSystem() {
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
			for (auto& f : c.fields) {
				const auto eit = enums.find(f.typeName);
				if (eit != std::end(enums)) {
					//f.type = &eit->second;
					c.deps.enumChildren.push_back(&eit->second);
					eit->second.deps.parents.push_back(&c);
				} else {
					const auto cit = classes.find(f.typeName);
					if (cit != std::end(classes)) {
						//f.type = &cit->second;
						c.deps.children.push_back(&cit->second);
						cit->second.deps.parents.push_back(&c);
					}
				}
			}
		}

		//auto sortAndUnique = [](auto& con) {
		//	std::sort(std::begin(con), std::end(con));
		//	con.erase(std::unique(std::begin(con), std::end(con)), std::end(con));
		//};
		SortAndUnique sortAndUnique;

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

		// output directory should already have been created my cmake when runtime files were copied
		//boost::filesystem::create_directories(outputLocation);

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
}
