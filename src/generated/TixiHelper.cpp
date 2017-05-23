// Copyright (c) 2016 RISC Software GmbH
//
// This file is part of the CPACSGen runtime.
// Do not edit, all changes are lost when files are re-generated.
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define BOOST_DATE_TIME_NO_LIB
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <stdexcept>

#include "TixiHelper.h"

namespace tigl
{
    namespace tixihelper
    {
        std::string errorToString(ReturnCode ret)
        {
            switch (ret) {
            case SUCCESS:                 return "SUCCESS";                  //  0: No error occurred
            case FAILED:                  return "FAILED";                   //  1: Unspecified error
            case INVALID_XML_NAME:        return "INVALID_XML_NAME";         //  2: Non XML standard compliant name specified
            case NOT_WELL_FORMED:         return "NOT_WELL_FORMED";          //  3: Document is not well formed
            case NOT_SCHEMA_COMPLIANT:    return "NOT_SCHEMA_COMPLIANT";     //  4: Document is not schema compliant
            case NOT_DTD_COMPLIANT:       return "NOT_DTD_COMPLIANT";        //  5: Document is not DTD compliant
            case INVALID_HANDLE:          return "INVALID_HANDLE";           //  6: Document handle is not valid
            case INVALID_XPATH:           return "INVALID_XPATH";            //  7: XPath expression is not valid
            case ELEMENT_NOT_FOUND:       return "ELEMENT_NOT_FOUND";        //  8: Element does not exist in document
            case INDEX_OUT_OF_RANGE:      return "INDEX_OUT_OF_RANGE";       //  9: Index supplied as argument is not inside the admissible range
            case NO_POINT_FOUND:          return "NO_POINT_FOUND";           // 10: No point element found a given XPath
            case NOT_AN_ELEMENT:          return "NOT_AN_ELEMENT";           // 11: XPath expression does not point to an XML-element node
            case ATTRIBUTE_NOT_FOUND:     return "ATTRIBUTE_NOT_FOUND";      // 12: Element does not have the attribute
            case OPEN_FAILED:             return "OPEN_FAILED";              // 13: Error on opening the file
            case OPEN_SCHEMA_FAILED:      return "OPEN_SCHEMA_FAILED";       // 14: Error on opening the schema file
            case OPEN_DTD_FAILED:         return "OPEN_DTD_FAILED";          // 15: Error on opening the DTD file
            case CLOSE_FAILED:            return "CLOSE_FAILED";             // 16: Error on closing the file
            case ALREADY_SAVED:           return "ALREADY_SAVED";            // 17: Trying to modify already saved document
            case ELEMENT_PATH_NOT_UNIQUE: return "ELEMENT_PATH_NOT_UNIQUE";  // 18: Path expression can not be resolved unambiguously
            case NO_ELEMENT_NAME:         return "NO_ELEMENT_NAME";          // 19: Element name argument is NULL
            case NO_CHILDREN:             return "NO_CHILDREN";              // 20: Node has no children
            case CHILD_NOT_FOUND:         return "CHILD_NOT_FOUND";          // 21: Named child is not child of element specified
            case EROROR_CREATE_ROOT_NODE: return "EROROR_CREATE_ROOT_NODE";  // 22: Error when adding root node to new document
            case DEALLOCATION_FAILED:     return "DEALLOCATION_FAILED";      // 23: On closing a document the deallocation of allocated memory fails
            case NO_NUMBER:               return "NO_NUMBER";                // 24: No number specified
            case NO_ATTRIBUTE_NAME:       return "NO_ATTRIBUTE_NAME";        // 25: No attribute name specified
            case STRING_TRUNCATED:        return "STRING_TRUNCATED";         // 26: String variable supplied is to small to hold the result  Fortran only
            case NON_MATCHING_NAME:       return "NON_MATCHING_NAME";        // 27: Row or column name specified do not match the names used in the document
            case NON_MATCHING_SIZE:       return "NON_MATCHING_SIZE";        // 28: Number of rows or columns specified do not match the sizes of the matrix in the document
            case MATRIX_DIMENSION_ERROR:  return "MATRIX_DIMENSION_ERROR";   // 29: if nRows or nColumns or both are less than 1
            case COORDINATE_NOT_FOUND:    return "COORDINATE_NOT_FOUND";     // 30: missing coordinate inside a point element
            case UNKNOWN_STORAGE_MODE:    return "UNKNOWN_STORAGE_MODE";     // 31: storage mode specified is neither ROW_WISE nor COLUMN_WISE
            case UID_NOT_UNIQUE:          return "UID_NOT_UNIQUE";           // 32: One or more uID's are not unique
            case UID_DONT_EXISTS:         return "UID_DONT_EXISTS";          // 33: A given uID's does not exist
            case UID_LINK_BROKEN:         return "UID_LINK_BROKEN";          // 33: A node the is specified as a Link has no correspoding uid in that data set
            default: throw std::logic_error("Invalid ReturnCode");
            }
        }

