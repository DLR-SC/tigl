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
#include "CPACSTreeView.h"
#include "ModificatorModel.h"

namespace Ui
{
class CPACSTreeWidget;
}

/**
 * @brief Main class to manage the tree view interface.
 *
 * It holds
 *      cpcr::CPACSTree containing the cpacs structure of the open file,
 *      QTreeView that displays the tree in the GUI,
 *      CPACSFilterModel that prepares the CPACSTree for the QTreeView,
 *      QItemSelectionModel emitting a signal when a new element is selected in the QTreeView and
 *      QLineEdit and QCheckBox to filter and search in the tree.
 *
 * The goal of this class is to articulate all the previous components together.
 * It implements functions to display a new tree based on a TixiHandle, to clear the tree and
 * to update it based on current TixiHandle.
 * It emits a signal when a new element in the tree is selected and connects the filter option of the cpacsFilterModel
 *
 *
 * @author Malo Drougard
 */
class CPACSTreeWidget : public QWidget
{
    Q_OBJECT

signals:

    void newSelectedTreeItem(cpcr::CPACSTreeItem*);
    void contextMenuClosed(); // signals the CPACSTreeView to update its paintEvent
    void deleteSectionRequested(cpcr::CPACSTreeItem* item);
    void addSectionRequested(CPACSTreeView::Where where, cpcr::CPACSTreeItem* item);

public slots:
    /**
   * Removes the columns header and initializes the expert view
   */
    void refresh();

private slots:

    void onCustomContextMenuRequested(QPoint const& globalPos, CPACSTreeView::Where where, QModelIndex index);

    void onSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection);

    void setNewSearch(const QString newText);

    void setExpertView();

public:
    explicit CPACSTreeWidget(QWidget* parent = nullptr);
    ~CPACSTreeWidget();

    void SetModel(ModificatorModel* model);

    /**
   * Clear the displayed tree and delete the CPACSTree data
   */
    void clear();

    void setSelectedUID(const QString & uid);

    QString getSelectedUID();
    std::string getLastSelectedUID();

private:

    void setTreeViewColumnsDisplay();

    Ui::CPACSTreeWidget* ui;

    std::string last_selected_uid;
    CPACSFilterModel* filterModel;
    QItemSelectionModel* selectionModel;
};

#endif // CPACSTREEWIDGET_H
