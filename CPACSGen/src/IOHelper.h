#pragma once

#include <tixi.h>

#include <vector>
#include <string>

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
bool TixiCheckElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
bool TixiCheckElement(const TixiDocumentHandle& tixiHandle, const std::string& parentPath, const std::string& element);

int TixiGetNamedChildrenCount(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& child);

std::string TixiGetTextAttribute   (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
double      TixiGetDoubleAttribute (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
bool        TixiGetBoolAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);

std::string TixiGetTextElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
double      TixiGetDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
bool        TixiGetBoolElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
int         TixiGetIntElement   (const TixiDocumentHandle& tixiHandle, const std::string& xpath);

void TixiSaveTextAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value);
void TixiSaveDoubleAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, double value);
void TixiSaveBoolAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, bool value);

void TixiSaveTextElement   (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, const std::string& value);
void TixiSaveDoubleElement (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, double value);
void TixiSaveBoolElement   (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, bool value);
void TixiSaveIntElement    (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, int value);


template<typename ChildType>
void ReadVectorElements(const TixiDocumentHandle& tixiHandle, const std::string& parentPath, const std::string& childName, std::vector<ChildType>& children) {
	const std::string childBasePath = parentPath + "/" + childName;

	// read number of child nodes
	const int childCount = TixiGetNamedChildrenCount(tixiHandle, parentPath, childName);

	// read child nodes
	for (int i = 0; i < childCount; i++) {
		ChildType child;
		child.ReadCPACS(tixiHandle, childBasePath + "[" + std_to_string(i + 1) + "]");
		children.push_back(std::move(child));
	}
}

template<typename ChildType>
void WriteVectorElements(const TixiDocumentHandle& tixiHandle, const std::string& parentPath, const std::string& childName, const std::vector<ChildType>& children) {
	const std::string childBasePath = parentPath + "/" + childName;

	// get number of children
	int childCount = 0;
	tixiGetNamedChildrenCount(tixiHandle, parentPath.c_str(), childName.c_str(), &childCount); // this call is allowed to fail if the element at parentPath does not exist

	// test if we have children to write
	if (children.size() > 0) {
		// it the container node does not exist, create it
		if (tixiCheckElement(tixiHandle, parentPath.c_str()) == ELEMENT_NOT_FOUND) {
			const std::string pathBeforeParentElement = parentPath.substr(0, parentPath.size() - (parentName.size() + 1));
			if (tixiCreateElement(tixiHandle, pathBeforeParentElement.c_str(), parentName.c_str()) != SUCCESS) {
				throw TixiError("tixiCreateElement failed creating element \"" + parentName + "\" at path \"" + pathBeforeParentElement + "\"");
			}
		}

		// iteratore over all child nodes
		for (std::size_t i = 0; i < children.size(); i++) {
			// if child node does not exist, create it
			const std::string childPath = childBasePath + "[" + std::to_string(i) + "]";
			if (tixiCheckElement(tixiHandle, childPath.c_str()) == ELEMENT_NOT_FOUND) {
				if (tixiCreateElement(tixiHandle, parentPath.c_str(), childName.c_str()) != SUCCESS) {
					throw TixiError("tixiCreateElement failed creating element \"" + childName + "\" at path \"" + parentPath + "\"");
				}
			}

			// write child node
			children[i].WriteCPACS(tixiHandle, childPath);
		}

		// delete old children which where not overwritten
		for (std::size_t i = children.size() + 1; i <= childCount; i++) {
			tixiRemoveElement(tixiHandle, (childBasePath + "[" + std::to_string(count + 1) + "]").c_str());
		}
	} else {
		// parent node must not exist if there are no child nodes
		tixiRemoveElement(tixiHandle, parentPath.c_str());
	}
}
