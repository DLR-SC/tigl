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

#include "ModificatorModel.h"
#include "CTiglUIDManager.h"
#include "TIGLCreatorUndoCommands.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSWingSectionElement.h"
#include "CTiglSectionElement.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSWingSection.h"
#include "CCPACSPositioning.h"
#include "CTiglStandardizer.h"
#include "TIGLCreatorContext.h"
#include "modificators/NewConnectedElementDialog.h"
#include "modificators/DeleteDialog.h"
#include "modificators/NewWingDialog.h"
#include "modificators/NewFuselageDialog.h"
#include "tixicpp.h"
#include "TIGLCreatorException.h"
#include "TIGLCreatorErrorDialog.h"


ModificatorModel::ModificatorModel(ModificatorContainerWidget* modificatorContainerWidget,
                                       TIGLCreatorContext* scene,
                                       QUndoStack* undoStack,
                                       QObject* parent)
 : QAbstractItemModel(parent)
{

    doc          = nullptr;
    this->modificatorContainerWidget = modificatorContainerWidget;
    this->modificatorContainerWidget->setProfilesManager( &profilesDB);
    this->myUndoStack = undoStack;
    this->scene = scene;

    // signals:
    connect(modificatorContainerWidget, SIGNAL(undoCommandRequired()), this, SLOT(createUndoCommand()));
    connect(modificatorContainerWidget, SIGNAL(addProfileRequested(QString)), this, SLOT(addProfile(QString)));
    connect(modificatorContainerWidget, SIGNAL(addWingRequested()), this, SLOT(onAddWingRequested()));
    connect(modificatorContainerWidget, SIGNAL(deleteWingRequested()), this, SLOT(onDeleteWingRequested()));
    connect(modificatorContainerWidget, SIGNAL(addFuselageRequested()), this, SLOT(onAddFuselageRequested()));
    connect(modificatorContainerWidget, SIGNAL(deleteFuselageRequested()), this, SLOT(onDeleteFuselageRequested()));
    connect(modificatorContainerWidget, SIGNAL(addSectionRequested(Ui::ElementModificatorInterface&)), this, SLOT(onAddSectionRequested(Ui::ElementModificatorInterface&)));
    connect(modificatorContainerWidget, SIGNAL(deleteSectionRequested(Ui::ElementModificatorInterface&)), this, SLOT(onDeleteSectionRequested(Ui::ElementModificatorInterface&)));
}

void ModificatorModel::setCPACSConfiguration(TIGLCreatorDocument* newDoc)
{

    doc = newDoc;
    resetTree();
    modificatorContainerWidget->setNoInterfaceWidget();
    profilesDB.cleanConfigProfiles();
    if (configurationIsSet()) {
        // when doc is destroyed, this connection is also destroyed
        connect(doc, SIGNAL(documentUpdated(TiglCPACSConfigurationHandle)), this, SLOT(resetTree()));
        profilesDB.setConfigProfiles(doc->GetConfiguration().GetProfiles());
    }
}

void ModificatorModel::updateCpacsConfigurationFromString(std::string const& config)
{
    if (!configurationIsSet()) {
        LOG(ERROR) << "ModificatorManager::updateCpacsConfigurationFromString: MODIFICATOR MANAGER IS NOT READY";
        return;
    }

    doc->updateCpacsConfigurationFromString(config);
    if (configurationIsSet()) {
        // We still need to reset the profilesDB
        profilesDB.cleanConfigProfiles();
        profilesDB.setConfigProfiles(doc->GetConfiguration().GetProfiles());
    }
    emit configurationEdited();
}

std::string ModificatorModel::getConfigurationAsString()
{
    TixiDocumentHandle tixiHandle = doc->GetConfiguration().GetTixiDocumentHandle();

    // save the old version
    try {
        return tixi::TixiExportDocumentAsString(tixiHandle);
    }
    catch (const tixi::TixiError& e) {
        QString errMsg =
            "ModificatorManager::getConfigurationAsString() Something went wrong during exporting the file from tixi handler. "
            "Tixi error message: \"" +
            QString(e.what()) + "\".";
        throw tigl::CTiglError(errMsg.toStdString());
    }
}

void ModificatorModel::writeCPACS()
{
    if (!configurationIsSet()) {
        LOG(ERROR) << "ModificatorManager::writeCPACS: MODIFICATOR MANAGER IS NOT READY";
        return;
    }

    try {
        doc->GetConfiguration().WriteCPACS(doc->GetConfiguration().GetUID());
    }
    catch (const tixi::TixiError& e) {
        QString errMsg =
            "ModificatorManager::writeCPACS() encountered an error saving the CPACS file. "
            "Tixi error message: \"" +
            QString(e.what()) + "\".";
        throw tigl::CTiglError(errMsg.toStdString());
    }
}

