
#include "IOHelper.h"

std::string errorToString(ReturnCode ret) {
	switch (ret) {
		case SUCCESS: return "SUCCESS";   //  0: No error occurred
		case FAILED: return "FAILED";   //  1: Unspecified error
		case INVALID_XML_NAME: return "INVALID_XML_NAME";   //  2: Non XML standard compliant name specified
		case NOT_WELL_FORMED: return "NOT_WELL_FORMED";   //  3: Document is not well formed
		case NOT_SCHEMA_COMPLIANT: return "NOT_SCHEMA_COMPLIANT";   //  4: Document is not schema compliant
		case NOT_DTD_COMPLIANT: return "NOT_DTD_COMPLIANT";   //  5: Document is not DTD compliant
		case INVALID_HANDLE: return "INVALID_HANDLE";   //  6: Document handle is not valid
		case INVALID_XPATH: return "INVALID_XPATH";   //  7: XPath expression is not valid
		case ELEMENT_NOT_FOUND: return "ELEMENT_NOT_FOUND";   //  8: Element does not exist in document
		case INDEX_OUT_OF_RANGE: return "INDEX_OUT_OF_RANGE";   //  9: Index supplied as argument is not inside the admissible range
		case NO_POINT_FOUND: return "NO_POINT_FOUND";   // 10: No point element found a given XPath
		case NOT_AN_ELEMENT: return "NOT_AN_ELEMENT";   // 11: XPath expression does not point to an XML-element node
		case ATTRIBUTE_NOT_FOUND: return "ATTRIBUTE_NOT_FOUND";   // 12: Element does not have the attribute
		case OPEN_FAILED: return "OPEN_FAILED";   // 13: Error on opening the file
		case OPEN_SCHEMA_FAILED: return "OPEN_SCHEMA_FAILED";   // 14: Error on opening the schema file
		case OPEN_DTD_FAILED: return "OPEN_DTD_FAILED";   // 15: Error on opening the DTD file
		case CLOSE_FAILED: return "CLOSE_FAILED";   // 16: Error on closing the file
		case ALREADY_SAVED: return "ALREADY_SAVED";   // 17: Trying to modify already saved document
		case ELEMENT_PATH_NOT_UNIQUE: return "ELEMENT_PATH_NOT_UNIQUE";   // 18: Path expression can not be resolved unambiguously
		case NO_ELEMENT_NAME: return "NO_ELEMENT_NAME";   // 19: Element name argument is NULL
		case NO_CHILDREN: return "NO_CHILDREN";   // 20: Node has no children
		case CHILD_NOT_FOUND: return "CHILD_NOT_FOUND";   // 21: Named child is not child of element specified
		case EROROR_CREATE_ROOT_NODE: return "EROROR_CREATE_ROOT_NODE";   // 22: Error when adding root node to new document
		case DEALLOCATION_FAILED: return "DEALLOCATION_FAILED";   // 23: On closing a document the deallocation of allocated memory fails
		case NO_NUMBER: return "NO_NUMBER";   // 24: No number specified
		case NO_ATTRIBUTE_NAME: return "NO_ATTRIBUTE_NAME";   // 25: No attribute name specified
		case STRING_TRUNCATED: return "STRING_TRUNCATED";   // 26: String variable supplied is to small to hold the result  Fortran only
		case NON_MATCHING_NAME: return "NON_MATCHING_NAME";   // 27: Row or column name specified do not match the names used in the document
		case NON_MATCHING_SIZE: return "NON_MATCHING_SIZE";   // 28: Number of rows or columns specified do not match the sizes of the matrix in the document
		case MATRIX_DIMENSION_ERROR: return "MATRIX_DIMENSION_ERROR";   // 29: if nRows or nColumns or both are less than 1
		case COORDINATE_NOT_FOUND: return "COORDINATE_NOT_FOUND";   // 30: missing coordinate inside a point element
		case UNKNOWN_STORAGE_MODE: return "UNKNOWN_STORAGE_MODE";   // 31: storage mode specified is neither ROW_WISE nor COLUMN_WISE
		case UID_NOT_UNIQUE: return "UID_NOT_UNIQUE";   // 32: One or more uID's are not unique
		case UID_DONT_EXISTS: return "UID_DONT_EXISTS";   // 33: A given uID's does not exist
		case UID_LINK_BROKEN: return "UID_LINK_BROKEN";   // 33: A node the is specified as a Link has no correspoding uid in that data set
		default: throw std::logic_error("Invalid ReturnCode");
	}
}

