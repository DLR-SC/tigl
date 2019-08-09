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
#include "TIGLViewerErrorDialog.h"
#include "NewWingDialog.h"

ModificatorWingsWidget::ModificatorWingsWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorWingsWidget)
{
    ui->setupUi(this);
    wings = nullptr;

    connect(ui->addNewWingBtn, SIGNAL(pressed()), this, SLOT(execNewWingDialog()));
}

ModificatorWingsWidget::~ModificatorWingsWidget()
{
    delete ui;
}

void ModificatorWingsWidget::setWings(tigl::CCPACSWings& wings, QStringList profilesUID)
{
    this->wings       = &wings;
    this->profilesUID = profilesUID;
}

void ModificatorWingsWidget::execNewWingDialog()
{
    NewWingDialog wingDialog(profilesUID, this);
    if (wings != nullptr && wingDialog.exec() == QDialog::Accepted) {
        int nbSection       = wingDialog.getNbSection();
        QString uid         = wingDialog.getUID();
        QString profilesUID = wingDialog.getProfileUID();
        try {
            wings->CreateWing(uid.toStdString(), nbSection, profilesUID.toStdString());
        }
        catch (const tigl::CTiglError& err) {
            TIGLViewerErrorDialog errDialog(this);
            errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to create the wing ").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            return;
        }
        emit undoCommandRequired();
    }
}