void ModificatorModel::dispatch(cpcr::CPACSTreeItem* item)
{

    // todo try catch on dispatch
    if ((!configurationIsSet()) || (!item->isInitialized())) {
        modificatorContainerWidget->hideAllSpecializedWidgets();
        LOG(ERROR) << "MODIFICATOR MANAGER IS NOT READY";
        return;
    }

    unHighlight();
    if (item->getType() == "transformation") {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSTransformation& transformation =
            uidManager.ResolveObject<tigl::CCPACSTransformation>(item->getUid());
        modificatorContainerWidget->setTransformationModificator(transformation, doc->GetConfiguration());
    }
    else if (item->getType() == "fuselage") {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSFuselage& fuselage    = uidManager.ResolveObject<tigl::CCPACSFuselage>(item->getUid());
        modificatorContainerWidget->setFuselageModificator(fuselage);
        highlight(fuselage.GetCTiglElements());
    }
    else if (item->getType() == "fuselages") {
        modificatorContainerWidget->setFuselagesModificator();
    }
    else if (item->getType() == "wing") {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSWing& wing            = uidManager.ResolveObject<tigl::CCPACSWing>(item->getUid());
        modificatorContainerWidget->setWingModificator(wing);
        highlight(wing.GetCTiglElements());
    }
    else if (item->getType() == "wings") {
        modificatorContainerWidget->setWingsModificator();
    }
    else if (item->getType() == "element") {
        // first, we need to determine whether this is a section or a fuselage element
        // then, we can retrieve the CTiglElement interface that manages the both cases.
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
        std::vector<tigl::CTiglSectionElement*> elements;
        elements.push_back(sectionElement);
        highlight(elements);
    }
    else if (item->getType() == "section") {
        tigl::CTiglUIDManager& uidManager       = doc->GetConfiguration().GetUIDManager();
        tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(item->getUid());
        std::vector<tigl::CTiglSectionElement*> cTiglElements;  // one for the highlight function
        QList<tigl::CTiglSectionElement*> qCTiglElements;   // one for the modificator widget
        if (typePtr.type == &typeid(tigl::CCPACSFuselageSection)) {
            tigl::CCPACSFuselageSection& fuselageSection = *reinterpret_cast<tigl::CCPACSFuselageSection*>(typePtr.ptr);
            // In fact for the moment multiple element is not supported by Tigl so the number of cTiglElements will allays be one
            for (int i = 1; i <= fuselageSection.GetSectionElementCount(); i++) {
                cTiglElements.push_back(fuselageSection.GetSectionElement(i).GetCTiglSectionElement());
                qCTiglElements.push_back(fuselageSection.GetSectionElement(i).GetCTiglSectionElement());
            }

        }
        else if (typePtr.type == &typeid(tigl::CCPACSWingSection)) {
            tigl::CCPACSWingSection& wingSection = *reinterpret_cast<tigl::CCPACSWingSection*>(typePtr.ptr);
            for (int i = 1; i <= wingSection.GetSectionElementCount(); i++) {
                cTiglElements.push_back(wingSection.GetSectionElement(i).GetCTiglSectionElement());
                qCTiglElements.push_back(wingSection.GetSectionElement(i).GetCTiglSectionElement());
            }
        }

        highlight(cTiglElements);
        modificatorContainerWidget->setSectionModificator(qCTiglElements);
    }
    else if (item->getType() == "sections" ) {
        std::string bodyUID = item->getParent()->getUid(); // return the fuselage or wing uid
        auto element = resolve(bodyUID);
        modificatorContainerWidget->setSectionsModificator(std::move(element));
    }
    else if (item->getType() == "positioning" ) {
        tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
        tigl::CCPACSPositioning& positioning    = uidManager.ResolveObject<tigl::CCPACSPositioning>(item->getUid());
        tigl::CTiglTransformation parentTransformation;

        // if fact we need the wing or fuselage parent to be able to invalidate the positionigs.
        std::string bodyUID = item->getParent()->getParent()->getUid();
        tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(bodyUID);
        if (typePtr.type == &typeid(tigl::CCPACSWing)) {
            tigl::CCPACSWing &wing = *reinterpret_cast<tigl::CCPACSWing *>(typePtr.ptr);
            modificatorContainerWidget->setPositioningModificator(wing, positioning);
            parentTransformation = wing.GetTransformationMatrix();
        }
        else if (typePtr.type == &typeid(tigl::CCPACSFuselage)) {
            tigl::CCPACSFuselage &fuselage = *reinterpret_cast<tigl::CCPACSFuselage *>(typePtr.ptr);
            modificatorContainerWidget->setPositioningModificator(fuselage, positioning);
            parentTransformation = fuselage.GetTransformationMatrix();
        }
        else {
            LOG(ERROR) << "ModificatorManager:: Unable to find expected parent for the uid type!";
            return;
        }
        highlight(positioning, parentTransformation);
        
    }
    else {
        modificatorContainerWidget->setNoInterfaceWidget();
        LOG(INFO) << "MODIFICATOR MANAGER: item not suported";
    }
}

