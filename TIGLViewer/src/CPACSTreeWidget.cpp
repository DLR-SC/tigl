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
    filterModel->setExpertView(ui->expertViewCheckBox->isChecked());

    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
            SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // connect the expert check box to its effect
    connect(ui->expertViewCheckBox, SIGNAL(toggled(bool)), this, SLOT(setExpertView(bool)));

    connect(ui->searchLineEdit, SIGNAL(textEdited(const QString)), this, SLOT(setNewSearch(const QString)));
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

    selectionModel->blockSignals(blockValue);

    if (!newText.isEmpty()) {
        ui->treeView->expandAll();
    }
}

void CPACSTreeWidget::setExpertView(bool value)
{
    // to avoid that on selectionChanged is called during the transformation of the tree
    bool blockValue = selectionModel->signalsBlocked();
    selectionModel->blockSignals(true);

    filterModel->setExpertView(value);

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
}

void CPACSTreeWidget::refresh()
{
    filterModel->disconnectInternalTree();
    tree.reload();
    filterModel->resetInternalTree(&tree);
}
