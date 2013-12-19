/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-19 Martin Siggel <martin.siggel@dlr.de>
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

#include "TIGLViewerFuseDialog.h"
#include "ui_TIGLViewerFuseDialog.h"


static  bool _useSymmetries = true;
static  bool _trimWithFF    = false;

FuseDialog::FuseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FuseDialog)
{
    ui->setupUi(this);

    ui->mirrorCheckBox->setChecked(_useSymmetries);
    ui->trimFFCheckBox->setChecked(_trimWithFF);
}

FuseDialog::~FuseDialog()
{
    _trimWithFF = TrimWithFarField();
    _useSymmetries = UseSymmetries();

    delete ui;
}

bool FuseDialog::UseSymmetries() const
{
    return ui->mirrorCheckBox->isChecked();
}

bool FuseDialog::TrimWithFarField() const
{
    return ui->trimFFCheckBox->isChecked();
}