void ModificatorModel::createUndoCommand()
{
    if (configurationIsSet()) {
        QUndoCommand* command = new TiGLCreator::ModifyTiglObject(*this);
        myUndoStack->push(command);
    }
    else {
        LOG(ERROR) << "ModificatorManager::createUndoCommand: Called but no document is set!";
    }
    emit configurationEdited();
}

void ModificatorModel::resetTree()
{
    QAbstractItemModel::beginResetModel();
    if (configurationIsSet()) {
        auto tixi_handle = doc->GetConfiguration().GetTixiDocumentHandle();
        tree.build(tixi_handle, "/cpacs/vehicles");
        // TODO multiple model in file case
    }
    else {
        tree.clean();
    }
    QAbstractItemModel::endResetModel();
}

void ModificatorModel::standardize(QString uid, bool useSimpleDecomposition)
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
                      "can be standardized at the moment!";
    }
    createUndoCommand();
}


void ModificatorModel::standardize(bool useSimpleDecomposition)
{
    if (!configurationIsSet()) {
        LOG(ERROR) << "ModificatorManager::standardize: Called but no document is set!";
    }
    tigl::CTiglStandardizer::StandardizeAircraft(doc->GetConfiguration(), useSimpleDecomposition );
    createUndoCommand();

}

Ui::ElementModificatorInterface ModificatorModel::resolve(const std::string &uid) const
{
    tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
    tigl::CTiglUIDManager::TypedPtr ptr = uidManager.ResolveObject(uid);
    if (ptr.type == &typeid(tigl::CCPACSWing)) {
        tigl::CCPACSWing &wing = *reinterpret_cast<tigl::CCPACSWing*>(ptr.ptr);
        return Ui::ElementModificatorInterface(wing);
    }
    else if (ptr.type == &typeid(tigl::CCPACSFuselage)) {
        tigl::CCPACSFuselage &fuselage = *reinterpret_cast<tigl::CCPACSFuselage*>(ptr.ptr);
        return Ui::ElementModificatorInterface(fuselage);
    }
    else {
        LOG(ERROR) << "ModificatorManager:: Unexpected sections type!";
        return Ui::ElementModificatorInterface(tigl::CCPACSFuselage(nullptr, nullptr));
    }
}

std::string ModificatorModel::sectionUidToElementUid(const std::string &uid) const
{
    //TODO is there a better way to get the element uid from the section uid?
    tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
    tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(uid);
    if (typePtr.type == &typeid(tigl::CCPACSFuselageSection)) {
        tigl::CCPACSFuselageSection& fuselageSection = *reinterpret_cast<tigl::CCPACSFuselageSection*>(typePtr.ptr);
        assert(fuselageSection.GetSectionElementCount() == 1); // TiGL supports only one element per section
        return fuselageSection.GetSectionElement(1).GetUID();
    } else if (typePtr.type == &typeid(tigl::CCPACSWingSection)) {
        tigl::CCPACSWingSection& wingSection = *reinterpret_cast<tigl::CCPACSWingSection*>(typePtr.ptr);
        assert(wingSection.GetSectionElementCount() == 1); // TiGL supports only one element per section
        return wingSection.GetSectionElement(1).GetUID();
    } else {
        LOG(ERROR) << "ModificatorManager:: Unexpected section type!";
        return "";
    }
}

std::string ModificatorModel::elementUidToSectionUid(const std::string &uid) const
{
    tigl::CTiglUIDManager& uidManager = doc->GetConfiguration().GetUIDManager();
    tigl::CTiglUIDManager::TypedPtr typePtr = uidManager.ResolveObject(uid);
    if (typePtr.type == &typeid(tigl::CCPACSFuselageSectionElement)) {
        tigl::CCPACSFuselageSectionElement& element = *reinterpret_cast<tigl::CCPACSFuselageSectionElement*>(typePtr.ptr);
        return element.GetParent()->GetParent()->GetUID();
    } else if (typePtr.type == &typeid(tigl::CCPACSWingSectionElement)) {
        tigl::CCPACSWingSectionElement& element = *reinterpret_cast<tigl::CCPACSWingSectionElement*>(typePtr.ptr);
        return element.GetParent()->GetParent()->GetUID();
    } else {
        LOG(ERROR) << "ModificatorManager:: Unexpected element type!";
        return "";
    }
}

cpcr::CPACSTreeItem *ModificatorModel::getAirfoils() const
{
    auto nodes = tree.getRoot()->findAllChildrenOfTypeRecursively("wingAirfoils"); //TODO: findFirstChildOfType would be faster
    if (nodes.size() != 1) {
        LOG(ERROR) << "ModificatorManager:: Could not find wingAirfoils node in CPACS Tree";
        return nullptr;
    }
    return nodes[0];
}