TixiError::TixiError(ReturnCode ret)
	: m_ret(ret) {
	m_message = "ReturnCode: " + errorToString(m_ret);
}

TixiError::TixiError(ReturnCode ret, const std::string& message)
	: m_ret(ret) {
	m_message = message + "\nReturnCode: " + errorToString(m_ret);
}

const char* TixiError::what() const {
	return m_message.c_str();
}

bool TixiCheckAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) {
	auto ret = tixiCheckAttribute(tixiHandle, xpath.c_str(), attribute.c_str());
	if (ret == ReturnCode::SUCCESS)
		return true;
	else if (ret == ReturnCode::ATTRIBUTE_NOT_FOUND)
		return false;
	else
		throw TixiError(ret,
			"Checking attribute existence failed\n"
			"xpath: " + xpath + "\n"
			"attribute: " + attribute
		);
}

bool TixiCheckElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {
	auto ret = tixiCheckElement(tixiHandle, xpath.c_str());
	if (ret == ReturnCode::SUCCESS)
		return true;
	else if (ret == ReturnCode::ELEMENT_NOT_FOUND)
		return false;
	else
		throw TixiError(ret,
			"Checking element existence failed\n"
			"xpath: " + xpath
		);
}

bool TixiCheckElement(const TixiDocumentHandle& tixiHandle, const std::string& parentPath, const std::string& element) {
	return TixiCheckElement(tixiHandle, parentPath + "/" + element);
}

int TixiGetNamedChildrenCount(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& child) {
	int count = 0;
	auto ret = tixiGetNamedChildrenCount(tixiHandle, xpath.c_str(), child.c_str(), &count);
	if (ret != ReturnCode::SUCCESS)
		throw TixiError(ret);
	return count;
}

namespace {
	template <typename T, typename GetFunc>
	auto TixiGetAttributeInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, GetFunc getFunc) {
		T value;
		const auto ret = getFunc(tixiHandle, xpath.c_str(), attribute.c_str(), &value);
		if (ret != ReturnCode::SUCCESS)
			throw TixiError(ret,
				"Error getting std::string attribute value\n"
				"xpath: " + xpath + "\n"
				"attribute: " + attribute
			);
		return value;
	}
}

std::string TixiGetTextAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) {
	return std::string(TixiGetAttributeInternal<char*>(tixiHandle, xpath, attribute, tixiGetTextAttribute));
}

double TixiGetDoubleAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) {
	return TixiGetAttributeInternal<double>(tixiHandle, xpath, attribute, tixiGetDoubleAttribute);
}

bool TixiGetBoolAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) {
	return TixiGetAttributeInternal<int>(tixiHandle, xpath, attribute, tixiGetBooleanAttribute) != 0;
}

int TixiGetIntAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) {
	return TixiGetAttributeInternal<int>(tixiHandle, xpath, attribute, tixiGetBooleanAttribute);
}

namespace {
	template <typename T, typename GetFunc>
	auto TixiGetElementInternal(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, GetFunc getFunc) {
		T value;
		const auto ret = getFunc(tixiHandle, (parentXPath + "/" + element).c_str(), &value);
		if (ret != ReturnCode::SUCCESS) {
			throw TixiError(ret,
				"Error getting element value\n"
				"element: " + element + "\n"
				"parentXPath: " + parentXPath
			);
		}
		return value;
	}
}

