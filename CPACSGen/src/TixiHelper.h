#pragma once

#include <tixi.h>

#include <vector>
#include <string>
#include <iostream>
#include <ctime>

#include "Optional.hpp"

namespace tigl {
    namespace tixihelper {
        std::string errorToString(ReturnCode ret);

        class TixiError : public std::exception {
        public:
            TixiError(ReturnCode ret);
            TixiError(ReturnCode ret, const std::string& message);

            virtual const char* what() const throw() override;

        private:
            ReturnCode m_ret;
            std::string m_message;
        };

        struct SplitXPath {
            std::string parentXPath;
            std::string element;
        };

        SplitXPath splitXPath(const std::string& xpath);

        TixiDocumentHandle TixiCreateDocument(const std::string& rootElement);
        TixiDocumentHandle TixiOpenDocument(const std::string& filename);
        TixiDocumentHandle TixiImportFromString(const std::string& xml);

        void TixiAddCpacsHeader(const TixiDocumentHandle& tixiHandle, const std::string& name, const std::string& creator, const std::string& version, const std::string& description, const std::string& cpacsVersion);

        bool TixiCheckAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
        bool TixiCheckElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        int TixiGetNamedChildrenCount(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        std::vector<std::string> TixiGetAttributeNames(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        std::string TixiGetTextAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
        double      TixiGetDoubleAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
        bool        TixiGetBoolAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
        int         TixiGetIntAttribute   (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);

        std::string TixiGetTextElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        double      TixiGetDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        bool        TixiGetBoolElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        int         TixiGetIntElement   (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        std::time_t TixiGetTimeTElement (const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const char*        value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, double             value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, bool               value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, int                value);

        template <typename T>
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const Optional<T>& value) {
            if (value) TixiSaveAttribute(tixiHandle, xpath, attribute, *value);
        }

        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const char*        value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, double             value, const std::string& format = "%g");
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, bool               value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, int                value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::time_t        value);

        template <typename T>
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const Optional<T>& value) {
            if (value) {
                TixiSaveElement(tixiHandle, xpath, *value);
            }
        }

        inline void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const Optional<double>& value, const std::string& format = "%g") {
            if (value) {
                TixiSaveElement(tixiHandle, xpath, *value, format);
            }
        }

        void TixiCreateElement (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        void TixiCreateElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        void TixiRemoveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
        void TixiRemoveElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        std::string TixiExportDocumentAsString(const TixiDocumentHandle& tixiHandle);

        void TixiRegisterNamespacesFromDocument(const TixiDocumentHandle& tixiHandle);

        template<typename ChildType, typename ReadChildFunc>
        void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<ChildType>& children, ReadChildFunc readChild, int minOccurs = -1, int maxOccurs = -1) {
            // read number of child nodes
            const int childCount = TixiGetNamedChildrenCount(tixiHandle, xpath);

            // validate number of child nodes
            if (minOccurs >= 0) {
                if (childCount < minOccurs) {
                    // TODO: replace by exception/warning
                    std::cerr
                        << "Not enough child nodes for element\n"
                        << "xpath: " << xpath << "\n"
                        << "minimum: " << minOccurs << "\n"
                        << "actual: " << childCount;
                }
            }
            if (maxOccurs >= 0) {
                if (childCount > maxOccurs) {
                    // TODO: replace by exception/warning
                    std::cerr
                        << "Too many child nodes for element\n"
                        << "xpath: " << xpath << "\n"
                        << "maximum: " << maxOccurs << "\n"
                        << "actual: " << childCount;
                }
            }

            // read child nodes
            for (int i = 0; i < childCount; i++)
                children.push_back(readChild(xpath + "[" + std::to_string(i + 1) + "]"));
        }

        template<typename ChildType, typename WriteChildFunc>
        void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<ChildType>& children, WriteChildFunc writeChild) {
            const auto sp = splitXPath(xpath);

            // get number of children
            int childCount = 0;
            tixiGetNamedChildrenCount(tixiHandle, sp.parentXPath.c_str(), sp.element.c_str(), &childCount); // this call is allowed to fail if the element at parentXPath does not exist

            // test if we have children to write
            if (children.size() > 0) {
                // it the container node does not exist, create it
                // TODO: is this really needed?
                if (tixiCheckElement(tixiHandle, sp.parentXPath.c_str()) == ELEMENT_NOT_FOUND) {
                    const auto pos = sp.parentXPath.find_last_of('/');
                    if (pos == std::string::npos)
                        throw std::invalid_argument("parentXPath must contain a /");
                    const std::string pathBeforeParentElement = sp.parentXPath.substr(0, pos);
                    const std::string parentName = sp.parentXPath.substr(pos + 1);
                    const auto ret = tixiCreateElement(tixiHandle, pathBeforeParentElement.c_str(), parentName.c_str());
                    if (ret != SUCCESS) {
                        throw TixiError(ret, "tixiCreateElement failed creating element \"" + parentName + "\" at path \"" + pathBeforeParentElement + "\"");
                    }
                }

                // iteratore over all child nodes
                for (std::size_t i = 0; i < children.size(); i++) {
                    // if child node does not exist, create it
                    const std::string childPath = xpath + "[" + std::to_string(i) + "]";
                    if (tixiCheckElement(tixiHandle, childPath.c_str()) == ELEMENT_NOT_FOUND) {
                        const auto ret = tixiCreateElement(tixiHandle, sp.parentXPath.c_str(), sp.element.c_str());
                        if (ret != SUCCESS) {
                            throw TixiError(ret, "tixiCreateElement failed creating element \"" + sp.element + "\" at path \"" + sp.parentXPath + "\"");
                        }
                    }

                    // write child node
                    writeChild(childPath, children[i]);
                }

                // delete old children which where not overwritten
                for (std::size_t i = children.size() + 1; i <= childCount; i++) {
                    tixiRemoveElement(tixiHandle, (xpath + "[" + std::to_string(i) + "]").c_str());
                }
            } else {
                // parent node must not exist if there are no child nodes
                tixiRemoveElement(tixiHandle, sp.parentXPath.c_str());
            }
        }
    }
}