#include "TypeSystem.h"

#include <iostream>
#include <cctype>

#include "NotImplementedException.h"
#include "Tables.h"
#include "SchemaParser.h"

namespace tigl {
    namespace {
        auto makeClassName(std::string name) -> std::string {
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

        auto resolveType(const SchemaParser& schema, const std::string& name, Tables& tables) -> std::string {
            const auto& types = schema.types();

            // search simple and complex types
            const auto cit = types.find(name);
            if (cit != std::end(types)) {
                const auto p = tables.m_typeSubstitutions.find(name);
                if (p)
                    return *p;
                else
                    return makeClassName(name);
            }

            // search predefined xml schema types and replace them
            const auto p = tables.m_typeSubstitutions.find(name);
            if (p)
                return *p;

            throw std::runtime_error("Unknown type: " + name);
        }

        auto buildFieldList(const SchemaParser& schema, const ComplexType& type, Tables& tables) -> std::vector<Field> {
            std::vector<Field> members;

            // attributes
            for (const auto& a : type.attributes) {
                Field m;
                m.origin = &a;
                m.cpacsName = a.name;
                m.typeName = resolveType(schema, a.type, tables);
                m.xmlType = XMLConstruct::Attribute;
                if (a.optional)
                    m.cardinality = Cardinality::Optional;
                else
                    m.cardinality = Cardinality::Mandatory;
                members.push_back(m);
            }

            // elements
            struct ContentVisitor : public boost::static_visitor<> {
                ContentVisitor(const SchemaParser& schema, std::vector<Field>& members, Tables& tables)
                    : schema(schema), members(members), tables(tables) {}

                void operator()(const Element& e) const {
                    Field m;
                    m.origin = &e;
                    m.cpacsName = e.name;
                    m.typeName = resolveType(schema, e.type, tables);
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
                        v.visit(ContentVisitor(schema, choiceMembers, tables));

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
                        v.visit(ContentVisitor(schema, members, tables));
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
                    m.typeName = resolveType(schema, g.type, tables);
                    m.xmlType = XMLConstruct::SimpleContent;
                    members.push_back(m);
                }

            private:
                const SchemaParser& schema;
                std::vector<Field>& members;
                Tables& tables;
            };

            type.content.visit(ContentVisitor(schema, members, tables));

            return members;
        }
    }

    TypeSystem::TypeSystem(SchemaParser& schema, Tables& tables) {
        for (const auto& p : schema.types()) {
            const auto& type = p.second;

            struct TypeVisitor {
                TypeVisitor(SchemaParser& schema, TypeSystem& types, Tables& tables)
                    : schema(schema), types(types), tables(tables) {}

                void operator()(const ComplexType& type) {
                    Class c;
                    c.origin = &type;
                    c.name = makeClassName(type.name);
                    c.fields = buildFieldList(schema, type, tables);
                    if (!type.base.empty()) {
                        c.base = resolveType(schema, type.base, tables);

                        // make base a field if fundamental type
                        if (tables.m_fundamentalTypes.contains(c.base)) {
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
                TypeSystem& types;
                Tables& tables;
            };

            type.visit(TypeVisitor(schema, *this, tables));
        };
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

    void TypeSystem::buildDependencies() {
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

    void TypeSystem::collapseEnums() {
        // convert enum map to vector for easier processing
        std::vector<Enum> enums;
        enums.reserve(this->enums.size());
        for (const auto& p : this->enums)
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
        this->enums.clear();
        this->enums.reserve(enums.size());
        for (auto& e : enums)
            this->enums[e.name] = std::move(e);
    }
}
