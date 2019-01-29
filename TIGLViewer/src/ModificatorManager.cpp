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

ModificatorManager::ModificatorManager(CPACSTreeWidget* treeWidget, EditorWidget*  editorWidget)
{

    currentModificator = nullptr;
    config             = nullptr;
    treeViewManager    = new CPACSTreeView(treeWidget->getQTreeView());

    this->widgetApply  = editorWidget->getApplyWidget() ;
    this->commitButton = widgetApply->findChild<QPushButton*>("commitButton");
    this->cancelButton = widgetApply->findChild<QPushButton*>("cancelButton");

    this->transformationModificator = editorWidget->getTransformationWidget();
    this->wingModificator           = editorWidget->getWingWidget();
    this->fuselageModificator       = editorWidget->getFuselageWidget();
    this->noInterfaceWidget         = editorWidget->getNoInterfaceWidget();

    this->hideAll();
    this->noInterfaceWidget->setVisible(true);

    // signals:
    connect(treeViewManager, SIGNAL(newSelectedTreeItem(cpcr::CPACSTreeItem*)), this,
            SLOT(dispatch(cpcr::CPACSTreeItem*)));

    connect(commitButton, SIGNAL(pressed()), this, SLOT(applyCurrentModifications()));

    connect(cancelButton, SIGNAL(pressed()), this, SLOT(applyCurrentCancellation()));
}

void ModificatorManager::setCPACSConfiguration(tigl::CCPACSConfiguration* newConfig)
{
    this->config = newConfig;
    if (configurationIsSet()) {
        // TODO allow to chose different model
        std::string rootXPath = "/cpacs/vehicles/aircraft/model[1]";
        treeViewManager->displayNewTree(newConfig->GetTixiDocumentHandle(), rootXPath);
        currentModificator = nullptr;
        hideAll();
        noInterfaceWidget->setVisible(true);
    }
    else {
        currentModificator = nullptr;
        treeViewManager->clear();
        hideAll();
        noInterfaceWidget->setVisible(true);
    }
}

void ModificatorManager::applyCurrentModifications()
{

    if (currentModificator != nullptr) {
        currentModificator->apply(); //
        // todo save in tixi memory, here ? or in apply function ?
        emit configurationEdited();
    }
    else {
        LOG(WARNING) << "ModificatorManager::applyCurrentModifications() called "
                        "but current modificator is null"
                     << std::endl;
    }
}

void ModificatorManager::applyCurrentCancellation()
{
    if (currentModificator != nullptr) {
        currentModificator->reset();
    }
    else {
        LOG(WARNING) << "ModificatorManager::applyCurrentCancellation() called but "
                        "current modificator is null"
                     << std::endl;
    }
}

void ModificatorManager::dispatch(cpcr::CPACSTreeItem* item)
{

    if ((!configurationIsSet()) || (!item->isInitialized())) {
        currentModificator = nullptr;
        hideAll();
        LOG(ERROR) << "MODIFICATOR MANAGER IS NOT READY";
    }
    else if (item->getType() == "transformation") {
        currentModificator = transformationModificator;
        this->setTransformationModificator(item);
    }
    else if (item->getType() == "fuselage") {
        currentModificator = fuselageModificator;
        this->setFuselageModificator(item);
    }
    else if (item->getType() == "wing") {
        currentModificator = wingModificator;
        this->setWingModificator(item);
    }
    else {
        currentModificator = nullptr;
        hideAll();
        noInterfaceWidget->setVisible(true);
        LOG(INFO) << "MODIFICATOR MANAGER: item not suported";
    }
}

void ModificatorManager::hideAll()
{
    bool visible = false;
    transformationModificator->setVisible(visible);
    wingModificator->setVisible(visible);
    fuselageModificator->setVisible(visible);
    widgetApply->setVisible(visible);
    noInterfaceWidget->setVisible(visible);
}

void ModificatorManager::setFuselageModificator(cpcr::CPACSTreeItem* item)
{
    hideAll();
    tigl::CTiglUIDManager& uidManager = config->GetUIDManager();
    tigl::CCPACSFuselage& fuselage    = uidManager.ResolveObject<tigl::CCPACSFuselage>(item->getUid());
    fuselageModificator->setFuselage(fuselage);
    fuselageModificator->setVisible(true);
    widgetApply->setVisible(true);
}

void ModificatorManager::setTransformationModificator(cpcr::CPACSTreeItem* item)
{

    hideAll();
    tigl::CTiglUIDManager& uidManager          = config->GetUIDManager();
    tigl::CCPACSTransformation& transformation = uidManager.ResolveObject<tigl::CCPACSTransformation>(item->getUid());
    transformationModificator->setTransformation(transformation);
    transformationModificator->setVisible(true);
    widgetApply->setVisible(true);
}

void ModificatorManager::setWingModificator(cpcr::CPACSTreeItem* item)
{
    hideAll();
    tigl::CTiglUIDManager& uidManager = config->GetUIDManager();
    tigl::CCPACSWing& wing            = uidManager.ResolveObject<tigl::CCPACSWing>(item->getUid());
    wingModificator->setWing(wing);
    wingModificator->setVisible(true);
    widgetApply->setVisible(true);
}
