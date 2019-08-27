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
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSWingSectionElement.h"
#include "CTiglSectionElement.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSWingSection.h"
#include "CreateConnectedElementI.h"
#include "CCPACSPositioning.h"
#include "CTiglStandardizer.h"

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
 // todo try catch on dispatch
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
    else if (item->getType() == "fuselages") {
        tigl::CCPACSFuselages& fuselages = doc->GetConfiguration().GetFuselages();
        modificatorContainerWidget->setFuselagesModificator(fuselages, getAvailableFuselageProfileUIDs());
    }
    else if (item->getType() == "wing") {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSWing& wing            = uidManager.ResolveObject<tigl::CCPACSWing>(item->getUid());
        modificatorContainerWidget->setWingModificator(wing);
    }
    else if (item->getType() == "wings") {
        tigl::CCPACSWings& wings = doc->GetConfiguration().GetWings();
        modificatorContainerWidget->setWingsModificator(wings, getAvailableWingProfileUIDs());
    }
    else if (item->getType() == "element") {
        // we need first to determine if this is a section element or a fuselage element
        // the we can retrieve the CTiglElement interface that manage the both case.
        tigl::CTiglUIDManager& uidManager       = doc->GetConfiguration().GetUIDManager();
        tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(item->getUid());
        tigl::CTiglSectionElement* sectionElement = nullptr;
        if (typePtr.type == &typeid(tigl::CCPACSFuselageSectionElement)) {
            tigl::CCPACSFuselageSectionElement& fuselageElement =
                *reinterpret_cast<tigl::CCPACSFuselageSectionElement*>(typePtr.ptr);
                sectionElement = fuselageElement.GetCTiglSectionElement();
        }
        else if (typePtr.type == &typeid(tigl::CCPACSWingSectionElement)) {
            tigl::CCPACSWingSectionElement& wingElement =
                *reinterpret_cast<tigl::CCPACSWingSectionElement*>(typePtr.ptr);
            sectionElement = wingElement.GetCTiglSectionElement();
        }

        modificatorContainerWidget->setElementModificator(*(sectionElement));
    }
    else if (item->getType() == "section") {
        tigl::CTiglUIDManager& uidManager       = doc->GetConfiguration().GetUIDManager();
        tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(item->getUid());
        QList<tigl::CTiglSectionElement*> cTiglElements;
        if (typePtr.type == &typeid(tigl::CCPACSFuselageSection)) {
            tigl::CCPACSFuselageSection& fuselageSection = *reinterpret_cast<tigl::CCPACSFuselageSection*>(typePtr.ptr);
            // In fact for the moment multiple element is not supported by Tigl so the number of cTiglElements will allays be one
            for (int i = 1; i <= fuselageSection.GetSectionElementCount(); i++) {
                cTiglElements.push_back(fuselageSection.GetSectionElement(i).GetCTiglSectionElement());
            }

        }
        else if (typePtr.type == &typeid(tigl::CCPACSWingSection)) {
            tigl::CCPACSWingSection& wingSection = *reinterpret_cast<tigl::CCPACSWingSection*>(typePtr.ptr);
            for (int i = 1; i <= wingSection.GetSectionElementCount(); i++) {
                cTiglElements.push_back(wingSection.GetSectionElement(i).GetCTiglSectionElement());
            }
        }

        modificatorContainerWidget->setSectionModificator(cTiglElements);
    }
    else if (item->getType() == "sections" ) {
        std::string bodyUID = item->getParent()->getUid(); // return the fuselage or wing uid
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(bodyUID);
        tigl::CreateConnectedElementI * elementI = nullptr;

        if (typePtr.type == &typeid(tigl::CCPACSWing)) {
            tigl::CCPACSWing &wing = *reinterpret_cast<tigl::CCPACSWing *>(typePtr.ptr);
            elementI = dynamic_cast<tigl::CreateConnectedElementI* >(&wing);
        }
        else if (typePtr.type == &typeid(tigl::CCPACSFuselage)) {
            tigl::CCPACSFuselage &fuselage = *reinterpret_cast<tigl::CCPACSFuselage *>(typePtr.ptr);
            elementI = dynamic_cast<tigl::CreateConnectedElementI* >(&fuselage);
        }
        else {
            LOG(ERROR) << "ModificatorManager:: Unexpected sections type!";
        }

        modificatorContainerWidget->setSectionsModificator(*elementI);
    }
    else if (item->getType() == "positioning" ) {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSPositioning& positioning    = uidManager.ResolveObject<tigl::CCPACSPositioning>(item->getUid());

        // if fact we need the wing or fuselage parent to be able to invalidate the positionigs.
        tigl::CCPACSPositionings* positionings;
        std::string bodyUID = item->getParent()->getParent()->getUid();
        tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(bodyUID);
        if (typePtr.type == &typeid(tigl::CCPACSWing)) {
            tigl::CCPACSWing &wing = *reinterpret_cast<tigl::CCPACSWing *>(typePtr.ptr);
            modificatorContainerWidget->setPositioningModificator(wing, positioning);
        }
        else if (typePtr.type == &typeid(tigl::CCPACSFuselage)) {
            tigl::CCPACSFuselage &fuselage = *reinterpret_cast<tigl::CCPACSFuselage *>(typePtr.ptr);
            modificatorContainerWidget->setPositioningModificator(fuselage, positioning);
        }
        else {
            LOG(ERROR) << "ModificatorManager:: Unable to find expected parent for the uid type!";
        }
        
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
        LOG(ERROR) << "ModificatorManager::createUndoCommand: Called but no document is set!";
    }
    emit configurationEdited();
    updateTree();
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

