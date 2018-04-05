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

#include "TIGLViewerVTKExportDialog.h"
#include "ui_TIGLViewerVTKExportDialog.h"

#include <tigl.h>
#include "CGlobalExporterConfigs.h"

TIGLViewerVTKExportDialog::TIGLViewerVTKExportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TIGLViewerVTKExportDialog)
{
    ui->setupUi(this);

    setNormalsEnabled(true);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onOkayPressed()));
}

void TIGLViewerVTKExportDialog::setDeflection(double d)
{
    ui->sbDeflection->setValue(d);
}

double TIGLViewerVTKExportDialog::getDeflection() const
{
    return ui->sbDeflection->value();
}

void TIGLViewerVTKExportDialog::setNormalsEnabled(bool enabled)
{
    ui->cbWriteNormals->setChecked(enabled);
}

bool TIGLViewerVTKExportDialog::normalsEnabled() const
{
    return ui->cbWriteNormals->isChecked();
}

TIGLViewerVTKExportDialog::~TIGLViewerVTKExportDialog()
{
    delete ui;
}

void TIGLViewerVTKExportDialog::onOkayPressed() const
{
    tigl::getExportConfig("vtk").Set("WriteNormals", normalsEnabled());
}
