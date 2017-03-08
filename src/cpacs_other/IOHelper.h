/*
* Copyright (C) 2016 Airbus Defence and Space
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef IOHELPER_H
#define IOHELPER_H

#include <string>
#include <cmath>

#include "CTiglLogging.h"
#include "TixiSaveExt.h"
#include "to_string.h"

namespace tigl
{
// tag type for ReadContainerElement() when the constructed child requires the current element index as ctor argument
class ContainerElementIndexTag {};

namespace
{
    ContainerElementIndexTag ContainerElementIndex;
}

// maps any type to itself, except instances of ContainerElementIndex, which are mapped to the integer passed as argument
namespace internal
{
    // TODO (bgruber): use perfect forwarding when C++11 is available
    template <typename Arg>
    Arg replace(Arg arg, int i)
    {
        return arg;
    }
    //template <typename Arg>
    //Arg&& replace(Arg&& arg, int i) {
    //	return std::forward<Arg>(arg);
    //}

    inline int replace(ContainerElementIndexTag arg, int i)
    {
        return i;
    }
}

// creator functions and overloads for various counts of ctor argument
// TODO (bgruber): remove this mechanism when ReadContainerElement becomes a variadic template when C++11 is available
namespace internal
{
    class UnusedTag {};

    template <typename ChildType>
    ChildType* createChild(UnusedTag, UnusedTag, UnusedTag, int i)
    {
        return new ChildType();
    }

    template <typename ChildType, typename ChildCtorArg1>
    ChildType* createChild(ChildCtorArg1 arg1, UnusedTag, UnusedTag, int i)
    {
        return new ChildType(replace(arg1, i));
    }

    template <typename ChildType, typename ChildCtorArg1, typename ChildCtorArg2>
    ChildType* createChild(ChildCtorArg1 arg1, ChildCtorArg2 arg2, UnusedTag, int i)
    {
        return new ChildType(replace(arg1, i), replace(arg2, i));
    }

    template <typename ChildType, typename ChildCtorArg1, typename ChildCtorArg2, typename ChildCtorArg3>
    ChildType* createChild(ChildCtorArg1 arg1, ChildCtorArg2 arg2, ChildCtorArg2 arg3, int i)
    {
        return new ChildType(replace(arg1, i), replace(arg2, i), replace(arg3, i));
    }
}

// utitlities
namespace internal
{
    inline bool endsWith(const std::string& str, const std::string& end)
    {
        if (str.length() < end.length()) {
            return false;
        }
        return str.compare(str.length() - end.length(), end.size(), end) == 0;
    }

    inline std::string checkXPath(std::string xpath, const std::string& name)
    {
        // current xpath should end with the element we are about to read or write
        if (!endsWith(xpath, name)) {
#ifdef _DEBUG
            LOG(INFO) << "Xpath does not end with current element. element: \"" + name + "\" path \"" + xpath + "\"";
#endif
            xpath += "/" + name;
        }
        return xpath;
    }

    template<class T>
    struct remove_pointer
    {
        typedef T type;
    };

    template<class T>
    struct remove_pointer<T*>
    { 
        typedef T type;
    };
}

// TODO (bgruber): replace by variadic template when C++11 is available
template<typename Container,
    //typename... ChildCtorArgs>
    typename ChildCtorArg1, typename ChildCtorArg2, typename ChildCtorArg3>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& parentName, const std::string& childName, const std::size_t minimumChildren, Container& children,
                          //ChildCtorArgs&&... args) {
                          ChildCtorArg1 arg1, ChildCtorArg2 arg2, ChildCtorArg3 arg3)
{
    // xpath already contains parent node (parentName)

    const std::string parentPath = internal::checkXPath(xpath, parentName);
    const std::string childBasePath = parentPath + "/" + childName;

    // check if the element itself exists
    // TODO (bgruber): its kind of strange to try to read the children of elements which do not exist themselves,
    //                 i think it is better to check the element's existence before allocating it and calling ReadCPACS which than calls ReadContainerElements
    if (tixiCheckElement(tixiHandle, parentPath.c_str()) != SUCCESS) {
        LOG(WARNING) << "An element in path \"" + parentPath + "\" is missing";
        return;
    }

    // read number of child nodes
    int childCount = 0;
    if (tixiGetNamedChildrenCount(tixiHandle, parentPath.c_str(), childName.c_str(), &childCount) != SUCCESS) {
        const std::string msg = "XML error: tixiGetNamedChildrenCount failed for child name \"" + childName + "\" at path \"" + parentPath + "\"";
        LOG(ERROR) << msg;
        throw tigl::CTiglError(msg, TIGL_XML_ERROR);
    }

    // validate that there are a minimum number of child nodes
    if (static_cast<std::size_t>(childCount) < minimumChildren) {
        const std::string msg = "Missing child nodes \"" + childName + "\" at path \"" + parentPath + "\". Requiring at least " + std_to_string(minimumChildren) + " child nodes, " + std_to_string(childCount) + " are present";
        LOG(WARNING) << msg;
        //throw CTiglError(msg, TIGL_XML_ERROR); // TODO (bgruber): if this error occures, the CPACS specification is violated. However, it seems we have to support such files.
    }

    // read child nodes
    // TODO: use std::remove_pointer when C++11 is available
    typedef typename internal::remove_pointer<typename Container::value_type>::type ChildType;
    for (int i = 0; i < childCount; i++) {
        // construct a child and pass it some arguments
        //ChildType* child = new ChildType(internal::replace(std::forward<ChildCtorArgs>(args), i)...);
        ChildType* child = internal::createChild<ChildType>(arg1, arg2, arg3, i + 1);
        children.push_back(child);
        child->ReadCPACS(tixiHandle, childBasePath + "[" + std_to_string(i + 1) + "]");
    }
}

template<typename Container>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& parentName, const std::string& childName, const std::size_t minimumChildren, Container& children)
{
    ReadContainerElement(tixiHandle, xpath, parentName, childName, minimumChildren, children, internal::UnusedTag(), internal::UnusedTag(), internal::UnusedTag());
}

template<typename Container, typename ChildCtorArg1>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& parentName, const std::string& childName, const std::size_t minimumChildren, Container& children,
                          ChildCtorArg1 arg1)
{
    ReadContainerElement(tixiHandle, xpath, parentName, childName, minimumChildren, children, arg1, internal::UnusedTag(), internal::UnusedTag());
}

template<typename Container, typename ChildCtorArg1, typename ChildCtorArg2>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& parentName, const std::string& childName, const std::size_t minimumChildren, Container& children,
    ChildCtorArg1 arg1, ChildCtorArg2 arg2)
{
    ReadContainerElement(tixiHandle, xpath, parentName, childName, minimumChildren, children, arg1, arg2, internal::UnusedTag());
}

// TODO (bgruber): replace by variadic when C++11 is available
template<typename Container,
    //typename... ChildCtorArgs>
    typename ChildCtorArg1, typename ChildCtorArg2, typename ChildCtorArg3>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& childName, const std::size_t minimumChildren, Container& children,
                          //ChildCtorArgs&&... args) {
                          ChildCtorArg1 arg1, ChildCtorArg2 arg2, ChildCtorArg3 arg3)
{
    ReadContainerElement(tixiHandle, xpath, childName + "s", childName, minimumChildren, children,
        //std::forward<ChildCtorArgs>(args)...
        arg1, arg2, arg3
    );
}

template<typename Container>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& childName, const std::size_t minimumChildren, Container& children)
{
    ReadContainerElement(tixiHandle, xpath, childName + "s", childName, minimumChildren, children, internal::UnusedTag(), internal::UnusedTag(), internal::UnusedTag());
}

template<typename Container, typename ChildCtorArg1>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& childName, const std::size_t minimumChildren, Container& children, ChildCtorArg1 arg1)
{
    ReadContainerElement(tixiHandle, xpath, childName + "s", childName, minimumChildren, children, arg1, internal::UnusedTag(), internal::UnusedTag());
}

template<typename Container, typename ChildCtorArg1, typename ChildCtorArg2>
void ReadContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& childName, const std::size_t minimumChildren, Container& children, ChildCtorArg1 arg1, ChildCtorArg2 arg2)
{
    ReadContainerElement(tixiHandle, xpath, childName + "s", childName, minimumChildren, children, arg1, arg2, internal::UnusedTag());
}


// distinguish the element type of the children's container
namespace internal
{
    // version for std::vector<Child*>
    template<typename Child>
    Child* getChild(Child* child)
    {
        return child;
    }

    // version for std::map<Key, Child*>
    template<typename Child, typename Key>
    Child* getChild(const std::pair<Key, Child*>& pair)
    {
        return pair.second;
    }
}

template<typename Container>
void WriteContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& parentName, const std::string& childName, const Container& children)
{
    // xpath already contains parent node (name)
    const std::string parentPath = internal::checkXPath(xpath, parentName);
    const std::string childBasePath = parentPath + "/" + childName;

    // get number of children
    int childCount = 0;
    tixiGetNamedChildrenCount(tixiHandle, parentPath.c_str(), childName.c_str(), &childCount); // this call is allowed to fail if the element at parentPath might not exist

    // test if we have children to write
    typedef typename Container::const_iterator Iterator;
    Iterator it = children.begin();
    Iterator end = children.end();

    const int count = static_cast<int>(std::distance(it, end));
    if (count > 0) {
        // it the container node does not exist, create it
        if (tixiCheckElement(tixiHandle, parentPath.c_str()) == ELEMENT_NOT_FOUND) {
            const std::string pathBeforeParentElement = parentPath.substr(0, parentPath.size() - (parentName.size() + 1));
            if (tixiCreateElement(tixiHandle, pathBeforeParentElement.c_str(), parentName.c_str()) != SUCCESS) {
                std::string msg = "XML error: tixiCreateElement failed creating element \"" + parentName + "\" at path \"" + pathBeforeParentElement + "\"";
                LOG(ERROR) << msg;
                throw CTiglError(msg, TIGL_XML_ERROR);
            }
        }

        // iteratore over all child nodes
        int nodeIndex = 1;
        for (; it != end; ++it) {
            // if child node does not exist, create it
            const std::string childPath = childBasePath + "[" + std_to_string(nodeIndex) + "]";
            if (tixiCheckElement(tixiHandle, childPath.c_str()) == ELEMENT_NOT_FOUND) {
                if (tixiCreateElement(tixiHandle, parentPath.c_str(), childName.c_str()) != SUCCESS) {
                    std::string msg = "XML error: tixiCreateElement failed creating element \"" + childName + "\" at path \"" + parentPath + "\"";
                    LOG(ERROR) << msg;
                    throw CTiglError(msg, TIGL_XML_ERROR);
                }
            }

            // write child node
            internal::getChild(*it)->WriteCPACS(tixiHandle, childPath);
            nodeIndex++;
        }

        // delete old children which where not overwritten
        for (int i = count + 1; i <= childCount; i++) {
            tixiRemoveElement(tixiHandle, (childBasePath + "[" + std_to_string(count + 1) + "]").c_str());
        }
    }
    else {
        // parent node must not exist if there are no child nodes
        tixiRemoveElement(tixiHandle, parentPath.c_str());
    }
}

template<typename Container>
void WriteContainerElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& childName, const Container& children)
{
    WriteContainerElement(tixiHandle, xpath, childName + "s", childName, children);
}
}
#endif