cpcr::CPACSTreeItem *ModificatorModel::getFuselageProfiles() const
{
    auto nodes = tree.getRoot()->findAllChildrenOfTypeRecursively("fuselageProfiles"); //TODO: findFirstChildOfType would be faster
    if (nodes.size() != 1) {
        LOG(ERROR) << "ModificatorManager:: Could not find fuselageProfiles node in CPACS Tree";
        return nullptr;
    }
    return nodes[0];
}

cpcr::CPACSTreeItem *ModificatorModel::getWings() const
{
    auto idx = getAircraftModelIndex();
    auto* root = getItem(idx);

    auto nodes = root->findAllChildrenOfTypeRecursively("wings"); //TODO: findFirstChildOfType would be faster
    if (nodes.size() != 1) {
        LOG(ERROR) << "ModificatorManager:: Could not find wings node in CPACS Tree";
        return nullptr;
    }
    return nodes[0];
}

cpcr::CPACSTreeItem *ModificatorModel::getFuselages() const
{
    auto idx = getAircraftModelIndex();
    auto* root = getItem(idx);

    auto nodes = root->findAllChildrenOfTypeRecursively("fuselages"); //TODO: findFirstChildOfType would be faster
    if (nodes.size() != 1) {
        LOG(ERROR) << "ModificatorManager:: Could not find fuselages node in CPACS Tree";
        return nullptr;
    }
    return nodes[0];
}

void ModificatorModel::unHighlight()
{
    for (int i = 0; i < highligthteds.size(); i++) {
        scene->removeShape(highligthteds[i]);
    }
    highligthteds.clear();
}

void ModificatorModel::deleteSection(cpcr::CPACSTreeItem* item)
{
    if (item == nullptr) {
        return;
    }

    auto* sections = item->getParent(); // should be sections
    if (sections == nullptr) {
        return;
    }
    auto* parent = sections->getParent(); // parent should be wing or fuselage
    if (parent == nullptr) {
        return;
    }
    auto element = resolve(parent->getUid());

    auto parentIdx = getIndex(sections, 0);
    auto row = item->positionRelativelyToParent();
    beginRemoveRows(parentIdx, row, row);

    // apply changes in cpacs configuration
    try {
        element.DeleteConnectedElement(sectionUidToElementUid(item->getUid()));
    }
    catch (const tigl::CTiglError& err) {
        TIGLCreatorErrorDialog errDialog(modificatorContainerWidget);
        errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to delete the section (aka connected element)").arg(err.what()));
        errDialog.setWindowTitle("Error");
        errDialog.setDetailsText(err.what());
        errDialog.exec();
        endRemoveRows();
        return;
    }
    createUndoCommand(); // invokes writeCPACS, which is needed to correctly modify the CPACSTree

    // apply changes to CPACSTree
    sections->removeChild(row);
    endRemoveRows();
}

void ModificatorModel::onDeleteSectionRequested(Ui::ElementModificatorInterface &element)
{
    std::vector<std::string> elementUIDs = element.GetOrderedConnectedElement();

    QStringList sectionUIDsQList;
    for (int i = 0; i < elementUIDs.size(); i++) {
        QString sectionUid = QString::fromStdString(elementUidToSectionUid(elementUIDs[i]));
        sectionUIDsQList.push_back(sectionUid);
    }

    DeleteDialog deleteDialog(sectionUIDsQList);
    if (deleteDialog.exec() == QDialog::Accepted) {
        QString uid = deleteDialog.getUIDToDelete();
        auto idx = getIdxForUID(uid.toStdString());
        auto* item = getItem(idx);
        if (item == nullptr) {
            return;
        }
        deleteSection(item);
    }
}

