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

#include "ModificatorManager.h"
#include "CTiglUIDManager.h"
#include "TIGLViewerUndoCommands.h"

ModificatorManager::ModificatorManager(CPACSTreeWidget* treeWidget,
                                       ModificatorContainerWidget* modificatorContainerWidget,
                                       QUndoStack* undoStack)
{

    doc          = nullptr;
    this->treeWidget = treeWidget;
    this->modificatorContainerWidget = modificatorContainerWidget;
    this->myUndoStack = undoStack;

    // signals:
    connect(treeWidget, SIGNAL(newSelectedTreeItem(cpcr::CPACSTreeItem*)), this,
            SLOT(dispatch(cpcr::CPACSTreeItem*)));

    connect(modificatorContainerWidget, SIGNAL(undoCommandRequired()), this, SLOT(createUndoCommand()) );
}

void ModificatorManager::setCPACSConfiguration(TIGLViewerDocument* newDoc)
{

    doc = newDoc;
    updateTree();
    modificatorContainerWidget->setNoInterfaceWidget();
    if (configurationIsSet()) {
        // when doc is destroy this connection is also destroy
        connect(doc, SIGNAL(documentUpdated(TiglCPACSConfigurationHandle)), this, SLOT(updateTree()));
    }
}

void ModificatorManager::dispatch(cpcr::CPACSTreeItem* item)
{

    if ((!configurationIsSet()) || (!item->isInitialized())) {

        modificatorContainerWidget->hideAllSpecializedWidgets();
        LOG(ERROR) << "MODIFICATOR MANAGER IS NOT READY";
    }
    else if (item->getType() == "transformation") {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSTransformation& transformation =
            uidManager.ResolveObject<tigl::CCPACSTransformation>(item->getUid());
        modificatorContainerWidget->setTransformationModificator(transformation);
    }
    else if (item->getType() == "fuselage") {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSFuselage& fuselage    = uidManager.ResolveObject<tigl::CCPACSFuselage>(item->getUid());
        modificatorContainerWidget->setFuselageModificator(fuselage);
    }
    else if (item->getType() == "wing") {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSWing& wing            = uidManager.ResolveObject<tigl::CCPACSWing>(item->getUid());
        modificatorContainerWidget->setWingModificator(wing);
    }
    else {
        modificatorContainerWidget->setNoInterfaceWidget();
        LOG(INFO) << "MODIFICATOR MANAGER: item not suported";
    }
}

void ModificatorManager::createUndoCommand()
{
    if (configurationIsSet()) {
        QUndoCommand* command = new TiGLViewer::ModifyTiglObject((*doc));
        myUndoStack->push(command);
    }
    else {
        LOG(ERROR) << "ModificatorManager::createUndoCommand: Called but no document is set!" << std::endl;
    }
    emit configurationEdited();
}

void ModificatorManager::updateTree()
{
    QString selectedUID = treeWidget->getSelectedUID();
    if (configurationIsSet()) {
        // TODO multiple model in file case
        std::string rootXPath = "/cpacs/vehicles";
        treeWidget->displayNewTree(doc->GetConfiguration().GetTixiDocumentHandle(), rootXPath);
        treeWidget->setSelectedUID(selectedUID); // to reset the display in a similar state as before the tree update
    }
    else {
        treeWidget->clear();
    }
}
