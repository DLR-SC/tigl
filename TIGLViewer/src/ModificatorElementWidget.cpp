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
}

ModificatorElementWidget::~ModificatorElementWidget()
{
    delete ui;
}

void ModificatorElementWidget::setElement(tigl::CTiglSectionElement& inElement)
{
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