#pragma once

#include <tixi.h>

#include <vector>
#include <string>
#include <iostream>
#include <ctime>

#include "Optional.hpp"

namespace tigl {
	std::string errorToString(ReturnCode ret);

	class TixiError : public std::exception {
	public:
		TixiError(ReturnCode ret);
		TixiError(ReturnCode ret, const std::string& message);

		virtual const char* what() const override;

	private:
		ReturnCode m_ret;
		std::string m_message;
	};

	bool TixiCheckAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
	bool TixiCheckElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
	bool TixiCheckElement  (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element);

	int TixiGetNamedChildrenCount(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& child);

	std::string TixiGetTextAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
	double      TixiGetDoubleAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
	bool        TixiGetBoolAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
	int         TixiGetIntAttribute   (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);

	std::string TixiGetTextElement  (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element);
	double      TixiGetDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element);
	bool        TixiGetBoolElement  (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element);
	int         TixiGetIntElement   (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element);
	std::time_t TixiGetTimeTElement (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element);

	void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value);
	void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, double             value);
	void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, bool               value);
	void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, int                value);

	template <typename T>
	void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const Optional<T>& value) {
		if (value.isValid()) TixiSaveAttribute(tixiHandle, xpath, attribute, value.get());
	}

	void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, const std::string& value);
	void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, double             value);
	void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, bool               value);
	void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, int                value);
	void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, std::time_t        value);

	template <typename T>
	void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, const Optional<T>& value) {
		if (value.isValid()) TixiSaveElement(tixiHandle, parentXPath, element, value.get());
	}

	template<typename ChildType, typename ReadChildFunc>
	void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& parentPath, const std::string& childName, std::vector<ChildType>& children, ReadChildFunc readChild, int minOcurrs = -1, int maxOccurs = -1) {
		const std::string childBasePath = parentPath + "/" + childName;

		// read number of child nodes
		const int childCount = TixiGetNamedChildrenCount(tixiHandle, parentPath, childName);

		// validate number of child nodes
		if (minOcurrs >= 0) {
			if (childCount < minOcurrs) {
				// TODO: replace by exception/warning
				std::cerr
					<< "Not enough child nodes for element\n"
					<< "xpath: " << parentPath << "\n"
					<< "minimum: " << minOcurrs << "\n"
					<< "actual: " << childCount;
			}
		}
		if (minOcurrs >= 0) {
			if (childCount > maxOccurs) {
				// TODO: replace by exception/warning
				std::cerr
					<< "Too many child nodes for element\n"
					<< "xpath: " << parentPath << "\n"
					<< "maximum: " << maxOccurs << "\n"
					<< "actual: " << childCount;
			}
		}

		// read child nodes
		for (int i = 0; i < childCount; i++)
			children.push_back(readChild(childBasePath + "[" + std::to_string(i + 1) + "]"));
	}

	template<typename ChildType, typename WriteChildFunc>
	void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& parentPath, const std::string& childName, const std::vector<ChildType>& children, WriteChildFunc writeChild) {
		const std::string childBasePath = parentPath + "/" + childName;

		// get number of children
		int childCount = 0;
		tixiGetNamedChildrenCount(tixiHandle, parentPath.c_str(), childName.c_str(), &childCount); // this call is allowed to fail if the element at parentPath does not exist

																								   // test if we have children to write
		if (children.size() > 0) {
			// it the container node does not exist, create it
			if (tixiCheckElement(tixiHandle, parentPath.c_str()) == ELEMENT_NOT_FOUND) {
				const auto pos = parentPath.find_last_of('/');
				if (pos == std::string::npos)
					throw std::invalid_argument("parentXpath must contain a /");
				const std::string pathBeforeParentElement = parentPath.substr(0, pos);
				const std::string parentName = parentPath.substr(pos + 1);
				const auto ret = tixiCreateElement(tixiHandle, pathBeforeParentElement.c_str(), parentName.c_str());
				if (ret != SUCCESS) {
					throw TixiError(ret, "tixiCreateElement failed creating element \"" + parentName + "\" at path \"" + pathBeforeParentElement + "\"");
				}
			}

			// iteratore over all child nodes
			for (std::size_t i = 0; i < children.size(); i++) {
				// if child node does not exist, create it
				const std::string childPath = childBasePath + "[" + std::to_string(i) + "]";
				if (tixiCheckElement(tixiHandle, childPath.c_str()) == ELEMENT_NOT_FOUND) {
					const auto ret = tixiCreateElement(tixiHandle, parentPath.c_str(), childName.c_str());
					if (ret != SUCCESS) {
						throw TixiError(ret, "tixiCreateElement failed creating element \"" + childName + "\" at path \"" + parentPath + "\"");
					}
				}

				// write child node
				writeChild(childPath, children[i]);
			}

			// delete old children which where not overwritten
			for (std::size_t i = children.size() + 1; i <= childCount; i++) {
				tixiRemoveElement(tixiHandle, (childBasePath + "[" + std::to_string(i) + "]").c_str());
			}
		} else {
			// parent node must not exist if there are no child nodes
			tixiRemoveElement(tixiHandle, parentPath.c_str());
		}
	}
}