QStringList ModificatorManager::getAvailableFuselageProfileUIDs()
{
    QStringList profileUIDs;

    if (!configurationIsSet()) {
        profileUIDs;
    }

    boost::optional<tigl::CCPACSFuselageProfiles&> profiles = doc->GetConfiguration().GetFuselageProfiles();

    if (profiles) {
        for (int i = 1; i <= profiles.value().GetProfileCount(); i++) {
            profileUIDs.push_back(profiles.value().GetProfile(i).GetUID().c_str());
        }
    }
    return profileUIDs;
}


QStringList ModificatorManager::getAvailableWingProfileUIDs()
{
    QStringList profileUIDs;

    if (!configurationIsSet()) {
        profileUIDs;
    }

    boost::optional<tigl::CCPACSWingProfiles&> profiles = doc->GetConfiguration().GetWingProfiles();

    if (profiles) {
        for (int i = 1; i <= profiles.value().GetProfileCount(); i++) {
            profileUIDs.push_back(profiles.value().GetProfile(i).GetUID().c_str());
        }
    }
    return profileUIDs;
}

void ModificatorManager::standardize(QString uid, bool useSimpleDecomposition)
{
    if (!configurationIsSet()) {
        LOG(ERROR) << "ModificatorManager::standardize: Called but no document is set!";
    }
    tigl::CTiglUIDManager& uidManager       = doc->GetConfiguration().GetUIDManager();
    tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(uid.toStdString());
    if (typePtr.type == &typeid(tigl::CCPACSWing)) {
        tigl::CCPACSWing& wing = *reinterpret_cast<tigl::CCPACSWing*>(typePtr.ptr);
        tigl::CTiglStandardizer::StandardizeWing(wing, useSimpleDecomposition);
    }
    else if (typePtr.type == &typeid(tigl::CCPACSFuselage)) {
        tigl::CCPACSFuselage& fuselage = *reinterpret_cast<tigl::CCPACSFuselage*>(typePtr.ptr);
        tigl::CTiglStandardizer::StandardizeFuselage(fuselage, useSimpleDecomposition);
    }
    else {
        LOG(ERROR) << " ModificatorManager::standardize: uid is not of type fuselage or wing. Only fuselage or wing "
                      "can be standardize for the moment!";
    }
    createUndoCommand();
}


void ModificatorManager::standardize(bool useSimpleDecomposition)
{
    if (!configurationIsSet()) {
        LOG(ERROR) << "ModificatorManager::standardize: Called but no document is set!";
    }
    tigl::CTiglStandardizer::StandardizeAircraft(doc->GetConfiguration(), useSimpleDecomposition );
    createUndoCommand();

}