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
// Created by cfse on 3/21/18.
//

#include "TIGLViewerTransformationWidget.h"
//#include "ModificatorManager.h"
#include <iostream>

TIGLViewerTransformationWidget::TIGLViewerTransformationWidget(QWidget *parent):
    ModificatorWidget(parent)
{}

/*

void TIGLViewerTransformationWidget::apply() {
    setInternalFromSpinBoxes();
    associateManager->adapter->setTransformation(transformationItem, transformation);
    associateManager->adapter->writeToFile();
}


void TIGLViewerTransformationWidget::init(ModificatorManager* associate)
{
    ModificatorWidget::init(associate);
    boxSX = this->findChild<QDoubleSpinBox*>("spinBoxSX");
    boxSY = this->findChild<QDoubleSpinBox*>("spinBoxSY");
    boxSZ = this->findChild<QDoubleSpinBox*>("spinBoxSZ");

    boxRX = this->findChild<QDoubleSpinBox*>("spinBoxRX");
    boxRY = this->findChild<QDoubleSpinBox*>("spinBoxRY");
    boxRZ = this->findChild<QDoubleSpinBox*>("spinBoxRZ");

    boxTX = this->findChild<QDoubleSpinBox*>("spinBoxTX");
    boxTY = this->findChild<QDoubleSpinBox*>("spinBoxTY");
    boxTZ = this->findChild<QDoubleSpinBox*>("spinBoxTZ");

    setSpinBoxesFromInternal();
}


void TIGLViewerTransformationWidget::setTransformation(cpcr::CPACSTreeItem* item) {

    this->transformationItem = item;
    transformation = this->associateManager->adapter->getTransformation(transformationItem);
    setSpinBoxesFromInternal();
}



void TIGLViewerTransformationWidget::setSpinBoxesFromInternal()
{
    boxSX->setValue(transformation.getScaling().x);
    boxSY->setValue(transformation.getScaling().y);
    boxSZ->setValue(transformation.getScaling().z);

    boxRX->setValue(transformation.getRotation().x);
    boxRY->setValue(transformation.getRotation().y);
    boxRZ->setValue(transformation.getRotation().z);

    boxTX->setValue(transformation.getTranslation().x);
    boxTY->setValue(transformation.getTranslation().y);
    boxTZ->setValue(transformation.getTranslation().z);

}


void TIGLViewerTransformationWidget::setInternalFromSpinBoxes()
{
    transformation.setScaling( cpcr::Point( boxSX->value(), boxSY->value(), boxSZ->value()) );
    transformation.setRotation(cpcr::Point (boxRX->value(), boxRY->value(), boxRZ->value() ));
    transformation.setTranslation(cpcr::Point( boxTX->value(), boxTY->value(), boxTZ->value() ));
}




void TIGLViewerTransformationWidget::keyPressEvent(QKeyEvent *event)
{
    std::cout << "event->key(): "  << std::endl;
    setInternalFromSpinBoxes();


}*/
