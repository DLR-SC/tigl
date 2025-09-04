/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-06-23 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "TIGLCreatorVTKExportDialog.h"
#include "ui_TIGLCreatorVTKExportDialog.h"

#include <tigl.h>
#include "CGlobalExporterConfigs.h"

TIGLCreatorVTKExportDialog::TIGLCreatorVTKExportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TIGLCreatorVTKExportDialog)
{
    ui->setupUi(this);

    setNormalsEnabled(true);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onOkayPressed()));
}

void TIGLCreatorVTKExportDialog::setDeflection(double d)
{
    ui->sbDeflection->setValue(d);
}

double TIGLCreatorVTKExportDialog::getDeflection() const
{
    return ui->sbDeflection->value();
}

void TIGLCreatorVTKExportDialog::setNormalsEnabled(bool enabled)
{
    ui->cbWriteNormals->setChecked(enabled);
}

bool TIGLCreatorVTKExportDialog::normalsEnabled() const
{
    return ui->cbWriteNormals->isChecked();
}

TIGLCreatorVTKExportDialog::~TIGLCreatorVTKExportDialog()
{
    delete ui;
}

void TIGLCreatorVTKExportDialog::onOkayPressed() const
{
    tigl::getExportConfig("vtk").Set("WriteNormals", normalsEnabled());
}
