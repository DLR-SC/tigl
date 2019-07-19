/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef MODIFICATORELEMENTWIDGET_H
#define MODIFICATORELEMENTWIDGET_H

#include "modificators/ModificatorWidget.h"
#include "modificators/CTiglPointWidget.h"
#include "CCPACSFuselageSectionElement.h"
#include "CTiglSectionElement.h"

namespace Ui
{
class ModificatorElementWidget;
}

/**
 * This widget is the interface for a CTiglElement.
 * The idea is to use this widget when we desire to display a CPACSElement (WingElement or SectionElement).
 * It contains the function to set the element in a high level description without taking care of
 * the underlying Cpacs structure.
 */
class ModificatorElementWidget : public ModificatorWidget
{
    Q_OBJECT



private slots:

public:
    explicit ModificatorElementWidget(QWidget* parent = nullptr);
    ~ModificatorElementWidget();

    void setElement(tigl::CTiglSectionElement& inElement);

    bool apply() override;

    void reset() override;

private:
    Ui::ModificatorElementWidget* ui;
    tigl::CTiglSectionElement* element;

    double internalRotAroundN;
    double internalHeight;
    double internalWidth;
    double internalArea;

    // tell us if a width, height or area was modified at last and gives the modification priority.
    QString lastModifiedDimensionalParameter;

};

#endif // MODIFICATORELEMENTWIDGET_H
