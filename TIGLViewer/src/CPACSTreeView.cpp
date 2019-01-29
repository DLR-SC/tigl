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

#include "CPACSTreeView.h"
#include "CPACSTreeItem.h"
#include "CTiglLogging.h"

CPACSTreeView::CPACSTreeView(QTreeView* view)
{

    treeView = view;
    model    = new CPACSAbstractModel(nullptr);
    treeView->setModel(model);
    selectionModel = treeView->selectionModel();
    // backupSelectedUID = "";

    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
            SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void CPACSTreeView::onSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{

    if (model->isValid()) {
        cpcr::CPACSTreeItem* newSelectedItem = model->getItemFromSelection(newSelection);
        emit newSelectedTreeItem(newSelectedItem);
    }
    else {
        LOG(WARNING) << "CPACSTreeView: onSelectionChanged called but no valid model is set" << std::endl;
    }
}

void CPACSTreeView::clear()
{
    model->disconnectInternalTree();
    tree.clean();
}

void CPACSTreeView::displayNewTree(TixiDocumentHandle handle, std::string root)
{
    tree.build(handle, root);
    model->resetInternalTree(&tree);
}

void CPACSTreeView::refresh()
{
    model->disconnectInternalTree();
    tree.reload();
    model->resetInternalTree(&tree);
}