        TixiError::TixiError(ReturnCode ret)
            : m_ret(ret)
        {
            m_message = "ReturnCode: " + errorToString(m_ret);
        }

        TixiError::TixiError(ReturnCode ret, const std::string& message)
            : m_ret(ret)
        {
            m_message = message + "\nReturnCode: " + errorToString(m_ret);
        }

        const char* TixiError::what() const throw()
        {
            return m_message.c_str();
        }

        SplitXPath splitXPath(const std::string& xpath)
        {
            const std::size_t pos = xpath.find_last_of('/');
            SplitXPath result;
            result.parentXPath = pos != std::string::npos ? xpath.substr(0, pos)  : "";
            result.element     = pos != std::string::npos ? xpath.substr(pos + 1) : xpath;
            return result;
        }

        TixiDocumentHandle TixiCreateDocument(const std::string& rootElement)
        {
            TixiDocumentHandle handle;
            const ReturnCode ret = tixiCreateDocument(rootElement.c_str(), &handle);
            if (ret != SUCCESS) {
                throw TixiError(ret, "Failed to create tixi document with root element: " + rootElement);
            }
            return handle;
        }

        TixiDocumentHandle TixiOpenDocument(const std::string& filename)
        {
            TixiDocumentHandle handle;
            const ReturnCode ret = tixiOpenDocument(filename.c_str(), &handle);
            if (ret != SUCCESS) {
                throw TixiError(ret, "Failed to open tixi document: " + filename);
            }
            return handle;
        }

        TixiDocumentHandle TixiImportFromString(const std::string& xml)
        {
            TixiDocumentHandle handle;
            const ReturnCode ret = tixiImportFromString(xml.c_str(), &handle);
            if (ret != SUCCESS) {
                throw TixiError(ret, "Failed to import tixi document from string. " + xml);
            }
            return handle;
        }

        void TixiAddCpacsHeader(const TixiDocumentHandle& tixiHandle, const std::string& name, const std::string& creator, const std::string& version, const std::string& description, const std::string& cpacsVersion)
        {
            const ReturnCode ret = tixiAddCpacsHeader(tixiHandle, name.c_str(), creator.c_str(), version.c_str(), description.c_str(), cpacsVersion.c_str());
            if (ret != SUCCESS) {
                throw TixiError(ret,
                    "Failed to add CPACS Header\n"
                    "name: " + name + "\n"
                    "creator: " + creator + "\n"
                    "version: " + version + "\n"
                    "description: " + description + "\n"
                    "cpacsVersion: " + cpacsVersion
                );
            }
        }

        bool TixiCheckAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute)
        {
            const ReturnCode ret = tixiCheckAttribute(tixiHandle, xpath.c_str(), attribute.c_str());
            if (ret == SUCCESS) {
                return true;
            } else if (ret == ATTRIBUTE_NOT_FOUND) {
                return false;
            } else {
                throw TixiError(ret,
                    "Checking attribute existence failed\n"
                    "xpath: " + xpath + "\n"
                    "attribute: " + attribute
                );
            }
        }

