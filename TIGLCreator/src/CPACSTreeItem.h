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

#ifndef TIGL_CPACSTREEITEM_H
#define TIGL_CPACSTREEITEM_H

#include <string>
#include <vector>

#include "XPathParser.h"

namespace cpcr
{

class CPACSTree;

/**
 * @brief CPACSTree node element.
 *
 * This class represents a node in a CPACSTree.
 * It holds information of the node such as the UID, the XPATH, the tixi Index, its
 * children and parents.
 * It also implements functions to simplify the access to
 * its children or parents like "getChildByUid()" or "getParentOfType()"
 *
 * @see CPACSTree
 */
class CPACSTreeItem
{

public:
    // should be only used to create the root
    CPACSTreeItem(CPACSTree* tree, std::string xpath, std::string cpacsType, int tixiIndex, std::string uid);
    CPACSTreeItem();

    // recursively delete its children
    ~CPACSTreeItem();

    // this should be the only way to add children
    CPACSTreeItem* addChild(std::string xpath, std::string cpacsType, int tixiIndex, std::string uid);

    CPACSTreeItem* getParent() const;

    std::vector<CPACSTreeItem*> getChildren() const;


    CPACSTreeItem* getChildByUid(std::string uid);

    bool hasParentOfType(std::string type);
    CPACSTreeItem* getParentOfType(std::string type, bool withoutWarning = false);

    std::vector<CPACSTreeItem*> findAllChildrenOfTypeRecursively(std::string type);

    /**
   *
   * @return Return its position in the children list of its parent.
   * @remark If there is no parent, it returns -1.
   */
    int positionRelativelyToParent() const;

    int getTixiIndex() const;

    std::string getType() const;

    std::string getXPath() const;

    inline std::string getUid() const
    {
        return uid;
    };

    CPACSTree* getTree() const;

    bool hasChildOfType(std::string name);

    inline bool isInitialized() const
    {
        return (xpath != "" );
    };

private:
    std::string type;
    std::string xpath;
    int tixiIdx;
    std::string uid;

    CPACSTreeItem* parent;
    std::vector<CPACSTreeItem*> children;

    // tree that the item belong
    CPACSTree* tree;
};

} // end namespace cpcr

#endif // TIGL_CPACSTREEITEM_H
