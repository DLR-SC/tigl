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

#include "ModificatorSectionsWidget.h"
#include "ui_ModificatorSectionsWidget.h"

ModificatorSectionsWidget::ModificatorSectionsWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ModificatorSectionsWidget)
{
    ui->setupUi(this);
    connect(ui->addConnectedElementBtn, SIGNAL(pressed()), this, SLOT(execNewConnectedElementDialog()));
    connect(ui->deleteConnectedElementBtn, SIGNAL(pressed()), this, SLOT(execDeleteConnectedElementDialog()));
}

ModificatorSectionsWidget::~ModificatorSectionsWidget()
{
    delete ui;
}

void ModificatorSectionsWidget::setCreateConnectedElement(Ui::ElementModificatorInterface const& element)
{
    createConnectedElement = element;
    update_delete_section_button_disabled_state();
}

void ModificatorSectionsWidget::update_delete_section_button_disabled_state()
{
    if (createConnectedElement == std::nullopt) {
        return;
    }
    // at least two sections required
    if (createConnectedElement->GetOrderedConnectedElement().size() <= 2) {
        ui->deleteConnectedElementBtn->setDisabled(true);
        ui->deleteConnectedElementBtn->setToolTip("Section deletion not allowed: At least two sections are required.");
    } else
    {
        ui->deleteConnectedElementBtn->setDisabled(false);
        ui->deleteConnectedElementBtn->setToolTip("");
    }
}

void ModificatorSectionsWidget::execNewConnectedElementDialog()
{
    if (createConnectedElement == std::nullopt) {
        LOG(ERROR) << "ModificatorSectionsWidget:: is not correctly set!";
        return;
    }

    emit addSectionRequested(*createConnectedElement);
    update_delete_section_button_disabled_state();
}


void ModificatorSectionsWidget::execDeleteConnectedElementDialog()
{

    if (createConnectedElement == std::nullopt) {
        LOG(ERROR) << "ModificatorWingsWidget::execDeleteWingDialog: wings is not set!";
        return;
    }

    emit deleteSectionRequested(*createConnectedElement);
    update_delete_section_button_disabled_state();
}
