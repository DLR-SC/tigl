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

#ifndef TIGL_TIGLVIEWERTRANSFORMATIONWIDGET_H
#define TIGL_TIGLVIEWERTRANSFORMATIONWIDGET_H

#include "ModificatorWidget.h"
//#include "CPACSCreatorLib/CPACSTreeItem.h"
//#include "CPACSCreatorLib/CPACSTransformation.h"
//#include "CPACSCreatorLib/Point.h"
#include <QDoubleSpinBox>

class TIGLViewerTransformationWidget : public ModificatorWidget
{
    Q_OBJECT

public:
    TIGLViewerTransformationWidget(QWidget* parent = 0);

    /*
    // Initialize the linked objects
    void init(ModificatorManager* associate) override ;

    void setTransformation(cpcr::CPACSTreeItem* transformationItem);
    void setSpinBoxesFromInternal();
    void setInternalFromSpinBoxes();

    void apply() override ;

protected:

    void keyPressEvent(QKeyEvent *event);

private:

    cpcr::CPACSTreeItem * transformationItem;

    QDoubleSpinBox* boxSX;
    QDoubleSpinBox* boxSY;
    QDoubleSpinBox* boxSZ;

    QDoubleSpinBox* boxRX;
    QDoubleSpinBox* boxRY;
    QDoubleSpinBox* boxRZ;

    QDoubleSpinBox* boxTX;
    QDoubleSpinBox* boxTY;
    QDoubleSpinBox* boxTZ;

    cpcr::CPACSTransformation transformation;

*/
};

#endif //TIGL_TIGLVIEWERTRANSFORMATIONWIDGET_H
