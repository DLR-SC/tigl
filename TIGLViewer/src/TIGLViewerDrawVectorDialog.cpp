/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-09-05 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "TIGLViewerDrawVectorDialog.h"
#include "ui_TIGLViewerDrawVectorDialog.h"

TIGLViewerDrawVectorDialog::TIGLViewerDrawVectorDialog(const QString& title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerDrawVectorDialog)
{
    ui->setupUi(this);
    setWindowTitle(title);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void TIGLViewerDrawVectorDialog::setDirectionEnabled(bool enabled)
{
    ui->groupDirection->setVisible(enabled);
}

tigl::CTiglPoint TIGLViewerDrawVectorDialog::getPoint() const
{
    tigl::CTiglPoint point(0,0,0);
    point.x = ui->pxSB->value();
    point.y = ui->pySB->value();
    point.z = ui->pzSB->value();
    
    return point;
}

tigl::CTiglPoint TIGLViewerDrawVectorDialog::getDirection() const
{
    tigl::CTiglPoint direction(0,0,0);
    direction.x = ui->nxSB->value();
    direction.y = ui->nySB->value();
    direction.z = ui->nzSB->value();
    
    return direction;
}

TIGLViewerDrawVectorDialog::~TIGLViewerDrawVectorDialog()
{
    delete ui;
}
