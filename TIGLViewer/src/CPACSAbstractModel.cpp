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

#include "CPACSAbstractModel.h"
#include "CCPACSWing.h"
#include "CCPACSWingSection.h"
#include "CCPACSWingSectionElement.h"
#include "CCPACSWings.h"
#include "TIGLViewerWindow.h"

#include "CCPACSWingSegment.h"
#include "TIGLViewerException.h"

CPACSAbstractModel::CPACSAbstractModel(cpcr::CPACSTree* tree, QObject* parent)
    : QAbstractItemModel(parent)
{
    this->tree = tree;
}

CPACSAbstractModel::~CPACSAbstractModel()
{
}

QVariant CPACSAbstractModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (isValid() && orientation == Qt::Horizontal && role == Qt::DisplayRole) {

        if (section == 0) {
            return "type";
        }
        else if (section == 1) {
            return "UID";
        }
        else {
            return "invalid";
        }
    }

    return QVariant();
}

QVariant CPACSAbstractModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    cpcr::CPACSTreeItem* item = getItem(index);
    QVariant data;
    if (index.column() == 0) {
        data = QString(item->getType().c_str());
    }
    else if (index.column() == 1) {
        data = QString(item->getUid().c_str());
    }
    else {
        data = QVariant();
    }

    return data;
}

// return the index of the parent
QModelIndex CPACSAbstractModel::parent(const QModelIndex& index) const
{
    if (!isValid()) {
        return QModelIndex();
    }
    cpcr::CPACSTreeItem* childItem  = getItem(index);
    cpcr::CPACSTreeItem* parentItem = childItem->getParent();

    return getIndex(parentItem, 0);
}

// Count the number of children of this item
int CPACSAbstractModel::rowCount(const QModelIndex& idx) const
{
    if (!isValid()) {
        return 0;
    }

    cpcr::CPACSTreeItem* item = getItem(idx);
    return item->getChildren().size();
}

int CPACSAbstractModel::columnCount(const QModelIndex& idx) const
{

    if (!isValid()) {
        return 0;
    }

    return 2; // we have for values in a tree item
}

// return the QModelindex from a parent and row and column information
QModelIndex CPACSAbstractModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!isValid()) {
        return QModelIndex();
    }
    cpcr::CPACSTreeItem* parentItem = getItem(parent); // return root if parent is empty or invalid
    cpcr::CPACSTreeItem* childItem  = parentItem->getChildren()[row];

    if (childItem) { // case where the child is not a null pointer
        return createIndex(row, column, childItem);
    }
    else {
        return QModelIndex();
    }
}

QModelIndex CPACSAbstractModel::getIdxForUID(std::string uid)
{
    if (!isValid() || uid == "") {
        return QModelIndex();
    }
    else {
        cpcr::CPACSTreeItem* item = tree->getRoot()->getChildByUid(uid);
        return getIndex(item, 0);
    }
}

std::string CPACSAbstractModel::getUidForIdx(QModelIndex idx)
{
    if (!isValid()) {
        return "";
    }
    else {
        return getItem(idx)->getUid();
    }
}

QModelIndex CPACSAbstractModel::getIndex(cpcr::CPACSTreeItem* item, int column) const
{
    if (!isValid() || item == tree->getRoot() || item == nullptr) {
        return QModelIndex(); // We use empty index for the root
    }

    int row = item->positionRelativelyToParent();
    return createIndex(row, column, item);
}

cpcr::CPACSTreeItem* CPACSAbstractModel::getItem(QModelIndex index) const
{
    if (!isValid()) {
        return nullptr;
    }
    // Internal identifier is the item pointer
    if (index.isValid()) {
        cpcr::CPACSTreeItem* item = static_cast<cpcr::CPACSTreeItem*>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return tree->getRoot(); // empty index is the root
}

cpcr::CPACSTreeItem* CPACSAbstractModel::getItemFromSelection(const QItemSelection& newSelection)
{
    if (!isValid()) {
        throw TIGLViewerException("CPACSAbstractModel: getItemWithError called but "
                                  "the model is not valid!");
    }

    cpcr::CPACSTreeItem* item = getItem(newSelection.indexes().at(0));

    return item;
}

bool CPACSAbstractModel::isValid() const
{
    return (tree != nullptr && tree->isBuild());
}

void CPACSAbstractModel::resetInternalTree(cpcr::CPACSTree* newTree)
{
    QAbstractItemModel::beginResetModel(); // inform that internal data are about
        // to change
    tree = newTree;
    QAbstractItemModel::endResetModel();
}

void CPACSAbstractModel::disconnectInternalTree()
{
    // notify the tree view? yes I think so
    QAbstractItemModel::beginResetModel();
    tree = nullptr;
    QAbstractItemModel::endResetModel();
}