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

#ifndef MODIFICATORCONTAINERWIDGET_H
#define MODIFICATORCONTAINERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "ModificatorWingWidget.h"
#include "ModificatorFuselageWidget.h"
#include "ModificatorTransformationWidget.h"
#include "CPACSFuselages.h"
#include "CPACSWing.h"
#include "CPACSTransformation.h"
#include "CTiglSectionElement.h"

namespace Ui
{
class ModificatorContainerWidget;
}

/**
 * @brief Container for the the spezialized modificator widgets.
 *
 * This class holds the specialized modificator widget as the Transformation widget,
 * the Wing widget, the Fuselage widget ...
 * Every moment there is only one modificator widget displayed based on the element
 * selected in the CPACS tree.
 * This class has function to set a particular modificator widget and to connect the
 * commit and cancel button to this particular widget.
 * For example, the function "setWingModificator" will set the wing modificator
 * with the value passed as argument and display the wing widget. Then, when the commit
 * or cancel button is pressed, the correct function will be called on the wing widget.
 *
 */
class ModificatorContainerWidget : public QWidget
{
    Q_OBJECT

signals:
    void undoCommandRequired();

public slots:
    void applyCurrentModifications();
    void applyCurrentCancellation();

public:
    explicit ModificatorContainerWidget(QWidget* parent = nullptr);
    ~ModificatorContainerWidget();

    void setTransformationModificator(tigl::CCPACSTransformation& transformation);
    void setWingModificator(tigl::CCPACSWing& wing);
    void setFuselageModificator(tigl::CCPACSFuselage& fuselage);
    void setElementModificator(tigl::CTiglSectionElement& element);
    void setSectionModificator(QList<tigl::CTiglSectionElement*> elements);

    void setNoInterfaceWidget();

    void hideAllSpecializedWidgets();

private:
    Ui::ModificatorContainerWidget* ui;

    ModificatorWidget* currentModificator;
};

#endif // MODIFICATORCONTAINERWIDGET_H
