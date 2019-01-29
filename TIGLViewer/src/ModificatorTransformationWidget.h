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

#ifndef TIGL_TIGLVIEWERTRANSFORMATIONWIDGET_H
#define TIGL_TIGLVIEWERTRANSFORMATIONWIDGET_H

#include "ModificatorWidget.h"
#include "CCPACSTransformation.h"
#include "geometry/CTiglTransformation.h"
#include <QDoubleSpinBox>

class ModificatorTransformationWidget : public ModificatorWidget
{
    Q_OBJECT

public:
    ModificatorTransformationWidget(QWidget* parent = 0);

    // Initialize the linked objects
    void init();

    void setTransformation(tigl::CCPACSTransformation& transformationItem);
    void setSpinBoxesFromInternal();
    void setInternalFromSpinBoxes();

    void apply() override;

private:
    tigl::CCPACSTransformation* tiglTransformation;

    QDoubleSpinBox* boxSX;
    QDoubleSpinBox* boxSY;
    QDoubleSpinBox* boxSZ;

    QDoubleSpinBox* boxRX;
    QDoubleSpinBox* boxRY;
    QDoubleSpinBox* boxRZ;

    QDoubleSpinBox* boxTX;
    QDoubleSpinBox* boxTY;
    QDoubleSpinBox* boxTZ;

    tigl::CTiglTransformation internalTransformation;
};

#endif // TIGL_TIGLVIEWERTRANSFORMATIONWIDGET_H
