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

#pragma once

#include <tixi.h>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include "UniquePtr.h"

namespace tigl
{
    namespace tixihelper
    {
        template <typename T>
        std::string to_string(const T& t) {
            std::stringstream s;
            s << t;
            return s.str();
        }

        std::string errorToString(ReturnCode ret);

        class TixiError : public std::exception
        {
        public:
            TixiError(ReturnCode ret);
            TixiError(ReturnCode ret, const std::string& message);

            virtual const char* what() const throw() /*override*/;

        private:
            ReturnCode m_ret;
            std::string m_message;
        };

        struct SplitXPath
        {
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

        template <typename T> T TixiGetAttribute             (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
        template <> inline std::string TixiGetAttribute<std::string>(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) { return TixiGetTextAttribute  (tixiHandle, xpath, attribute); }
        template <> inline double      TixiGetAttribute<double     >(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) { return TixiGetDoubleAttribute(tixiHandle, xpath, attribute); }
        template <> inline bool        TixiGetAttribute<bool       >(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) { return TixiGetBoolAttribute  (tixiHandle, xpath, attribute); }
        template <> inline int         TixiGetAttribute<int        >(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute) { return TixiGetIntAttribute   (tixiHandle, xpath, attribute); }

        std::string TixiGetTextElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        double      TixiGetDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        bool        TixiGetBoolElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        int         TixiGetIntElement   (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        std::time_t TixiGetTimeTElement (const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        template <typename T> T           TixiGetElement             (const TixiDocumentHandle& tixiHandle, const std::string& xpath) { /* static_assert(false, "TixiGetElement<T> cannot be used for the given T"); */ }
        template <> inline    std::string TixiGetElement<std::string>(const TixiDocumentHandle& tixiHandle, const std::string& xpath) { return TixiGetTextElement  (tixiHandle, xpath); }
        template <> inline    double      TixiGetElement<double     >(const TixiDocumentHandle& tixiHandle, const std::string& xpath) { return TixiGetDoubleElement(tixiHandle, xpath); }
        template <> inline    bool        TixiGetElement<bool       >(const TixiDocumentHandle& tixiHandle, const std::string& xpath) { return TixiGetBoolElement  (tixiHandle, xpath); }
        template <> inline    int         TixiGetElement<int        >(const TixiDocumentHandle& tixiHandle, const std::string& xpath) { return TixiGetIntElement   (tixiHandle, xpath); }
        template <> inline    std::time_t TixiGetElement<std::time_t>(const TixiDocumentHandle& tixiHandle, const std::string& xpath) { return TixiGetTimeTElement (tixiHandle, xpath); }

        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const char*        value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, double             value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, bool               value);
        void TixiSaveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, int                value);

        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const char*        value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, double             value, const std::string& format = "%g");
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, bool               value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, int                value);
        void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::time_t        value);

        void TixiCreateElement            (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        void TixiCreateElementIfNotExists (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        void TixiCreateElementsIfNotExists(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        void TixiRemoveAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
        void TixiRemoveElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);

        std::string TixiExportDocumentAsString(const TixiDocumentHandle& tixiHandle);

        void TixiRegisterNamespacesFromDocument(const TixiDocumentHandle& tixiHandle);

        template<typename T, typename ReadChildFunc>
        void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<T>& children, ReadChildFunc readChild, int minOccurs = -1, int maxOccurs = -1)
        {
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
            for (int i = 0; i < childCount; i++) {
                children.push_back(readChild(tixiHandle, xpath + "[" + to_string(i + 1) + "]"));
            }
        }

        template<typename T>
        struct PrimitiveChildReader
        {
            T operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
            {
                return TixiGetElement<T>(tixiHandle, xpath);
            }
        };

        template<typename T>
        void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<T>& children, int minOccurs = -1, int maxOccurs = -1)
        {
            TixiReadElements(tixiHandle, xpath, children, PrimitiveChildReader<T>(), minOccurs, maxOccurs);
        }

        template<typename T>
        struct ChildReader
        {
            unique_ptr<T> operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
            {
                unique_ptr<T> child = make_unique<T>();
                child->ReadCPACS(tixiHandle, xpath);
                return child;
            }
        };

        template<typename T>
        void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<unique_ptr<T> >& children, int minOccurs = -1, int maxOccurs = -1)
        {
            TixiReadElements(tixiHandle, xpath, children, ChildReader<T>(), minOccurs, maxOccurs);
        }

        template<typename T, typename Parent>
        struct ChildWithParentReader
        {
            ChildWithParentReader(Parent* p) : m_p(p) {}

            unique_ptr<T> operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
            {
                unique_ptr<T> child = make_unique<T>(m_p);
                child->ReadCPACS(tixiHandle, xpath);
                return child;
            }

        private:
            Parent* m_p;
        };

        template<typename T, typename Parent>
        void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<unique_ptr<T> >& children, Parent* parent, int minOccurs = -1, int maxOccurs = -1)
        {
            TixiReadElements(tixiHandle, xpath, children, ChildWithParentReader<T, Parent>(parent), minOccurs, maxOccurs);
        }

        template<typename T, typename WriteChildFunc>
        void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<T>& children, WriteChildFunc writeChild)
        {
            const SplitXPath& sp = splitXPath(xpath);

            // get number of children
            const int childCount = TixiGetNamedChildrenCount(tixiHandle, xpath);

            // test if we have children to write
            if (children.size() > 0) {
                // iteratore over all child nodes
                for (std::size_t i = 0; i < children.size(); i++) {
                    // if child node does not exist, create it
                    const std::string& childPath = xpath + "[" + to_string(i + 1) + "]";
                    if (!TixiCheckElement(tixiHandle, childPath)) {
                        TixiCreateElement(tixiHandle, xpath);
                    }

                    // write child node
                    writeChild(tixiHandle, childPath, children[i]);
                }
            }
            
            // delete old children which where not overwritten
            for (std::size_t i = children.size() + 1; i <= static_cast<std::size_t>(childCount); i++) {
                TixiRemoveElement(tixiHandle, xpath + "[" + to_string(i) + "]");
            }
            
            // remove parent node if there are no child nodes
            if(children.size() == 0) {
                TixiRemoveElement(tixiHandle, sp.parentXPath);
            }
        }

        template<typename T>
        struct PrimitiveChildWriter
        {
            void operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const T& child) const
            {
                TixiSaveElement(tixiHandle, xpath, child);
            }
        };

        template<typename T>
        void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<T>& children)
        {
            TixiSaveElements(tixiHandle, xpath, children, PrimitiveChildWriter<T>());
        }

        template<typename T>
        struct ChildWriter
        {
            void operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const unique_ptr<T>& child) const
            {
                child->WriteCPACS(tixiHandle, xpath);
            }
        };

        template<typename T>
        void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<unique_ptr<T> >& children)
        {
            TixiSaveElements(tixiHandle, xpath, children, ChildWriter<T>());
        }
    }
}