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

#ifndef CPACSCREATORLIBANDTESTS_CPACSTREE_H
#define CPACSCREATORLIBANDTESTS_CPACSTREE_H

#include <memory>
#include <string>
#include <vector>

#include "CPACSTreeItem.h"
#include <tixi.h>

namespace cpcr
{

typedef std::string UID;

/**
 * @brief Construct and manage a tree structure over a CPACS file trough a tixi
 * handle .
 *
 * The xml CPACS format can be represented as a tree. This class create for each
 * xml node a CPACSTreeItem starting from the given root. The access to the
 * underlying data is done by the TIXI library. So basically, this class is a a
 * tree structure that represent a subset of the Tixi data.
 * @remark The root need not to be the first element of the CPACS, but can be
 * every where, typically the "modelType" of CPACS can be chosen.
 *
 * @author Malo Drougard
 */
class CPACSTree
{

public:
    CPACSTree();
    ~CPACSTree();

    virtual void build(TixiDocumentHandle handle, std::string xpathRoot);
    inline bool isBuild()
    {
        return m_isBuild;
    }

    inline CPACSTreeItem* getRoot() const
    {
        return m_root;
    }

    /**
   * Delete all the CPACSTreeItem used by the tree and set the root to nullptr
   * @remark if another object still used some of the CPACSTreeItem it can leads
   * to segmentation fault.
   */
    void clean();

    /**
   * Rebuild the tree from the same handle with the same root ;
   */
    void reload();

protected:
    /*
   * Helper function to retrieve the uid of an element using the internal tixi
   * handle
   */
    std::string getUid(std::string xpathTarget, std::string defaultRetrunedValue = "");

    /*
   * Helper function to retrieve the number of children of an element using the
   * internal tixi handle
   */
    int getNumberOfChildren(std::string xpathObj);

    void createChildrenRecursively(CPACSTreeItem& parent);

    TixiDocumentHandle tixiHandle;

    // The xpath in cpacs file of the root element
    // We can start the tree where ever we want
    std::string rootXPath;

    CPACSTreeItem* m_root;

    bool m_isBuild;
};
} // namespace cpcr

#endif // CPACSCREATORLIBANDTESTS_CPACSTREE_H
