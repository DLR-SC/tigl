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

#include "ModificatorContainerWidget.h"
#include "ui_ModificatorContainerWidget.h"
#include "CTiglLogging.h"
#include <QVBoxLayout>

ModificatorContainerWidget::ModificatorContainerWidget(QWidget* parent)
    : CpacsEditorBase(parent)
    , ui(new Ui::ModificatorContainerWidget)
{
    ui->setupUi(this);


    connect(ui->editorTabWidget, &QTabWidget::currentChanged, this, &ModificatorContainerWidget::onEditorTabChanged);

    setNoInterfaceWidget();

    connect(ui->commitButton, SIGNAL(pressed()), this, SLOT(applyCurrentModifications()));

    connect(ui->cancelButton, SIGNAL(pressed()), this, SLOT(applyCurrentCancellation()));


    // Addition and Removal of CPACS nodes must be delegated to the ModificatorModel.
    // This is necessary to keep the QAbstractItemModel, CPACSTree and CCPACSConfiguration in sync

    connect(ui->wingModificator, SIGNAL(addProfileRequested(QString)), this, SLOT(forwardAddProfileRequested(QString)));
    connect(ui->sectionModificator, SIGNAL(addProfileRequested(QString)), this, SLOT(forwardAddProfileRequested(QString)));
    connect(ui->elementModificator, SIGNAL(addProfileRequested(QString)), this, SLOT(forwardAddProfileRequested(QString)));

    connect(ui->wingsModificator, SIGNAL(addWingRequested()), this, SLOT(forwardAddWingRequested()));
    connect(ui->wingsModificator, SIGNAL(deleteWingRequested()), this, SLOT(forwardDeleteWingRequested()));

    connect(ui->fuselagesModificator, SIGNAL(addFuselageRequested()), this, SLOT(forwardAddFuselageRequested()));
    connect(ui->fuselagesModificator, SIGNAL(deleteFuselageRequested()), this, SLOT(forwardDeleteFuselageRequested()));

    connect(ui->sectionsModificator, SIGNAL(addSectionRequested(Ui::ElementModificatorInterface&)), this, SLOT(forwardAddSectionRequested(Ui::ElementModificatorInterface&)));
    connect(ui->sectionsModificator, SIGNAL(deleteSectionRequested(Ui::ElementModificatorInterface&)), this, SLOT(forwardDeleteSectionRequested(Ui::ElementModificatorInterface&)));

    // Signals from the display options widget to allow the main window
    // to apply viewer/context changes (transparency, color, material, rendering)
    connect(ui->displayOptionsWidget, &ModificatorDisplayOptionsWidget::setTransparencyRequested,
        this, &ModificatorContainerWidget::setTransparencyRequested);
    connect(ui->displayOptionsWidget, &ModificatorDisplayOptionsWidget::setRenderingModeRequested,
        this, &ModificatorContainerWidget::setRenderingModeRequested);
    connect(ui->displayOptionsWidget, &ModificatorDisplayOptionsWidget::setColorRequested,
        this, &ModificatorContainerWidget::setColorRequested);
    connect(ui->displayOptionsWidget, &ModificatorDisplayOptionsWidget::setMaterialRequested,
        this, &ModificatorContainerWidget::setMaterialRequested);
}

ModificatorContainerWidget::~ModificatorContainerWidget()
{
    delete ui;
}

void ModificatorContainerWidget::setProfilesManager(ProfilesDBManager* profilesDB)
{
    this->profilesDB = profilesDB;
}


void ModificatorContainerWidget::hideAllSpecializedWidgets()
{
    bool visible = false;
    ui->transformationModificator->setVisible(visible);
    ui->wingModificator->setVisible(visible);
    ui->wingsModificator->setVisible(visible);
    ui->fuselageModificator->setVisible(visible);
    ui->fuselagesModificator->setVisible(visible);
    ui->applyWidget->setVisible(visible);
    ui->noInterfaceWidget->setVisible(visible);
    ui->elementModificator->setVisible(visible);
    ui->sectionModificator->setVisible(visible);
    ui->sectionsModificator->setVisible(visible);
    ui->positioningModificator->setVisible(visible);
    ui->displayOptionsWidget->setVisible(visible);
    currentModificator = nullptr;
}

void ModificatorContainerWidget::setTransformationModificator(tigl::CCPACSTransformation& transformation, tigl::CCPACSConfiguration& config)
{
    hideAllSpecializedWidgets();
    ui->transformationModificator->setTransformation(transformation, config);
    ui->transformationModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->transformationModificator;
}

void ModificatorContainerWidget::setWingModificator(tigl::CCPACSWing& wing)
{
    hideAllSpecializedWidgets();
    ui->wingModificator->setWing(wing, profilesDB);
    ui->wingModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->wingModificator;
}


void ModificatorContainerWidget::setWingsModificator()
{
    hideAllSpecializedWidgets();
    ui->wingsModificator->setVisible(true);
    ui->applyWidget->setVisible(false);
    currentModificator = ui->wingsModificator;
}

