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
#include "CTiglUIDManager.h"
#include <iostream>

ModificatorTransformationWidget::ModificatorTransformationWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorTransformationWidget)
{
    ui->setupUi(this);

    tiglTransformation = nullptr;
    config = nullptr;
    ui->scale->setLabel("Scale");
    ui->rotation->setLabel("Rotation");
    ui->translation->setLabel("Translation");
}

ModificatorTransformationWidget::~ModificatorTransformationWidget()
{
    delete ui;
}

bool ModificatorTransformationWidget::apply()
{
    if( tiglTransformation && config &&
            (ui->scale->hasChanged() || ui->rotation->hasChanged() || ui->translation->hasChanged())) {
        // first we set the internal from the UI
        ui->scale->setInternalFromGUI();
        ui->rotation->setInternalFromGUI();
        ui->translation->setInternalFromGUI();
        // apply the transformation in cpacs order: scale, rotation euler (XYZ),translation
        tiglTransformation->reset();
        tiglTransformation->setScaling(ui->scale->getInternalPoint());
        tiglTransformation->setRotation(ui->rotation->getInternalPoint());
        tiglTransformation->setTranslation(ui->translation->getInternalPoint());
        // used to invalidate all structure because otherwise the internal value of tigl is no update
        // and we are not sure about which part is to updated
        // TODO find a way to avoid invalidate the whole configuration
        config ->Invalidate();
        return true;
    }
    return false;
}

void ModificatorTransformationWidget::reset()
{
    setSpinBoxesFromInternal();
}

void ModificatorTransformationWidget::setTransformation(tigl::CCPACSTransformation& newTiglTransformation, tigl::CCPACSConfiguration& config)
{
    this->config = &config;
    this->tiglTransformation = &newTiglTransformation;
    setSpinBoxesFromInternal();
}

void ModificatorTransformationWidget::setSpinBoxesFromInternal()
{
    if (tiglTransformation == nullptr) {
        return;
    }

    tigl::CTiglPoint scaling = tiglTransformation->getScaling();
    ui->scale->setInternal(scaling);

    tigl::CTiglPoint rotation = tiglTransformation->getRotation();
    ui->rotation->setInternal(rotation);

    tigl::CTiglPoint translation = tiglTransformation->getTranslationVector();
    ui->translation->setInternal(translation);
}

