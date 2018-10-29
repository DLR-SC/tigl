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
            const std::string childXPath = xpath + "[" + internal::to_string(i + 1) + "]";
            try {
                children.push_back(readChild(tixiHandle, childXPath));
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
        tigl::unique_ptr<T> operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
        {
            tigl::unique_ptr<T> child = tigl::make_unique<T>();
            child->ReadCPACS(tixiHandle, xpath);
            return child;
        }
    };

    template<typename T>
    void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<tigl::unique_ptr<T> >& children, int minOccurs = -1, int maxOccurs = -1)
    {
        TixiReadElements(tixiHandle, xpath, children, ChildReader<T>(), minOccurs, maxOccurs);
    }

    template<typename T, typename Arg1>
    struct ChildWithArgsReader1
    {
        ChildWithArgsReader1(Arg1* arg1) : m_arg1(arg1) {}

        tigl::unique_ptr<T> operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
        {
            tigl::unique_ptr<T> child = tigl::make_unique<T>(m_arg1);
            child->ReadCPACS(tixiHandle, xpath);
            return child;
        }

    private:
        Arg1* m_arg1;
    };

    template<typename T, typename Arg1>
    void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<tigl::unique_ptr<T> >& children, Arg1* arg1, int minOccurs = -1, int maxOccurs = -1)
    {
        TixiReadElements(tixiHandle, xpath, children, ChildWithArgsReader1<T, Arg1>(arg1), minOccurs, maxOccurs);
    }

    template<typename T, typename Arg1, typename Arg2>
    struct ChildWithArgsReader2
    {
        ChildWithArgsReader2(Arg1* arg1, Arg2* arg2) : m_arg1(arg1), m_arg2(arg2) {}

        tigl::unique_ptr<T> operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
        {
            tigl::unique_ptr<T> child = tigl::make_unique<T>(m_arg1, m_arg2);
            child->ReadCPACS(tixiHandle, xpath);
            return child;
        }

    private:
        Arg1* m_arg1;
        Arg2* m_arg2;
    };

    template<typename T, typename Arg1, typename Arg2>
    void TixiReadElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, std::vector<tigl::unique_ptr<T> >& children, Arg1* arg1, Arg2* arg2, int minOccurs = -1, int maxOccurs = -1)
    {
        TixiReadElements(tixiHandle, xpath, children, ChildWithArgsReader2<T, Arg1, Arg2>(arg1, arg2), minOccurs, maxOccurs);
    }

    template<typename T, typename WriteChildFunc>
    void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<T>& children, WriteChildFunc writeChild)
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
                writeChild(tixiHandle, childPath, children[i]);
            }
        }
            
        // delete old children which where not overwritten
        for (std::size_t i = children.size() + 1; i <= static_cast<std::size_t>(childCount); i++) {
            TixiRemoveElement(tixiHandle, xpath + "[" + internal::to_string(children.size() + 1) + "]");
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
        void operator()(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const tigl::unique_ptr<T>& child) const
        {
            child->WriteCPACS(tixiHandle, xpath);
        }
    };

    template<typename T>
    void TixiSaveElements(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::vector<tigl::unique_ptr<T> >& children)
    {
        TixiSaveElements(tixiHandle, xpath, children, ChildWriter<T>());
    }
}
