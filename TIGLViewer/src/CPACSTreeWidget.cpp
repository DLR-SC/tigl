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

    model = new CPACSAbstractModel(nullptr);
    ui->treeView->setModel(model);
    selectionModel = ui->treeView->selectionModel();
    // backupSelectedUID = "";

    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
            SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

CPACSTreeWidget::~CPACSTreeWidget()
{
    delete ui;
}

void CPACSTreeWidget::onSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{

    if (model->isValid()) {
        cpcr::CPACSTreeItem* newSelectedItem = model->getItemFromSelection(newSelection);
        emit newSelectedTreeItem(newSelectedItem);
    }
    else {
        LOG(WARNING) << "CPACSTreeWidget: onSelectionChanged called but no valid model is set" << std::endl;
    }
}

void CPACSTreeWidget::clear()
{
    model->disconnectInternalTree();
    tree.clean();
}

void CPACSTreeWidget::displayNewTree(TixiDocumentHandle handle, std::string root)
{
    tree.build(handle, root);
    model->resetInternalTree(&tree);
}

void CPACSTreeWidget::refresh()
{
    model->disconnectInternalTree();
    tree.reload();
    model->resetInternalTree(&tree);
}
