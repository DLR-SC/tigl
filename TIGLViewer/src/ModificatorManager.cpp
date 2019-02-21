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

ModificatorManager::ModificatorManager(CPACSTreeWidget* treeWidget,
                                       ModificatorContainerWidget* modificatorContainerWidget)
{

    config          = nullptr;
    treeViewManager = new CPACSTreeView(treeWidget->getQTreeView());

    this->modificatorContainerWidget = modificatorContainerWidget;

    // signals:
    connect(treeViewManager, SIGNAL(newSelectedTreeItem(cpcr::CPACSTreeItem*)), this,
            SLOT(dispatch(cpcr::CPACSTreeItem*)));
}

void ModificatorManager::setCPACSConfiguration(tigl::CCPACSConfiguration* newConfig)
{
    this->config = newConfig;
    if (configurationIsSet()) {
        // TODO allow to chose different model
        std::string rootXPath = "/cpacs/vehicles/aircraft/model[1]";
        treeViewManager->displayNewTree(newConfig->GetTixiDocumentHandle(), rootXPath);
    }
    else {
        treeViewManager->clear();
    }
    modificatorContainerWidget->setNoInterfaceWidget();
}

void ModificatorManager::dispatch(cpcr::CPACSTreeItem* item)
{

    if ((!configurationIsSet()) || (!item->isInitialized())) {

        modificatorContainerWidget->hideAllSecializedWidgets();
        LOG(ERROR) << "MODIFICATOR MANAGER IS NOT READY";
    }
    else if (item->getType() == "transformation") {
        tigl::CTiglUIDManager& uidManager = config->GetUIDManager();
        tigl::CCPACSTransformation& transformation =
            uidManager.ResolveObject<tigl::CCPACSTransformation>(item->getUid());
        modificatorContainerWidget->setTransformationModificator(transformation);
    }
    else if (item->getType() == "fuselage") {
        tigl::CTiglUIDManager& uidManager = config->GetUIDManager();
        tigl::CCPACSFuselage& fuselage    = uidManager.ResolveObject<tigl::CCPACSFuselage>(item->getUid());
        modificatorContainerWidget->setFuselageModificator(fuselage);
    }
    else if (item->getType() == "wing") {
        tigl::CTiglUIDManager& uidManager = config->GetUIDManager();
        tigl::CCPACSWing& wing            = uidManager.ResolveObject<tigl::CCPACSWing>(item->getUid());
        modificatorContainerWidget->setWingModificator(wing);
    }
    else {
        modificatorContainerWidget->setNoInterfaceWidget();
        LOG(INFO) << "MODIFICATOR MANAGER: item not suported";
    }
}
