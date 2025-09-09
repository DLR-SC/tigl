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

#include "CPACSTreeItem.h"
#include <iostream>

#include "CPACSTree.h"
#include "CTiglLogging.h"

namespace cpcr
{

CPACSTreeItem::CPACSTreeItem()
{
    this->tixiIdx  = -1;
    this->type     = "";
    this->parent   = nullptr;
    this->xpath    = "";
    this->tree     = nullptr;
    this->children = std::vector<CPACSTreeItem*>();
    this->uid      = "";
}

CPACSTreeItem::CPACSTreeItem(CPACSTree* tree, std::string xpath, std::string cpacsType, int tixiIndex, std::string uid)
{
    this->tixiIdx  = tixiIndex;
    this->type     = cpacsType;
    this->parent   = nullptr;
    this->xpath    = xpath;
    this->tree     = tree;
    this->children = std::vector<CPACSTreeItem*>();
    this->uid      = uid;
}

CPACSTreeItem::~CPACSTreeItem()
{
    // recursive call of destructor
    for (std::vector<CPACSTreeItem*>::iterator it = children.begin(); it != children.end(); ++it) {
        // std::string ref =  (*it)->toString();
        delete (*it);
        // LOG(INFO) << "Deleted: " + ref;
    }
    children.clear();
}

CPACSTreeItem* CPACSTreeItem::addChild(std::string xpath, std::string cpacsType, int tixiIndex, std::string uid)
{

    CPACSTreeItem* newChild = new CPACSTreeItem(this->tree, xpath, cpacsType, tixiIndex, uid);
    newChild->parent        = this;
    children.push_back(newChild);
    return newChild;
}

CPACSTreeItem* CPACSTreeItem::getParent() const
{
    return parent;
};

std::vector<CPACSTreeItem*> CPACSTreeItem::getChildren() const
{
    return children;
};

int CPACSTreeItem::getTixiIndex() const
{
    return tixiIdx;
}

std::string CPACSTreeItem::getType() const
{
    return type;
}

std::string CPACSTreeItem::getXPath() const
{
    return xpath;
}

// todo: test this function
int CPACSTreeItem::positionRelativelyToParent() const
{
    int position          = -1;
    CPACSTreeItem* parent = getParent();

    if (parent != nullptr) {
        for (int i = 0; i < parent->getChildren().size(); i++) {
            if (parent->getChildren()[i] == this) {
                position = i;
            }
        }
    }
    return position;
}

bool CPACSTreeItem::hasChildOfType(std::string type)
{
    for (std::vector<CPACSTreeItem*>::iterator it = children.begin(); it != children.end(); ++it) {
        if ((*it)->getType() == type) {
            return true;
        }
    }
    return false;
}

CPACSTree* CPACSTreeItem::getTree() const
{
    return tree;
}

std::vector<CPACSTreeItem*> CPACSTreeItem::findAllChildrenOfTypeRecursively(std::string type)
{

    std::vector<CPACSTreeItem*> r;

    for (auto p : children) { // acces by value -> so it's a pointer to
        // CPACSTreeItem object
        std::vector<CPACSTreeItem*> temp = p->findAllChildrenOfTypeRecursively(type);
        r.insert(r.end(), temp.begin(), temp.end());
        if (p->getType() == type) {
            r.push_back(p);
        }
    }

    return r;
}

CPACSTreeItem* CPACSTreeItem::getChildByUid(std::string searchedUid)
{

    // return the child with the wanted uid if it exists
    for (auto p : children) {
        if (p->getUid() == searchedUid) {
            return p;
        }
    }

    // otherwise we continue to search
    CPACSTreeItem* r = nullptr;
    if (r == nullptr) {
        for (auto p : children) {
            r = p->getChildByUid(searchedUid);
            if (r != nullptr) {
                return r;
            }
        }
    }

    return r;
}

CPACSTreeItem* CPACSTreeItem::getParentOfType(std::string type, bool withoutWarning)
{

    CPACSTreeItem* parent = this->getParent();
    if (parent == nullptr) {
        if (!withoutWarning) {
            LOG(WARNING) << "The parent of the required type \"" << type << " \" was not found.";
        }
        return nullptr;
    }
    if (parent->getType() == type) {
        return parent;
    }

    // else recursive call
    return parent->getParentOfType(type, withoutWarning);
}

bool CPACSTreeItem::hasParentOfType(std::string type)
{
    if (getParentOfType(type, true) != nullptr) {
        return true;
    }
    return false;
}

} // namespace cpcr
