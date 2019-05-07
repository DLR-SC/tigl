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
    ui->spinBoxSweepChord->setValue(0);
    ui->comboBoxSweepMethod->addItem("Translation");
    ui->comboBoxSweepMethod->addItem("Shearing");
    ui->comboBoxSweepMethod->setCurrentIndex(0);

    ui->spinBoxSweep->setValue(-1.0);
    ui->spinBoxDihedralChord->setValue(0);

    ui->spinBoxArea->setValue(-1.0);

    ui->spinBoxAR->setValue(-1);

    ui->spinBoxSpan->setValue(-1);

    ui->comboBoxSymmetry->addItem("x-y-plane");
    ui->comboBoxSymmetry->addItem("x-z-plane");
    ui->comboBoxSymmetry->addItem("y-z-plane");
    ui->comboBoxSymmetry->addItem("no-symmetry");

    // alterable span area ar
    ui->checkBoxIsAreaConstant->setChecked(false);
    ui->checkBoxIsSpanConstant->setChecked(false);
    ui->checkBoxIsARConstant->setChecked(true);

    // hide the advanced options
    ui->widgetDihedralDetails->hide();
    ui->widgetSweepDetails->hide();

    // connect the extend buttons with their slot
    connect(ui->btnExpendDihedralDetails, SIGNAL(clicked(bool)), this, SLOT(expendDihedralDetails(bool)));
    connect(ui->btnExpendSweepDetails, SIGNAL(clicked(bool)), this, SLOT(expendSweepDetails(bool)));

    // connect change alterable
    connect(ui->checkBoxIsAreaConstant, SIGNAL(clicked(bool)), this, SLOT(setAreaConstant(bool)));
    connect(ui->checkBoxIsSpanConstant, SIGNAL(clicked(bool)), this, SLOT(setSpanConstant(bool)));
    connect(ui->checkBoxIsARConstant, SIGNAL(clicked(bool)), this, SLOT(setARConstant(bool)));
}

// inverse the visibility
void ModificatorWingWidget::expendDihedralDetails(bool checked)
{
    ui->widgetDihedralDetails->setVisible(!(ui->widgetDihedralDetails->isVisible()));
}

// inverse the visibility
void ModificatorWingWidget::expendSweepDetails(bool checked)
{
    ui->widgetSweepDetails->setVisible(!(ui->widgetSweepDetails->isVisible()));
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
    // TODO set the internal elements using tiglWing
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

    bool anchorHasChanged = ((!isApprox(internalAnchorX, ui->spinBoxAnchorX->value())) ||
                             (!isApprox(internalAnchorY, ui->spinBoxAnchorY->value())) ||
                             (!isApprox(internalAnchorZ, ui->spinBoxAnchorZ->value())));

    bool symmetryHasChanged = (internalSymmetry != ui->comboBoxSymmetry->currentText());

    bool sweepHasChanged = ((!isApprox(internalSweep, ui->spinBoxSweep->value())) ||
                            (!isApprox(internalSweepChord, ui->spinBoxSweepChord->value())) ||
                            internalMethod != ui->comboBoxSweepMethod->currentText());

    bool dihedralHasChanged = ((!isApprox(internalDihedral, ui->spinBoxDihedral->value())) ||
                               (!isApprox(internalDihedralChord, ui->spinBoxDihedralChord->value())));

    bool spanHasChanged = (!isApprox(internalSpan, ui->spinBoxSpan->value()));

    bool aRHasChanged = (!isApprox(internalAR, ui->spinBoxAR->value()));

    bool areaXYHasChanged = (!isApprox(internalArea, ui->spinBoxArea->value()));

    if (anchorHasChanged) {
        internalAnchorX = ui->spinBoxAnchorX->value();
        internalAnchorY = ui->spinBoxAnchorY->value();
        internalAnchorZ = ui->spinBoxAnchorZ->value();

        // todo
    }

    if (symmetryHasChanged) {
        internalSymmetry = ui->comboBoxSymmetry->currentText();

        // todo
    }

    if (sweepHasChanged) { // TODO do not change if the change is to small
        internalSweep      = ui->spinBoxSweep->value();
        internalMethod     = ui->comboBoxSweepMethod->currentText();
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