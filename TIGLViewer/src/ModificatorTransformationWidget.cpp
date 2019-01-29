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

#include "ModificatorTransformationWidget.h"
//#include "ModificatorManager.h"
#include <iostream>

ModificatorTransformationWidget::ModificatorTransformationWidget(QWidget* parent)
    : ModificatorWidget(parent)
{
}

void ModificatorTransformationWidget::apply()
{
    setInternalFromSpinBoxes();
    // TODO: save in tigl object
    // Not done yet because we first need to have  a working
    // setSpinBoxesFromInternal
    // tiglTransformation->setTransformationMatrix(transformation);
}
void ModificatorTransformationWidget::init()
{
    boxSX = this->findChild<QDoubleSpinBox*>("spinBoxSX");
    boxSY = this->findChild<QDoubleSpinBox*>("spinBoxSY");
    boxSZ = this->findChild<QDoubleSpinBox*>("spinBoxSZ");

    boxRX = this->findChild<QDoubleSpinBox*>("spinBoxRX");
    boxRY = this->findChild<QDoubleSpinBox*>("spinBoxRY");
    boxRZ = this->findChild<QDoubleSpinBox*>("spinBoxRZ");

    boxTX = this->findChild<QDoubleSpinBox*>("spinBoxTX");
    boxTY = this->findChild<QDoubleSpinBox*>("spinBoxTY");
    boxTZ = this->findChild<QDoubleSpinBox*>("spinBoxTZ");

    internalTransformation.SetIdentity();
    setSpinBoxesFromInternal();
}

void ModificatorTransformationWidget::setTransformation(tigl::CCPACSTransformation& newTiglTransformation)
{

    this->tiglTransformation = &newTiglTransformation;
    internalTransformation   = tiglTransformation->getTransformationMatrix();
    setSpinBoxesFromInternal();
}

void ModificatorTransformationWidget::setSpinBoxesFromInternal()
{
    // TODO set from transformation -> need new CTigl functionalities
    boxSX->setValue(3);
    boxSY->setValue(3);
    boxSZ->setValue(3);

    boxRX->setValue(-1);
    boxRY->setValue(-1);
    boxRZ->setValue(-1);

    boxTX->setValue(internalTransformation.GetValue(0, 3));
    boxTY->setValue(internalTransformation.GetValue(1, 3));
    boxTZ->setValue(internalTransformation.GetValue(2, 3));
}

void ModificatorTransformationWidget::setInternalFromSpinBoxes()
{
    // apply the transformation in cpacs order: scale, rotation euler (XYZ),
    // translation
    internalTransformation.SetIdentity();
    internalTransformation.AddScaling(boxSX->value(), boxSY->value(), boxSZ->value());
    internalTransformation.AddRotationX(boxRX->value());
    internalTransformation.AddRotationY(boxRY->value());
    internalTransformation.AddRotationZ(boxRZ->value());
    internalTransformation.AddTranslation(boxTX->value(), boxTY->value(), boxTZ->value());
}
