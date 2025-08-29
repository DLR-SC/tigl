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
#include "CTiglError.h"
#include "CTiglLogging.h"

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

QString SymmetryComboBoxWidget::TiglSymmetryAxisToQString(TiglSymmetryAxis symmetryAxis)
{
    switch (symmetryAxis) {
    case TiglSymmetryAxis::TIGL_X_Y_PLANE:
        return QString("x-y-plane");
    case TiglSymmetryAxis::TIGL_X_Z_PLANE:
        return QString("x-z-plane");
    case TiglSymmetryAxis::TIGL_Y_Z_PLANE:
        return QString("y-z-plane");
    case TiglSymmetryAxis::TIGL_NO_SYMMETRY:
        return QString("no-symmetry");
    default:
        throw tigl::CTiglError("Unexpected symmetry axis! If a new symmetry enum was added, please update this class.");
    }
}

TiglSymmetryAxis SymmetryComboBoxWidget::QStringToTiglAxis(QString symmetry)
{
    if (symmetry == "x-y-plane") {
        return TiglSymmetryAxis::TIGL_X_Y_PLANE;
    }
    else if (symmetry == "x-z-plane") {
        return TiglSymmetryAxis::TIGL_X_Z_PLANE;
    }
    else if (symmetry == "y-z-plane") {
        return TiglSymmetryAxis::TIGL_Y_Z_PLANE;
    }
    else if (symmetry == "no-symmetry") {
        return TiglSymmetryAxis::TIGL_NO_SYMMETRY;
    }
    else {
        LOG(ERROR) << "SymmetryComboBoxWidget::QStringToTiglAxis: Unexpected symmetry axis given as input.";
        return TiglSymmetryAxis::TIGL_NO_SYMMETRY;
    }
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
    int index = ui->comboBox->findText(TiglSymmetryAxisToQString(internalSymmetry));
    ui->comboBox->setCurrentIndex(index);
}

void SymmetryComboBoxWidget::setInternalFromGUI()
{
    internalSymmetry = QStringToTiglAxis(ui->comboBox->currentText());
}

bool SymmetryComboBoxWidget::hasChanged()
{
    if (internalSymmetry == QStringToTiglAxis(ui->comboBox->currentText())) {
        return false;
    }
    else {
        return true;
    }
}

TiglSymmetryAxis SymmetryComboBoxWidget::getInternalSymmetry()
{
    return internalSymmetry;
}
