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

#include "NewFuselageDialog.h"
#include "ui_NewFuselageDialog.h"

NewFuselageDialog::NewFuselageDialog(QStringList profilesUID, tigl::CTiglUIDManager const& uid_mgr, QWidget* parent)
    : make_unique([&](QString const& in){ return QString::fromStdString(uid_mgr.MakeUIDUnique(in.toStdString())); })
    , QDialog(parent)
    , ui(new Ui::NewFuselageDialog)
{
    ui->setupUi(this);
    ui->comboBoxProfilesUID->addItems(profilesUID);
    ui->lineEditUID->setText(make_unique("generatedFuselage"));
}

NewFuselageDialog::~NewFuselageDialog()
{
    delete ui;
}

int NewFuselageDialog::getNbSection() const
{
    return ui->spinBoxNbSection->value();
}

QString NewFuselageDialog::getUID() const
{
    QString inputUID = ui->lineEditUID->text();
    QString uniqueUID = make_unique(inputUID);

    if (uniqueUID != inputUID) {
        LOG(WARNING) << "The wanted uID " << inputUID.toStdString() << " is already taken. It is changed to the uID " << uniqueUID.toStdString() << "." << std::endl;
    }
    return uniqueUID;
}

QString NewFuselageDialog::getProfileUID() const
{
    return ui->comboBoxProfilesUID->currentText();
}
