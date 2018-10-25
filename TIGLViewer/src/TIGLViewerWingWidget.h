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

#ifndef TIGL_TIGLVIEWERWINGWIDGET_H
#define TIGL_TIGLVIEWERWINGWIDGET_H

#include "ModificatorWidget.h"
#include "QDoubleSpinBox"
#include "QPushButton"
#include "QWidget"
#include "QComboBox"
//#include "CPACSCreatorLib/CPACSTreeItem.h"
#include "QCheckBox"

class TIGLViewerWingWidget : public ModificatorWidget
{

    Q_OBJECT
    /*

public slots:

    void expendAreaDetails(bool checked);
    void expendDihedralDetails(bool checked);
    void expendSweepDetails(bool checked);
    void expendAirfoilDetails(bool checked);
    void expendStandardizationDetails(bool checked);

    void setAreaConstant(bool checked);
    void setSpanConstant(bool checked);
    void setARConstant(bool checked);

    void checkStdAirfoils(bool checked);
    void checkStdSections(bool checked);
    void checkStdPositionings(bool checked);
    void checkStdAnchor(bool checked);
    void setStdCheckBoxesFromComboBox(int ix);

*/
public:
    TIGLViewerWingWidget(QWidget* parent = 0);
    /*
    void init(ModificatorManager * associate ) override ;
    void apply() override ;
    void reset() override ;

    void setWing(cpcr::CPACSTreeItem* wing);

protected:
    // set the combobox from the checkbox
    void setStdComboBoxFromStdCheckBoxes();


private:

    cpcr::CPACSTreeItem * wingItem;

    // anchor interface
    QDoubleSpinBox * spinBoxAnchorX ;
    QDoubleSpinBox * spinBoxAnchorY ;
    QDoubleSpinBox * spinBoxAnchorZ ;

    // orientation interface
    QComboBox * comboBoxWingOrientation;
    QComboBox * comboBoxSymmetry;


    // sweep interface
    QDoubleSpinBox * spinBoxSweep ;
    QPushButton* btnExpendSweepDetails;
    QWidget* widgetSweepDetails;
    QDoubleSpinBox* spinBoxSweepChord;
    QComboBox* comboBoxSweepMethod;

    // dihedral interface
    QDoubleSpinBox * spinBoxDihedral ;
    QPushButton* btnExpendDihedralDetails;
    QWidget* widgetDihedralDetails;
    QDoubleSpinBox* spinBoxDihedralChord;


    // area interface
    QPushButton* btnExpendAreaDetails;
    QWidget* widgetAreaDetails;
    QDoubleSpinBox * spinBoxAreaXY;
    QDoubleSpinBox * spinBoxAreaXZ;
    QDoubleSpinBox * spinBoxAreaYZ;
    QDoubleSpinBox * spinBoxAreaT;
    QCheckBox * checkBoxIsAreaConstant;

    // span interface;
    QDoubleSpinBox * spinBoxSpan;
    QCheckBox * checkBoxIsSpanConstant;

    // AR interface;
    QDoubleSpinBox * spinBoxAR;
    QCheckBox * checkBoxIsARConstant;

    // airfoil interface
    QPushButton* btnExpendAirfoilDetails;
    QWidget* widgetAirfoilDetails;
    QComboBox* comboBoxAirfoil;

    // standardization interface
    QComboBox* comboBoxStdGlobal;
    QPushButton* btnExpendStdDetails;
    QWidget* widgetStdDetails;
    QCheckBox* checkBoxStdAirfoils;
    QCheckBox* checkBoxStdPositionings;
    QCheckBox* checkBoxStdSections;
    QCheckBox* checkBoxStdAnchor;
    // use to inform the setStdCheckBoxesFromComboBox that the change is called form setStdComboBoxFromStdCheckBoxes
    bool callFromSetStdComboBox;

    // internal values are used to keep track of the previous values to notice if a change occurs.

    // internal anchor
    double internalAnchorX;
    double internalAnchorY;
    double internalAnchorZ;

    // internal orientation
    QString internalWingOrientation;
    QString internalSymmetry;

    // internal standardization
    bool internalStdAirfoils;
    bool internalStdPositionings;
    bool internalStdSections;
    bool internalStdAnchor;
    QString internalStGlobal;



    // internal sweep
    double internalSweep;
    double internalSweepChord;
    QString internalMethod;


    // internal dihedral
    double internalDihedral;
    double internalDihedralChord;

    // internal area
    double internalAreaXY;
    double internalAreaXZ;
    double internalAreaYZ;
    double internalAreaT;

    // internal span
    double internalSpan;

    // internal AR
    double internalAR;


    // internal airfoil
    QString internalAirfoilUID;
*/
};

#endif //TIGL_TIGLVIEWERWINGWIDGET_H
