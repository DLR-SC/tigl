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

CPACSTreeWidget::CPACSTreeWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CPACSTreeWidget)
{
    ui->setupUi(this);

    filterModel = new CPACSFilterModel(nullptr);
    ui->treeView->setModel(filterModel);
    selectionModel = ui->treeView->selectionModel();
    setExpertView();
    setShowUID();

    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
            SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // connect the expert check box to its effect
    connect(ui->expertViewCheckBox, SIGNAL(toggled(bool)), this, SLOT(setExpertView()));

    connect(ui->searchLineEdit, SIGNAL(textEdited(const QString)), this, SLOT(setNewSearch(const QString)));

    connect(ui->showUIDCheckBox, SIGNAL(toggled(bool)), this, SLOT(setShowUID()));

}

CPACSTreeWidget::~CPACSTreeWidget()
{
    delete ui;
}

void CPACSTreeWidget::onSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{
    // to avoid that the tree is transformed during selection of item
    bool blockValue1 = ui->searchLineEdit->signalsBlocked();
    ui->searchLineEdit->blockSignals(true);
    bool blockValue2 = ui->expertViewCheckBox->signalsBlocked();
    ui->expertViewCheckBox->blockSignals(true);
    bool blockValue3 = ui->showUIDCheckBox->signalsBlocked();
    ui->showUIDCheckBox->blockSignals(true);

    if (filterModel->isValid()) {
        cpcr::CPACSTreeItem* newSelectedItem = filterModel->getItemFromSelection(newSelection);
        emit newSelectedTreeItem(newSelectedItem);
    }
    else {
        LOG(WARNING) << "CPACSTreeWidget: onSelectionChanged called but no valid model is set" << std::endl;
    }

    ui->searchLineEdit->blockSignals(blockValue1);
    ui->expertViewCheckBox->blockSignals(blockValue2);
    ui->showUIDCheckBox->blockSignals(blockValue3);

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

void CPACSTreeWidget::setShowUID()
{
    // to avoid that on selectionChanged is called during the transformation of the tree
    bool blockValue = selectionModel->signalsBlocked();
    selectionModel->blockSignals(true);

    bool showUID = ui->showUIDCheckBox->isChecked();
    ui->treeView->setColumnHidden(1, !showUID);
    // set the search to search also in uid iff they are visible
    filterModel->enableMatchOnUID(showUID);
    // resize the type column
    ui->treeView->resizeColumnToContents(0);
    if (ui->treeView->columnWidth(0) < 200) {
        ui->treeView->setColumnWidth(0, 200);
    }
    // as for the setNewSearch, the root of the treeView can change during this operation (I dont know why)
    setExpertView();

    selectionModel->blockSignals(blockValue);
}

void CPACSTreeWidget::clear()
{
    filterModel->disconnectInternalTree();
    tree.clean();
}

void CPACSTreeWidget::displayNewTree(TixiDocumentHandle handle, std::string root)
{
    tree.build(handle, root);
    filterModel->resetInternalTree(&tree);
    setExpertView();
    setShowUID();
}

void CPACSTreeWidget::refresh()
{
    filterModel->disconnectInternalTree();
    tree.reload();
    filterModel->resetInternalTree(&tree);
    setExpertView();
    setShowUID();
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