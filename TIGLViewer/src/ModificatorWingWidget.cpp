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
#include "CTiglLogging.h"

ModificatorWingWidget::ModificatorWingWidget(QWidget* parent)
    : ModificatorWidget(parent)
{
}

void ModificatorWingWidget::init()
{

    // Retrieve component of the anchor interface
    spinBoxAnchorX = this->findChild<QDoubleSpinBox*>("spinBoxAnchorX");
    spinBoxAnchorY = this->findChild<QDoubleSpinBox*>("spinBoxAnchorY");
    spinBoxAnchorZ = this->findChild<QDoubleSpinBox*>("spinBoxAnchorZ");

    // orientation interface
    comboBoxSymmetry = this->findChild<QComboBox*>("comboBoxSymmetry");

    // Retrieve component of the sweep interface
    btnExpendSweepDetails = this->findChild<QPushButton*>("btnExpendSweepDetails");
    spinBoxSweep          = this->findChild<QDoubleSpinBox*>("spinBoxSweep");
    widgetSweepDetails    = this->findChild<QWidget*>("widgetSweepDetails");
    spinBoxSweepChord     = this->findChild<QDoubleSpinBox*>("spinBoxSweepChord");
    comboBoxSweepMethod   = this->findChild<QComboBox*>("comboBoxSweepMethod");

    // Retrieve component of the dihedral interface
    btnExpendDihedralDetails = this->findChild<QPushButton*>("btnExpendDihedralDetails");
    spinBoxDihedral          = this->findChild<QDoubleSpinBox*>("spinBoxDihedral");
    widgetDihedralDetails    = this->findChild<QWidget*>("widgetDihedralDetails");
    spinBoxDihedralChord     = this->findChild<QDoubleSpinBox*>("spinBoxDihedralChord");

    // Retrieve component of the area interface
    btnExpendAreaDetails   = this->findChild<QPushButton*>("btnExpendAreaDetails");
    spinBoxAreaXY          = this->findChild<QDoubleSpinBox*>("spinBoxAreaXY");
    widgetAreaDetails      = this->findChild<QWidget*>("widgetAreaDetails");
    spinBoxAreaXZ          = this->findChild<QDoubleSpinBox*>("spinBoxAreaXZ");
    spinBoxAreaYZ          = this->findChild<QDoubleSpinBox*>("spinBoxAreaYZ");
    spinBoxAreaT           = this->findChild<QDoubleSpinBox*>("spinBoxAreaT");
    checkBoxIsAreaConstant = this->findChild<QCheckBox*>("checkBoxIsAreaConstant");

    // Retrieve component of the span interface
    spinBoxSpan            = this->findChild<QDoubleSpinBox*>("spinBoxSpan");
    checkBoxIsSpanConstant = this->findChild<QCheckBox*>("checkBoxIsSpanConstant");

    // Retrieve component of the span interface
    spinBoxAR            = this->findChild<QDoubleSpinBox*>("spinBoxAR");
    checkBoxIsARConstant = this->findChild<QCheckBox*>("checkBoxIsARConstant");

    // set the initials values of the display interface (should be overwritten
    // when the wingItem is set)
    spinBoxSweep->setValue(-1.0);
    spinBoxSweepChord->setValue(0);
    comboBoxSweepMethod->addItem("Translation");
    comboBoxSweepMethod->addItem("Shearing");
    comboBoxSweepMethod->setCurrentIndex(0);

    spinBoxSweep->setValue(-1.0);
    spinBoxDihedralChord->setValue(0);

    spinBoxAreaXY->setValue(-1.0);
    spinBoxAreaXZ->setValue(-1);
    spinBoxAreaYZ->setValue(-1);
    spinBoxAreaT->setValue(-1);

    spinBoxAreaXZ->setReadOnly(true);
    spinBoxAreaYZ->setReadOnly(true);
    spinBoxAreaT->setReadOnly(true);

    spinBoxAR->setValue(-1);

    spinBoxSpan->setValue(-1);

    comboBoxSymmetry->addItem("x-y-plane");
    comboBoxSymmetry->addItem("x-z-plane");
    comboBoxSymmetry->addItem("y-z-plane");
    comboBoxSymmetry->addItem("no-symmetry");

    // alterable span area ar
    checkBoxIsAreaConstant->setChecked(false);
    checkBoxIsSpanConstant->setChecked(false);
    checkBoxIsARConstant->setChecked(true);

    // hide the advanced options
    widgetAreaDetails->hide();
    widgetDihedralDetails->hide();
    widgetSweepDetails->hide();

    // connect the extend buttons with their slot
    connect(btnExpendAreaDetails, SIGNAL(clicked(bool)), this, SLOT(expendAreaDetails(bool)));
    connect(btnExpendDihedralDetails, SIGNAL(clicked(bool)), this, SLOT(expendDihedralDetails(bool)));
    connect(btnExpendSweepDetails, SIGNAL(clicked(bool)), this, SLOT(expendSweepDetails(bool)));

    // connect change alterable
    connect(checkBoxIsAreaConstant, SIGNAL(clicked(bool)), this, SLOT(setAreaConstant(bool)));
    connect(checkBoxIsSpanConstant, SIGNAL(clicked(bool)), this, SLOT(setSpanConstant(bool)));
    connect(checkBoxIsARConstant, SIGNAL(clicked(bool)), this, SLOT(setARConstant(bool)));
}