void ModificatorModel::addSection(
        Ui::ElementModificatorInterface& element,
        NewConnectedElementDialog::Where where,
        std::string startUID,
        std::string sectionName,
        std::optional<double> eta
)
{
    auto idx = getIdxForUID(startUID);
    auto* item = getItem(idx);
    if (item == nullptr) {
        return;
    }
    std::string elemUID = sectionUidToElementUid(startUID);

    // apply the changes

    int row = item->positionRelativelyToParent();
    if (where == NewConnectedElementDialog::Where::After) {
        ++row;
    }

    auto* sections = item->getParent();
    if (sections == nullptr) {
        return;
    }
    auto sectionsIdx = getIndex(sections, 0);

    beginInsertRows(sectionsIdx, row, row);

    // add the section in the cpacs configuration
    try {
        if (where == NewConnectedElementDialog::Before) {
            auto elementUIDBefore = element.GetElementUIDBeforeNewElement(elemUID);
            if (elementUIDBefore) {
                if (eta) { // Security check. Should be set if elementUIDBefore is true
                    element.CreateNewConnectedElementBetween(*elementUIDBefore, elemUID, *eta, sectionName);
                }
                else {
                    endInsertRows();
                    throw tigl::CTiglError("No eta value set!");
                }
            }
            else {
                element.CreateNewConnectedElementBefore(elemUID, sectionName);
            }
        }
        else if (where == NewConnectedElementDialog::After) {
            auto elementUIDAfter = element.GetElementUIDAfterNewElement(elemUID);
            if (elementUIDAfter) {
                if (eta) { // Security check. Should be set if elementUIDAfter is true
                    element.CreateNewConnectedElementBetween(elemUID, *elementUIDAfter, *eta, sectionName);
                }
                else {
                    endInsertRows();
                    throw tigl::CTiglError("No eta value set!");
                }
            }
            else {
                element.CreateNewConnectedElementAfter(elemUID, sectionName);
            }
        }
    }
    catch (const tigl::CTiglError& err) {
        TIGLCreatorErrorDialog errDialog(modificatorContainerWidget);
        errDialog.setMessage(
            QString("<b>%1</b><br /><br />%2").arg("Fail to create the new connected element ").arg(err.what()));
        errDialog.setWindowTitle("Error");
        errDialog.setDetailsText(err.what());
        errDialog.exec();
        endInsertRows();
        return;
    }

    createUndoCommand(); // invokes writeCPACS, which is needed to correctly modify the CPACSTree

    // apply changes to CPACSTree
    std::string xpath = sections->getXPath() + "/" + item->getType() + "[" + std::to_string(row+1) + "]";
    std::string uid = sectionName;
    auto* new_item = sections->addChildAt(row, xpath, item->getType(), row, uid);
    tree.createChildrenRecursively(*new_item);

    endInsertRows();
}

void ModificatorModel::onAddSectionRequested(Ui::ElementModificatorInterface &element)
{
    if (!configurationIsSet()) {
        return;
    }

    // open a new connected element dialog
    std::vector<std::string> elementUIDs = element.GetOrderedConnectedElement();
    QStringList sectionUIDsQList;
    for (int i = 0; i < elementUIDs.size(); i++) {
        QString sectionUid = QString::fromStdString(elementUidToSectionUid(elementUIDs[i]));
        sectionUIDsQList.push_back(sectionUid);
    }

    NewConnectedElementDialog newElementDialog(sectionUIDsQList, doc->GetConfiguration().GetUIDManager());

    if (newElementDialog.exec() == QDialog::Accepted) {

        addSection(
            element,
            newElementDialog.getWhere(),
            newElementDialog.getStartUID().toStdString(),
            newElementDialog.getSectionName().toStdString(),
            newElementDialog.getEta()
        );

    }
}

void ModificatorModel::onAddSectionRequested(CPACSTreeView::Where where, cpcr::CPACSTreeItem *item)
{
    if (!isValid() || !configurationIsSet()) {
        return;
    }

    if (item == nullptr) {
        return;
    }

    auto* sections = item->getParent(); // should be sections
    if (sections == nullptr) {
        return;
    }

    auto* parent = sections->getParent(); // parent should be wing or fuselage
    if (parent == nullptr) {
        return;
    }
    auto element = resolve(parent->getUid());

    // open a new connected element dialog
    std::vector<std::string> elementUIDs = element.GetOrderedConnectedElement();
    QStringList sectionUIDsQList;
    for (int i = 0; i < elementUIDs.size(); i++) {
        QString sectionUid = QString::fromStdString(elementUidToSectionUid(elementUIDs[i]));
        sectionUIDsQList.push_back(sectionUid);
    }

    NewConnectedElementDialog newElementDialog(sectionUIDsQList, doc->GetConfiguration().GetUIDManager());

    // Set the comboboxes' initial values according to the arguments
    if (item != nullptr) {
        QString uid = QString::fromStdString(item->getUid());
        newElementDialog.setStartUID(uid);
        if (where == CPACSTreeView::Where::Before) {
            newElementDialog.setWhere(NewConnectedElementDialog::Before);
        } else {
            newElementDialog.setWhere(NewConnectedElementDialog::After);
        }
    }

    if (newElementDialog.exec() == QDialog::Accepted) {

        addSection(
            element,
            newElementDialog.getWhere(),
            newElementDialog.getStartUID().toStdString(),
            newElementDialog.getSectionName().toStdString(),
            newElementDialog.getEta()
        );
    }
}

