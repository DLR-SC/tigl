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

//
// Created by makem on 30/05/18.
//

#include "TIGLViewerWingWidget.h"
//#include "ModificatorManager.h"

TIGLViewerWingWidget::TIGLViewerWingWidget(QWidget* parent)
    : ModificatorWidget(parent)
{
}
/*

void TIGLViewerWingWidget::init(ModificatorManager * associate ) {
    ModificatorWidget::init(associate);

    // Retrieve component of the anchor interface
    spinBoxAnchorX = this->findChild<QDoubleSpinBox*>("spinBoxAnchorX");
    spinBoxAnchorY = this->findChild<QDoubleSpinBox*>("spinBoxAnchorY");
    spinBoxAnchorZ = this->findChild<QDoubleSpinBox*>("spinBoxAnchorZ");

    // orientation interface
    comboBoxWingOrientation = this->findChild<QComboBox*>("comboBoxWingOrientation");
    comboBoxSymmetry = this->findChild<QComboBox*>("comboBoxSymmetry");

    // Retrieve component of the sweep interface
    btnExpendSweepDetails = this->findChild<QPushButton*>("btnExpendSweepDetails");
    spinBoxSweep = this->findChild<QDoubleSpinBox*>("spinBoxSweep");
    widgetSweepDetails = this->findChild<QWidget*>("widgetSweepDetails");
    spinBoxSweepChord = this->findChild<QDoubleSpinBox*>("spinBoxSweepChord");
    comboBoxSweepMethod = this->findChild<QComboBox*>("comboBoxSweepMethod");

    // Retrieve component of the dihedral interface
    btnExpendDihedralDetails = this->findChild<QPushButton*>("btnExpendDihedralDetails");
    spinBoxDihedral = this->findChild<QDoubleSpinBox*>("spinBoxDihedral");
    widgetDihedralDetails = this->findChild<QWidget*>("widgetDihedralDetails");
    spinBoxDihedralChord = this->findChild<QDoubleSpinBox*>("spinBoxDihedralChord");

    // Retrieve component of the area interface
    btnExpendAreaDetails = this->findChild<QPushButton*>("btnExpendAreaDetails");
    spinBoxAreaXY = this->findChild<QDoubleSpinBox*>("spinBoxAreaXY");
    widgetAreaDetails = this->findChild<QWidget*>("widgetAreaDetails");
    spinBoxAreaXZ = this->findChild<QDoubleSpinBox*>("spinBoxAreaXZ");
    spinBoxAreaYZ = this->findChild<QDoubleSpinBox*>("spinBoxAreaYZ");
    spinBoxAreaT = this->findChild<QDoubleSpinBox*>("spinBoxAreaT");
    checkBoxIsAreaConstant = this->findChild<QCheckBox*>("checkBoxIsAreaConstant");

    // Retrieve component of the span interface
    spinBoxSpan = this->findChild<QDoubleSpinBox*>("spinBoxSpan");
    checkBoxIsSpanConstant = this->findChild<QCheckBox*>("checkBoxIsSpanConstant");

    // Retrieve component of the span interface
    spinBoxAR = this->findChild<QDoubleSpinBox*>("spinBoxAR");
    checkBoxIsARConstant = this->findChild<QCheckBox*>("checkBoxIsARConstant");

    // Retrieve component of the airfoil interface
    btnExpendAirfoilDetails = this->findChild<QPushButton*>("btnExpendAirfoilDetails");
    comboBoxAirfoil = this->findChild<QComboBox*>("comboBoxAirfoil");
    widgetAirfoilDetails = this->findChild<QWidget*>("widgetAirfoilDetails");


    // Retrieve component of the standardization interface
    btnExpendStdDetails = this->findChild<QPushButton*>("btnExpendStandardizationDetails");
    comboBoxStdGlobal = this->findChild<QComboBox*>("comboBoxStdGlobal");
    checkBoxStdAirfoils = this->findChild<QCheckBox*>("checkBoxStdAirfoils");
    checkBoxStdSections = this->findChild<QCheckBox*>("checkBoxStdSections");
    checkBoxStdPositionings = this->findChild<QCheckBox*>("checkBoxStdPositionings");
    checkBoxStdAnchor = this->findChild<QCheckBox*>("checkBoxStdAnchor");
    widgetStdDetails = this->findChild<QWidget*>("widgetStandardizationDetails");

    // set the initials values of the display interface (should be overwritten when the wingItem is set)
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

    comboBoxAirfoil->addItems(associate->profilesDB->getAvailableAirfoils());

    comboBoxWingOrientation->addItem("horizontal");
    comboBoxWingOrientation->addItem("vertical");

    comboBoxSymmetry->addItem("x-y-plane");
    comboBoxSymmetry->addItem("x-z-plane");
    comboBoxSymmetry->addItem("y-z-plane");
    comboBoxSymmetry->addItem("no-symmetry");



    // alterable span area ar
    checkBoxIsAreaConstant->setChecked(false);
    checkBoxIsSpanConstant->setChecked(false);
    checkBoxIsARConstant->setChecked(true);

    // anchor orientation



    // hide the advanced options
    widgetAreaDetails->hide();
    widgetDihedralDetails->hide();
    widgetSweepDetails->hide();
    widgetAirfoilDetails->hide();
    widgetStdDetails->hide();

    // connect the extend buttons with their slot
    connect(btnExpendAreaDetails, SIGNAL(clicked(bool)), this, SLOT(expendAreaDetails(bool)) );
    connect(btnExpendDihedralDetails, SIGNAL(clicked(bool)), this, SLOT(expendDihedralDetails(bool)) );
    connect(btnExpendSweepDetails, SIGNAL(clicked(bool)), this, SLOT(expendSweepDetails(bool)));
    connect(btnExpendAirfoilDetails, SIGNAL(clicked(bool)), this, SLOT(expendAirfoilDetails(bool)));
    connect(btnExpendStdDetails, SIGNAL(clicked(bool)), this, SLOT(expendStandardizationDetails(bool)));



    // connect change alterable
    connect(checkBoxIsAreaConstant,SIGNAL(clicked(bool)), this, SLOT(setAreaConstant(bool)));
    connect(checkBoxIsSpanConstant,SIGNAL(clicked(bool)), this, SLOT(setSpanConstant(bool)));
    connect(checkBoxIsARConstant,SIGNAL(clicked(bool)), this, SLOT(setARConstant(bool)));

    // logical connection to remain std checkbox in a logical state
    connect(checkBoxStdAirfoils,SIGNAL(clicked(bool)), this, SLOT(checkStdAirfoils(bool)));
    connect(checkBoxStdSections,SIGNAL(clicked(bool)), this, SLOT(checkStdSections(bool)));
    connect(checkBoxStdPositionings,SIGNAL(clicked(bool)), this, SLOT(checkStdPositionings(bool)));
    connect(checkBoxStdAnchor,SIGNAL(clicked(bool)), this, SLOT(checkStdAnchor(bool)));
    connect(comboBoxStdGlobal,SIGNAL(currentIndexChanged(int )), this, SLOT(setStdCheckBoxesFromComboBox(int)) );
    callFromSetStdComboBox = false;
}

// inverse the visibility
void TIGLViewerWingWidget::expendAreaDetails(bool checked) {
    widgetAreaDetails->setVisible(! (widgetAreaDetails->isVisible() ));
}


// inverse the visibility
void TIGLViewerWingWidget::expendDihedralDetails(bool checked) {
    widgetDihedralDetails->setVisible(! (widgetDihedralDetails->isVisible() ));
}

// inverse the visibility
void TIGLViewerWingWidget::expendSweepDetails(bool checked) {
    widgetSweepDetails->setVisible(! (widgetSweepDetails->isVisible() ));
}

void TIGLViewerWingWidget::expendAirfoilDetails(bool checked) {
    widgetAirfoilDetails->setVisible( ! (widgetAirfoilDetails->isVisible()));
}

void TIGLViewerWingWidget::expendStandardizationDetails(bool checked) {
    widgetStdDetails->setVisible( !(widgetStdDetails->isVisible()));
}




void TIGLViewerWingWidget::setAreaConstant(bool checked) {
    checkBoxIsARConstant->setChecked(false);
    spinBoxAR->setReadOnly(false);
    checkBoxIsSpanConstant->setChecked(false);
    spinBoxSpan->setReadOnly(false);
    checkBoxIsAreaConstant->setChecked(true);
    spinBoxAreaXY->setReadOnly(true);

}

void TIGLViewerWingWidget::setSpanConstant(bool checked) {

    checkBoxIsARConstant->setChecked(false);
    spinBoxAR->setReadOnly(false);
    checkBoxIsAreaConstant->setChecked(false);
    spinBoxAreaXY->setReadOnly(false);
    checkBoxIsSpanConstant->setChecked(true);
    spinBoxSpan->setReadOnly(true);
}

void TIGLViewerWingWidget::setARConstant(bool checked) {
    checkBoxIsAreaConstant->setChecked(false);
    spinBoxAreaXY->setReadOnly(false);
    checkBoxIsSpanConstant->setChecked(false);
    spinBoxSpan->setReadOnly(false);
    checkBoxIsARConstant->setChecked(true);
    spinBoxAR->setReadOnly(true);

}


void TIGLViewerWingWidget::checkStdAirfoils(bool checked) {
    checkBoxStdAirfoils->setChecked(internalStdAirfoils || checked);
    setStdComboBoxFromStdCheckBoxes();
}

void TIGLViewerWingWidget::checkStdSections(bool checked) {
    checkBoxStdSections->setChecked(internalStdSections || checked);
    if( checked == false && internalStdSections == false && checkBoxStdPositionings->isChecked()){
        checkBoxStdPositionings->setChecked(false);     // we do no allow to have section not check an positioning checked
    }
    setStdComboBoxFromStdCheckBoxes();
}

void TIGLViewerWingWidget::checkStdPositionings(bool checked) {
    checkBoxStdPositionings->setChecked(internalStdPositionings || checked);
    if(! checkBoxStdSections->isChecked() && checked ){
        checkBoxStdSections->setChecked(checked);   // positioning requier section std
    }
    setStdComboBoxFromStdCheckBoxes();
}

void TIGLViewerWingWidget::checkStdAnchor(bool checked) {
    checkBoxStdAnchor->setChecked(internalStdAnchor || checked);
    setStdComboBoxFromStdCheckBoxes();
}


void TIGLViewerWingWidget::setStdCheckBoxesFromComboBox(int idx) {
    if(callFromSetStdComboBox) return; // we ignore the signal from "setStdComboBoxFromStdCheckBoxes" because it will overrie the correct value

    if( comboBoxStdGlobal->currentText() == "Total"){
        checkBoxStdAnchor->setChecked(true);
        checkBoxStdSections->setChecked(true);
        checkBoxStdPositionings->setChecked(true);
        checkBoxStdAirfoils->setChecked(true);
    }else{
        checkBoxStdAnchor->setChecked(internalStdAnchor);
        checkBoxStdSections->setChecked(internalStdSections);
        checkBoxStdPositionings->setChecked(internalStdPositionings);
        checkBoxStdAirfoils->setChecked(internalStdAirfoils);
    }


}

void TIGLViewerWingWidget::setStdComboBoxFromStdCheckBoxes() {

    callFromSetStdComboBox = true;
    int stdCount = 0;
    if(checkBoxStdAirfoils->isChecked()) stdCount += 1;
    if(checkBoxStdPositionings->isChecked()) stdCount += 1;
    if(checkBoxStdSections->isChecked()) stdCount += 1;
    if(checkBoxStdAnchor->isChecked()) stdCount += 1;

    QString stdG = "";
    if( stdCount == 0 ) {
        stdG = "None";
    }else if ( stdCount < 4){
        stdG = "Partial";
    }else if( stdCount == 4){
        stdG = "Total";
    }

    int index = comboBoxStdGlobal->findText(stdG);
    if ( index == -1 ) { // -1 for not found
        comboBoxStdGlobal->addItem(stdG);
        index = comboBoxStdGlobal->findText(stdG);
    }
    comboBoxStdGlobal->setCurrentIndex(index);
    callFromSetStdComboBox = false;
}




void TIGLViewerWingWidget::setWing(cpcr::CPACSTreeItem *wing) {
    wingItem = wing;

    // set anchor
    associateManager->adapter->getAnchorValues(wing, internalAnchorX, internalAnchorY, internalAnchorZ);
    spinBoxAnchorX->setValue(internalAnchorX);
    spinBoxAnchorY->setValue(internalAnchorY);
    spinBoxAnchorZ->setValue(internalAnchorZ);

    // set orientration & symmetry
    internalWingOrientation = associateManager->adapter->getWingOrientation(wingItem);
    int idx = comboBoxWingOrientation->findText(internalWingOrientation);
    if(idx == -1){  // case for custom
        idx = comboBoxWingOrientation->count();
        comboBoxWingOrientation->addItem(internalWingOrientation);
    }
    comboBoxWingOrientation->setCurrentIndex(idx);

    internalSymmetry = associateManager->adapter->getWingSymmetry(wingItem);
    idx = comboBoxSymmetry->findText(internalSymmetry);
    comboBoxSymmetry->setCurrentIndex(idx);


    // set sweep
    internalMethod = comboBoxSweepMethod->currentText(); // retrieve the information of the interface -> when we switch from one wing to the other method and chord are conserved
    internalSweepChord = spinBoxSweepChord->value();
    internalSweep = associateManager->adapter->getSweepAngle(wingItem, internalSweepChord);
    spinBoxSweep->setValue(internalSweep);

    // set dihedral
    internalDihedralChord = spinBoxDihedralChord->value();
    internalDihedral = associateManager->adapter->getDihedralAngle(wingItem, internalDihedralChord);
    spinBoxDihedral->setValue(internalDihedral);

    // set area
    internalAreaXY = associateManager->adapter->getWingArea(wingItem, cpcr::PLANE::XY_PLANE);
    spinBoxAreaXY->setValue(internalAreaXY);

    internalAreaXZ = associateManager->adapter->getWingArea(wingItem, cpcr::PLANE::XZ_PLANE);
    spinBoxAreaXZ->setValue(internalAreaXZ);

    internalAreaYZ = associateManager->adapter->getWingArea(wingItem, cpcr::PLANE::YZ_PLANE);
    spinBoxAreaYZ->setValue(internalAreaYZ);

    internalAreaT = associateManager->adapter->getWingArea(wingItem, cpcr::PLANE::NO_PLANE);
    spinBoxAreaT->setValue(internalAreaT);

    // set span
    internalSpan = associateManager->adapter->getWingSpan(wingItem);
    spinBoxSpan->setValue(internalSpan);

    // set AR
    internalAR = associateManager->adapter->getWingAR(wingItem);
    spinBoxAR->setValue(internalAR);

    // set constant between ar, span and area
    setARConstant(true);


    // set wingAirfoil1
    comboBoxAirfoil->clear();
    comboBoxAirfoil->addItems(associateManager->profilesDB->getAvailableAirfoils());

    internalAirfoilUID = associateManager->adapter->getAirfoilValueForWing(wingItem);
    idx = comboBoxAirfoil->findText(internalAirfoilUID);
    if(idx == -1){  // case for combined or None
        idx = comboBoxAirfoil->count();
        comboBoxAirfoil->addItem(internalAirfoilUID);
    }
    comboBoxAirfoil->setCurrentIndex(idx);

    // set standarization
    associateManager->adapter->getStdValues(wingItem, internalStdAirfoils, internalStdSections, internalStdPositionings, internalStdAnchor);
    comboBoxStdGlobal->clear();
    comboBoxStdGlobal->addItem("Total");
    checkBoxStdAnchor->setChecked(internalStdAnchor);
    checkBoxStdSections->setChecked(internalStdSections);
    checkBoxStdPositionings->setChecked(internalStdPositionings);
    checkBoxStdAirfoils->setChecked(internalStdAirfoils);
    this->setStdComboBoxFromStdCheckBoxes();


}


void TIGLViewerWingWidget::reset() {
    if(wingItem != nullptr){
        this->setWing(this->wingItem);
    }else{
        LOG(WARNING) << "TIGLViewerWingWidget: reset call but wing is not set!";
    }

}


void TIGLViewerWingWidget::apply() {

    bool anchorHasChanged = ( (!isApprox(internalAnchorX, spinBoxAnchorX->value()))
                              || (! isApprox(internalAnchorY, spinBoxAnchorY->value()))
                              || (! isApprox(internalAnchorZ , spinBoxAnchorZ->value())) );

    bool orientationHasChanged = ( internalWingOrientation != comboBoxWingOrientation->currentText() );

    bool symmetryHasChanged = (internalSymmetry != comboBoxSymmetry->currentText() );

    bool sweepHasChanged = ( (!isApprox(internalSweep, spinBoxSweep->value()) )
                             ||(!isApprox(internalSweepChord, spinBoxSweepChord->value() ) )
                             || internalMethod != comboBoxSweepMethod->currentText()) ;

    bool dihedralHasChanged = ( (!isApprox(internalDihedral, spinBoxDihedral->value()))
                               || (!isApprox(internalDihedralChord, spinBoxDihedralChord->value())) );

    bool airfoilHasChanged = (internalAirfoilUID != comboBoxAirfoil->currentText() );

    bool spanHasChanged = ( ! isApprox( internalSpan, spinBoxSpan->value()) );

    bool aRHasChanged = ( ! isApprox(internalAR, spinBoxAR->value() ) );

    bool areaXYHasChanged = ( ! isApprox(internalAreaXY, spinBoxAreaXY->value() ));


    if( anchorHasChanged ){
        internalAnchorX = spinBoxAnchorX->value();
        internalAnchorY = spinBoxAnchorY->value();
        internalAnchorZ = spinBoxAnchorZ->value();
        associateManager->adapter->setAnchorValues(wingItem, internalAnchorX, internalAnchorY, internalAnchorZ);
    }

    if( orientationHasChanged ){
        internalWingOrientation = comboBoxWingOrientation->currentText();
        associateManager->adapter->setWingOrientation(wingItem, internalWingOrientation);
    }


    if (symmetryHasChanged){
        internalSymmetry = comboBoxSymmetry->currentText();
        associateManager->adapter->setWingSymmetry(wingItem, internalSymmetry);
    }

    if(sweepHasChanged){ //TODO do not change if the change is to small
        internalSweep = spinBoxSweep->value();
        internalMethod = comboBoxSweepMethod->currentText();
        internalSweepChord = spinBoxSweepChord->value();
        associateManager->adapter->setSweepAngle(wingItem, internalSweep, internalSweepChord, internalMethod );
    }

    if(dihedralHasChanged){
        internalDihedral = spinBoxDihedral->value();
        internalDihedralChord = spinBoxDihedralChord->value();
        associateManager->adapter->setDihedralAngle(wingItem, internalDihedral, internalDihedralChord);
    }

    if(airfoilHasChanged){
        internalAirfoilUID = comboBoxAirfoil->currentText();
        associateManager->adapter->setAllAirfoilsInWing(wingItem, internalAirfoilUID);

    }

    if( areaXYHasChanged){
        internalAreaXY = spinBoxAreaXY->value();
        if(checkBoxIsSpanConstant->isChecked()){
            associateManager->adapter->setWingAreaKeepSpan(wingItem, internalAreaXY);
        }
        else if( checkBoxIsARConstant->isChecked() ){
            associateManager->adapter->setWingAreaKeepAR(wingItem, internalAreaXY);
        }
        else {
            LOG(ERROR) << "TIGLViewerWingWidget: set area called, but not correct constant checkbox set";
        }

    }


    if(spanHasChanged ){
        internalSpan = spinBoxSpan->value();
        if(checkBoxIsAreaConstant->isChecked()){
            associateManager->adapter->setWingSpanKeepArea(wingItem, internalSpan);
        }
        else if(checkBoxIsARConstant->isChecked()){
            associateManager->adapter->setWingSpanKeepAR(wingItem, internalSpan);
        }
        else {
            LOG(ERROR) << "TIGLViewerWingWidget: set span called, but not correct constant checkbox set";
        }
    }

    if(aRHasChanged ){
        internalAR = spinBoxAR->value();
        if(checkBoxIsAreaConstant->isChecked()){
            associateManager->adapter->setWingARKeepArea(wingItem, internalAR);
        }
        else if(checkBoxIsSpanConstant->isChecked()){
            associateManager->adapter->setWingARKeepSpan(wingItem, internalAR);
        }
        else {
            LOG(ERROR) << "TIGLViewerWingWidget: set AR called, but not correct constant checkbox set";
        }
    }




    if(sweepHasChanged || airfoilHasChanged || dihedralHasChanged || areaXYHasChanged || spanHasChanged
       || aRHasChanged || anchorHasChanged || symmetryHasChanged || orientationHasChanged){
        associateManager->adapter->writeToFile();   // we do this here to update all the change at once in the file
    }

    // we reset the internal values form the file, because standardization is not guarantee by the functions above
    associateManager->adapter->getStdValues(wingItem, internalStdAirfoils, internalStdSections, internalStdPositionings, internalStdAnchor);
    bool stdardizationHasChanged = ( internalStdAirfoils != checkBoxStdAirfoils->isChecked()
                                     || internalStdPositionings != checkBoxStdPositionings->isChecked()
                                     || internalStdSections != checkBoxStdSections->isChecked()
                                     || internalStdAnchor != checkBoxStdAnchor->isChecked() );

    if( stdardizationHasChanged ){

        internalStdAnchor = checkBoxStdAnchor->isChecked();
        internalStdSections = checkBoxStdSections->isChecked();
        internalStdPositionings = checkBoxStdPositionings->isChecked();
        internalStdAirfoils = checkBoxStdAirfoils->isChecked();

        associateManager->adapter->setStdValues(wingItem, internalStdAirfoils, internalStdSections,
                                                internalStdPositionings, internalStdAnchor);

    }


}






*/
