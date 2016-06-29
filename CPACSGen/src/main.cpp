#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <limits>

#include "TypeSubstitutionTable.h"
#include "CustomTypesTable.h"
#include "NotImplementedException.h"
#include "schemaparser.h"
#include "codegen.h"

std::unordered_map<std::string, std::string> xsdTypes = {
	{ "xsd:byte",         "int8_t"      },
	{ "xsd:unsignedByte", "uint8_t"     },
	{ "xsd:short",        "int16_t"     },
	{ "xsd:unsignedShort","uint16_t"    },
	{ "xsd:int",          "int32_t"     },
	{ "xsd:unsignedInt",  "uint32_t"    },
	{ "xsd:long"    ,     "int64_t"     },
	{ "xsd:unsignedLong", "uint64_t"    },
	{ "xsd:integer",      "int"         },
	{ "xsd:boolean",      "bool"        },
	{ "xsd:float",        "float"       },
	{ "xsd:double",       "double"      },
	{" xsd:decimal",      "double"      }, // TODO: implement custom type?
	{ "xsd:dateTime",     "time_t"      },
	{ "xsd:string",       "std::string" },
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

std::string resolveType(const SchemaParser& schema, const std::string& name) {
	const auto& types = schema.types();

	// search simple and complex types
	const auto cit = types.find(name);
	if (cit != std::end(types)) {
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
		m.origin = &a;
		m.name = a.name;
		m.type = resolveType(schema, a.type);
		m.xmlType = XMLConstruct::Attribute;
		if (a.optional)
			m.cardinality = Cardinality::Optional;
		else
			m.cardinality = Cardinality::Mandatory;
		members.push_back(m);
	}

	// elements
	struct ContentVisitor : public boost::static_visitor<> {
		ContentVisitor(const SchemaParser& schema, std::vector<Field>& members)
			: schema(schema), members(members) {}

		void operator()(const Element& e) const {
			Field m;
			m.origin = &e;
			m.name = e.name;
			m.type = resolveType(schema, e.type);
			m.xmlType = XMLConstruct::Element;
			if (e.minOccurs == 0 && e.maxOccurs == 1)
				m.cardinality = Cardinality::Optional;
			else if (e.minOccurs == 1 && e.maxOccurs == 1)
				m.cardinality = Cardinality::Mandatory;
			else if (e.minOccurs >= 0 && e.maxOccurs > 1)
				m.cardinality = Cardinality::Vector;
			else if (e.minOccurs == 0 && e.maxOccurs == 0) {
				std::cerr << "Element " + e.name + " with type " + e.type + " was omitted as minOccurs and maxOccurs are both zero" << std::endl;
				return; // skip this type
			} else
				throw std::runtime_error("Invalid cardinalities, min: " + std::to_string(e.minOccurs) + ", max: " + std::to_string(e.maxOccurs));
			members.push_back(m);
		}

		void operator()(const Choice& c) const {
			for (const auto& v : c.elements)
				v.visit(ContentVisitor(schema, members));
		}

		void operator()(const Sequence& s) const {
			for (const auto& v : s.elements)
				v.visit(ContentVisitor(schema, members));
		}

		void operator()(const All& a) const {
			for (const auto& e : a.elements)
				operator()(e);
		}

		void operator()(const Any& a) const {
			throw NotImplementedException("Generating fields for any is not implemented");
		}

		void operator()(const Group& g) const {
			throw NotImplementedException("Generating fields for group is not implemented");
		}

		void operator()(const SimpleContent& g) const {
			Field m;
			m.origin = &g;
			m.name = "simpleContent";
			m.cardinality = Cardinality::Mandatory;
			m.type = resolveType(schema, g.type);
			m.xmlType = XMLConstruct::SimpleContent;
			members.push_back(m);
		}

	private:
		const SchemaParser& schema;
		std::vector<Field>& members;
	};

	type.content.visit(ContentVisitor(schema, members));

	return members;
}

const std::string cpacsLocation = "../schema/cpacs_schema.xsd"; // TODO: inject this path by cmake or pass by command line
const std::vector<std::string> copyFiles = {
	"../src/IOHelper.h",
	"../src/IOHelper.cpp",
	"../src/optional.hpp"
};

int main(int argc, char* argv[]) {

	// parse command line arguments
	std::string outputDirectory;
	if (argc > 2) {
		std::cerr << "Usage: CPACSGen outputDirectory" << std::endl;
		return -1;
	} else if (argc == 2) {
		outputDirectory = argv[1];
	} else if (argc == 1) {
		outputDirectory = "generated";
	}

	try {
		// read types and elements
		std::cout << "Parsing " << cpacsLocation << std::endl;
		SchemaParser schema(cpacsLocation);

		// generate classes from complex types
		Types types;
		for(const auto& p : schema.types()) {
			const auto& type = p.second;

			struct TypeVisitor {
				TypeVisitor(SchemaParser& schema, Types& types)
					: schema(schema), types(types) {}

				void operator()(const ComplexType& type) {
					Class c;
					c.origin = &type;
					c.name = makeClassName(type.name);
					if (!type.base.empty())
						c.base = resolveType(schema, type.base);
					c.fields = buildFieldList(schema, type);
					types.classes[c.name] = c;
				}

				void operator()(const SimpleType& type) {
					if (type.restrictionValues.size() > 0) {
						// create enum
						Enum e;
						e.origin = &type;
						e.name = makeClassName(type.name);
						for (const auto& v : type.restrictionValues)
							e.values.push_back(EnumValue(v));
						types.enums[e.name] = e;
					} else
						throw NotImplementedException("Simple times which are not enums are not implemented");
				}

			private:
				SchemaParser& schema;
				Types& types;
			};

			type.visit(TypeVisitor(schema, types));
		};

		// generate code
		std::cout << "Generating classes" << std::endl;
		CodeGen codegen(outputDirectory, types);

		// copy IOHelper ("runtime")
		for (const auto& filename : copyFiles) {
			auto src = boost::filesystem::path(filename);
			auto dst = boost::filesystem::path(outputDirectory) / src.filename();
			boost::filesystem::copy_file(src, dst, boost::filesystem::copy_option::overwrite_if_exists);
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}