std::string TixiGetTextElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element) {
	return std::string(TixiGetElementInternal<char*>(tixiHandle, parentXPath, element, tixiGetTextElement));
}

double TixiGetDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element) {
	return TixiGetElementInternal<double>(tixiHandle, parentXPath, element, tixiGetDoubleElement);
}

bool TixiGetBoolElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element) {
	return TixiGetElementInternal<int>(tixiHandle, parentXPath, element, tixiGetBooleanElement) != 0;
}

int TixiGetIntElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element) {
	return TixiGetElementInternal<int>(tixiHandle, parentXPath, element, tixiGetBooleanElement);
}

namespace {
	template<typename SaveFunc, typename... ValueAndFurtherArgs>
	void TixiSaveAttributeInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, SaveFunc saveFunc, ValueAndFurtherArgs&&... args) {
		const auto ret1 = tixiCheckElement(tixiHandle, xpath.c_str());
		if (ret1 != SUCCESS) {
			throw TixiError(ret1,
				"Error setting attribute, element does not exist\n"
				"xpath: " + xpath
			);
		}

		const auto ret2 = saveFunc(tixiHandle, xpath.c_str(), attribute.c_str(), std::forward<ValueAndFurtherArgs>(args)...);
		if (ret2 != SUCCESS) {
			throw TixiError(ret2,
				"Error setting attribute\n"
				"xpath: " + xpath + "\n"
				"attribute: " + attribute + "\n"
			);
		}
	}
}

void TixiSaveTextAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value) {
	TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddTextAttribute, value.c_str());
}

void TixiSaveDoubleAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, double value) {
	TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddDoubleAttribute, value, nullptr);
}

void TixiSaveBoolAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, bool value) {
	TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddIntegerAttribute, value ? 1 : 0, nullptr); // TODO: no tixiAddBooleanAttribute in Tixi
}

namespace {
	template<typename SaveFunc, typename... ValueAndFurtherArgs>
	void TixiSaveElementInternal(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, SaveFunc saveFunc, ValueAndFurtherArgs&&... args) {
		const std::string xpath = parentXPath + "/" + element;

		// check if the parent elements exist
		const auto ret1 = tixiCheckElement(tixiHandle, parentXPath.c_str());
		if (ret1 != SUCCESS) {
			throw TixiError(ret1,
				"Error saving element, parent element does not exist\n"
				"parent xpath: " + parentXPath
			);
		}

		// first, delete the element
		if (TixiCheckElement(tixiHandle, parentXPath, element)) {
			const auto ret = tixiRemoveElement(tixiHandle, xpath.c_str());
			if (ret != SUCCESS) {
				throw TixiError(ret,
					"Error saving element, failed to remove previous element\n"
					"xpath: " + xpath
				);
			}
		}

		// then, the new element is created with the text
		const auto ret2 = saveFunc(tixiHandle, parentXPath.c_str(), element.c_str(), std::forward<ValueAndFurtherArgs>(args)...);
		if (ret2 != SUCCESS) {
			throw TixiError(ret2,
				"Error saving element\n"
				"xpath: " + xpath
			);
		}
	}

}

// TODO: updating an empty text doesn't work : no error, but the text is not updated
void TixiSaveTextElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, const std::string& value) {
	TixiSaveElementInternal(tixiHandle, parentXPath, element, tixiAddTextElement, value.c_str());
}

void TixiSaveDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, double value) {
	TixiSaveElementInternal(tixiHandle, parentXPath, element, tixiAddDoubleElement, value, nullptr);
}

void TixiSaveBoolElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, bool value) {
	TixiSaveElementInternal(tixiHandle, parentXPath, element, tixiAddBooleanElement, value ? 0 : 1);
}

void TixiSaveIntElement(const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, int value) {
	TixiSaveElementInternal(tixiHandle, parentXPath, element, tixiAddIntegerElement, value, nullptr);
}