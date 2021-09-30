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

#include "NewConnectedElementDialog.h"
#include "ui_NewConnectedElementDialog.h"

NewConnectedElementDialog::NewConnectedElementDialog(QStringList connectedElements, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NewConnectedElementDialog)
{
    ui->setupUi(this);
    ui->comboBoxStartUID->addItems(connectedElements);
    ui->comboBoxWhere->addItem("After");
    ui->comboBoxWhere->addItem("Before");
}

NewConnectedElementDialog::~NewConnectedElementDialog()
{
    delete ui;
}

QString NewConnectedElementDialog::getStartUID()
{
    return ui->comboBoxStartUID->currentText();
    ;
}

NewConnectedElementDialog::Where NewConnectedElementDialog::getWhere()
{
    if (ui->comboBoxWhere->currentText() == "Before") {
        return Before;
    }
    return After;
}
