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

#ifndef CREATORWIDGET_H
#define CREATORWIDGET_H

#include <QWidget>
#include <QTreeView>
#include "ModificatorWingWidget.h"
#include "ModificatorFuselageWidget.h"
#include "ModificatorTransformationWidget.h"

namespace Ui
{
class CreatorWidget;
}

class CreatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CreatorWidget(QWidget* parent = nullptr);
    ~CreatorWidget();

    // getter for modificators
    ModificatorWingWidget* getWingWidget();
    ModificatorFuselageWidget* getFuselageWidget();
    ModificatorTransformationWidget* getTransformationWidget();

    QTreeView* getQTreeView();
    QWidget* getApplyWidget();

private:
    Ui::CreatorWidget* ui;
};

#endif // CREATORWIDGET_H
