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

#include "CTiglPointWidget.h"
#include "ui_CTiglPointWidget.h"
#include "TIGLViewerCommonFunctions.h"

CTiglPointWidget::CTiglPointWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CTiglPointWidget)
{
    ui->setupUi(this);
}

CTiglPointWidget::~CTiglPointWidget()
{
    delete ui;
}

void CTiglPointWidget::setGUIFromInternal()
{
    ui->spinBoxX->setValue(internalPoint.x);
    ui->spinBoxY->setValue(internalPoint.y);
    ui->spinBoxZ->setValue(internalPoint.z);
}

void CTiglPointWidget::setInternalFromGUI()
{
    internalPoint.x = ui->spinBoxX->value();
    internalPoint.y = ui->spinBoxY->value();
    internalPoint.z = ui->spinBoxZ->value();
}

void CTiglPointWidget::setLabel(QString labelText)
{
    ui->label->setText(labelText);
}

void CTiglPointWidget::setInternal(tigl::CTiglPoint newInternalPoint)
{
    internalPoint = newInternalPoint;
    setGUIFromInternal();
}

bool CTiglPointWidget::hasChanged()
{
    bool noChangeInX = isApprox(ui->spinBoxX->value(), internalPoint.x);
    bool noChangeInY = isApprox(ui->spinBoxY->value(), internalPoint.y);
    bool noChangeInZ = isApprox(ui->spinBoxZ->value(), internalPoint.z);

    return !(noChangeInX && noChangeInY && noChangeInZ);
}
