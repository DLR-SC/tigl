#include <iostream>

#include "NotImplementedException.h"
#include "schemaparser.h"

SchemaParser::SchemaParser(const std::string& cpacsLocation)
	: document(cpacsLocation) {
	document.forEachChild("/schema", "simpleType", [&](auto xpath) {
		readSimpleType(xpath);
	});

	document.forEachChild("/schema", "complexType", [&](auto xpath) {
		readComplexType(xpath);
	});

	document.forEachChild("/schema", "element", [&](auto xpath) {
		readElement(xpath);
	});
}

Group SchemaParser::readGroup(const std::string& xpath, const std::string& containingTypeName) {
	throw NotImplementedException("XSD group is not implemented");
	return Group();
}

All SchemaParser::readAll(const std::string& xpath, const std::string& containingTypeName) {
	// <all
	// id = ID
	// maxOccurs = 1
	// minOccurs = 0 | 1
	// any attributes
	// >
	// (annotation? , element*)
	// </all>
	All all;
	all.xpath = xpath;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		all.elements.push_back(readElement(xpath, containingTypeName));
	});
	return all;
}

Choice SchemaParser::readChoice(const std::string& xpath, const std::string& containingTypeName) {
	// <choice
	// id=ID
	// maxOccurs=nonNegativeInteger|unbounded
	// minOccurs=nonNegativeInteger
	// any attributes
	// >
	// (annotation?,(element|group|choice|sequence|any)*)
	// </choice>
	Choice ch;
	ch.xpath;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		ch.elements.push_back(readElement(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "group", [&](auto xpath) {
		ch.elements.push_back(readGroup(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "choice", [&](auto xpath) {
		ch.elements.push_back(readChoice(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "sequence", [&](auto xpath) {
		ch.elements.push_back(readSequence(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "any", [&](auto xpath) {
		ch.elements.push_back(readAny(xpath, containingTypeName));
	});
	return ch;
}

Sequence SchemaParser::readSequence(const std::string& xpath, const std::string& containingTypeName) {
	// <sequence
	// id=ID
	// maxOccurs=nonNegativeInteger|unbounded
	// minOccurs=nonNegativeInteger
	// any attributes
	// >
	// (annotation?,(element|group|choice|sequence|any)*)
	// </sequence>
	Sequence seq;
	seq.xpath = xpath;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		seq.elements.push_back(readElement(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "group", [&](auto xpath) {
		seq.elements.push_back(readGroup(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "choice", [&](auto xpath) {
		seq.elements.push_back(readChoice(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "sequence", [&](auto xpath) {
		seq.elements.push_back(readSequence(xpath, containingTypeName));
	});
	document.forEachChild(xpath, "any", [&](auto xpath) {
		seq.elements.push_back(readAny(xpath, containingTypeName));
	});
	return seq;
}

Any SchemaParser::readAny(const std::string& xpath, const std::string& containingTypeName) {
	// <any
	// id=ID
	// maxOccurs=nonNegativeInteger|unbounded
	// minOccurs=nonNegativeInteger
	// namespace=namespace
	// processContents=lax|skip|strict
	// any attributes
	// >
	// (annotation?)
	// </any>
	throw NotImplementedException("XSD any is not implemented");
	return Any();
}

void SchemaParser::readExtension(const std::string& xpath, ComplexType& type) {
	type.base = document.textAttribute(xpath, "base");
	readComplexTypeElementConfiguration(xpath, type);
}

void SchemaParser::readSimpleContent(const std::string& xpath, ComplexType& type) {
	// <simpleContent
	// id=ID
	// any attributes
	// >
	// (annotation?,(restriction|extension))
	// </simpleContent>

	if (document.checkElement(xpath, "restriction")) {
		if (document.checkElement(xpath + "/restriction/enumeration")) {
			// generating an additional type for this enum
			SimpleType stype;
			stype.xpath = xpath;
			stype.name = stripTypeSuffix(type.name) + "_SimpleContentType";
			readRestriction(xpath + "/restriction", stype);
			m_types[stype.name] = stype;

			SimpleContent sc;
			sc.xpath = xpath;
			sc.type = stype.name;
			type.content = sc;

		} else {
			// we ignore other kinds of restrictions as those are hard to support
			std::cerr << "Warning: restricted simpleContent is not an enum: " << type.name << std::endl;
		}
	} else if (document.checkElement(xpath, "extension")) {
		// we simplify this case be creating a field for the value of the simpleContent
		SimpleContent sc;
		sc.xpath = xpath;
		sc.type = document.textAttribute(xpath + "/extension", "base");
		type.content = sc;
	}
}

void SchemaParser::readComplexContent(const std::string& xpath, ComplexType& type) {
	// <complexContent
	// id=ID
	// mixed=true|false
	// any attributes
	// >
	// (annotation?,(restriction|extension))
	// </complexContent>

	if (document.checkElement(xpath, "restriction"))
		throw NotImplementedException("XSD complexType complexContent restriction is not implemented");
	else if (document.checkElement(xpath, "extension"))
		readExtension(xpath + "/extension", type);
}

void SchemaParser::readComplexTypeElementConfiguration(const std::string& xpath, ComplexType& type) {
	     if (document.checkElement(xpath, "all"))      type.content = readAll     (xpath + "/all",      stripTypeSuffix(type.name));
	else if (document.checkElement(xpath, "sequence")) type.content = readSequence(xpath + "/sequence", stripTypeSuffix(type.name));
	else if (document.checkElement(xpath, "choice"))   type.content = readChoice  (xpath + "/choice",   stripTypeSuffix(type.name));
	else if (document.checkElement(xpath, "group"))    throw NotImplementedException("XSD complexType group is not implemented");
	else if (document.checkElement(xpath, "any"))      throw NotImplementedException("XSD complexType any is not implemented");

	     if (document.checkElement(xpath, "complexContent")) readComplexContent(xpath + "/complexContent", type);
	else if (document.checkElement(xpath, "simpleContent" )) readSimpleContent (xpath + "/simpleContent",  type);
}

Attribute SchemaParser::readAttribute(const std::string& xpath, const std::string& containingTypeName) {
	Attribute att;
	att.xpath = xpath;
	att.name = document.textAttribute(xpath, "name");

	if (document.checkAttribute(xpath, "type"))
		// referencing other type
		att.type = document.textAttribute(xpath, "type");
	else
		// type defined inline
		att.type = readInlineType(xpath, containingTypeName + "_" + att.name);

	if (document.checkAttribute(xpath, "use")) {
		const auto use = document.textAttribute(xpath, "use");
		if (use == "optional")
			att.optional = true;
		else if (use == "required")
			att.optional = false;
		else
			throw std::runtime_error("Invalid value for optional attribute at xpath: " + xpath);
	} else {
		att.optional = false;
	}

	if (document.checkAttribute(xpath, "default"))
		att.default = document.textAttribute(xpath, "default");
	if (document.checkAttribute(xpath, "fixed"))
		att.fixed = document.textAttribute(xpath, "fixed");

	return att;
}

std::string SchemaParser::readComplexType(const std::string& xpath, const std::string& nameHint) {
// <complexType
// id=ID
// name=NCName
// abstract=true|false
// mixed=true|false
// block=(#all|list of (extension|restriction))
// final=(#all|list of (extension|restriction))
// any attributes
// >
// (annotation?,(simpleContent|complexContent|((group|all|choice|sequence)?,((attribute|attributeGroup)*,anyAttribute?))))
// </complexType>

	// read or generate type name
	const std::string name = [&] {
		if (document.checkAttribute(xpath, "name"))
			return document.textAttribute(xpath, "name");
		else
			return generateUniqueTypeName(nameHint);
	}();

	if (m_types.find(name) != std::end(m_types))
		throw std::runtime_error("Type with name " + name + " already exists");

	ComplexType type;
	type.xpath = xpath;
	type.name = name;

	if (document.checkAttribute(xpath, "id"))
		throw NotImplementedException("XSD complextype id is not implemented");
	if (document.checkAttribute(xpath, "abstract"))
		throw NotImplementedException("XSD complextype abstract is not implemented");
	if (document.checkAttribute(xpath, "mixed"))
		throw NotImplementedException("XSD complextype mixed is not implemented");
	if (document.checkAttribute(xpath, "block"))
		throw NotImplementedException("XSD complextype block is not implemented");
	if (document.checkAttribute(xpath, "final"))
		throw NotImplementedException("XSD complextype final is not implemented");

	// read element configuration
	readComplexTypeElementConfiguration(xpath, type);

	// read attributes
	for (auto path : {
		xpath,
		xpath + "/complexContent/restriction",
		xpath + "/complexContent/extension",
		xpath + "/simpleContent/restriction",
		xpath + "/simpleContent/extension",
	}) {
		if (document.checkElement(path)) {
			document.forEachChild(path, "attribute", [&](auto xpath) {
				type.attributes.push_back(readAttribute(xpath, name));
			});
			if (document.checkElement(xpath, "attributeGroup")) {
				throw NotImplementedException("XSD complexType attributeGroup is not implemented");
			}
		}
	}

	// add
	m_types[name] = type;

	return name;
}

void SchemaParser::readRestriction(const std::string& xpath, SimpleType& type) {
	type.base = document.textAttribute(xpath, "base");

	document.forEachChild(xpath, "enumeration", [&](auto expath) {
		const auto enumValue = document.textAttribute(expath, "value");
		type.restrictionValues.push_back(enumValue);
	});

	if (type.restrictionValues.size() == 0)
		std::cerr << "XSD restriction without enumeration is not implemented: " << xpath << std::endl;
		//throw NotImplementedException("XSD restriction without enumeration is not implemented");
}

std::string SchemaParser::readSimpleType(const std::string& xpath, const std::string& nameHint) {
	// <simpleType
	// id=ID
	// name=NCName
	// any attributes
	// >
	// (annotation?,(restriction|list|union))
	// </simpleType>

	// read or generate type name
	const std::string name = [&] {
		if (document.checkAttribute(xpath, "name"))
			return document.textAttribute(xpath, "name");
		else {
			return generateUniqueTypeName(nameHint);
		}
	}();

	if (m_types.find(name) != std::end(m_types))
		throw std::runtime_error("Type with name " + name + " already exists");

	SimpleType type;
	type.xpath = xpath;
	type.name = name;

	if (document.checkAttribute(xpath, "id"))
		throw NotImplementedException("XSD complextype id is not implemented");

	if (document.checkElement(xpath, "restriction"))
		readRestriction(xpath + "/restriction", type);
	else if (document.checkElement(xpath, "list"))
		throw NotImplementedException("XSD simpleType list is not implemented");
	else if (document.checkElement(xpath, "union"))
		throw NotImplementedException("XSD simpleType union is not implemented");

	// add
	m_types[name] = type;

	return name;
}

std::string SchemaParser::readInlineType(const std::string& xpath, const std::string& nameHint) {
	     if (document.checkElement(xpath, "complexType")) return readComplexType(xpath + "/complexType", nameHint);
	else if (document.checkElement(xpath, "simpleType" )) return readSimpleType (xpath + "/simpleType",  nameHint);
	else throw std::runtime_error("Unexpected type or no type at xpath: " + xpath);
}

Element SchemaParser::readElement(const std::string& xpath, const std::string& containingTypeName) {
	Element element;
	element.xpath = xpath;
	element.name = document.textAttribute(xpath, "name");

	// minOccurs
	if (!document.checkAttribute(xpath, "minOccurs"))
		element.minOccurs = 1;
	else {
		const auto minOccurs = document.textAttribute(xpath, "minOccurs");
		element.minOccurs = std::stoi(minOccurs);
	}

	// maxOccurs
	if (!document.checkAttribute(xpath, "maxOccurs"))
		element.maxOccurs = 1;
	else {
		const auto maxOccurs = document.textAttribute(xpath, "maxOccurs");
		if (maxOccurs == "unbounded")
			element.maxOccurs = std::numeric_limits<decltype(element.maxOccurs)>::max();
		else
			element.maxOccurs = std::stoi(maxOccurs);
	}

	if (document.checkAttribute(xpath, "type"))
		// referencing other type
		element.type = document.textAttribute(xpath, "type");
	else
		element.type = readInlineType(xpath, containingTypeName + "_" + element.name);

	assert(!element.type.empty());

	return element;
}

std::string SchemaParser::generateUniqueTypeName(const std::string& newNameSuggestion) {
	auto toString = [](unsigned int id) {
		if (id == 0)
			return std::string();
		else
			return std::to_string(id);
	};

	unsigned int id = 0;
	while (m_types.find(newNameSuggestion + "Type" + toString(id)) != std::end(m_types))
		id++;
	const auto n = newNameSuggestion + "Type" + toString(id);
	return n;
}

std::string stripTypeSuffix(std::string name) {
	if (name.size() > 4 && name.compare(name.size() - 4, 4, "Type") == 0)
		name.erase(std::end(name) - 4, std::end(name));
	return name;
}
