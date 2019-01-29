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
#include "ModificatorManager.h"

ModificatorFuselageWidget::ModificatorFuselageWidget(QWidget* parent)
    : ModificatorWidget(parent)
{
}

void ModificatorFuselageWidget::setFuselage(tigl::CCPACSFuselage& newFuselage)
{
    /*
    this->fuselage = &newFuselage;

    // we disconnect signals from comboboxes because we do not want to call this routine when we set the combobox item
    disconnect(comboBoxLengthE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    disconnect(comboBoxLengthE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    disconnect(spinBoxPartialLength, SIGNAL(valueChanged(double)), this, SLOT(recomputeTotalLength(double)));
    disconnect(comboBoxRadiusBE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
    disconnect(comboBoxRadiusBE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));

    internalLength = fuselage->GetLength();
    spinBoxLength->setValue(internalLength);

    std::vector<std::string> fusleageGraph = fuselage->GetCreatorGraph().getGraphAsVector();
    QStringList elementsUids;
    for (int i = 0; i < fusleageGraph.size(); i++) {
        elementsUids.push_back(QString(fusleageGraph[i].c_str()));
    }

    comboBoxLengthE1->clear();
    comboBoxLengthE1->addItems(elementsUids);
    comboBoxLengthE2->clear();
    comboBoxLengthE2->addItems(elementsUids);
    comboBoxLengthE2->setCurrentIndex(elementsUids.size() - 1); // set the last element of the list
    setPartialLengthFromComboBoxes();
    // do total length after partial length, because changing partial can change total
    internalLength = fuselage->GetLength();
    spinBoxLength->setValue(internalLength);
    //widgetLengthDetails->setVisible(false);

    // radius & circumference
    comboBoxRadiusBE1->clear();
    comboBoxRadiusBE1->addItems(elementsUids);
    comboBoxRadiusBE2->clear();
    comboBoxRadiusBE2->addItems(elementsUids);
    comboBoxRadiusBE2->setCurrentIndex(elementsUids.size() - 1); // set the last element of the list
    setRadiusBetweenFromComboBoxes();

    internalRadius = (fuselage->GetMaximalCircumferenceOfElements()) / M_PI;
    spinBoxRadius->setValue(internalRadius);
    //widgetRadiusDetails->setVisible(false);

    connect(comboBoxLengthE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(comboBoxLengthE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(spinBoxPartialLength, SIGNAL(valueChanged(double)), this, SLOT(recomputeTotalLength(double)));
    connect(comboBoxRadiusBE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
    connect(comboBoxRadiusBE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
     */
}

void ModificatorFuselageWidget::init()
{

    spinBoxLength          = this->findChild<QDoubleSpinBox*>("spinBoxLength");
    widgetLengthDetails    = this->findChild<QWidget*>("widgetLengthDetails");
    btnExpendLengthDetails = this->findChild<QPushButton*>("btnExpendLengthDetails");
    comboBoxLengthE1       = this->findChild<QComboBox*>("comboBoxLengthE1");
    comboBoxLengthE2       = this->findChild<QComboBox*>("comboBoxLengthE2");
    spinBoxPartialLength   = this->findChild<QDoubleSpinBox*>("spinBoxPartialLength");

    spinBoxRadius          = this->findChild<QDoubleSpinBox*>("spinBoxRadius");
    spinBoxRadiusBetween   = this->findChild<QDoubleSpinBox*>("spinBoxRadiusBetween");
    btnExpendRadiusDetails = this->findChild<QPushButton*>("btnExpendRadiusDetails");
    comboBoxRadiusBE1      = this->findChild<QComboBox*>("comboBoxRadiusBE1");
    comboBoxRadiusBE2      = this->findChild<QComboBox*>("comboBoxRadiusBE2");
    widgetRadiusDetails    = this->findChild<QWidget*>("widgetRadiusDetails");

    widgetLengthDetails->hide();
    widgetRadiusDetails->hide();

    // connect the extend buttons with their slot
    connect(btnExpendLengthDetails, SIGNAL(clicked(bool)), this, SLOT(expendLengthDetails(bool)));
    connect(comboBoxLengthE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(comboBoxLengthE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setPartialLengthFromComboBoxes()));
    connect(spinBoxPartialLength, SIGNAL(valueChanged(double)), this, SLOT(recomputeTotalLength(double)));

    connect(btnExpendRadiusDetails, SIGNAL(clicked(bool)), this, SLOT(expendRadiusDetails(bool)));
    connect(comboBoxRadiusBE1, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
    connect(comboBoxRadiusBE2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRadiusBetweenFromComboBoxes()));
}

// inverse the visibility
void ModificatorFuselageWidget::expendRadiusDetails(bool checked)
{

    widgetRadiusDetails->setVisible(!(widgetRadiusDetails->isVisible()));
    /*
     if (widgetRadiusDetails->isVisible()) {
         // Reset the values to the file values, avoid modifying from details and main at the same time
         internalRadius = fuselage->GetMaximalCircumferenceOfElements() / M_PI;
         spinBoxRadius->setValue(internalRadius);
         setRadiusBetweenFromComboBoxes();
         spinBoxRadius->setReadOnly(true);
     }
     else {
         // Reset the values to the file values, avoid modifying from details and main at the same time
         internalRadius = fuselage->GetMaximalCircumferenceOfElements() / M_PI;
         spinBoxRadius->setValue(internalRadius);
         setRadiusBetweenFromComboBoxes();
         spinBoxRadius->setReadOnly(false);
     }
 */
}

