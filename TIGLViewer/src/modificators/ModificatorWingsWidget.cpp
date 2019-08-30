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
#include "DeleteDialog.h"
#include "CCPACSWing.h"

ModificatorWingsWidget::ModificatorWingsWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorWingsWidget)
{
    ui->setupUi(this);
    wings = nullptr;
    profilesDB = nullptr;

    connect(ui->addNewWingBtn, SIGNAL(pressed()), this, SLOT(execNewWingDialog()));
    connect(ui->deleteWingBtn, SIGNAL(pressed()), this, SLOT(execDeleteWingDialog()));
}

ModificatorWingsWidget::~ModificatorWingsWidget()
{
    delete ui;
}

void ModificatorWingsWidget::setWings(tigl::CCPACSWings& wings, ProfilesDBManager* profilesDB)
{
    this->wings = &wings;
    this->profilesDB = profilesDB;
}

void ModificatorWingsWidget::execNewWingDialog()
{

    NewWingDialog wingDialog(profilesDB->getAllWingProfiles(), this);
    if (wings != nullptr && wingDialog.exec() == QDialog::Accepted) {
        int nbSection       = wingDialog.getNbSection();
        QString uid         = wingDialog.getUID();
        QString profileID = wingDialog.getProfileUID();
        try {
            if ( ! profilesDB->isProfileInConfig(profileID) ) {
                profilesDB->copyProfileFromLocalToConfig(profileID) ;
            }
            wings->CreateWing(uid.toStdString(), nbSection, profilesDB->removeSuffix(profileID).toStdString());
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

void ModificatorWingsWidget::execDeleteWingDialog()
{
    if (wings == nullptr) {
        LOG(ERROR) << "ModificatorWingsWidget::execDeleteWingDialog: wings is not set!";
        return;
    }

    QStringList wingUIDs;
    for (int i = 1; i <= wings->GetWingCount(); i++) {
        wingUIDs.push_back(wings->GetWing(i).GetUID().c_str());
    }

    DeleteDialog deleteDialog(wingUIDs);
    if (deleteDialog.exec() == QDialog::Accepted) {
        QString uid = deleteDialog.getUIDToDelete();
        try {
            tigl::CCPACSWing& wing = wings->GetWing(uid.toStdString());
            wings->RemoveWing(wing);
        }
        catch (const tigl::CTiglError& err) {
            TIGLViewerErrorDialog errDialog(this);
            errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to delete the wing ").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            return;
        }
        emit undoCommandRequired();
    }
}
