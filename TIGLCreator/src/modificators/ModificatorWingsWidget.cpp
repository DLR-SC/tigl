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

#include "ModificatorWingsWidget.h"
#include "ui_ModificatorWingsWidget.h"
#include "TIGLCreatorErrorDialog.h"
#include "NewWingDialog.h"
#include "DeleteDialog.h"
#include "CCPACSWing.h"

ModificatorWingsWidget::ModificatorWingsWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorWingsWidget)
{
    ui->setupUi(this);

    connect(ui->addNewWingBtn, SIGNAL(pressed()), this, SLOT(execNewWingDialog()));
    connect(ui->deleteWingBtn, SIGNAL(pressed()), this, SLOT(execDeleteWingDialog()));
}

ModificatorWingsWidget::~ModificatorWingsWidget()
{
    delete ui;
}

void ModificatorWingsWidget::execNewWingDialog()
{
    emit addWingRequested();
}

void ModificatorWingsWidget::execDeleteWingDialog()
{
    emit deleteWingRequested();
}
