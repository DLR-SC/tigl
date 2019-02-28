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

ModificatorContainerWidget::ModificatorContainerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ModificatorContainerWidget)
{
    ui->setupUi(this);

    setNoInterfaceWidget();

    connect(ui->commitButton, SIGNAL(pressed()), this, SLOT(applyCurrentModifications()));

    connect(ui->cancelButton, SIGNAL(pressed()), this, SLOT(applyCurrentCancellation()));
}

ModificatorContainerWidget::~ModificatorContainerWidget()
{
    delete ui;
}

void ModificatorContainerWidget::hideAllSecializedWidgets()
{
    bool visible = false;
    ui->transformationModificator->setVisible(visible);
    ui->wingModificator->setVisible(visible);
    ui->fuselageModificator->setVisible(visible);
    ui->applyWidget->setVisible(visible);
    ui->noInterfaceWidget->setVisible(visible);
    currentModificator = nullptr;
}

void ModificatorContainerWidget::setTransformationModificator(tigl::CCPACSTransformation& transformation)
{
    hideAllSecializedWidgets();
    ui->transformationModificator->setTransformation(transformation);
    ui->transformationModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->transformationModificator;
}

void ModificatorContainerWidget::setWingModificator(tigl::CCPACSWing& wing)
{
    hideAllSecializedWidgets();
    ui->wingModificator->setWing(wing);
    ui->wingModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->wingModificator;
}

void ModificatorContainerWidget::setFuselageModificator(tigl::CCPACSFuselage& fuselage)
{
    hideAllSecializedWidgets();
    ui->fuselageModificator->setFuselage(fuselage);
    ui->fuselageModificator->setVisible(true);
    ui->applyWidget->setVisible(true);
    currentModificator = ui->fuselageModificator;
}

void ModificatorContainerWidget::setNoInterfaceWidget()
{
    hideAllSecializedWidgets();
    ui->noInterfaceWidget->setVisible(true);
    currentModificator = nullptr;
}

void ModificatorContainerWidget::applyCurrentModifications()
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