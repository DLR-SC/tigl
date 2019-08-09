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

#include "NewWingDialog.h"
#include "ui_NewWingDialog.h"

NewWingDialog::NewWingDialog(QStringList profilesUID, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewWingDialog)
{
    ui->setupUi(this);
    ui->airfoilComboBox->addItems(profilesUID);
    ui->uidLineEdit->setText("generatedWing");
}

NewWingDialog::~NewWingDialog()
{
    delete ui;
}

int NewWingDialog::getNbSection() const
{
    return ui->nbSectionSpinBox->value();
}

QString NewWingDialog::getUID() const
{
    return ui->uidLineEdit->text();
}

QString NewWingDialog::getProfileUID() const
{
    return ui->airfoilComboBox->currentText();
}
