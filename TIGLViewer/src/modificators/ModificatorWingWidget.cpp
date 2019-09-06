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

#include "ModificatorWingWidget.h"
#include "ui_ModificatorWingWidget.h"
#include "CTiglLogging.h"
#include "TIGLViewerErrorDialog.h"

ModificatorWingWidget::ModificatorWingWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorWingWidget)
{
    ui->setupUi(this);
    tiglWing = nullptr;
    profilesDB = nullptr;
    this->init();
}

ModificatorWingWidget::~ModificatorWingWidget()
{
    delete ui;
}

void ModificatorWingWidget::init()
{
    // set the initials values of the display interface (will be overwritten
    // when the wingItem is set)
    ui->spinBoxSweep->setValue(-1.0);
    ui->spinBoxDihedral->setValue(-1.0);
    ui->spinBoxArea->setValue(-1.0);
    ui->spinBoxAR->setValue(-1.0);
    ui->spinBoxSpan->setValue(-1.0);

    // for the moment changing the AR is not supported
    ui->spinBoxAR->setReadOnly(true); 
    
    ui->rootLE->setLabel(QString("Root LE"));
    ui->rotation->setLabel(QString("Rotation"));

    // alterable span area ar
    setARConstant(true);

    // connect change alterable
    connect(ui->checkBoxIsAreaConstant, SIGNAL(clicked(bool)), this, SLOT(setAreaConstant(bool)));
    connect(ui->checkBoxIsSpanConstant, SIGNAL(clicked(bool)), this, SLOT(setSpanConstant(bool)));
    connect(ui->checkBoxIsARConstant, SIGNAL(clicked(bool)), this, SLOT(setARConstant(bool)));

    connect(ui->spinBoxSweepChord, SIGNAL(valueChanged(double)), this, SLOT(updateSweepAccordingChordValue(double)) );
    connect(ui->spinBoxDihedralChord, SIGNAL(valueChanged(double)), this, SLOT(updateDihedralAccordingChordValue(double)) );
}


void ModificatorWingWidget::updateSweepAccordingChordValue(double)
{
    internalSweepChord = ui->spinBoxSweepChord->value();
    internalSweep = tiglWing->GetSweep(internalSweepChord);
    ui->spinBoxSweep->setValue(internalSweep);
}

void ModificatorWingWidget::updateDihedralAccordingChordValue(double)
{
    internalDihedralChord = ui->spinBoxDihedralChord->value();
    internalDihedral = tiglWing->GetDihedral(internalDihedralChord);
    ui->spinBoxDihedral->setValue(internalDihedral);
}


void ModificatorWingWidget::setAreaConstant(bool checked)
{
    ui->checkBoxIsARConstant->setChecked(false);
    ui->spinBoxAR->setReadOnly(false);
    ui->checkBoxIsSpanConstant->setChecked(false);
    ui->spinBoxSpan->setReadOnly(false);
    ui->checkBoxIsAreaConstant->setChecked(true);
    ui->spinBoxArea->setReadOnly(true);
}

void ModificatorWingWidget::setSpanConstant(bool checked)
{

    ui->checkBoxIsARConstant->setChecked(false);
    ui->spinBoxAR->setReadOnly(false);
    ui->checkBoxIsAreaConstant->setChecked(false);
    ui->spinBoxArea->setReadOnly(false);
    ui->checkBoxIsSpanConstant->setChecked(true);
    ui->spinBoxSpan->setReadOnly(true);
}

void ModificatorWingWidget::setARConstant(bool checked)
{
    ui->checkBoxIsAreaConstant->setChecked(false);
    ui->spinBoxArea->setReadOnly(false);
    ui->checkBoxIsSpanConstant->setChecked(false);
    ui->spinBoxSpan->setReadOnly(false);
    ui->checkBoxIsARConstant->setChecked(true);
    ui->spinBoxAR->setReadOnly(true);
}

void ModificatorWingWidget::setWing(tigl::CCPACSWing& wing, ProfilesDBManager* profilesDB)
{
    tiglWing = &wing;
    this->profilesDB = profilesDB;
    
    internalSpan = tiglWing->GetWingHalfSpan();
    ui->spinBoxSpan->setValue(internalSpan);
    internalAR = tiglWing->GetAspectRatio();
    ui->spinBoxAR->setValue(internalAR);
    internalArea = tiglWing->GetReferenceArea();
    ui->spinBoxArea->setValue(internalArea);

    ui->spinBoxSweepChord->setValue(0.25);
    updateSweepAccordingChordValue();   // call it explicitly because if the value of the chord is similar has before
                                        // the signal "valueChanged" will not rise up and sweep value will not be updated
    ui->spinBoxDihedralChord->setValue(0.25);
    updateDihedralAccordingChordValue();

    ui->rootLE->setInternal(tiglWing->GetRootLEPosition());

    ui->rotation->setInternal(tiglWing->GetRotation());

    ui->symmetry->setInternal(tiglWing->GetSymmetryAxis()) ;


    ui->profileComboBox->clear();
    std::vector<std::string> profilesList = tiglWing->GetAllUsedAirfoils();
    if ( profilesList.size() == 1) {
        internalProfile = profilesList.at(0).c_str()  + profilesDB->getConfigSuffix() ;
    } else {
        internalProfile = "<multi-airfoils>";
        ui->profileComboBox->addItem(internalProfile);
    }
    ui->profileComboBox->addItems(profilesDB->getAllWingProfiles());
    int index = ui->profileComboBox->findText(internalProfile);
    ui->profileComboBox->setCurrentIndex(index);
}

