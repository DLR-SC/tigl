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

#include "CPACSTreeWidget.h"
#include "ui_CPACSTreeWidget.h"
#include "CTiglLogging.h"

#include<QMenu>
#include <QDebug>

CPACSTreeWidget::CPACSTreeWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CPACSTreeWidget)
{
    ui->setupUi(this);

    filterModel = new CPACSFilterModel(nullptr);
    ui->treeView->setModel(filterModel);
    selectionModel = ui->treeView->selectionModel();


    // set the search to search also in uid
    filterModel->enableMatchOnUID(true);

    setTreeViewColumnsDisplay();
    setExpertView();
    
    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
            SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // connect the expert check box to its effect
    connect(ui->expertViewCheckBox, SIGNAL(toggled(bool)), this, SLOT(setExpertView()));

    connect(ui->searchLineEdit, SIGNAL(textEdited(const QString)), this, SLOT(setNewSearch(const QString)));

    connect(ui->treeView, &CPACSTreeView::customContextMenuRequested, this, &CPACSTreeWidget::onCustomContextMenuRequested);

}

CPACSTreeWidget::~CPACSTreeWidget()
{
    delete ui;
}

void CPACSTreeWidget::onCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    if (!index.isValid()){
        return;
    }

    QModelIndex parent = index.parent();
    if (!parent.isValid() || !parent.data(Qt::UserRole).toBool()) {
        return;
    }

    QRect rect = ui->treeView->visualRect(index);
    cpcr::CPACSTreeItem* item = filterModel->getItem(index);

    int item_idx = item->positionRelativelyToParent();
    cpcr::CPACSTreeItem* item_parent = item->getParent();
    auto siblings = item_parent->getChildren();

    const int margin = 4;
    QPoint globalPos = ui->treeView->viewport()->mapToGlobal(pos);

    if (pos.y() < rect.top() + margin) {
        if (item_idx == 0 ) {
            qDebug() << "Before " << item->getUid().c_str();
        } else {
            qDebug() << "Between " << siblings[item_idx-1]->getUid().c_str() << " and " << item->getUid().c_str();
        }
        showInsertMenu(globalPos);//, *above, index);
    } else if (pos.y() > rect.bottom() - margin) {
        if (item_idx < siblings.size()-1 ) {
            qDebug() << "Between " << item->getUid().c_str() << " and " << siblings[item_idx+1]->getUid().c_str();
        } else {
            qDebug() << "After " << item->getUid().c_str();
        }
        showInsertMenu(globalPos);//, index, *below);
    } else {
        // right-click inside an item -> normal item menu
        QMenu menu;
        menu.addAction(QStringLiteral("Item action: %1").arg(index.data(0).toString()));
        menu.exec(globalPos);
    }
}

void CPACSTreeWidget::showInsertMenu(const QPoint &globalPos)//, QTreeWidgetItem* above, QTreeWidgetItem* below)
{
    QMenu menu;
    QAction *act = menu.addAction(QStringLiteral("Add element between"));
    connect(act, &QAction::triggered, this, [this/*, above, below*/]() {
        auto *newItem = new QTreeWidgetItem();
        newItem->setText(0, QStringLiteral("New Element"));

//        QModelIndex parent = above.isValid() ? above.parent() : below.parent();
//        if (parent.isValid()) {
//            int idx = parent.indexOfChild(below);
//            parent->insertChild(idx, newItem);
//        } else {
//            int idx = indexOfTopLevelItem(below);
//            insertTopLevelItem(idx, newItem);
//        }
    });
    menu.exec(globalPos);
}


void CPACSTreeWidget::onSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{
    // to avoid that the tree is transformed during selection of item
    bool blockValue1 = ui->searchLineEdit->signalsBlocked();
    ui->searchLineEdit->blockSignals(true);
    bool blockValue2 = ui->expertViewCheckBox->signalsBlocked();
    ui->expertViewCheckBox->blockSignals(true);

    if (filterModel->isValid()) {
        cpcr::CPACSTreeItem* newSelectedItem = filterModel->getItemFromSelection(newSelection);
        emit newSelectedTreeItem(newSelectedItem);
    }
    else {
        LOG(WARNING) << "CPACSTreeWidget: onSelectionChanged called but no valid model is set" << std::endl;
    }

    ui->searchLineEdit->blockSignals(blockValue1);
    ui->expertViewCheckBox->blockSignals(blockValue2);
}

void CPACSTreeWidget::setNewSearch(const QString newText)
{

    // to avoid that on selectionChanged is called during the transformation of the tree
    bool blockValue = selectionModel->signalsBlocked();
    selectionModel->blockSignals(true);

    filterModel->setSearchPattern(newText);

    // If we do not put this line, once we get a empty match
    // -> the tree change the root index, why?
    setExpertView();

    selectionModel->blockSignals(blockValue);

    if (!newText.isEmpty()) {
        ui->treeView->expandAll();
    }
}

void CPACSTreeWidget::setExpertView()
{
    bool expertMode = ui->expertViewCheckBox->isChecked();

    // to avoid that on selectionChanged is called during the transformation of the tree
    bool blockValue = selectionModel->signalsBlocked();
    selectionModel->blockSignals(true);

    filterModel->setExpertView(expertMode);
    // we change the root of the qtree view depending the view mode (expert view show also profiles)
    if (expertMode == false) {
        ui->treeView->setRootIndex(filterModel->getAircraftModelRoot());
    }
    else {
        ui->treeView->setRootIndex(QModelIndex()); // the empty index is the root by default.
    }

    selectionModel->blockSignals(blockValue);
}

void CPACSTreeWidget::setTreeViewColumnsDisplay()
{
    // hide uid column
    ui->treeView->setColumnHidden(1, true);
    // hide type column
    ui->treeView->setColumnHidden(2, true);
}

void CPACSTreeWidget::clear()
{
    if (filterModel) {
        filterModel->disconnectInternalTree();
    }
    tree.clean();
}

void CPACSTreeWidget::displayNewTree(TixiDocumentHandle handle, std::string root)
{
    tree.build(handle, root);
    filterModel->resetInternalTree(&tree);
    setTreeViewColumnsDisplay();
    setExpertView();
}

void CPACSTreeWidget::refresh()
{
    filterModel->disconnectInternalTree();
    tree.reload();
    filterModel->resetInternalTree(&tree);
    setTreeViewColumnsDisplay();
    setExpertView();
}

void CPACSTreeWidget::setSelectedUID(const QString& uid)
{
    QModelIndex idxToSelect = filterModel->getIdxForUID(uid);
    selectionModel->setCurrentIndex(idxToSelect, QItemSelectionModel::Select);
}

QString CPACSTreeWidget::getSelectedUID()
{
    QModelIndex currentIdx = selectionModel->currentIndex();
    return filterModel->getUidForIdx(currentIdx);
}
