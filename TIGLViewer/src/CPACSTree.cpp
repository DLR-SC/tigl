/*
 * Copyright (C) 2018 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#include "CPACSTree.h"

#include <list>
#include <map>

#include "CTiglLogging.h"
#include "TIGLViewerException.h"
//#include "UniqueXPath.h"
#include "XPathParser.h"

#include <tixicpp.h>

namespace cpcr
{

CPACSTree::CPACSTree()
{
    rootXPath = "";
    m_isBuild = false;
    m_root = nullptr;
}

void CPACSTree::build(TixiDocumentHandle handle, std::string rootXPath)
{

    // TODO check if the given handle is valid

    tixiHandle = handle;
    // clean the old one if present
    if (isBuild()) {
        clean();
    }

    this->rootXPath = rootXPath;

    if (!tixi::TixiCheckElement(tixiHandle, rootXPath )) {
        throw TIGLViewerException("CPACSTree: The given unique xpath \"" + rootXPath +
                                  "\" seems not to be valid");
    }

    std::string rootType = XPathParser::GetLastNodeType(rootXPath);
    int rootIndex        = XPathParser::GetLastNodeIndex(rootXPath); // todo what happend if the root is given by uid
    std::string rootUid  = getUid(rootXPath, "");

    m_root = std::unique_ptr<CPACSTreeItem>(new CPACSTreeItem(this, rootXPath, rootType, rootIndex, rootUid));

    // recursive call to create element
    createChildrenRecursively(*m_root);
    m_isBuild = true;
}

void CPACSTree::createChildrenRecursively(CPACSTreeItem& parent)
{

    std::string parentXpath = parent.getXPath();

    int childrenCount = getNumberOfChildren(parentXpath);

    int tixiRet = -1;

    // find out which type of children this element has
    char* childType = NULL;
    std::list<std::string> types;
    for (int i = 1; i <= childrenCount; i++) {
        tixiRet = tixiGetChildNodeName(tixiHandle, parentXpath.c_str(), i, &childType);
        // TODO: FIND HOW TIXI SHOULD MANAGE THE NONE NODE ELEMENT LIKE TEXT
        if (childType[0] == '#') {
            // std::cout << childType << std::endl;
        }
        else {
            types.push_back(childType);
        }
    }
    types.unique();

    // for each child, create the associated CPACSItem and call this function
    std::string newXPath;
    int counterChildrenOfSameType = 0;
    for (std::string currentType : types) {

        tixiRet = tixiGetNamedChildrenCount(tixiHandle, parentXpath.c_str(), currentType.c_str(),
                                            &counterChildrenOfSameType);
        if (counterChildrenOfSameType == 1) {
            newXPath = XPathParser::AddNodeAtEnd(parentXpath, currentType);
            CPACSTreeItem* newChildren = parent.addChild(newXPath, currentType, 1, getUid(newXPath));
            createChildrenRecursively(*newChildren); // Recursive call
        }
        else {
            for (int idx = 1; idx <= counterChildrenOfSameType; idx++) {
                newXPath = XPathParser::AddNodeAtEnd( parentXpath, (currentType + "[" + std::to_string(idx) + "]" ) );
                CPACSTreeItem* newChildren = parent.addChild(newXPath, currentType, idx, getUid(newXPath));
                createChildrenRecursively(*newChildren); // Recursive call
            }
        }
    }

    return;
}

CPACSTree::~CPACSTree()
{
    clean();
}

void CPACSTree::clean()
{
    m_isBuild = false;
    m_root = nullptr;
}

std::string CPACSTree::getUid(std::string target, std::string defaultRetrunedValue)
{

    std::string r = defaultRetrunedValue;

    try {
        r = tixi::TixiGetTextAttribute(tixiHandle, target.c_str(), "uID");
    }
    catch (tixi::TixiError error) {
        // we expect that some times this function is call on element that have no
        // UID
        if (error.returnCode() != ATTRIBUTE_NOT_FOUND) {
            throw error;
        }
    }

    return r;
}

int CPACSTree::getNumberOfChildren(std::string xpathObj)
{
    int childrenCount = -1;
    int tixiRet       = -1;
    tixiRet           = tixiGetNumberOfChilds(tixiHandle, xpathObj.c_str(), &childrenCount);
    return childrenCount;
}

void CPACSTree::reload()
{
    std::string rootXPath = getRoot()->getXPath();
    this->build(tixiHandle, rootXPath);
}

} // end namespace cpcr
