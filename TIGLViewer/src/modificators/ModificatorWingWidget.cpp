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

ModificatorWingWidget::ModificatorWingWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorWingWidget)
{
    ui->setupUi(this);
    tiglWing = nullptr;
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
    ui->spinBoxSweep->setValue(-1.0);
    ui->spinBoxArea->setValue(-1.0);
    ui->spinBoxAR->setValue(-1.0);
    ui->spinBoxSpan->setValue(-1.0);

    ui->comboBoxSymmetry->addItem("x-y-plane");
    ui->comboBoxSymmetry->addItem("x-z-plane");
    ui->comboBoxSymmetry->addItem("y-z-plane");
    ui->comboBoxSymmetry->addItem("no-symmetry");

    ui->rootLE->setLabel(QString("Root LE"));

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

void ModificatorWingWidget::setWing(tigl::CCPACSWing& wing)
{
    tiglWing = &wing;

    // set constant between ar, span and area
    setARConstant(true);

    internalSpan = wing.GetWingHalfSpan();
    ui->spinBoxSpan->setValue(internalSpan);
    internalAR = wing.GetAspectRatio();
    ui->spinBoxAR->setValue(internalAR);
    internalArea = wing.GetReferenceArea();
    ui->spinBoxArea->setValue(internalArea);

    ui->spinBoxSweepChord->setValue(0.25);
    updateSweepAccordingChordValue();   // call it explicitly because if the value of the chord is similar has before
                                        // the signal "valueChanged" will not rise up and sweep value will not be updated
    ui->spinBoxDihedralChord->setValue(0.25);
    updateDihedralAccordingChordValue();

    ui->rootLE->setInternal(wing.GetRootLEPosition());

}

void ModificatorWingWidget::reset()
{
    if (tiglWing != nullptr) {
        this->setWing(*tiglWing);
    }
    else {
        LOG(WARNING) << "ModificatorWingWidget: reset call but wing is not set!";
    }
}

bool ModificatorWingWidget::apply()
{

    bool anchorHasChanged = false;

    bool symmetryHasChanged = (internalSymmetry != ui->comboBoxSymmetry->currentText());

    bool sweepHasChanged = ((!isApprox(internalSweep, ui->spinBoxSweep->value())) ||
                            (!isApprox(internalSweepChord, ui->spinBoxSweepChord->value())) );

    bool dihedralHasChanged = ((!isApprox(internalDihedral, ui->spinBoxDihedral->value())) ||
                               (!isApprox(internalDihedralChord, ui->spinBoxDihedralChord->value())));

    bool spanHasChanged = (!isApprox(internalSpan, ui->spinBoxSpan->value()));

    bool aRHasChanged = (!isApprox(internalAR, ui->spinBoxAR->value()));

    bool areaXYHasChanged = (!isApprox(internalArea, ui->spinBoxArea->value()));

    if (anchorHasChanged) {


        // todo
    }

    if (symmetryHasChanged) {
        internalSymmetry = ui->comboBoxSymmetry->currentText();

        // todo
    }

    if (sweepHasChanged) {
        internalSweep      = ui->spinBoxSweep->value();
        internalSweepChord = ui->spinBoxSweepChord->value();

        // todo
    }

    if (dihedralHasChanged) {
        internalDihedral      = ui->spinBoxDihedral->value();
        internalDihedralChord = ui->spinBoxDihedralChord->value();

        // todo
    }

    if (areaXYHasChanged) {
        internalArea = ui->spinBoxArea->value();
        if (ui->checkBoxIsSpanConstant->isChecked()) {

            // todo
        }
        else if (ui->checkBoxIsARConstant->isChecked()) {

            // todo
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set area called, but not correct "
                          "constant checkbox set";
        }
    }

    if (spanHasChanged) {
        internalSpan = ui->spinBoxSpan->value();
        if (ui->checkBoxIsAreaConstant->isChecked()) {

            // todo
        }
        else if (ui->checkBoxIsARConstant->isChecked()) {

            // todo
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set span called, but not correct "
                          "constant checkbox set";
        }
    }

    if (aRHasChanged) {
        internalAR = ui->spinBoxAR->value();
        if (ui->checkBoxIsAreaConstant->isChecked()) {

            // todo
        }
        else if (ui->checkBoxIsSpanConstant->isChecked()) {

            // todo
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set AR called, but not correct "
                          "constant checkbox set";
        }
    }

    return false;
}