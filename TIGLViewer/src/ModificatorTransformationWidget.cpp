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

#include "ModificatorTransformationWidget.h"
#include "ui_ModificatorTransformationWidget.h"
#include <iostream>

ModificatorTransformationWidget::ModificatorTransformationWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorTransformationWidget)
{
    ui->setupUi(this);

    tiglTransformation = nullptr;
    setSpinBoxesFromInternal();
}

ModificatorTransformationWidget::~ModificatorTransformationWidget()
{
    delete ui;
}

void ModificatorTransformationWidget::apply()
{
    // todo check if there are relevent changes
    // setInternalFromSpinBoxes();
    // TODO: save in tigl object
    // Not done yet because we first need to have  a working
    // setSpinBoxesFromInternal
    // tiglTransformation->setTransformationMatrix(transformation);
}

void ModificatorTransformationWidget::reset()
{
    setSpinBoxesFromInternal();
}

void ModificatorTransformationWidget::setTransformation(tigl::CCPACSTransformation& newTiglTransformation)
{

    this->tiglTransformation = &newTiglTransformation;
    setSpinBoxesFromInternal();
}

void ModificatorTransformationWidget::setSpinBoxesFromInternal()
{
    if (tiglTransformation == nullptr) {
        return;
    }

    tigl::CTiglPoint scaling = tiglTransformation->getScaling();
    ui->spinBoxSX->setValue(scaling.x);
    ui->spinBoxSY->setValue(scaling.y);

    ui->spinBoxSZ->setValue(scaling.z);

    tigl::CTiglPoint rotation = tiglTransformation->getRotation();
    ui->spinBoxRX->setValue(rotation.x);
    ui->spinBoxRY->setValue(rotation.y);
    ui->spinBoxRZ->setValue(rotation.z);

    tigl::CTiglPoint translation = tiglTransformation->getTranslationVector();
    ui->spinBoxTX->setValue(translation.x);
    ui->spinBoxTY->setValue(translation.y);
    ui->spinBoxTZ->setValue(translation.z);
}

void ModificatorTransformationWidget::setInternalFromSpinBoxes()
{
    if (tiglTransformation == nullptr) {
        return;
    }
    // apply the transformation in cpacs order: scale, rotation euler (XYZ),
    // translation
    // todo verifiy the behavior how to save into tigl and then into the xml
    tiglTransformation->reset();
    tiglTransformation->setScaling(
        tigl::CTiglPoint(ui->spinBoxSX->value(), ui->spinBoxSY->value(), ui->spinBoxSZ->value()));
    tiglTransformation->setRotation(
        tigl::CTiglPoint(ui->spinBoxRX->value(), ui->spinBoxRY->value(), ui->spinBoxRZ->value()));
    tiglTransformation->setTranslation(
        tigl::CTiglPoint(ui->spinBoxTX->value(), ui->spinBoxTY->value(), ui->spinBoxTZ->value()));
}
