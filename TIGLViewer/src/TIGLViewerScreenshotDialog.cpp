/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-05-08 Martin Siggel <martin.siggel@dlr.de>
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

#include "TIGLViewerScreenshotDialog.h"
#include "ui_TIGLViewerScreenshotDialog.h"

#include <QFileInfo>

TIGLViewerScreenshotDialog::TIGLViewerScreenshotDialog(QString filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TiglViewerScreenshotDialog)
{
    ui->setupUi(this);
    ui->widthBox->selectAll();
    if (QFileInfo(filename).suffix() != "jpg") {
        // hide quality parameter box
        ui->qualityBox->setVisible(false);
    }
}

TIGLViewerScreenshotDialog::~TIGLViewerScreenshotDialog()
{
    delete ui;
}

void TIGLViewerScreenshotDialog::setImageSize(int width, int height)
{
    ui->widthBox->setValue(width);
    ui->heightBox->setValue(height);
    ui->widthBox->selectAll();
}

void TIGLViewerScreenshotDialog::getImageSize(int& width, int& height) const
{
    width = ui->widthBox->value();
    height = ui->heightBox->value();
}

void TIGLViewerScreenshotDialog::setQualityValue(int quality)
{
    ui->qualityBox_2->setValue(quality);
}

int TIGLViewerScreenshotDialog::getQualityValue() const
{
    return ui->qualityBox_2->value();
}

bool TIGLViewerScreenshotDialog::getWhiteBGEnabled() const
{
    return ui->whiteBGCheckbox->isChecked();
}