// inverse the visibility
void ModificatorFuselageWidget::expendLengthDetails(bool checked)
{

    widgetLengthDetails->setVisible(!(widgetLengthDetails->isVisible()));
    /*
    if (widgetLengthDetails->isVisible()) {
        // Reset the values to the file values, avoid modifying from details and main at the same time
        internalLength = fuselage->GetLength();
        spinBoxLength->setValue(internalLength);
        setPartialLengthFromComboBoxes();
        spinBoxLength->setReadOnly(true);
    }
    else {
        // Reset the values to the file values, avoid modifying from details and main at the same time
        internalLength = fuselage->GetLength();
        spinBoxLength->setValue(internalLength);
        setPartialLengthFromComboBoxes();
        spinBoxLength->setReadOnly(false);
    }
     */
}

void ModificatorFuselageWidget::setRadiusBetweenFromComboBoxes()
{
    /*
    QString uid1          = comboBoxRadiusBE1->currentText();
    QString uid2          = comboBoxRadiusBE2->currentText();
    internalRadiusBetween = (fuselage->GetMaximalCircumferenceOfElementsBetween(uid1.toStdString(), uid2.toStdString())) / M_PI;
    spinBoxRadiusBetween->setValue(internalRadiusBetween);*/
}

void ModificatorFuselageWidget::setPartialLengthFromComboBoxes()
{
    /*
    QString uid1          = comboBoxLengthE1->currentText();
    QString uid2          = comboBoxLengthE2->currentText();
    internalPartialLength = fuselage->GetLengthBetween(uid1.toStdString(), uid2.toStdString());
    // we reset the display value of total length, because the old displayed value can be have modified by recomputeTotalLength
    spinBoxLength->setValue(internalLength);
    spinBoxPartialLength->setValue(internalPartialLength);*/
}

// call when a new partial length is set
void ModificatorFuselageWidget::recomputeTotalLength(double newPartialLength)
{

    /*    if (!(isApprox(newPartialLength, internalPartialLength))) { // avoid diff between spin box implementation and double
        double diff = newPartialLength - internalPartialLength;
        spinBoxLength->setValue(internalLength + diff);
    }*/
}

void ModificatorFuselageWidget::apply()
{

    /*   bool lengthHasChanged        = ((!isApprox(internalLength, spinBoxLength->value())));
    bool partialLengthHasChanged = (!isApprox(internalPartialLength, spinBoxPartialLength->value()));
    // if expend length details is shown, the details modifications prime on the main modif interface
    bool isPartialLengthCase = widgetLengthDetails->isVisible();
    bool radiusHasChanged = ( ! isApprox(internalRadius, spinBoxRadius->value()) );
    bool radiusBetweenHasChanged = ( ! isApprox(internalRadiusBetween, spinBoxRadiusBetween->value()));
    bool isRadiusBetweenCase = widgetRadiusDetails->isVisible();

    bool wasModified = false;

    if (lengthHasChanged && (!isPartialLengthCase)) {
        internalLength = spinBoxLength->value();
        fuselage->SetLength(internalLength);
        wasModified = true;
    }
    if (partialLengthHasChanged && isPartialLengthCase) {
        internalPartialLength = spinBoxPartialLength->value();
        QString uid1          = comboBoxLengthE1->currentText();
        QString uid2          = comboBoxLengthE2->currentText();
        fuselage->SetLengthBetween(uid1.toStdString(), uid2.toStdString(), internalPartialLength);
        wasModified = true;
    }

    if(radiusHasChanged && (!isRadiusBetweenCase)){
        internalRadius = spinBoxRadius->value();
        fuselage->SetMaximalCircumferenceOfElements(internalRadius * M_PI);
        wasModified = true;
    }

    if(radiusBetweenHasChanged && isRadiusBetweenCase){
        internalRadiusBetween = spinBoxRadiusBetween->value();
        QString uid1          = comboBoxRadiusBE1->currentText();
        QString uid2          = comboBoxRadiusBE2->currentText();
        fuselage->SetMaximalCircumferenceOfElementsBetween(uid1.toStdString(), uid2.toStdString(), internalRadiusBetween * M_PI);
        wasModified = true;
    }

    if(wasModified){
        // we reset to be sure that each internal values is coorectly set
        reset();
    }*/

    //Todo: what we need to do XD
    /*
   if(lengthHasChanged || partialLengthHasChanged || circumferenceHasChanged ){
       associateManager->adapter->writeToFile();
   }
   */
}

void ModificatorFuselageWidget::reset()
{
    /* if (fuselage != nullptr) {
        this->setFuselage(*fuselage);
    }
    else {
        LOG(WARNING) << "ModificatorWingWidget: reset call but wing is not set!";
    }*/
}
