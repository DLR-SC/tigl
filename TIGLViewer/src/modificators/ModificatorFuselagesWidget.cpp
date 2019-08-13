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
#include "TIGLViewerErrorDialog.h"
#include "DeleteDialog.h"

ModificatorFuselagesWidget::ModificatorFuselagesWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorFuselagesWidget)
{
    ui->setupUi(this);
    fuselages = nullptr;

    connect(ui->addNewFuselageBtn, SIGNAL(pressed()), this, SLOT(execNewFuselageDialog()));
    connect(ui->deleteFuselageBtn, SIGNAL(pressed()), this, SLOT(execDeleteFuselageDialog()));
}

ModificatorFuselagesWidget::~ModificatorFuselagesWidget()
{
    delete ui;
}

void ModificatorFuselagesWidget::setFuselages(tigl::CCPACSFuselages& fuselages, QStringList profilesUID)
{
    this->fuselages = &fuselages;
    this->profilesUID = profilesUID;
}

void ModificatorFuselagesWidget::execNewFuselageDialog()
{
    NewFuselageDialog fuselageDialog(profilesUID, this);
    if (fuselages != nullptr && fuselageDialog.exec() == QDialog::Accepted) {
        int nbSection       = fuselageDialog.getNbSection();
        QString uid         = fuselageDialog.getUID();
        QString profilesUID = fuselageDialog.getProfileUID();
        try {
            fuselages->CreateFuselage(uid.toStdString(), nbSection, profilesUID.toStdString());
        }
        catch (const tigl::CTiglError& err) {
            TIGLViewerErrorDialog errDialog(this);
            errDialog.setMessage(
                QString("<b>%1</b><br /><br />%2").arg("Fail to create the fuselage ").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            return;
        }
        emit undoCommandRequired();
    }
}


void ModificatorFuselagesWidget::execDeleteFuselageDialog()
{
    if (fuselages == nullptr) {
        LOG(ERROR) << "ModificatorFuselagesWidget::execDeleteFuselageDialog fuselages is not set!";
        return;
    }

    QStringList wingUIDs;
    for (int i = 1; i <= fuselages->GetFuselageCount(); i++) {
        wingUIDs.push_back(fuselages->GetFuselage(i).GetUID().c_str());
    }

    DeleteDialog deleteDialog(wingUIDs);
    if (deleteDialog.exec() == QDialog::Accepted) {
        QString uid = deleteDialog.getUIDToDelete();
        try {
            tigl::CCPACSFuselage& fuselage = fuselages->GetFuselage(uid.toStdString());
            fuselages->RemoveFuselage(fuselage);
        }
        catch (const tigl::CTiglError& err) {
            TIGLViewerErrorDialog errDialog(this);
            errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to delete the fuselage ").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            return;
        }
        emit undoCommandRequired();
    }
}