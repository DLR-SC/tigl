#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <limits>

#include "TypeSubstitutionTable.h"
#include "schemaparser.h"
#include "codegen.h"

std::unordered_map<std::string, std::string> xsdTypes = {
	{ "xsd:boolean", "bool" },
	{ "xsd:double", "double" },
	{ "xsd:string", "std::string" }
};

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

auto resolveType(const SchemaParser& schema, const std::string& name) {
	const auto& complexTypes = schema.complexTypes();
	const auto& simpleTypes = schema.simpleTypes();

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

	throw std::runtime_error("Unknown type: " + name);
}

auto buildFieldList(const SchemaParser& schema, const ComplexType& type) {
	std::vector<Field> members;

	// attributes
	for (const auto& a : type.attributes) {
		Field m;
		m.name = a.name;
		m.type = resolveType(schema, a.type);
		m.attribute = true;
		if (a.optional)
			m.cardinality = Cardinality::ZeroOrOne;
		else
			m.cardinality = Cardinality::One;
		members.push_back(m);
	}

	// elements
	struct Visitor : public boost::static_visitor<> {
		Visitor(const SchemaParser& schema, std::vector<Field>& members)
			: schema(schema), members(members) {}

		void operator()(const Element& e) const {
			Field m;
			m.name = e.name;
			m.type = resolveType(schema, e.type);
			m.attribute = false;
			if (e.minOccurs == 0 && e.maxOccurs == 1)
				m.cardinality = Cardinality::ZeroOrOne;
			else if (e.minOccurs == 0 && e.maxOccurs > 1)
				m.cardinality = Cardinality::ZeroOrMany;
			else if (e.minOccurs == 1 && e.maxOccurs == 1)
				m.cardinality = Cardinality::One;
			else if (e.minOccurs >= 1 && e.maxOccurs > 1)
				m.cardinality = Cardinality::Many;
			else if (e.minOccurs == 0 && e.maxOccurs == 0) {
				std::cerr << "Element " + e.name + " with type " + e.type + " was omitted as minOccurs and maxOccurs are both zero" << std::endl;
				return; // skip this type
			} else
				throw std::runtime_error("Invalid cardinalities, min: " + std::to_string(e.minOccurs) + ", max: " + std::to_string(e.maxOccurs));
			members.push_back(m);
		}

		void operator()(const Choice& c) const {
			for (const auto& v : c.elements)
				v.visit(Visitor(schema, members));
		}

		void operator()(const Sequence& s) const {
			for (const auto& v : s.elements)
				v.visit(Visitor(schema, members));
		}

		void operator()(const All& a) const {
			for (const auto& e : a.elements)
				operator()(e);
		}

	private:
		const SchemaParser& schema;
		std::vector<Field>& members;
	};

	type.elements.visit(Visitor(schema, members));

	return members;
}

const std::string cpacsLocation = "../schema/cpacs_schema.xsd"; // TODO: inject this path by cmake
const std::vector<std::string> copyFiles = {
	"../src/IOHelper.h",
	"../src/IOHelper.cpp"
};
const std::string outputLocation = "generated";

int main() {
	try {
		// read types and elements
		std::cout << "Parsing " << cpacsLocation << std::endl;
		SchemaParser schema(cpacsLocation);
		const auto& complexTypes = schema.complexTypes();
		const auto& simpleTypes = schema.simpleTypes();

		Types types;

		// generate classes from complex types
		for(const auto& p : complexTypes) {
			Class c;
			const auto& type = p.second;
			c.name = makeClassName(type.name);
			c.base = makeClassName(type.base);
			c.fields = buildFieldList(schema, type);
			types.classes[c.name] = c;
		};

		// generate enums from simple types
		for (const auto& p : simpleTypes) {
			const auto& s = p.second;
			if (s.restrictionValues.size() > 0) {
				// create enum
				Enum e;
				e.name = makeClassName(s.name);
				for (const auto& v : s.restrictionValues)
					e.values.push_back(EnumValue(v));
				types.enums[e.name] = e;
			} else
				throw NotImplementedException("Simple times which are not enums are not implemented");
		}
		
		// generate code
		std::cout << "Generating classes" << std::endl;
		generateCode(outputLocation, types);

		// copy IOHelper
		for (const auto& filename : copyFiles) {
			auto src = boost::filesystem::path(filename);
			auto dst = boost::filesystem::path(outputLocation) / src.filename();
			boost::filesystem::copy_file(src, dst, boost::filesystem::copy_option::overwrite_if_exists);
		}
		
	} catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}