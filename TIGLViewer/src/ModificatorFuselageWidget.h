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

#ifndef TIGL_MODIFICATORFUSELAGEWIDGET_H
#define TIGL_MODIFICATORFUSELAGEWIDGET_H

#include "CCPACSFuselage.h"
#include "CPACSTreeItem.h"
#include "ModificatorWidget.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>

class ModificatorFuselageWidget : public ModificatorWidget
{

    Q_OBJECT

public slots:

    void setPartialLengthFromComboBoxes();
    // If a new partial length is set it will recompute the the global length
    void recomputeTotalLength(double newPartialLength);

    void setRadiusBetweenFromComboBoxes();

    void expendLengthDetails(bool checked);
    void expendRadiusDetails(bool checked);

public:
    ModificatorFuselageWidget(QWidget* parent = 0);

    void setFuselage(tigl::CCPACSFuselage& fuselage);

    // Initialize the linked objects
    void init();

    void apply() override;

    void reset() override;

private:
    // Length parameters
    QDoubleSpinBox* spinBoxLength;
    QPushButton* btnExpendLengthDetails;
    QWidget* widgetLengthDetails;
    QComboBox* comboBoxLengthE1;
    QComboBox* comboBoxLengthE2;
    QDoubleSpinBox* spinBoxPartialLength;

    // Internal length parameters
    double internalLength;
    double internalPartialLength;

    // Circumference parameters:
    QDoubleSpinBox* spinBoxRadius;
    QPushButton* btnExpendRadiusDetails;
    QWidget* widgetRadiusDetails;
    QComboBox* comboBoxRadiusBE1;
    QComboBox* comboBoxRadiusBE2;
    QDoubleSpinBox* spinBoxRadiusBetween;

    // Internal circumference parameters:
    double internalRadius;
    double internalRadiusBetween;

private:
    tigl::CCPACSFuselage* fuselage;
};

#endif // TIGL_TIGLVIEWERFUSELAGEWIDGET_H
