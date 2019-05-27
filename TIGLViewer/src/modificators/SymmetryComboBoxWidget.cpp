/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 * Author: Malo Drougard
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

#include "SymmetryComboBoxWidget.h"
#include "ui_SymmetryComboBoxWidget.h"

SymmetryComboBoxWidget::SymmetryComboBoxWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SymmetryComboBoxWidget)
{
    ui->setupUi(this);

    ui->comboBox->addItem("x-y-plane");
    ui->comboBox->addItem("x-z-plane");
    ui->comboBox->addItem("y-z-plane");
    ui->comboBox->addItem("no-symmetry");

    setInternal(TiglSymmetryAxis::TIGL_NO_SYMMETRY);
}

SymmetryComboBoxWidget::~SymmetryComboBoxWidget()
{
    delete ui;
}

void SymmetryComboBoxWidget::setInternal(TiglSymmetryAxis symmetryAxis)
{
    internalSymmetry = symmetryAxis;
    setGUIFromInternal();
}

void SymmetryComboBoxWidget::setGUIFromInternal()
{
    switch (internalSymmetry) {
    case TiglSymmetryAxis::TIGL_X_Y_PLANE:
        ui->comboBox->setCurrentText("x-y-plane");
        break;
    case TiglSymmetryAxis::TIGL_X_Z_PLANE:
        ui->comboBox->setCurrentText("x-z-plane");
        break;
    case TiglSymmetryAxis::TIGL_Y_Z_PLANE:
        ui->comboBox->setCurrentText("y-z-plane");
        break;
    case TiglSymmetryAxis::TIGL_NO_SYMMETRY:
        ui->comboBox->setCurrentText("no-symmetry");
        break;
    }
}

void SymmetryComboBoxWidget::setInternalFromGUI()
{
    QString currentText = ui->comboBox->currentText();
    if (currentText == "x-y-plane") {
        internalSymmetry = TiglSymmetryAxis::TIGL_X_Y_PLANE;
    }
    else if (currentText == "x-z-plane") {
        internalSymmetry = TiglSymmetryAxis::TIGL_X_Z_PLANE;
    }
    else if (currentText == "y-z-plane") {
        internalSymmetry = TiglSymmetryAxis::TIGL_Y_Z_PLANE;
    }
    else if (currentText == "no-symmetry") {
        internalSymmetry = TiglSymmetryAxis::TIGL_NO_SYMMETRY;
    }
    else {
        internalSymmetry = TiglSymmetryAxis::TIGL_NO_SYMMETRY;
    }
}
