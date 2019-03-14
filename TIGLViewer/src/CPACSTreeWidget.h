/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef CPACSTREEWIDGET_H
#define CPACSTREEWIDGET_H

#include <QWidget>
#include <QTreeView>
#include "CPACSTree.h"
#include "CPACSFilterModel.h"

namespace Ui
{
class CPACSTreeWidget;
}

/**
 * @brief Main class to manage the tree view interface.
 *
 * It holds the cpcr::CPACSTree that contains the cpacs structure of the open
 * file . It holds the QTreeView that display the tree in the GUI. It holds the
 * CPACSFilterModel that prepare the CPACSTree for the QTreeView. It holds the
 * QItemSelectionModel that emit a signal when a new element is selected in the
 * QTreeView. It holds QLineEdit and QCheckBox to filter and search in the tree.
 *
 * The goal of this class is to articulate all the previous components together.
 * It has one function to display a new tree based on a TixiHandle.
 * It has one function to clear the tree.
 * It has one function to update the tree based on current TixiHandle.
 * It emits a signal when a new element in the tree is selected.
 * It connect the filter option of the cpacsFilterModel
 *
 *
 * @author Malo Drougard
 */
class CPACSTreeWidget : public QWidget
{
    Q_OBJECT

signals:

    void newSelectedTreeItem(cpcr::CPACSTreeItem*);

private slots:

    void onSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection);

    void setNewSearch(const QString newText);

    void setExpertView();

public:
    explicit CPACSTreeWidget(QWidget* parent = nullptr);
    ~CPACSTreeWidget();

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

    void setTreeViewColumnsDisplay();

    Ui::CPACSTreeWidget* ui;

    cpcr::CPACSTree tree;
    CPACSFilterModel* filterModel;
    QItemSelectionModel* selectionModel;
};

#endif // CPACSTREEWIDGET_H