void ModificatorModel::addProfile(QString const& profileID)
{

    cpcr::CPACSTreeItem* profiles = nullptr; // either Airfoils or fuselageProfiles
    std::string profile_type;
    if (profilesDB.isAWingProfile(profileID)) {
        profiles = getAirfoils();
        profile_type = "wingAirfoil";
    } else {
        profiles = getFuselageProfiles();
        profile_type = "fuselageProfile";
    }
    std::string profile_name = profilesDB.removeSuffix(profileID).toStdString();

    auto profiles_idx = getIndex(profiles, 0);
    auto profile_row = profiles->getChildren().size();
    beginInsertRows(profiles_idx, profile_row, profile_row);

    // apply changes to configuration
    try {
        profilesDB.copyProfileFromLocalToConfig(profileID);
    }
    catch (const tigl::CTiglError& err) {
        TIGLCreatorErrorDialog errDialog(modificatorContainerWidget);
        errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to create the wing ").arg(err.what()));
        errDialog.setWindowTitle("Error");
        errDialog.setDetailsText(err.what());
        errDialog.exec();
        endInsertRows();
        return;
    }

    createUndoCommand(); // this is a bit unfortunate: if a profile has to be added, we have an additional undo command

    // apply changes to CPACSTree
    std::string xpath = profiles->getXPath() + "/" + profile_type + "[" + std::to_string(profile_row+1) + "]";
    auto* new_item = profiles->addChild(xpath, "wingAirfoil", profile_row, profile_name);
    tree.createChildrenRecursively(*new_item);

    endInsertRows();
}

void ModificatorModel::onAddWingRequested()
{
    if (!configurationIsSet()) {
        return;
    }

    NewWingDialog wingDialog(profilesDB.getAllWingProfiles(), doc->GetConfiguration().GetUIDManager(), modificatorContainerWidget);
    if (wingDialog.exec() == QDialog::Accepted) {
        int nbSection       = wingDialog.getNbSection();
        QString uid         = wingDialog.getUID();
        QString profileID = wingDialog.getProfileUID();

        std::string profile_name = profilesDB.removeSuffix(profileID).toStdString();

        if ( !profilesDB.hasProfileConfigSuffix(profileID) ) {
            addProfile(profileID);
        } else {
            LOG(WARNING) << "ModificatorManager: Cannot add the airfoil. An airfoil with the same name already exists in the configuration.";
        }

        auto* wings = getWings();
        auto wings_idx = getIndex(wings, 0);
        auto row = wings->getChildren().size();

        beginInsertRows(wings_idx, row, row);

        // apply changes to configuration
        try {
            doc->GetConfiguration().GetWings().CreateWing(
                uid.toStdString(),
                nbSection,
                profile_name
            );
        }
        catch (const tigl::CTiglError& err) {
            TIGLCreatorErrorDialog errDialog(modificatorContainerWidget);
            errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to create the wing ").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            endInsertRows();
            return;
        }

        createUndoCommand(); // invokes writeCPACS, which is needed to correctly modify the CPACSTree

        // apply changes to CPACSTree
        std::string xpath = wings->getXPath() + "/" + "wing[" + std::to_string(row+1) + "]";
        auto* new_item = wings->addChild(xpath, "wing", row, uid.toStdString());
        tree.createChildrenRecursively(*new_item);

        endInsertRows();
    }
}

void ModificatorModel::deleteWing(std::string const& uid)
{
    if (!configurationIsSet()) {
        return;
    }
    auto& wings = doc->GetConfiguration().GetWings();

    auto idx = getIdxForUID(uid);
    auto* wing_node = getItem(idx);
    if (wing_node == nullptr) {
        return;
    }
    auto* parent = wing_node->getParent();
    if (parent == nullptr) {
        return;
    }
    auto parentIdx = getIndex(parent, 0);
    auto row = wing_node->positionRelativelyToParent();
    beginRemoveRows(parentIdx, row, row);

    // apply changes in cpacs configuration
    try {
        tigl::CCPACSWing& wing = wings.GetWing(uid);
        wings.RemoveWing(wing);
    }
    catch (const tigl::CTiglError& err) {
        TIGLCreatorErrorDialog errDialog(modificatorContainerWidget);
        errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to delete the wing ").arg(err.what()));
        errDialog.setWindowTitle("Error");
        errDialog.setDetailsText(err.what());
        errDialog.exec();
        endRemoveRows();
        return;
    }

    createUndoCommand(); // invokes writeCPACS, which is needed to correctly modify the CPACSTree

    // apply changes to CPACSTree
    parent->removeChild(row);
    endRemoveRows();
}

