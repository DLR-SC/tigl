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

#include "ModificatorElementWidget.h"
#include "ui_ModificatorElementWidget.h"

ModificatorElementWidget::ModificatorElementWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorElementWidget)
{
    ui->setupUi(this);

    // connect change alterable
    connect(ui->widthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setWidth(double)));
    connect(ui->heightSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setHeight(double)));
    connect(ui->areaSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setArea(double)));

}

void ModificatorElementWidget::setWidth(double newWidth)
{
    bool block1 = ui->areaSpinBox->blockSignals(true);
    bool block2 = ui->heightSpinBox->blockSignals(true);
    bool block3 = ui->widthSpinBox->blockSignals(true);

    if (fabs(internalWidth) > 0.0001) {
        double scaleFactor = newWidth / internalWidth;
        ui->areaSpinBox->setValue(scaleFactor * scaleFactor * internalArea);
        ui->heightSpinBox->setValue(scaleFactor * internalHeight);
    }
    lastModifiedDimensionalParameter = "width";

    ui->areaSpinBox->blockSignals(block1);
    ui->heightSpinBox->blockSignals(block2);
    ui->widthSpinBox->blockSignals(block3);
}

void ModificatorElementWidget::setHeight(double newHeight)
{
    bool block1 = ui->areaSpinBox->blockSignals(true);
    bool block2 = ui->heightSpinBox->blockSignals(true);
    bool block3 = ui->widthSpinBox->blockSignals(true);

    if (fabs(internalHeight) > 0.0001) {
        double scaleFactor = newHeight / internalHeight;
        ui->areaSpinBox->setValue(scaleFactor * scaleFactor * internalArea);
        ui->widthSpinBox->setValue(scaleFactor * internalWidth);
    }

    lastModifiedDimensionalParameter = "height";

    ui->areaSpinBox->blockSignals(block1);
    ui->heightSpinBox->blockSignals(block2);
    ui->widthSpinBox->blockSignals(block3);
}

void ModificatorElementWidget::setArea(double newArea)
{
    bool block1 = ui->areaSpinBox->blockSignals(true);
    bool block2 = ui->heightSpinBox->blockSignals(true);
    bool block3 = ui->widthSpinBox->blockSignals(true);

    if (fabs(internalHeight) > 0.0001) {
        double scaleFactor = sqrt(newArea / internalArea);
        ui->widthSpinBox->setValue(scaleFactor * internalWidth);
        ui->heightSpinBox->setValue(scaleFactor * internalHeight);
    }

    lastModifiedDimensionalParameter = "area";

    ui->areaSpinBox->blockSignals(block1);
    ui->heightSpinBox->blockSignals(block2);
    ui->widthSpinBox->blockSignals(block3);
}

ModificatorElementWidget::~ModificatorElementWidget()
{
    delete ui;
}



void ModificatorElementWidget::setElement(tigl::CTiglSectionElement& inElement)
{

    bool block1 = ui->areaSpinBox->blockSignals(true);
    bool block2 = ui->heightSpinBox->blockSignals(true);
    bool block3 = ui->widthSpinBox->blockSignals(true);

    element = &inElement;
    ui->center->setInternal(element->GetCenter());
    ui->center->setLabel("Center");
    ui->origin->setInternal(element->GetOrigin());
    ui->origin->setLabel("Origin");
    ui->normal->setInternal(element->GetNormal());
    ui->normal->setLabel("Normal");
    internalHeight = element->GetHeight();
    ui->heightSpinBox->setValue(internalHeight);
    internalWidth = element->GetWidth();
    ui->widthSpinBox->setValue(internalWidth);
    internalArea = element->GetArea();
    ui->areaSpinBox->setValue(internalArea);

    lastModifiedDimensionalParameter = "";

    ui->areaSpinBox->blockSignals(block1);
    ui->heightSpinBox->blockSignals(block2);
    ui->widthSpinBox->blockSignals(block3);


}

bool ModificatorElementWidget::apply()
{
    bool centerHasChanged = ui->center->hasDiff();
    bool originHasChanged = ui->origin->hasDiff();
    bool wasModified      = false;

    if (centerHasChanged) {
        ui->center->setInternalFromGUI();
        element->SetCenter(ui->center->getInternalPoint());
        wasModified = true;
    }
    if (originHasChanged) {
        ui->origin->setInternalFromGUI();
        element->SetOrigin(ui->origin->getInternalPoint());
        wasModified = true;
    }

    if (lastModifiedDimensionalParameter == "width" && (!isApprox(internalWidth, ui->widthSpinBox->value()))) {
        internalWidth = ui->widthSpinBox->value();
        element->SetWidth(internalWidth);
        wasModified = true;
    }

    if (lastModifiedDimensionalParameter == "height" && (!isApprox(internalHeight, ui->heightSpinBox->value()))) {
        internalHeight = ui->heightSpinBox->value();
        element->SetHeight(internalHeight);
        wasModified = true;
    }

    if (lastModifiedDimensionalParameter == "area" && (!isApprox(internalArea, ui->areaSpinBox->value()))) {
        internalArea = ui->areaSpinBox->value();
        element->SetArea(internalArea);
        wasModified = true;
    }

    if (wasModified) {
        // we reset to be sure that each internal values is correctly set
        reset();
    }
    return wasModified;
}

void ModificatorElementWidget::reset()
{
    setElement(*element);
}