// inverse the visibility
void ModificatorWingWidget::expendAreaDetails(bool checked)
{
    widgetAreaDetails->setVisible(!(widgetAreaDetails->isVisible()));
}

// inverse the visibility
void ModificatorWingWidget::expendDihedralDetails(bool checked)
{
    widgetDihedralDetails->setVisible(!(widgetDihedralDetails->isVisible()));
}

// inverse the visibility
void ModificatorWingWidget::expendSweepDetails(bool checked)
{
    widgetSweepDetails->setVisible(!(widgetSweepDetails->isVisible()));
}

void ModificatorWingWidget::setAreaConstant(bool checked)
{
    checkBoxIsARConstant->setChecked(false);
    spinBoxAR->setReadOnly(false);
    checkBoxIsSpanConstant->setChecked(false);
    spinBoxSpan->setReadOnly(false);
    checkBoxIsAreaConstant->setChecked(true);
    spinBoxAreaXY->setReadOnly(true);
}

void ModificatorWingWidget::setSpanConstant(bool checked)
{

    checkBoxIsARConstant->setChecked(false);
    spinBoxAR->setReadOnly(false);
    checkBoxIsAreaConstant->setChecked(false);
    spinBoxAreaXY->setReadOnly(false);
    checkBoxIsSpanConstant->setChecked(true);
    spinBoxSpan->setReadOnly(true);
}

void ModificatorWingWidget::setARConstant(bool checked)
{
    checkBoxIsAreaConstant->setChecked(false);
    spinBoxAreaXY->setReadOnly(false);
    checkBoxIsSpanConstant->setChecked(false);
    spinBoxSpan->setReadOnly(false);
    checkBoxIsARConstant->setChecked(true);
    spinBoxAR->setReadOnly(true);
}

void ModificatorWingWidget::setWing(tigl::CCPACSWing& wing)
{
    tiglWing = &wing;

    // set constant between ar, span and area
    setARConstant(true);

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

void ModificatorWingWidget::apply()
{

    bool anchorHasChanged = ((!isApprox(internalAnchorX, spinBoxAnchorX->value())) ||
                             (!isApprox(internalAnchorY, spinBoxAnchorY->value())) ||
                             (!isApprox(internalAnchorZ, spinBoxAnchorZ->value())));

    bool symmetryHasChanged = (internalSymmetry != comboBoxSymmetry->currentText());

    bool sweepHasChanged = ((!isApprox(internalSweep, spinBoxSweep->value())) ||
                            (!isApprox(internalSweepChord, spinBoxSweepChord->value())) ||
                            internalMethod != comboBoxSweepMethod->currentText());

    bool dihedralHasChanged = ((!isApprox(internalDihedral, spinBoxDihedral->value())) ||
                               (!isApprox(internalDihedralChord, spinBoxDihedralChord->value())));

    bool spanHasChanged = (!isApprox(internalSpan, spinBoxSpan->value()));

    bool aRHasChanged = (!isApprox(internalAR, spinBoxAR->value()));

    bool areaXYHasChanged = (!isApprox(internalAreaXY, spinBoxAreaXY->value()));

    if (anchorHasChanged) {
        internalAnchorX = spinBoxAnchorX->value();
        internalAnchorY = spinBoxAnchorY->value();
        internalAnchorZ = spinBoxAnchorZ->value();

        // todo
    }

    if (symmetryHasChanged) {
        internalSymmetry = comboBoxSymmetry->currentText();

        // todo
    }

    if (sweepHasChanged) { // TODO do not change if the change is to small
        internalSweep      = spinBoxSweep->value();
        internalMethod     = comboBoxSweepMethod->currentText();
        internalSweepChord = spinBoxSweepChord->value();

        // todo
    }

    if (dihedralHasChanged) {
        internalDihedral      = spinBoxDihedral->value();
        internalDihedralChord = spinBoxDihedralChord->value();

        // todo
    }

    if (areaXYHasChanged) {
        internalAreaXY = spinBoxAreaXY->value();
        if (checkBoxIsSpanConstant->isChecked()) {

            // todo
        }
        else if (checkBoxIsARConstant->isChecked()) {

            // todo
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set area called, but not correct "
                          "constant checkbox set";
        }
    }

    if (spanHasChanged) {
        internalSpan = spinBoxSpan->value();
        if (checkBoxIsAreaConstant->isChecked()) {

            // todo
        }
        else if (checkBoxIsARConstant->isChecked()) {

            // todo
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set span called, but not correct "
                          "constant checkbox set";
        }
    }

    if (aRHasChanged) {
        internalAR = spinBoxAR->value();
        if (checkBoxIsAreaConstant->isChecked()) {

            // todo
        }
        else if (checkBoxIsSpanConstant->isChecked()) {

            // todo
        }
        else {
            LOG(ERROR) << "ModificatorWingWidget: set AR called, but not correct "
                          "constant checkbox set";
        }
    }
}
