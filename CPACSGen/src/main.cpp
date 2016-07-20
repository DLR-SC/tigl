#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <limits>
#include <algorithm>

#include "NotImplementedException.h"
#include "schemaparser.h"
#include "codegen.h"
#include "Tables.h"

namespace tigl {
	auto makeClassName(std::string name) {
		if (!name.empty()) {
			// capitalize first letter
			name[0] = std::toupper(name[0]);

			// strip Type suffix if exists
			name = stripTypeSuffix(name);

			// prefix CPACS
			name = "CPACS" + name;
		}

		return name;
	}

	std::string resolveType(const SchemaParser& schema, const std::string& name) {
		const auto& types = schema.types();

		// search simple and complex types
		const auto cit = types.find(name);
		if (cit != std::end(types)) {
			const auto p = s_typeSubstitutions.find(name);
			if (p)
				return *p;
			else
				return makeClassName(name);
		}

		// search predefined xml schema types and replace them
		const auto xp = s_xsdTypes.find(name);
		if (xp) {
			const auto p = s_typeSubstitutions.find(name);
			if (p)
				return *p;
			else
				return *xp;
		}

		throw std::runtime_error("Unknown type: " + name);
	}

	auto buildFieldList(const SchemaParser& schema, const ComplexType& type) {
		std::vector<Field> members;

		// attributes
		for (const auto& a : type.attributes) {
			Field m;
			m.origin = &a;
			m.cpacsName = a.name;
			m.typeName = resolveType(schema, a.type);
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
				m.cpacsName = e.name;
				m.typeName = resolveType(schema, e.type);
				m.xmlType = XMLConstruct::Element;
				if (e.minOccurs == 0 && e.maxOccurs == 1)
					m.cardinality = Cardinality::Optional;
				else if (e.minOccurs == 1 && e.maxOccurs == 1)
					m.cardinality = Cardinality::Mandatory;
				else if (e.minOccurs >= 0 && e.maxOccurs > 1)
					m.cardinality = Cardinality::Vector;
				else if (e.minOccurs == 0 && e.maxOccurs == 0) {
					std::cerr << "Warning: Element " + e.name + " with type " + e.type + " was omitted as minOccurs and maxOccurs are both zero" << std::endl;
					return; // skip this type
				} else
					throw std::runtime_error("Invalid cardinalities, min: " + std::to_string(e.minOccurs) + ", max: " + std::to_string(e.maxOccurs));
				members.push_back(m);
			}

			void operator()(const Choice& c) const {
				unsigned int choiceIndex = 1;
				std::vector<Field> allChoiceMembers;
				for (const auto& v : c.elements) {
					// collect members of one choice
					std::vector<Field> choiceMembers;
					v.visit(ContentVisitor(schema, choiceMembers));

					// make all optional
					for (auto& f : choiceMembers)
						if (f.cardinality == Cardinality::Mandatory)
							f.cardinality = Cardinality::Optional;

					// give custom names
					for (auto& f : choiceMembers)
						f.customFieldName = f.cpacsName + "_choice" + std::to_string(choiceIndex);

					// copy to output
					std::copy(std::begin(choiceMembers), std::end(choiceMembers), std::back_inserter(allChoiceMembers));

					choiceIndex++;
				}

				// consistency check, two types with the same name but different types or cardinality are problematic
				for (std::size_t i = 0; i < allChoiceMembers.size(); i++) {
					const auto& f1 = allChoiceMembers[i];
					for (std::size_t j = i + 1; j < allChoiceMembers.size(); j++) {
						const auto& f2 = allChoiceMembers[j];
						if (f1.cpacsName == f2.cpacsName && (f1.cardinality != f2.cardinality || f1.typeName != f2.typeName)) {
							std::cerr << "Elements with same name but different cardinality or type inside choice" << std::endl;
							for (const auto& f : { f1, f2 })
								std::cerr << f.cpacsName << " " << toString(f.cardinality) << " " << f.typeName << std::endl;
						}
					}
				}

				// copy to output
				std::copy(std::begin(allChoiceMembers), std::end(allChoiceMembers), std::back_inserter(members));
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
				m.cpacsName = "";
				m.customFieldName = "simpleContent";
				m.cardinality = Cardinality::Mandatory;
				m.typeName = resolveType(schema, g.type);
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

	void collapseEnums(Types& types) {

		// convert enum map to vector for easier processing
		std::vector<Enum> enums;
		enums.reserve(types.enums.size());
		for (const auto& p : types.enums)
			enums.push_back(p.second);

		for (std::size_t i = 0; i < enums.size(); i++) {
			auto& e1 = enums[i];
			for (std::size_t j = i + 1; j < enums.size(); j++) {
				auto& e2 = enums[j];

				auto stripNumber = [](std::string name) {
					while (!name.empty() && std::isdigit(name.back()))
						name.pop_back();
					return name;
				};

				if (e1.values.size() == e2.values.size() &&
					stripNumber(e1.name) == stripNumber(e2.name) &&
					std::equal(std::begin(e1.values), std::end(e1.values), std::begin(e2.values))) {
					// choose new name
					const std::string newName = std::min(e1.name, e2.name);

					if (newName == e2.name) {
						// the algorithm is implemented to delete e2, swap e1 and e2 therefore
						using std::swap;
						swap(e1, e2);
					}

					std::cout << "Collapsed enums " << e1.name << " and " << e2.name << " to " << newName << std::endl;
					//for (auto& e : { e1, e2 }) {
					//	std::cout << "Enum " << e.name << std::endl;
					//	for (const auto& v : e.values)
					//		std::cout << "\t" << v.name << " " << std::endl;
					//}

					// remove e2 from other types
					for (auto& c : e2.deps.parents) {
						//std::cout << "\t\tparent: " << c->name << std::endl;
						auto& ec = c->deps.enumChildren;

						// delete e2 from c's dependencies
						const auto e2it = std::find_if(std::begin(ec), std::end(ec), [&](const Enum* e) {
							return e->name == e2.name;
						});
						if (e2it != std::end(ec))
							ec.erase(e2it);

						// add e1 to c's dependencies
						const auto e1it = std::find_if(std::begin(ec), std::end(ec), [&](const Enum* e) {
							return e->name == e1.name;
						});
						if (e1it == std::end(ec))
							ec.push_back(&e1);

						// add c to e1's parents
						const auto e1pit = std::find(std::begin(e1.deps.parents), std::end(e1.deps.parents), c);
						if (e1pit == std::end(e1.deps.parents))
							e1.deps.parents.push_back(c);

						// change type of c's fields which are of type e2
						for (auto& f : c->fields) {
							if (f.typeName == e2.name)
								f.typeName = newName;
						}
					}

					// remove e2
					enums.erase(std::begin(enums) + j);

					// rename e1
					e1.name = newName;
				}
			}
		}

		// fill enum back again from vector
		types.enums.clear();
		types.enums.reserve(enums.size());
		for (auto& e : enums)
			types.enums[e.name] = std::move(e);
	}

	const std::string cpacsLocation = "../schema/cpacs_schema.xsd"; // TODO: inject this path by cmake or pass by command line
	const std::vector<std::string> runtimeFiles = {
		"../src/IOHelper.h",
		"../src/IOHelper.cpp",
		"../src/optional.hpp"
	};

	void run(const std::string& outputDirectory) {
		// read types and elements
		std::cout << "Parsing " << cpacsLocation << std::endl;
		SchemaParser schema(cpacsLocation);

		// generate type system from schema
		Types types;
		for (const auto& p : schema.types()) {
			const auto& type = p.second;

			struct TypeVisitor {
				TypeVisitor(SchemaParser& schema, Types& types)
					: schema(schema), types(types) {}

				void operator()(const ComplexType& type) {
					Class c;
					c.origin = &type;
					c.name = makeClassName(type.name);
					c.fields = buildFieldList(schema, type);
					if (!type.base.empty()) {
						c.base = resolveType(schema, type.base);

						// make base a field if fundamental type
						if (s_fundamentalTypes.contains(c.base)) {
							Field f;
							f.cpacsName = "";
							f.customFieldName = "base";
							f.cardinality = Cardinality::Mandatory;
							f.typeName = c.base;
							f.xmlType = XMLConstruct::FundamentalTypeBase;

							c.fields.insert(std::begin(c.fields), f);
							c.base.clear();
						}
					}

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

		// build dependencies
		types.buildTypeSystem();

		// apply a few reductions and cleanups
		collapseEnums(types);

		// generate code
		std::cout << "Generating classes" << std::endl;
		CodeGen codegen(outputDirectory, types);

		// copy runtime files used by generated code
		for (const auto& filename : runtimeFiles) {
			auto src = boost::filesystem::path(filename);
			auto dst = boost::filesystem::path(outputDirectory) / src.filename();
			boost::filesystem::copy_file(src, dst, boost::filesystem::copy_option::overwrite_if_exists);
		}
	}
}

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
		tigl::run(outputDirectory);
	} catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}