// Copyright (c) 2017 RISC Software GmbH
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

#include <tixicpp.h>

#define BOOST_DATE_TIME_NO_LIB
#include <boost/date_time/posix_time/posix_time.hpp>

#include <ctime>

#include "UniquePtr.h"
#ifndef CPACS_GEN
#include "CTiglLogging.h"
#endif

// some extensions to tixi
namespace tixi
{
    inline std::time_t TixiGetTimeTElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        return boost::posix_time::to_time_t(boost::posix_time::from_iso_extended_string(TixiGetTextElement(tixiHandle, xpath)));
    }

    template<>
    inline std::time_t TixiGetElement<std::time_t>(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        return TixiGetTimeTElement(tixiHandle, xpath);
    }

    inline void TixiSaveElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::time_t value)
    {
        TixiSaveElement(tixiHandle, xpath, boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(value)));
    }


    template<typename T, typename ReadChildFunc>
    void TixiReadElementsInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<T>& children, int minOccurs, int maxOccurs, ReadChildFunc readChild)
    {
        // read number of child nodes
        const int childCount = TixiGetNamedChildrenCount(tixiHandle, xpath);

        // validate number of child nodes
        if (childCount < minOccurs) {
#ifndef CPACS_GEN
            LOG(ERROR)
                << "Not enough child nodes for element\n"
                << "xpath: " << xpath << "\n"
                << "minimum: " << minOccurs << "\n"
                << "actual: " << childCount;
#endif
        }

        if (childCount > maxOccurs) {
            // TODO: replace by exception/warning
#ifndef CPACS_GEN
            LOG(ERROR)
                << "Too many child nodes for element\n"
                << "xpath: " << xpath << "\n"
                << "maximum: " << maxOccurs << "\n"
                << "actual: " << childCount;
#endif
        }

        // read child nodes
        for (int i = 0; i < childCount; i++) {
            const std::string childXPath = xpath + "[" + internal::to_string(i + 1) + "]";
            try {
                children.push_back(readChild(childXPath));
            } catch (const std::exception& e) {
#ifdef CPACS_GEN
                throw;
#else
                LOG(ERROR) << "Failed to read element at xpath " << childXPath << ": " << e.what();
#endif
            }
        }
    }

    template<typename T>
    void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<T>& children, int minOccurs, int maxOccurs)
    {
        auto reader = [&](const std::string& childXPath) {
            return TixiGetElement<T>(tixiHandle, childXPath);
        };
        TixiReadElementsInternal(tixiHandle, xpath, children, minOccurs, maxOccurs, reader);
    }

    template<typename T, typename... ChildCtorArgs>
    void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<tigl::unique_ptr<T>>& children, int minOccurs, int maxOccurs, ChildCtorArgs&&... args)
    {
        auto reader = [&](const std::string& childXPath) {
            auto child = tigl::make_unique<T>(std::forward<ChildCtorArgs>(args)...);
            child->ReadCPACS(tixiHandle, childXPath);
            return child;
        };
        TixiReadElementsInternal(tixiHandle, xpath, children, minOccurs, maxOccurs, reader);
    }

    template<typename T, typename WriteChildFunc>
    void TixiSaveElementsInternal(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<T>& children, WriteChildFunc writeChild)
    {
        // get number of children
        const int childCount = TixiGetNamedChildrenCount(tixiHandle, xpath);

        // test if we have children to write
        if (children.size() > 0) {
            // iteratore over all child nodes
            for (std::size_t i = 0; i < children.size(); i++) {
                // if child node does not exist, create it
                const std::string& childPath = xpath + "[" + internal::to_string(i + 1) + "]";
                if (!TixiCheckElement(tixiHandle, childPath)) {
                    TixiCreateElement(tixiHandle, xpath);
                }

                // write child node
                writeChild(childPath, children[i]);
            }
        }
            
        // delete old children which where not overwritten
        for (std::size_t i = children.size() + 1; i <= static_cast<std::size_t>(childCount); i++) {
            TixiRemoveElement(tixiHandle, xpath + "[" + internal::to_string(children.size() + 1) + "]");
        }
    }

    template<typename T>
    void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<T>& children)
    {
        auto writer = [&](const std::string& childXPath, const T& child) {
            TixiSaveElement(tixiHandle, childXPath, child);
        };
        TixiSaveElementsInternal(tixiHandle, xpath, children, writer);
    }

    template<typename T>
    void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<tigl::unique_ptr<T>>& children)
    {
        auto writer = [&](const std::string& childXPath, const tigl::unique_ptr<T>& child) {
            child->WriteCPACS(tixiHandle, childXPath);
        };
        TixiSaveElementsInternal(tixiHandle, xpath, children, writer);
    }
}
