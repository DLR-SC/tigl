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

#include "ModificatorFuselageWidget.h"
#include "ui_ModificatorFuselageWidget.h"
#include "CCPACSConfiguration.h"

ModificatorFuselageWidget::ModificatorFuselageWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorFuselageWidget)
{
    ui->setupUi(this);

    connect(ui->comboBoxE1, SIGNAL(currentIndexChanged(int)), this, SLOT(recomputeProperties()));
    connect(ui->comboBoxE2, SIGNAL(currentIndexChanged(int)), this, SLOT(recomputeProperties()));

    connect(ui->spinBoxWidth, SIGNAL(valueChanged(double)), this, SLOT(setWidth(double)));
    connect(ui->spinBoxHeight, SIGNAL(valueChanged(double)), this, SLOT(setHeight(double)));
    connect(ui->spinBoxArea, SIGNAL(valueChanged(double)), this, SLOT(setArea(double)));
}

ModificatorFuselageWidget::~ModificatorFuselageWidget()
{
    delete ui;
}

void ModificatorFuselageWidget::setFuselage(tigl::CCPACSFuselage& newFuselage)
{

    this->fuselage = &newFuselage;

    // we disconnect signals from comboboxes because we do not want to call this routine when we set the combobox item
    bool block1 = ui->comboBoxE1->blockSignals(true);
    bool block2 = ui->comboBoxE2->blockSignals(true);

    std::vector<std::string> tempElementUIDs = fuselage->GetElementUIDsInOrder();
    QStringList elementsUids;
    for (int i = 0; i < tempElementUIDs.size(); i++) {
        elementsUids.push_back(QString(tempElementUIDs[i].c_str()));
    }

    ui->comboBoxE1->clear();
    ui->comboBoxE1->addItems(elementsUids);
    ui->comboBoxE2->clear();
    ui->comboBoxE2->addItems(elementsUids);
    ui->comboBoxE2->setCurrentIndex(elementsUids.size() - 1); // set the last element of the list

    recomputeProperties();

    lastModifiedDimensionalParameter = "";

    ui->noiseCenter->setInternal(fuselage->GetNoiseCenter());
    ui->noiseCenter->setLabel("NoiseCenter");


    ui->comboBoxE1->blockSignals(block1);
    ui->comboBoxE2->blockSignals(block2);
}

bool ModificatorFuselageWidget::apply()
{

    bool lengthHasChanged = ((!isApprox(internalLength, ui->spinBoxLength->value())));

    bool areaHasChanged   = (!(isApprox(internalArea, ui->spinBoxArea->value())));
    bool widthHasChanged  = (!(isApprox(internalWidth, ui->spinBoxWidth->value())));
    bool heightHasChanged = (!(isApprox(internalHeight, ui->spinBoxHeight->value())));

    bool noiseCenterHasChanged = ui->noiseCenter->hasChanged();

    bool wasModified = false;

    QString uid1 = ui->comboBoxE1->currentText();
    QString uid2 = ui->comboBoxE2->currentText();

    if(noiseCenterHasChanged) {
        ui->noiseCenter->setInternalFromGUI();
        tigl::CTiglPoint newCenter = ui->noiseCenter->getInternalPoint();
        fuselage->SetNoiseCenter(newCenter);
        wasModified = true;
    }

    if (lengthHasChanged) {
        internalLength = ui->spinBoxLength->value();
        fuselage->SetLengthBetween(uid1.toStdString(), uid2.toStdString(), internalLength);
        wasModified = true;
    }

    if (areaHasChanged && lastModifiedDimensionalParameter == "area") {
        internalArea = ui->spinBoxArea->value();
        fuselage->SetMaxAreaBetween(internalArea, uid1.toStdString(), uid2.toStdString());
        wasModified = true;
    }

    if (widthHasChanged && lastModifiedDimensionalParameter == "width") {
        internalWidth = ui->spinBoxWidth->value();
        fuselage->SetMaxWidthBetween(internalWidth, uid1.toStdString(), uid2.toStdString());
        wasModified = true;
    }

    if (heightHasChanged && lastModifiedDimensionalParameter == "height") {
        internalHeight = ui->spinBoxHeight->value();
        fuselage->SetMaxHeightBetween(internalHeight, uid1.toStdString(), uid2.toStdString());
        wasModified = true;
    }

    if (wasModified) {
        // we reset to be sure that each internal values is correctly set
        reset();
    }

    return wasModified;
}

