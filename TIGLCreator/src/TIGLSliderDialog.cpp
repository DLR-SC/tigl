/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-06-29 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "TIGLSliderDialog.h"
#include "ui_TIGLSliderDialog.h"

#include <QEvent>


TIGLSliderDialog::TIGLSliderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLSliderDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::Popup);
    ui->slider->setFocus();
    ui->slider->installEventFilter(this);

    connect(ui->slider, SIGNAL(valueChanged(int)), this, SIGNAL(intValueChanged(int)));
}

TIGLSliderDialog::~TIGLSliderDialog()
{
    delete ui;
}

void TIGLSliderDialog::setIntValue(int value)
{
    ui->slider->setValue(value);
}

void TIGLSliderDialog::setIntRange(int min, int max)
{
    ui->slider->setMinimum(min);
    ui->slider->setMaximum(max);
}

bool TIGLSliderDialog::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::FocusOut && obj == ui->slider)
    {
      this->close();
      return true;
    }

    return false;
}

