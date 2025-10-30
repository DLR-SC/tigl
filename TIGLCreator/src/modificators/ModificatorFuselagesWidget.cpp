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

#include "ModificatorFuselagesWidget.h"
#include "ui_ModificatorFuselagesWidget.h"
#include "modificators/NewFuselageDialog.h"
#include "TIGLCreatorErrorDialog.h"
#include "DeleteDialog.h"

ModificatorFuselagesWidget::ModificatorFuselagesWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorFuselagesWidget)
{
    ui->setupUi(this);

    connect(ui->addNewFuselageBtn, SIGNAL(pressed()), this, SLOT(execNewFuselageDialog()));
    connect(ui->deleteFuselageBtn, SIGNAL(pressed()), this, SLOT(execDeleteFuselageDialog()));
}

ModificatorFuselagesWidget::~ModificatorFuselagesWidget()
{
    delete ui;
}

void ModificatorFuselagesWidget::execNewFuselageDialog()
{
    emit addFuselageRequested();
}


void ModificatorFuselagesWidget::execDeleteFuselageDialog()
{
    emit deleteFuselageRequested();
}
