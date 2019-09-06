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

#include "ModificatorSectionWidget.h"
#include "ui_ModificatorSectionWidget.h"

ModificatorSectionWidget::ModificatorSectionWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorSectionWidget)
{
    ui->setupUi(this);
}

ModificatorSectionWidget::~ModificatorSectionWidget()
{
    deleteElementWidgets();
    delete ui;
}

void ModificatorSectionWidget::setAssociatedElements(QList<tigl::CTiglSectionElement*> elements, ProfilesDBManager* profilesDB)
{

    deleteElementWidgets();

    for (int i = 0; i < elements.size(); i++) {
        ModificatorElementWidget* elementWidget = new ModificatorElementWidget(this);
        elementWidget->setElement(*(elements[i]), profilesDB);
        ui->modificatorSectionLayout->addWidget(elementWidget);
        elementWidgets.push_back(elementWidget);
    }
}

void ModificatorSectionWidget::deleteElementWidgets()
{

    for (int i = 0; i < elementWidgets.size(); i++) {
        delete elementWidgets[i];
    }
    elementWidgets.clear();
}

bool ModificatorSectionWidget::apply()
{
    bool wasModified = false;
    for (int i = 0; i < elementWidgets.size(); i++) {
        wasModified = wasModified || elementWidgets[i]->apply();
    }
    return wasModified;
}

void ModificatorSectionWidget::reset()
{
    for (int i = 0; i < elementWidgets.size(); i++) {
        elementWidgets[i]->reset();
    }
}