void ModificatorWingWidget::reset()
{
    if (tiglWing != nullptr) {
        this->setWing(*tiglWing, profilesDB);
    }
    else {
        LOG(WARNING) << "ModificatorWingWidget: reset call but wing is not set!";
    }
}

bool ModificatorWingWidget::apply()
{

    bool rootLEHasChanged = ui->rootLE->hasChanged();

    bool rotationHasChanged = ui->rotation->hasChanged();

    bool symmetryHasChanged = ui->symmetry->hasChanged();

    bool sweepHasChanged = ((!isApprox(internalSweep, ui->spinBoxSweep->value())) ||
                            (!isApprox(internalSweepChord, ui->spinBoxSweepChord->value())) );

    bool dihedralHasChanged = ((!isApprox(internalDihedral, ui->spinBoxDihedral->value())) ||
                               (!isApprox(internalDihedralChord, ui->spinBoxDihedralChord->value())));

    bool spanHasChanged = (!isApprox(internalSpan, ui->spinBoxSpan->value()));

    bool areaXYHasChanged = (!isApprox(internalArea, ui->spinBoxArea->value()));

    bool arHasChanged = (!isApprox(internalAR, ui->spinBoxAR->value()));

    bool profileHasChanged = (internalProfile != ui->profileComboBox->currentText());


    bool wasModified = false;

    if (rootLEHasChanged) {
        ui->rootLE->setInternalFromGUI();
        tigl::CTiglPoint newRootLE = ui->rootLE->getInternalPoint();
        tiglWing->SetRootLEPosition(newRootLE);
        wasModified = true;
    }

    if (rotationHasChanged) {
        ui->rotation->setInternalFromGUI();
        tigl::CTiglPoint newRot = ui->rotation->getInternalPoint();
        tiglWing->SetRotation(newRot);
        wasModified = true;
    }

    if (symmetryHasChanged) {
        ui->symmetry->setInternalFromGUI();
        tiglWing->SetSymmetryAxis(ui->symmetry->getInternalSymmetry());
        wasModified = true; 
    }

    if (sweepHasChanged) {
        internalSweep      = ui->spinBoxSweep->value();
        internalSweepChord = ui->spinBoxSweepChord->value();
        tiglWing->SetSweep(internalSweep, internalDihedralChord);
        wasModified = true;
    }

    if (dihedralHasChanged) {
        internalDihedral      = ui->spinBoxDihedral->value();
        internalDihedralChord = ui->spinBoxDihedralChord->value();
        tiglWing->SetDihedral(internalDihedral, internalDihedralChord);
        wasModified = true;
    }

    if (areaXYHasChanged) {
        internalArea = ui->spinBoxArea->value();
        if (ui->checkBoxIsSpanConstant->isChecked()) {
            tiglWing->SetAreaKeepSpan(internalArea);
            wasModified = true;
        }
        else if (ui->checkBoxIsARConstant->isChecked()) {
            tiglWing->SetAreaKeepAR(internalArea);
            wasModified = true;
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set area called, but not correct "
                          "constant checkbox set";
        }
    }

    if (spanHasChanged) {
        internalSpan = ui->spinBoxSpan->value();
        if (ui->checkBoxIsAreaConstant->isChecked()) {
            tiglWing->SetHalfSpanKeepArea(internalSpan);
            wasModified = true;
        }
        else if (ui->checkBoxIsARConstant->isChecked()) {
            tiglWing->SetHalfSpanKeepAR(internalSpan);
            wasModified = true;
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set span called, but not correct "
                          "constant checkbox set";
        }
    }

    if (arHasChanged) {
        internalAR = ui->spinBoxAR->value();
        if (ui->checkBoxIsAreaConstant->isChecked()) {
            tiglWing->SetARKeepArea(internalAR);
            wasModified = true;
        }
        else if (ui->checkBoxIsSpanConstant->isChecked()) {
            tiglWing->SetARKeepSpan(internalAR);
            wasModified = true;
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set AR called, but not correct "
                          "constant checkbox set";
        }
    }


    if (profileHasChanged) {
        internalProfile = ui->profileComboBox->currentText();
        try {
            if (!profilesDB->hasProfileConfigSuffix(internalProfile)) {
                profilesDB->copyProfileFromLocalToConfig(internalProfile);
            }
            tiglWing->SetAllAirfoils(profilesDB->removeSuffix(internalProfile).toStdString());
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
        reset();
    }

    return wasModified;
}