/*
 * Copyright (C) 2018 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#include "ModificatorFuselageWidget.h"
#include "ui_ModificatorFuselageWidget.h"
#include "CCPACSConfiguration.h"
#include "TIGLViewerErrorDialog.h"

ModificatorFuselageWidget::ModificatorFuselageWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorFuselageWidget)
{
    ui->setupUi(this);
    profilesDB = nullptr;
}

ModificatorFuselageWidget::~ModificatorFuselageWidget()
{
    delete ui;
}

void ModificatorFuselageWidget::setFuselage(tigl::CCPACSFuselage& newFuselage, ProfilesDBManager* profilesDB)
{

    this->fuselage = &newFuselage;
    this->profilesDB = profilesDB;

    ui->noseCenter->setInternal(fuselage->GetNoseCenter());
    ui->noseCenter->setLabel("NoseCenter");

    ui->rotation->setInternal(fuselage->GetRotation());
    ui->rotation->setLabel("Rotation");

    internalLength = fuselage->GetLength();
    ui->spinBoxLength->setValue(internalLength);
    internalHeight = fuselage->GetMaximalHeight();
    ui->spinBoxHeight->setValue(internalHeight);
    internalWidth = fuselage->GetMaximalWidth();
    ui->spinBoxWidth->setValue(internalWidth);

    ui->profileComboBox->clear();
    std::vector<std::string> profilesList = fuselage->GetAllUsedProfiles();
    if ( profilesList.size() == 1) {
        internalProfile = profilesList.at(0).c_str()  + profilesDB->getConfigSuffix() ;
    } else {
        internalProfile = "<multi-profiles>";
        ui->profileComboBox->addItem(internalProfile);
    }
    ui->profileComboBox->addItems(profilesDB->getAllFuselagesProfiles());
    int index = ui->profileComboBox->findText(internalProfile);
    ui->profileComboBox->setCurrentIndex(index);
}

bool ModificatorFuselageWidget::apply()
{

    bool lengthHasChanged = ((!isApprox(internalLength, ui->spinBoxLength->value())));
    bool widthHasChanged  = (!(isApprox(internalWidth, ui->spinBoxWidth->value())));
    bool heightHasChanged = (!(isApprox(internalHeight, ui->spinBoxHeight->value())));

    bool noiseCenterHasChanged = ui->noseCenter->hasChanged();
    bool rotationHasChanged = ui->rotation->hasChanged();

    bool profileHasChanged = (internalProfile != ui->profileComboBox->currentText());

    bool wasModified = false;

    if (noiseCenterHasChanged) {
        ui->noseCenter->setInternalFromGUI();
        tigl::CTiglPoint newCenter = ui->noseCenter->getInternalPoint();
        fuselage->SetNoseCenter(newCenter);
        wasModified = true;
    }

    if (rotationHasChanged) {
        ui->rotation->setInternalFromGUI();
        tigl::CTiglPoint newRotation = ui->rotation->getInternalPoint();
        fuselage->SetRotation(newRotation);
        wasModified = true;
    }

    if (lengthHasChanged) {
        internalLength = ui->spinBoxLength->value();
        fuselage->SetLength(internalLength);
        wasModified = true;
    }

    if (widthHasChanged) {
        internalWidth = ui->spinBoxWidth->value();
        fuselage->SetMaxWidth(internalWidth);
        wasModified = true;
    }

    if (heightHasChanged) {
        internalHeight = ui->spinBoxHeight->value();
        fuselage->SetMaxHeight(internalHeight);
        wasModified = true;
    }

    if (profileHasChanged) {
        internalProfile = ui->profileComboBox->currentText();
        try {
            if (!profilesDB->hasProfileConfigSuffix(internalProfile)) {
                profilesDB->copyProfileFromLocalToConfig(internalProfile);
            }
            fuselage->SetAllProfiles(profilesDB->removeSuffix(internalProfile).toStdString());
            wasModified = true;
        }
        catch (const tigl::CTiglError &err) {
            TIGLViewerErrorDialog errDialog(this);
            errDialog.setMessage(QString("<b>%1</b><br /><br />%2")
                                         .arg("Fail to set the profile")
                                         .arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
        }
    }

    if (wasModified) {
        // we reset to be sure that each internal values is correctly set
        reset();
    }

    return wasModified;
}

void ModificatorFuselageWidget::reset()
{
    if (fuselage != nullptr) {
        this->setFuselage(*fuselage, profilesDB);
    }
    else {
        LOG(WARNING) << "ModificatorWingWidget: reset call but wing is not set!";
    }
}
