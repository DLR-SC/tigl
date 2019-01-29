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

#ifndef TIGL_TIGLVIEWERWINGWIDGET_H
#define TIGL_TIGLVIEWERWINGWIDGET_H

#include "CCPACSWing.h"
#include "ModificatorWidget.h"
#include "QCheckBox"
#include "QComboBox"
#include "QDoubleSpinBox"
#include "QPushButton"
#include "QWidget"

class ModificatorWingWidget : public ModificatorWidget
{

    Q_OBJECT

public slots:

    void expendAreaDetails(bool checked);
    void expendDihedralDetails(bool checked);
    void expendSweepDetails(bool checked);

    void setAreaConstant(bool checked);
    void setSpanConstant(bool checked);
    void setARConstant(bool checked);

public:
    ModificatorWingWidget(QWidget* parent = 0);

    void init();
    void apply() override;
    void reset() override;

    void setWing(tigl::CCPACSWing& wing);

private:
    tigl::CCPACSWing* tiglWing;

    // anchor interface
    QDoubleSpinBox* spinBoxAnchorX;
    QDoubleSpinBox* spinBoxAnchorY;
    QDoubleSpinBox* spinBoxAnchorZ;

    QComboBox* comboBoxSymmetry;

    // sweep interface
    QDoubleSpinBox* spinBoxSweep;
    QPushButton* btnExpendSweepDetails;
    QWidget* widgetSweepDetails;
    QDoubleSpinBox* spinBoxSweepChord;
    QComboBox* comboBoxSweepMethod;

    // dihedral interface
    QDoubleSpinBox* spinBoxDihedral;
    QPushButton* btnExpendDihedralDetails;
    QWidget* widgetDihedralDetails;
    QDoubleSpinBox* spinBoxDihedralChord;

    // area interface
    QPushButton* btnExpendAreaDetails;
    QWidget* widgetAreaDetails;
    QDoubleSpinBox* spinBoxAreaXY;
    QDoubleSpinBox* spinBoxAreaXZ;
    QDoubleSpinBox* spinBoxAreaYZ;
    QDoubleSpinBox* spinBoxAreaT;
    QCheckBox* checkBoxIsAreaConstant;

    // span interface;
    QDoubleSpinBox* spinBoxSpan;
    QCheckBox* checkBoxIsSpanConstant;

    // AR interface;
    QDoubleSpinBox* spinBoxAR;
    QCheckBox* checkBoxIsARConstant;

    // internal anchor
    double internalAnchorX;
    double internalAnchorY;
    double internalAnchorZ;

    // internal symmetry
    QString internalSymmetry;

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
};

#endif // TIGL_TIGLVIEWERWINGWIDGET_H