void ModificatorModel::deleteFuselage(std::string const& uid)
{
    if (!configurationIsSet()) {
        return;
    }
    auto& fuselages = doc->GetConfiguration().GetFuselages();

    auto idx = getIdxForUID(uid);
    auto* fuselage_node = getItem(idx);
    if (fuselage_node == nullptr) {
        return;
    }
    auto* parent = fuselage_node->getParent();
    if (parent == nullptr) {
        return;
    }
    auto parentIdx = getIndex(parent, 0);
    auto row = fuselage_node->positionRelativelyToParent();
    beginRemoveRows(parentIdx, row, row);

    // apply changes in cpacs configuration
    try {
        tigl::CCPACSFuselage& fuselage = fuselages.GetFuselage(uid);
        fuselages.RemoveFuselage(fuselage);
    }
    catch (const tigl::CTiglError& err) {
        TIGLCreatorErrorDialog errDialog(modificatorContainerWidget);
        errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to delete the fuselage ").arg(err.what()));
        errDialog.setWindowTitle("Error");
        errDialog.setDetailsText(err.what());
        errDialog.exec();
        endRemoveRows();
        return;
    }

    createUndoCommand(); // invokes writeCPACS, which is needed to correctly modify the CPACSTree

    // apply changes to CPACSTree
    parent->removeChild(row);
    endRemoveRows();
}

void ModificatorModel::onDeleteWingRequested()
{
    if (!configurationIsSet()) {
        return;
    }
    auto& wings = doc->GetConfiguration().GetWings();

    QStringList wingUIDs;
    for (int i = 1; i <= wings.GetWingCount(); i++) {
        wingUIDs.push_back(wings.GetWing(i).GetUID().c_str());
    }

    DeleteDialog deleteDialog(wingUIDs);
    if (deleteDialog.exec() == QDialog::Accepted) {
        std::string uid = deleteDialog.getUIDToDelete().toStdString();
        deleteWing(uid);
    }
}

void ModificatorModel::onAddFuselageRequested()
{
    if (!configurationIsSet()) {
        return;
    }

    auto& fuselages = doc->GetConfiguration().GetFuselages();

    NewFuselageDialog fuselageDialog(profilesDB.getAllFuselagesProfiles(), doc->GetConfiguration().GetUIDManager(), modificatorContainerWidget);
    if (fuselageDialog.exec() == QDialog::Accepted) {
        int nbSection       = fuselageDialog.getNbSection();
        QString uid         = fuselageDialog.getUID();
        QString profileID = fuselageDialog.getProfileUID();

        if (!profilesDB.hasProfileConfigSuffix(profileID)) {
            addProfile(profileID);
        } else {
            LOG(WARNING) << "ModificatorManager: Cannot add the airfoil. An airfoil with the same name already exists in the configuration.";
        }

        auto* fuselages_node = getFuselages();
        auto fuselages_idx = getIndex(fuselages_node, 0);
        auto row = fuselages_node->getChildren().size();

        beginInsertRows(fuselages_idx, row, row);

        // apply changes to configuration
        try {
            fuselages.CreateFuselage(
                        uid.toStdString(),
                        nbSection,
                        profilesDB.removeSuffix(profileID).toStdString()
            );
        }
        catch (const tigl::CTiglError& err) {
            TIGLCreatorErrorDialog errDialog(modificatorContainerWidget);
            errDialog.setMessage(
                QString("<b>%1</b><br /><br />%2").arg("Fail to create the fuselage ").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            endInsertRows();
            return;
        }

        createUndoCommand(); // invokes writeCPACS, which is needed to correctly modify the CPACSTree

        // apply changes to CPACSTree
        std::string xpath = fuselages_node->getXPath() + "/" + "fuselage[" + std::to_string(row+1) + "]";
        auto* new_item = fuselages_node->addChild(xpath, "fuselage", row, uid.toStdString());
        tree.createChildrenRecursively(*new_item);

        endInsertRows();
    }
}

void ModificatorModel::onDeleteFuselageRequested()
{
    if (!configurationIsSet()) {
        return;
    }
    auto& fuselages = doc->GetConfiguration().GetFuselages();

    QStringList wingUIDs;
    for (int i = 1; i <= fuselages.GetFuselageCount(); i++) {
        wingUIDs.push_back(fuselages.GetFuselage(i).GetUID().c_str());
    }

    DeleteDialog deleteDialog(wingUIDs);
    if (deleteDialog.exec() == QDialog::Accepted) {
        std::string uid = deleteDialog.getUIDToDelete().toStdString();
        deleteFuselage(uid);
    }
}

void ModificatorModel::highlight(std::vector<tigl::CTiglSectionElement*> elements)
{
    try {
        for (size_t i = 0; i < elements.size(); i++) {
            Handle(AIS_InteractiveObject) shape = scene->displayShapeHLMode(elements[i]->GetWire());
            highligthteds.push_back(shape);
        }
    } catch (...) {
        LOG(ERROR) << "ModificatorManager::highlight: Error, No highlighting!";
    }

}

void ModificatorModel::highlight(tigl::CCPACSPositioning &positioning, const tigl::CTiglTransformation& parentTransformation)
{
    // We need the parent transform because the positiong live in fuselage coordinate system.
    // Thus if we do not transform the point we do not draw the positioning at its final place
    try {
        tigl::CTiglPoint aPoint = positioning.GetFromPoint();
        tigl::CTiglPoint bPoint = positioning.GetToPoint();
        aPoint = parentTransformation * aPoint;
        bPoint = parentTransformation * bPoint;
        Handle(AIS_InteractiveObject) shape = scene->displayLineHLMode(aPoint.x, aPoint.y, aPoint.z, bPoint.x, bPoint.y,
                                                                       bPoint.z);
        highligthteds.push_back(shape);
    } catch (...) {
        LOG(ERROR) << "ModificatorManager::highlight: Error, No highlighting!";
    }

}

void ModificatorModel::updateProfilesDB(QString newDBPath)
{
    profilesDB.setLocalProfiles(newDBPath);
}

QVariant ModificatorModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (isValid() && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0){
            return "Type or UID";
        }
        else if (section == 1) {
            return "Type";
        }
        else if (section == 2) {
            return "UID";
        }
        else {
            return "invalid";
        }
    }

    return QVariant();
}

