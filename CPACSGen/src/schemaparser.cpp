#include <iostream>

#include "schemaparser.h"

static const std::string inlineTypePrefix = "_inline_";

NotImplementedException::NotImplementedException(const std::string& msg)
	: m_msg(msg) {}

const char* NotImplementedException::what() const {
	return m_msg.c_str();
}

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

All SchemaParser::readAll(const std::string& xpath) {
	All all;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		all.elements.push_back(readElement(xpath));
	});
	return all;
}

Sequence SchemaParser::readSequence(const std::string& xpath) {
	Sequence seq;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		seq.elements.push_back(readElement(xpath));
	});
	return seq;
}

Choice SchemaParser::readChoice(const std::string& xpath) {
	Choice ch;
	document.forEachChild(xpath, "element", [&](auto xpath) {
		ch.elements.push_back(readElement(xpath));
	});
	return ch;
}

void SchemaParser::readComplexTypeElementConfiguration(const std::string& xpath, ComplexType& type) {
	if (document.checkElement(xpath + "/choice")) {
		document.forEachChild(xpath + "/choice", "element", [&](auto xpath) {
			readElement(xpath);
		});
	} else if (document.checkElement(xpath + "/all")) {
		type.elements = readAll(xpath + "/all");
	} else if (document.checkElement(xpath + "/sequence")) {
		type.elements = readSequence(xpath + "/sequence");
	} else if (document.checkElement(xpath + "/choice")) {
		type.elements = readChoice(xpath + "/choice");
	} else if (document.checkElement(xpath + "/complexContent")) {
		if (document.checkElement(xpath + "/complexContent/restriction")) {
			throw NotImplementedException("XSD complextype complexcontent restriction is not implemented");
		} else if (document.checkElement(xpath + "/complexContent/extension")) {
			type.base = document.textAttribute(xpath + "/complexContent/extension", "base");
			readComplexTypeElementConfiguration(xpath + "/complexContent/extension", type);
		}
	} else if (document.checkElement(xpath + "/simpleContent")) {

	}
}

Attribute SchemaParser::readAttribute(const std::string& xpath) {
	Attribute att;
	att.name = document.textAttribute(xpath, "name");

	if (document.checkAttribute(xpath, "type"))
		// referencing other type
		att.type = document.textAttribute(xpath, "type");
	else {
		// type defined inline
		const auto name = readType(xpath);
		att.type = renameType(name, att.name + "Type");
	}

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

std::string SchemaParser::readComplexType(const std::string& xpath) {
	// read or generate type name
	const std::string name = [&] {
		if (document.checkAttribute(xpath, "name"))
			return document.textAttribute(xpath, "name");
		else {
			static auto id = 0;
			return inlineTypePrefix + "complexType" + std::to_string(id++);
		}
	}();

	ComplexType& type = m_complexTypes[name];
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
				type.attributes.push_back(readAttribute(xpath));
			});
		}
	}

	return name;
}

std::string SchemaParser::readSimpleType(const std::string& xpath) {
	// read or generate type name
	const std::string name = [&] {
		if (document.checkAttribute(xpath, "type"))
			return document.textAttribute(xpath, "name");
		else {
			static auto id = 0;
			return inlineTypePrefix + "simpleType" + std::to_string(id++);
		}
	}();

	SimpleType& type = m_simpleTypes[name];
	type.name = name;

	if (document.checkElement(xpath + "/restriction")) {
		type.base = document.textAttribute(xpath + "/restriction", "base");

		document.forEachChild(xpath + "/restriction", "enumeration", [&](auto expath) {
			const auto enumValue = document.textAttribute(expath, "value");
			type.restrictionValues.push_back(enumValue);
		});
	}

	if (document.checkAttribute(xpath, "id"))
		throw NotImplementedException("XSD complextype id is not implemented");

	return name;
}

std::string SchemaParser::readType(const std::string& xpath) {
	if (document.checkElement(xpath + "/complexType"))
		return readComplexType(xpath + "/complexType");
	else if (document.checkElement(xpath + "/simpleType"))
		return readSimpleType(xpath + "/simpleType");
	else
		std::cerr << "Element at xpath " << xpath << " has no type" << std::endl;
		// this happens if no type is specified for an element
		//return "__AnyContentType"; // TODO: provide this type
	throw std::runtime_error("Unexpected type or no type at xpath: " + xpath);
}

Element SchemaParser::readElement(const std::string& xpath) {
	Element element;
	element.name = document.textAttribute(xpath, "name");

	auto parseOccurs = [&](const std::string& att) {
		if (!document.checkAttribute(xpath, att))
			return -1;
		const auto value = document.textAttribute(xpath, att);
		if (value == "unbounded")
			return -1;
		else
			return std::stoi(value);
	};

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
	else {
		// type defined inline
		const auto name = readType(xpath);
		element.type = renameType(name, element.name + "Type");
	}

	assert(!element.type.empty());

	return element;
}

std::string SchemaParser::renameType(const std::string& oldName, std::string newNameSuggestion) {
	auto renameType = [&](auto& types) {
		const auto it = types.find(oldName);
		if (it != std::end(types)) {
			auto type = it->second;
			types.erase(it);

			// try to find unique name based on suggestion
			while (types.find(newNameSuggestion) != std::end(types))
				newNameSuggestion += '_'; // TODO: replies on newNameSuggestion to have at least 4 chars (assumes suggestion ends with Type)

			// insert with new name
			type.name = newNameSuggestion;
			types[newNameSuggestion] = type;
			return true;
		}
		return false;
	};
	if (renameType(m_complexTypes))
		return newNameSuggestion;
	if (renameType(m_simpleTypes))
		return newNameSuggestion;
	return oldName; // TODO: remove after debugging
	throw std::logic_error("Could not find type " + oldName);
}