void ModificatorFuselageWidget::reset()
{
    if (fuselage != nullptr) {
        this->setFuselage(*fuselage);
    }
    else {
        LOG(WARNING) << "ModificatorWingWidget: reset call but wing is not set!";
    }
}

void ModificatorFuselageWidget::recomputeProperties()
{
    bool block1 = ui->spinBoxArea->blockSignals(true);
    bool block2 = ui->spinBoxHeight->blockSignals(true);
    bool block3 = ui->spinBoxWidth->blockSignals(true);

    QString uid1   = ui->comboBoxE1->currentText();
    QString uid2   = ui->comboBoxE2->currentText();
    internalLength = fuselage->GetLengthBetween(uid1.toStdString(), uid2.toStdString());
    ui->spinBoxLength->setValue(internalLength);

    internalArea = fuselage->GetMaximalWireAreaBetween(uid1.toStdString(), uid2.toStdString());
    ui->spinBoxArea->setValue(internalArea);

    internalWidth = fuselage->GetMaximalWidthBetween(uid1.toStdString(), uid2.toStdString());
    ui->spinBoxWidth->setValue(internalWidth);

    internalHeight = fuselage->GetMaximalHeightBetween(uid1.toStdString(), uid2.toStdString());
    ui->spinBoxHeight->setValue(internalHeight);

    ui->spinBoxArea->blockSignals(block1);
    ui->spinBoxHeight->blockSignals(block2);
    ui->spinBoxWidth->blockSignals(block3);
}

void ModificatorFuselageWidget::setWidth(double newWidth)
{
    bool block1 = ui->spinBoxArea->blockSignals(true);
    bool block2 = ui->spinBoxHeight->blockSignals(true);
    bool block3 = ui->spinBoxWidth->blockSignals(true);

    if (fabs(internalWidth) > 0.0001) {
        double scaleFactor = newWidth / internalWidth;
        ui->spinBoxArea->setValue(scaleFactor * scaleFactor * internalArea);
        ui->spinBoxHeight->setValue(scaleFactor * internalHeight);
    }
    lastModifiedDimensionalParameter = "width";

    ui->spinBoxArea->blockSignals(block1);
    ui->spinBoxHeight->blockSignals(block2);
    ui->spinBoxWidth->blockSignals(block3);
}

void ModificatorFuselageWidget::setHeight(double newHeight)
{
    bool block1 = ui->spinBoxArea->blockSignals(true);
    bool block2 = ui->spinBoxHeight->blockSignals(true);
    bool block3 = ui->spinBoxWidth->blockSignals(true);

    if (fabs(internalHeight) > 0.0001) {
        double scaleFactor = newHeight / internalHeight;
        ui->spinBoxArea->setValue(scaleFactor * scaleFactor * internalArea);
        ui->spinBoxWidth->setValue(scaleFactor * internalWidth);
    }

    lastModifiedDimensionalParameter = "height";

    ui->spinBoxArea->blockSignals(block1);
    ui->spinBoxHeight->blockSignals(block2);
    ui->spinBoxWidth->blockSignals(block3);
}

void ModificatorFuselageWidget::setArea(double newArea)
{
    bool block1 = ui->spinBoxArea->blockSignals(true);
    bool block2 = ui->spinBoxHeight->blockSignals(true);
    bool block3 = ui->spinBoxWidth->blockSignals(true);

    if (fabs(internalHeight) > 0.0001) {
        double scaleFactor = sqrt(newArea / internalArea);
        ui->spinBoxWidth->setValue(scaleFactor * internalWidth);
        ui->spinBoxHeight->setValue(scaleFactor * internalHeight);
    }

    lastModifiedDimensionalParameter = "area";

    ui->spinBoxArea->blockSignals(block1);
    ui->spinBoxHeight->blockSignals(block2);
    ui->spinBoxWidth->blockSignals(block3);
}