QVariant ModificatorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role !=Qt::UserRole) {
        return QVariant();
    }

    cpcr::CPACSTreeItem* item = getItem(index);
    QVariant data;

    if (role == Qt::UserRole) {
        // we use Qt::UserRole to mark eligable parents for context menus in the tree view
        // For now, these are fuselage sections and wing setions
        data = (item->getType() == "sections");
        return data;
    }

    if (index.column() == 0) { // combine uid and type
        data = QString(item->getUid().c_str());
        if (data == "") {
            data = QString(item->getType().c_str());
        }
    }
    else if (index.column() == 1) {
        data = QString(item->getType().c_str());
    }
    else if (index.column() == 2 ) {
        data = QString(item->getUid().c_str());
    }
    else {
        data = QVariant();
    }

    return data;
}

// return the index of the parent
QModelIndex ModificatorModel::parent(const QModelIndex& index) const
{
    if (!isValid()) {
        return QModelIndex();
    }
    cpcr::CPACSTreeItem* childItem  = getItem(index);
    cpcr::CPACSTreeItem* parentItem = childItem->getParent();

    return getIndex(parentItem, 0);
}

// Count the number of children of this item
int ModificatorModel::rowCount(const QModelIndex& idx) const
{
    if (!isValid()) {
        return 0;
    }

    cpcr::CPACSTreeItem* item = getItem(idx);
    return item->getChildren().size();
}

int ModificatorModel::columnCount(const QModelIndex& idx) const
{

    if (!isValid()) {
        return 0;
    }

    return 3;
}

// return the QModelindex from a parent and row and column information
QModelIndex ModificatorModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!isValid()) {
        return QModelIndex();
    }
    cpcr::CPACSTreeItem* parentItem = getItem(parent); // return root if parent is empty or invalid
    cpcr::CPACSTreeItem* childItem  = parentItem->getChildren()[row];

    if (childItem) { // case where the child is not a null pointer
        return createIndex(row, column, childItem);
    }
    else {
        return QModelIndex();
    }
}

QModelIndex ModificatorModel::getIdxForUID(std::string uid) const
{
    if (!isValid() || uid == "") {
        return QModelIndex();
    }
    else {
        cpcr::CPACSTreeItem* item = tree.getRoot()->getChildByUid(uid);
        return getIndex(item, 0);
    }
}

std::string ModificatorModel::getUidForIdx(QModelIndex idx)
{
    if (!isValid()) {
        return "";
    }
    else {
        return getItem(idx)->getUid();
    }
}

QModelIndex ModificatorModel::getIndex(cpcr::CPACSTreeItem* item, int column) const
{
    if (!isValid() || item == tree.getRoot() || item == nullptr) {
        return QModelIndex(); // We use empty index for the root
    }

    int row = item->positionRelativelyToParent();
    return createIndex(row, column, item);
}

cpcr::CPACSTreeItem* ModificatorModel::getItem(QModelIndex index) const
{
    if (!isValid()) {
        return nullptr;
    }
    // Internal identifier is the item pointer
    if (index.isValid()) {
        cpcr::CPACSTreeItem* item = static_cast<cpcr::CPACSTreeItem*>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return tree.getRoot(); // empty index is the root
}

cpcr::CPACSTreeItem* ModificatorModel::getItemFromSelection(const QItemSelection& newSelection)
{
    if (!isValid()) {
        throw TIGLCreatorException("CPACSAbstractModel: getItemWithError called but "
                                  "the model is not valid!");
    }

    cpcr::CPACSTreeItem* item = getItem(newSelection.indexes().at(0));

    return item;
}

bool ModificatorModel::isValid() const
{
    return tree.isBuild();
}

QModelIndex ModificatorModel::getAircraftModelIndex() const
{
    if (!isValid() || !configurationIsSet()) {
        return QModelIndex();
    }
    else {
        auto const& config = doc->GetConfiguration();
        return getIdxForUID(config.GetUID());
    }
}
