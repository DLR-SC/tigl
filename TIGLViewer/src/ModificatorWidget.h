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

#ifndef TIGL_MODIFICATORWIDGET_H
#define TIGL_MODIFICATORWIDGET_H

#include <QWidget>

// class ModificatorManager; // forward delcaration

class ModificatorWidget : public QWidget
{

    Q_OBJECT

public:
    ModificatorWidget(QWidget* parent = 0);

    // init the associate manager and the coorespendance between GUI interface and the object
    //    virtual void init(ModificatorManager * associate);

    // reset the dispay value from the value of cpacs file (called when cancel button is pressed)
    virtual void reset();

    // should check if modification was perform , if yes -> apply it on the cpacs file
    // modification check generally if some modification has occur with isApprox to not have false positif
    // call when commit button is pressed
    virtual void apply();

protected:
    double precision;
    // return true if the abs(a-b) < precision
    bool isApprox(double a, double b);

    //    ModificatorManager * associateManager;
};

#endif // TIGL_MODIFICATORWIDGET_H