        bool TixiCheckElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            const ReturnCode ret = tixiCheckElement(tixiHandle, xpath.c_str());
            if (ret == SUCCESS) {
                return true;
            } else if (ret == ELEMENT_NOT_FOUND) {
                return false;
            } else {
                throw TixiError(ret,
                    "Checking element existence failed\n"
                    "xpath: " + xpath
                );
            }
        }

        int TixiGetNamedChildrenCount(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            const SplitXPath sp = splitXPath(xpath);
            int count = 0;
            ReturnCode ret = tixiGetNamedChildrenCount(tixiHandle, sp.parentXPath.c_str(), sp.element.c_str(), &count);
            if (ret != SUCCESS) {
                throw TixiError(ret);
            }
            return count;
        }

        std::vector<std::string> TixiGetAttributeNames(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            int count = 0;
            const ReturnCode ret = tixiGetNumberOfAttributes(tixiHandle, xpath.c_str(), &count);
            if (ret != SUCCESS) {
                throw TixiError(ret, "Failed to get number of attributes at xpath: " + xpath);
            }
            std::vector<std::string> names;
            names.reserve(count);
            for (int i = 1; i <= count; i++) {
                char* name = NULL;
                const ReturnCode ret = tixiGetAttributeName(tixiHandle, xpath.c_str(), i, &name);
                if (ret != SUCCESS) {
                    throw TixiError(ret,
                        "Failed to get the name an attribute\n"
                        "Index: " + to_string(i) + "\n"
                        "xpath: " + xpath
                    );
                }
                names.push_back(name);
            }
            return names;
        }

        namespace
        {
            template <typename T, typename GetFunc>
            T TixiGetAttributeInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, GetFunc getFunc)
            {
                T value;
                const ReturnCode ret = getFunc(tixiHandle, xpath.c_str(), attribute.c_str(), &value);
                if (ret != SUCCESS) {
                    throw TixiError(ret,
                        "Error getting std::string attribute value\n"
                        "xpath: " + xpath + "\n"
                        "attribute: " + attribute
                    );
                }
                return value;
            }
        }

        std::string TixiGetTextAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute)
        {
            return std::string(TixiGetAttributeInternal<char*>(tixiHandle, xpath, attribute, tixiGetTextAttribute));
        }

        double TixiGetDoubleAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute)
        {
            return TixiGetAttributeInternal<double>(tixiHandle, xpath, attribute, tixiGetDoubleAttribute);
        }

        bool TixiGetBoolAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute)
        {
            return TixiGetAttributeInternal<int>(tixiHandle, xpath, attribute, tixiGetBooleanAttribute) != 0;
        }

        int TixiGetIntAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute)
        {
            return TixiGetAttributeInternal<int>(tixiHandle, xpath, attribute, tixiGetBooleanAttribute);
        }

        namespace
        {
            template <typename T, typename GetFunc>
            T TixiGetElementInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, GetFunc getFunc)
            {
                T value;
                const ReturnCode ret = getFunc(tixiHandle, xpath.c_str(), &value);
                if (ret != SUCCESS) {
                    throw TixiError(ret,
                        "Error getting element value\n"
                        "xpath: " + xpath
                    );
                }
                return value;
            }
        }

        std::string TixiGetTextElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            return std::string(TixiGetElementInternal<char*>(tixiHandle, xpath, tixiGetTextElement));
        }

        double TixiGetDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            return TixiGetElementInternal<double>(tixiHandle, xpath, tixiGetDoubleElement);
        }

        bool TixiGetBoolElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            return TixiGetElementInternal<int>(tixiHandle, xpath, tixiGetBooleanElement) != 0;
        }

        int TixiGetIntElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            return TixiGetElementInternal<int>(tixiHandle, xpath, tixiGetIntegerElement);
        }

        namespace fromboost
        {
            std::time_t to_time_t(boost::posix_time::ptime pt)
            {
                boost::posix_time::time_duration dur = pt - boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
                return std::time_t(dur.total_seconds());
            }

            boost::posix_time::ptime from_iso_extended_string(const std::string& s)
            {
                return boost::date_time::parse_delimited_time<boost::posix_time::ptime>(s, 'T');
            }
        }

        std::time_t TixiGetTimeTElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            //return boost::posix_time::to_time_t(boost::posix_time::from_iso_extended_string(TixiGetTextElement(tixiHandle, xpath))); // enable, when Boost >= 1.62 is available
            return fromboost::to_time_t(fromboost::from_iso_extended_string(TixiGetTextElement(tixiHandle, xpath)));
        }

        namespace
        {
            //template<typename SaveFunc, typename... ValueAndFurtherArgs>
            //void TixiSaveAttributeInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, SaveFunc saveFunc, ValueAndFurtherArgs&&... args)
            //{
            //    const ReturnCode ret = saveFunc(tixiHandle, xpath.c_str(), attribute.c_str(), std::forward<ValueAndFurtherArgs>(args)...);
            //    if (ret != SUCCESS) {
            //        throw TixiError(ret,
            //            "Error setting attribute\n"
            //            "xpath: " + xpath + "\n"
            //            "attribute: " + attribute + "\n"
            //        );
            //    }
            //}

            template<typename SaveFunc, typename Value>
            void TixiSaveAttributeInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, SaveFunc saveFunc, const Value& value)
            {
                const ReturnCode ret = saveFunc(tixiHandle, xpath.c_str(), attribute.c_str(), value);
                if (ret != SUCCESS) {
                    throw TixiError(ret,
                        "Error setting attribute\n"
                        "xpath: " + xpath + "\n"
                        "attribute: " + attribute + "\n"
                    );
                }
            }

            template<typename SaveFunc, typename Value, typename Arg>
            void TixiSaveAttributeInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, SaveFunc saveFunc, const Value& value, const Arg& arg)
            {
                const ReturnCode ret = saveFunc(tixiHandle, xpath.c_str(), attribute.c_str(), value, arg);
                if (ret != SUCCESS) {
                    throw TixiError(ret,
                        "Error setting attribute\n"
                        "xpath: " + xpath + "\n"
                        "attribute: " + attribute + "\n"
                    );
                }
            }
        }

        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const char* value)
        {
            TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddTextAttribute, value);
        }

        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value)
        {
            TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddTextAttribute, value.c_str());
        }

        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, double value)
        {
            TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddDoubleAttribute, value, (const char*)NULL);
        }

        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, bool value)
        {
            TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddIntegerAttribute, value ? 1 : 0, "%d"); // TODO: no tixiAddBooleanAttribute in Tixi
        }

        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, int value)
        {
            TixiSaveAttributeInternal(tixiHandle, xpath, attribute, tixiAddIntegerAttribute, value, "%d");
        }

        namespace
        {
            //template<typename UpdateFunc, typename SaveFunc, typename... ValueAndFurtherArgs>
            //void TixiSaveElementInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, UpdateFunc updateFunc, SaveFunc saveFunc, ValueAndFurtherArgs&&... args)
            //{
            //    const SplitXPath sp = splitXPath(xpath);
            //
            //    // if it exist, update it
            //    const bool exists = TixiCheckElement(tixiHandle, xpath);
            //    if (exists) {
            //        const ReturnCode ret = updateFunc(tixiHandle, xpath.c_str(), std::forward<ValueAndFurtherArgs>(args)...);
            //        if (ret != SUCCESS) {
            //            throw TixiError(ret,
            //                "Error updating element\n"
            //                "xpath: " + xpath
            //            );
            //        }
            //    }
            //    // else, create it
            //    else {
            //        const ReturnCode ret = saveFunc(tixiHandle, sp.parentXPath.c_str(), sp.element.c_str(), std::forward<ValueAndFurtherArgs>(args)...);
            //        if (ret != SUCCESS) {
            //            throw TixiError(ret,
            //                "Error saving element\n"
            //                "xpath: " + xpath
            //            );
            //        }
            //    }
            //}

            template<typename UpdateFunc, typename SaveFunc, typename Value>
            void TixiSaveElementInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, UpdateFunc updateFunc, SaveFunc saveFunc, const Value& value)
            {
                const SplitXPath sp = splitXPath(xpath);

                // if it exist, update it
                const bool exists = TixiCheckElement(tixiHandle, xpath);
                if (exists) {
                    const ReturnCode ret = updateFunc(tixiHandle, xpath.c_str(), value);
                    if (ret != SUCCESS) {
                        throw TixiError(ret,
                            "Error updating element\n"
                            "xpath: " + xpath
                        );
                    }
                }
                // else, create it
                else {
                    const ReturnCode ret = saveFunc(tixiHandle, sp.parentXPath.c_str(), sp.element.c_str(), value);
                    if (ret != SUCCESS) {
                        throw TixiError(ret,
                            "Error saving element\n"
                            "xpath: " + xpath
                        );
                    }
                }
            }

            template<typename UpdateFunc, typename SaveFunc, typename Value, typename Arg>
            void TixiSaveElementInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, UpdateFunc updateFunc, SaveFunc saveFunc, const Value& value, const Arg& arg)
            {
                const SplitXPath sp = splitXPath(xpath);

                // if it exist, update it
                const bool exists = TixiCheckElement(tixiHandle, xpath);
                if (exists) {
                    const ReturnCode ret = updateFunc(tixiHandle, xpath.c_str(), value, arg);
                    if (ret != SUCCESS) {
                        throw TixiError(ret,
                            "Error updating element\n"
                            "xpath: " + xpath
                        );
                    }
                }
                // else, create it
                else {
                    const ReturnCode ret = saveFunc(tixiHandle, sp.parentXPath.c_str(), sp.element.c_str(), value, arg);
                    if (ret != SUCCESS) {
                        throw TixiError(ret,
                            "Error saving element\n"
                            "xpath: " + xpath
                        );
                    }
                }
            }
        }

        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const char* value)
        {
            TixiSaveElementInternal(tixiHandle, xpath, tixiUpdateTextElement, tixiAddTextElement, value);
        }

        // TODO: updating an empty text doesn't work : no error, but the text is not updated
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& value)
        {
            TixiSaveElementInternal(tixiHandle, xpath, tixiUpdateTextElement, tixiAddTextElement, value.c_str());
        }

        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, double value, const std::string& format)
        {
            TixiSaveElementInternal(tixiHandle, xpath, tixiUpdateDoubleElement, tixiAddDoubleElement, value, format.c_str());
        }

        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, bool value)
        {
            TixiSaveElementInternal(tixiHandle, xpath, tixiUpdateBooleanElement, tixiAddBooleanElement, value ? 1 : 0);
        }

        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, int value)
        {
            TixiSaveElementInternal(tixiHandle, xpath, tixiUpdateIntegerElement, tixiAddIntegerElement, value, "%d");
        }

        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::time_t value)
        {
            TixiSaveElement(tixiHandle, xpath, boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(value)));
        }

        void TixiCreateElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            const SplitXPath sp = splitXPath(xpath);
            const ReturnCode ret = tixiCreateElement(tixiHandle, sp.parentXPath.c_str(), sp.element.c_str());
            if (ret != SUCCESS) {
                throw TixiError(ret,
                    "Error creating element\n"
                    "xpath: " + xpath
                );
            }
        }

        void TixiCreateElementIfNotExists(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            if (!TixiCheckElement(tixiHandle, xpath)) {
                TixiCreateElement(tixiHandle, xpath);
            }
        }

        void TixiCreateElementsIfNotExists(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            std::size_t pos = xpath.find_first_of('/', 1); // find first slash after initial one
            while (pos != std::string::npos) {
                const std::string path = xpath.substr(0, pos);
                TixiCreateElementIfNotExists(tixiHandle, path);
                pos = xpath.find_first_of('/', pos + 1);
            }
            TixiCreateElementIfNotExists(tixiHandle, xpath);
        }

        void TixiRemoveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute)
        {
            const ReturnCode ret = tixiRemoveAttribute(tixiHandle, xpath.c_str(), attribute.c_str());
            if (ret != SUCCESS) {
                throw TixiError(ret,
                    "Error removing attribute\n"
                    "xpath: " + xpath
                );
            }
        }

        void TixiRemoveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            const ReturnCode ret = tixiRemoveElement(tixiHandle, xpath.c_str());
            if (ret != SUCCESS) {
                throw TixiError(ret,
                    "Error removing element\n"
                    "xpath: " + xpath
                );
            }
        }

        std::string TixiExportDocumentAsString(const TixiDocumentHandle& tixiHandle)
        {
            char* ptr;
            tixiExportDocumentAsString(tixiHandle, &ptr);
            return ptr;
        }

        void TixiRegisterNamespacesFromDocument(const TixiDocumentHandle& tixiHandle)
        {
            const ReturnCode ret = tixiRegisterNamespacesFromDocument(tixiHandle);
            if (ret != SUCCESS) {
                throw TixiError(ret, "Failed to register all document namespaces");
            }
        }

        void TixiRegisterNamespace(const TixiDocumentHandle& tixiHandle, const std::string& namespaceURI, const std::string& prefix)
        {
            const ReturnCode ret =  tixiRegisterNamespace(tixiHandle, namespaceURI.c_str(), prefix.c_str());
            if (ret != SUCCESS) {
                throw TixiError(ret, "Failed to register all document namespaces");
            }
        }
    }
}
