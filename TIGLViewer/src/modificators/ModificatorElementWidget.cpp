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

#include "ModificatorElementWidget.h"
#include "ui_ModificatorElementWidget.h"
#include "CTiglError.h"
#include "TIGLViewerErrorDialog.h"

ModificatorElementWidget::ModificatorElementWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorElementWidget)
{
    ui->setupUi(this);
}



ModificatorElementWidget::~ModificatorElementWidget()
{
    delete ui;
}



void ModificatorElementWidget::setElement(tigl::CTiglSectionElement& inElement,  ProfilesDBManager* profilesDB)
{


    element = &inElement;
    this->profilesDB = profilesDB;

    ui->center->setInternal(element->GetCenter());
    ui->center->setLabel("Center");
    ui->normal->setInternal(element->GetNormal());
    ui->normal->setLabel("Normal");

    internalRotAroundN = element->GetRotationAroundNormal();
    ui->rotAroundNSpinBox->setValue(internalRotAroundN);

    internalHeight = element->GetHeight();
    ui->heightSpinBox->setValue(internalHeight);
    internalWidth = element->GetWidth();
    ui->widthSpinBox->setValue(internalWidth);
    internalArea = element->GetArea();
    ui->areaSpinBox->setValue(internalArea);

    // we add the suffix for the config, so that the profile name has the same format as the ones in the DB
    ui->profileComboBox->clear();
    internalProfileUID = element->GetProfileUID().c_str() + profilesDB->getConfigSuffix();
    ui->profileComboBox->addItems(profilesDB->getAllAvailableProfilesFor(element));
    int index = ui->profileComboBox->findText(internalProfileUID);
    ui->profileComboBox->setCurrentIndex(index);

}

bool ModificatorElementWidget::apply()
{
    bool centerHasChanged = ui->center->hasChanged();
    bool heightHasChanged = (!isApprox(internalHeight, ui->heightSpinBox->value()) );
    bool widthHasChanged = (!isApprox(internalWidth, ui->widthSpinBox->value()));
    bool areaHasChanged = (!isApprox(internalArea, ui->areaSpinBox->value()));
    bool normalHasChanged = ui->normal->hasChanged();
    bool rotAroundNHasChanged = (!isApprox(internalRotAroundN , ui->rotAroundNSpinBox->value()));
    bool profileHasChanged = ( internalProfileUID != ui->profileComboBox->currentText() );
    bool wasModified      = false;

    if (centerHasChanged) {
        ui->center->setInternalFromGUI();
        element->SetCenter(ui->center->getInternalPoint());
        wasModified = true;
    }

    if (heightHasChanged ) {
        internalHeight = ui->heightSpinBox->value();
        element->SetHeight(internalHeight);
        wasModified = true;
    }

    if (widthHasChanged ) {
        internalWidth = ui->widthSpinBox->value();
        element->SetWidth(internalWidth);
        wasModified = true;
    }

    if (areaHasChanged) {
        internalArea = ui->areaSpinBox->value();
        element->SetArea(internalArea);
        wasModified = true;
    }

    if (normalHasChanged) {
        ui->normal->setInternalFromGUI();
        element->SetNormal(ui->normal->getInternalPoint());
        wasModified = true;
    }

    if (rotAroundNHasChanged ) {
        internalRotAroundN = ui->rotAroundNSpinBox->value();
        element->SetRotationAroundNormal(internalRotAroundN);
        wasModified = true;
    }

    if (profileHasChanged) {
        internalProfileUID = ui->profileComboBox->currentText();
        try {
            if (!profilesDB->hasProfileConfigSuffix(internalProfileUID)) {
                profilesDB->copyProfileFromLocalToConfig(internalProfileUID);
            }
            element->SetProfileUID(profilesDB->removeSuffix(internalProfileUID).toStdString());
            wasModified = true;
            }
        catch (const tigl::CTiglError& err) {
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

void ModificatorElementWidget::reset()
{
    setElement(*element, profilesDB);
}