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

ModificatorFuselageWidget::ModificatorFuselageWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorFuselageWidget)
{
    ui->setupUi(this);

    ui->widgetLengthDetails->hide();
    ui->widgetRadiusDetails->hide();

    // connect the extend buttons with their slot
    connect(ui->btnExpendLengthDetails, SIGNAL(clicked(bool)), this, SLOT(expendLengthDetails(bool)));
    connect(ui->comboBoxLengthE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(ui->comboBoxLengthE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(ui->spinBoxPartialLength, SIGNAL(valueChanged(double)), this, SLOT(recomputeTotalLength(double)));

    connect(ui->btnExpendRadiusDetails, SIGNAL(clicked(bool)), this, SLOT(expendRadiusDetails(bool)));
    connect(ui->comboBoxRadiusBE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
    connect(ui->comboBoxRadiusBE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
}

ModificatorFuselageWidget::~ModificatorFuselageWidget()
{
    delete ui;
}

void ModificatorFuselageWidget::setFuselage(tigl::CCPACSFuselage& newFuselage)
{
    
    this->fuselage = &newFuselage;

    // we disconnect signals from comboboxes because we do not want to call this routine when we set the combobox item
    disconnect(ui->comboBoxLengthE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    disconnect(ui->comboBoxLengthE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    disconnect(ui->spinBoxPartialLength, SIGNAL(valueChanged(double)), this, SLOT(recomputeTotalLength(double)));
    disconnect(ui->comboBoxRadiusBE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
    disconnect(ui->comboBoxRadiusBE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));

    internalLength = fuselage->GetLength();
    ui->spinBoxLength->setValue(internalLength);


    std::vector<std::string> connectionElementUIDs = fuselage->GetConnectionElementUIDs();
    QStringList elementsUids;
    for (int i = 0; i < connectionElementUIDs.size(); i++) {
        elementsUids.push_back(QString(connectionElementUIDs[i].c_str()));
    }
     
    ui->comboBoxLengthE1->clear();
    ui->comboBoxLengthE1->addItems(elementsUids);
    ui->comboBoxLengthE2->clear();
    ui->comboBoxLengthE2->addItems(elementsUids);
    ui->comboBoxLengthE2->setCurrentIndex(elementsUids.size() - 1); // set the last element of the list
    setPartialLengthFromComboBoxes();
    // do total length after partial length, because changing partial can change total
    internalLength = fuselage->GetLength();
    ui->spinBoxLength->setValue(internalLength);

    // radius & circumference
    ui->comboBoxRadiusBE1->clear();
    ui->comboBoxRadiusBE1->addItems(elementsUids);
    ui->comboBoxRadiusBE2->clear();
    ui->comboBoxRadiusBE2->addItems(elementsUids);
    ui->comboBoxRadiusBE2->setCurrentIndex(elementsUids.size() - 1); // set the last element of the list
    setRadiusBetweenFromComboBoxes();

    internalRadius = (fuselage->GetMaximalCircumference()) / M_PI;
    ui->spinBoxRadius->setValue(internalRadius);

     
    connect(ui->comboBoxLengthE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(ui->comboBoxLengthE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(ui->spinBoxPartialLength, SIGNAL(valueChanged(double)), this, SLOT(recomputeTotalLength(double)));
    connect(ui->comboBoxRadiusBE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
    connect(ui->comboBoxRadiusBE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
    
}

// inverse the visibility
void ModificatorFuselageWidget::expendRadiusDetails(bool checked)
{

    ui->widgetRadiusDetails->setVisible(!(ui->widgetRadiusDetails->isVisible()));

    if (ui->widgetRadiusDetails->isVisible()) {
        // Reset the values to the file values, avoid modifying from details and main at the same time
        internalRadius = fuselage->GetMaximalCircumference() / M_PI;
        ui->spinBoxRadius->setValue(internalRadius);
        setRadiusBetweenFromComboBoxes();
        ui->spinBoxRadius->setReadOnly(true);
    }
    else {
        // Reset the values to the file values, avoid modifying from details and main at the same time
        internalRadius = fuselage->GetMaximalCircumference() / M_PI;
        ui->spinBoxRadius->setValue(internalRadius);
        setRadiusBetweenFromComboBoxes();
        ui->spinBoxRadius->setReadOnly(false);
    }
}

// inverse the visibility
void ModificatorFuselageWidget::expendLengthDetails(bool checked)
{

    ui->widgetLengthDetails->setVisible(!(ui->widgetLengthDetails->isVisible()));

    if (ui->widgetLengthDetails->isVisible()) {
        // Reset the values to the file values, avoid modifying from details and main at the same time
        internalLength = fuselage->GetLength();
        ui->spinBoxLength->setValue(internalLength);
        setPartialLengthFromComboBoxes();
        ui->spinBoxLength->setReadOnly(true);
    }
    else {
        // Reset the values to the file values, avoid modifying from details and main at the same time
        internalLength = fuselage->GetLength();
        ui->spinBoxLength->setValue(internalLength);
        setPartialLengthFromComboBoxes();
        ui->spinBoxLength->setReadOnly(false);
    }

}

void ModificatorFuselageWidget::setRadiusBetweenFromComboBoxes()
{
    QString uid1          = ui->comboBoxRadiusBE1->currentText();
    QString uid2          = ui->comboBoxRadiusBE2->currentText();
    internalRadiusBetween = (fuselage->GetMaximalCircumferenceBetween(uid1.toStdString(), uid2.toStdString())) / M_PI;
    ui->spinBoxRadiusBetween->setValue(internalRadiusBetween);
}

void ModificatorFuselageWidget::setPartialLengthFromComboBoxes()
{

    QString uid1          = ui->comboBoxLengthE1->currentText();
    QString uid2          = ui->comboBoxLengthE2->currentText();
    internalPartialLength = fuselage->GetLengthBetween(uid1.toStdString(), uid2.toStdString());
    // we reset the display value of total length, because the old displayed value can be have modified by recomputeTotalLength
    ui->spinBoxLength->setValue(internalLength);
    ui->spinBoxPartialLength->setValue(internalPartialLength);
}

// call when a new partial length is set
void ModificatorFuselageWidget::recomputeTotalLength(double newPartialLength)
{

    if (!(isApprox(newPartialLength, internalPartialLength))) { // avoid diff between spin box implementation and double
        double diff = newPartialLength - internalPartialLength;
        ui->spinBoxLength->setValue(internalLength + diff);
    }
}

bool ModificatorFuselageWidget::apply()
{

    bool lengthHasChanged = ((!isApprox(internalLength, ui->spinBoxLength->value())));

    bool partialLengthHasChanged = (!isApprox(internalPartialLength, ui->spinBoxPartialLength->value()));
    // if expend length details is shown, the details modifications prime on the main modif interface
    bool isPartialLengthCase = ui->widgetLengthDetails->isVisible();

    bool radiusHasChanged = ( ! isApprox(internalRadius, ui->spinBoxRadius->value()) );
    bool radiusBetweenHasChanged = ( ! isApprox(internalRadiusBetween, ui->spinBoxRadiusBetween->value()));
    bool isRadiusBetweenCase = ui->widgetRadiusDetails->isVisible();

    bool wasModified = false;

    if (lengthHasChanged && (!isPartialLengthCase)) {
        internalLength = ui->spinBoxLength->value();
        fuselage->SetLength(internalLength);
        wasModified = true;
    }

    if (partialLengthHasChanged && isPartialLengthCase) {
        internalPartialLength = ui->spinBoxPartialLength->value();
        QString uid1          = ui->comboBoxLengthE1->currentText();
        QString uid2          = ui->comboBoxLengthE2->currentText();
        fuselage->SetLengthBetween(uid1.toStdString(), uid2.toStdString(), internalPartialLength);
        wasModified = true;
    }

    if(radiusHasChanged && (!isRadiusBetweenCase)){
        internalRadius = ui->spinBoxRadius->value();
        fuselage->SetMaximalCircumference(internalRadius * M_PI);
        wasModified = true;
    }

    if(radiusBetweenHasChanged && isRadiusBetweenCase){
        internalRadiusBetween = ui->spinBoxRadiusBetween->value();
        QString uid1          = ui->comboBoxRadiusBE1->currentText();
        QString uid2          = ui->comboBoxRadiusBE2->currentText();
        fuselage->SetMaximalCircumferenceBetween(uid1.toStdString(), uid2.toStdString(), internalRadiusBetween * M_PI);
        wasModified = true;
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
        this->setFuselage(*fuselage);
    }
    else {
        LOG(WARNING) << "ModificatorWingWidget: reset call but wing is not set!";
    }
}
