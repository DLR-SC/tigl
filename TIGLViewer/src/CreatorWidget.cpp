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

#include "CreatorWidget.h"
#include "ui_CreatorWidget.h"

CreatorWidget::CreatorWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatorWidget)
{
    ui->setupUi(this);
}

CreatorWidget::~CreatorWidget()
{
    delete ui;
}

ModificatorWingWidget* CreatorWidget::getWingWidget()
{
    return ui->wingModificator;
}

ModificatorFuselageWidget* CreatorWidget::getFuselageWidget()
{
    return ui->fuselageModificator;
}

ModificatorTransformationWidget* CreatorWidget::getTransformationWidget()
{

    return ui->transformationModificator;
}

QTreeView* CreatorWidget::getQTreeView()
{
    return ui->treeView;
}

QWidget* CreatorWidget::getApplyWidget()
{
    return ui->applyWidget;
}
