/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-03-18 Martin Siggel <martin.siggel@dlr.de>
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

#include "TIGLViewerShapeIntersectionDialog.h"
#include "ui_TIGLViewerShapeIntersectionDialog.h"

#include "CTiglUIDManager.h"

#include <algorithm>

tigl::CTiglPoint TIGLViewerShapeIntersectionDialog::n(0,0,1);
tigl::CTiglPoint TIGLViewerShapeIntersectionDialog::p(0,0,0);
int TIGLViewerShapeIntersectionDialog::shape1Selected = -1;
int TIGLViewerShapeIntersectionDialog::shape2Selected = -1;
int TIGLViewerShapeIntersectionDialog::shapeSelected  = -1;
int TIGLViewerShapeIntersectionDialog::lastMode = 0;

TIGLViewerShapeIntersectionDialog::TIGLViewerShapeIntersectionDialog(tigl::CTiglUIDManager& m, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerShapeIntersectionDialog),
    uidManager(m)
{
    ui->setupUi(this);

    // fill combo boxes
    const tigl::ShapeContainerType& uids = m.GetShapeContainer();
    tigl::ShapeContainerType::const_iterator it;
    std::vector<std::string> uidStrings;

    for (it = uids.begin(); it != uids.end(); ++it) {
        uidStrings.push_back(it->first);
    }

    // sort uids
    std::sort(uidStrings.begin(), uidStrings.end());
    std::vector<std::string>::iterator it2;
    for (it2 = uidStrings.begin(); it2 != uidStrings.end(); ++it2) {
        std::string uid = *it2;
        ui->shape1Combo->addItem(uid.c_str());
        ui->shape2Combo->addItem(uid.c_str());
        ui->shapeCombo->addItem(uid.c_str());
    }

    // set point and normal
    ui->pxSB->setValue(p.x);
    ui->pySB->setValue(p.y);
    ui->pzSB->setValue(p.z);
    ui->nxSB->setValue(n.x);
    ui->nySB->setValue(n.y);
    ui->nzSB->setValue(n.z);
    // set selected comboboxes
    
    if (shape1Selected >= 0) {
        ui->shape1Combo->setCurrentIndex(shape1Selected);
    }
    if (shape2Selected >= 0) {
        ui->shape2Combo->setCurrentIndex(shape2Selected);
    }
    if (shapeSelected >= 0) {
        ui->shapeCombo->setCurrentIndex(shapeSelected);
    }

    // select tab
    ui->TabView->setCurrentIndex(lastMode);

    // connect signals
    connect(ui->pxSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->pySB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->pzSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->nxSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->nySB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->nzSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->shape1Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemChanged()));
    connect(ui->shape2Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemChanged()));
    connect(ui->shapeCombo,  SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemChanged()));
    connect(ui->TabView, SIGNAL(currentChanged(int)), this, SLOT(OnItemChanged()));
}

void TIGLViewerShapeIntersectionDialog::OnItemChanged()
{
    p.x = ui->pxSB->value();
    p.y = ui->pySB->value();
    p.z = ui->pzSB->value();

    n.x = ui->nxSB->value();
    n.y = ui->nySB->value();
    n.z = ui->nzSB->value();

    shape1Selected = ui->shape1Combo->currentIndex();
    shape2Selected = ui->shape2Combo->currentIndex();
    shapeSelected  = ui->shapeCombo->currentIndex();

    lastMode = ui->TabView->currentIndex();
}

tigl::CTiglPoint TIGLViewerShapeIntersectionDialog::GetPoint()
{
    return p;
}

tigl::CTiglPoint TIGLViewerShapeIntersectionDialog::GetNormal()
{
    return n;
}

QString TIGLViewerShapeIntersectionDialog::GetShape1UID()
{
    return ui->shape1Combo->currentText();
}

QString TIGLViewerShapeIntersectionDialog::GetShape2UID()
{
    return ui->shape2Combo->currentText();
}

QString TIGLViewerShapeIntersectionDialog::GetShapeUID()
{
    return ui->shapeCombo->currentText();
}


int TIGLViewerShapeIntersectionDialog::GetMode()
{
    return lastMode;
}

TIGLViewerShapeIntersectionDialog::~TIGLViewerShapeIntersectionDialog()
{
    delete ui;
}