void ModificatorContainerWidget::setFuselageModificator(tigl::CCPACSFuselage& fuselage)
{
    hideAllSpecializedWidgets();
    ui->fuselageModificator->setFuselage(fuselage, profilesDB);
    ui->fuselageModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->fuselageModificator;
}


void ModificatorContainerWidget::setFuselagesModificator()
{
    hideAllSpecializedWidgets();
    ui->fuselagesModificator->setVisible(true);
    ui->applyWidget->setVisible(false);
    currentModificator = ui->fuselagesModificator;
}

void ModificatorContainerWidget::setElementModificator(tigl::CTiglSectionElement& element)
{
    hideAllSpecializedWidgets();
    ui->elementModificator->setElement(element, profilesDB);
    ui->elementModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->elementModificator;
}

void ModificatorContainerWidget::setSectionModificator(QList<tigl::CTiglSectionElement*> elements)
{
    hideAllSpecializedWidgets();
    ui->sectionModificator->setAssociatedElements(elements, profilesDB);
    ui->sectionModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->sectionModificator;
}


void ModificatorContainerWidget::setSectionsModificator(const Ui::ElementModificatorInterface& element)
{
    hideAllSpecializedWidgets();
    ui->sectionsModificator->setCreateConnectedElement(element);
    ui->sectionsModificator->setVisible(true);
    currentModificator = ui->sectionModificator;
}


void ModificatorContainerWidget::setPositioningModificator(tigl::CCPACSWing& wing, tigl::CCPACSPositioning &positioning)
{
    hideAllSpecializedWidgets();
    ui->positioningModificator->setPositioning(wing, positioning);
    ui->positioningModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->positioningModificator;
}



void ModificatorContainerWidget::setPositioningModificator(tigl::CCPACSFuselage& fuselage, tigl::CCPACSPositioning &positioning)
{
    hideAllSpecializedWidgets();
    ui->positioningModificator->setPositioning(fuselage, positioning);
    ui->positioningModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->positioningModificator;
}


void ModificatorContainerWidget::setNoInterfaceWidget()
{
    hideAllSpecializedWidgets();
    if (ui->editorTabWidget) {
        ui->editorTabWidget->setCurrentIndex(0);
    }
    ui->noInterfaceWidget->setVisible(true);
    currentModificator = nullptr;
}

void ModificatorContainerWidget::setDisplayOptionsModificator()
{
    hideAllSpecializedWidgets();
    ui->displayOptionsWidget->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->displayOptionsWidget;
}

void ModificatorContainerWidget::updateDisplayOptionsIfActive(cpcr::CPACSTreeItem* item, TIGLCreatorDocument* doc, TIGLCreatorContext* context)
{
    if (!ui) return;
    // If the display options tab is currently active (assumed index 1), reload the widget
    if (ui->editorTabWidget && ui->editorTabWidget->currentIndex() == 1) {
        setDisplayOptionsModificator();
        setDisplayOptionsFromItem(item, doc, context);
    }
}

void ModificatorContainerWidget::onEditorTabChanged(int idx)
{
    if (idx == 1) {
        emit displayOptionsRequested();
        setDisplayOptionsModificator();
    }
    else if (idx == 0) {
        setNoInterfaceWidget();
        emit parametersTabRequested();
    }
}

void ModificatorContainerWidget::setDisplayOptionsFromItem(cpcr::CPACSTreeItem* item, TIGLCreatorDocument* doc, TIGLCreatorContext* context)
{
    if (!ui) return;
    if (ui->displayOptionsWidget) {
        ui->displayOptionsWidget->setFromItem(item, doc, context);
    }
}

void ModificatorContainerWidget::applyCurrentModifications()
{

    if (currentModificator != nullptr) {
        if ( currentModificator->apply() ) {
            // will create the undoCommand in modificator manager (only called if there was some modifications)
            emit undoCommandRequired();
        }
    }
    else {
        LOG(WARNING) << "ModificatorManager::applyCurrentModifications() called "
                        "but current modificator is null"
                     << std::endl;
    }
}

void ModificatorContainerWidget::applyCurrentCancellation()
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

void ModificatorContainerWidget::forwardAddProfileRequested(QString const& profile_id)
{
    emit addProfileRequested(profile_id);
}

void ModificatorContainerWidget::forwardAddWingRequested()
{
    emit addWingRequested();
}

void ModificatorContainerWidget::forwardDeleteWingRequested()
{
    emit deleteWingRequested();
}

void ModificatorContainerWidget::forwardAddFuselageRequested()
{
    emit addFuselageRequested();
}

void ModificatorContainerWidget::forwardDeleteFuselageRequested()
{
    emit deleteFuselageRequested();
}

void ModificatorContainerWidget::forwardAddSectionRequested(Ui::ElementModificatorInterface& emi)
{
    emit addSectionRequested(emi);
}

void ModificatorContainerWidget::forwardDeleteSectionRequested(Ui::ElementModificatorInterface& emi)
{
    emit deleteSectionRequested(emi);
}
