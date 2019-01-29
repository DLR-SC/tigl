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

#ifndef TIGL_CPACSTREEVIEW_H
#define TIGL_CPACSTREEVIEW_H

#include "CPACSAbstractModel.h"
#include "CPACSTree.h"
#include <QObject>
#include <QTreeView>

/**
 * @brief Main class to manage the tree view interface.
 *
 * It holds the cpcr::CPACSTree that contains the cpacs structure of the open
 * file . It holds the QTreeView that display the tree in the GUI. It holds the
 * CPACSAbstractModel that prepare the CPACSTree for the QTreeView. It holds the
 * QItemSelectionModel that emit a signal when a new element is selected in the
 * QTreeView.
 *
 * The goal of this class is to articulate all the previous components together.
 * It has one function to display a new tree based on a TixiHandle.
 * It has one function to clear the tree.
 * It has one function to update the tree based on current TixiHandle.
 * It emit a signal when a new element in the tree is selected.
 *
 * @author Malo Drougard
 */
class CPACSTreeView : public QObject
{

    Q_OBJECT

signals:

    void newSelectedTreeItem(cpcr::CPACSTreeItem*);

private slots:

    void onSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection);

public:
    CPACSTreeView(QTreeView* view);

    /**
   * Clear the displayed tree and delete the CPACSTree data
   */
    void clear();

    /**
   * Build the new tree based on the TixiHandle and update the model and the
   * display
   * @param handle : the TixiHandle used to retrieve the cpacs data
   * @param root :where the tree need to start (xpath)
   */
    void displayNewTree(TixiDocumentHandle handle, std::string root);

    /**
   * Rebuild the current tree based on the tixi data
   * @remark: the internal tixi handle remain the same
   */
    void refresh();

private:
    cpcr::CPACSTree tree;
    CPACSAbstractModel* model;
    QTreeView* treeView;
    QItemSelectionModel* selectionModel;
};

#endif // TIGL_CPACSTREEVIEW_H
