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

namespace Ui
{
class ModificatorContainerWidget;
}

class ModificatorContainerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModificatorContainerWidget(QWidget* parent = nullptr);
    ~ModificatorContainerWidget();

    // getter for specialized modificator
    ModificatorWingWidget* getWingWidget();
    ModificatorFuselageWidget* getFuselageWidget();
    ModificatorTransformationWidget* getTransformationWidget();
    QWidget* getNoInterfaceWidget();
    QWidget* getApplyWidget();
    QPushButton* getCommitButton();
    QPushButton* getCancelButton();

private:
    Ui::ModificatorContainerWidget* ui;
};

#endif // MODIFICATORCONTAINERWIDGET_H
