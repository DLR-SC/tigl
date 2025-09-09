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

#include "TIGLCreatorShapeIntersectionDialog.h"
#include "ui_TIGLCreatorShapeIntersectionDialog.h"

#include "CTiglUIDManager.h"

#include <algorithm>

tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::n(0,0,1);
tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::w(0,0,1);
tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::p1(0,0,0);
tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::p2(1,0,0);
tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::p(0,0,0);
int TIGLCreatorShapeIntersectionDialog::shape1Selected = -1;
int TIGLCreatorShapeIntersectionDialog::shape2Selected = -1;
int TIGLCreatorShapeIntersectionDialog::shapeSelected  = -1;
int TIGLCreatorShapeIntersectionDialog::shapeSSelected = -1;
int TIGLCreatorShapeIntersectionDialog::lastMode = 0;

TIGLCreatorShapeIntersectionDialog::TIGLCreatorShapeIntersectionDialog(tigl::CTiglUIDManager& m, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLCreatorShapeIntersectionDialog),
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
        ui->shapeComboS->addItem(uid.c_str());
    }

    // set point and normal
    ui->pxSB->setValue(p.x);
    ui->pySB->setValue(p.y);
    ui->pzSB->setValue(p.z);
    ui->nxSB->setValue(n.x);
    ui->nySB->setValue(n.y);
    ui->nzSB->setValue(n.z);

    // set points and normal
    ui->p1xSB->setValue(p1.x);
    ui->p1ySB->setValue(p1.y);
    ui->p1zSB->setValue(p1.z);
    ui->p2xSB->setValue(p2.x);
    ui->p2ySB->setValue(p2.y);
    ui->p2zSB->setValue(p2.z);
    ui->wxSB->setValue(w.x);
    ui->wySB->setValue(w.y);
    ui->wzSB->setValue(w.z);

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
    connect(ui->pxSB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->pySB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->pzSB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->nxSB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->nySB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->nzSB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->p1xSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->p1ySB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->p1zSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->p2xSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->p2ySB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->p2zSB, SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->wxSB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->wySB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->wzSB,  SIGNAL(valueChanged(double)), this, SLOT(OnItemChanged()));
    connect(ui->shape1Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemChanged()));
    connect(ui->shape2Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemChanged()));
    connect(ui->shapeCombo,  SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemChanged()));
    connect(ui->shapeComboS, SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemChanged()));
    connect(ui->TabView, SIGNAL(currentChanged(int)), this, SLOT(OnItemChanged()));
}

void TIGLCreatorShapeIntersectionDialog::OnItemChanged()
{
    p.x = ui->pxSB->value();
    p.y = ui->pySB->value();
    p.z = ui->pzSB->value();

    n.x = ui->nxSB->value();
    n.y = ui->nySB->value();
    n.z = ui->nzSB->value();

    p1.x = ui->p1xSB->value();
    p1.y = ui->p1ySB->value();
    p1.z = ui->p1zSB->value();

    p2.x = ui->p2xSB->value();
    p2.y = ui->p2ySB->value();
    p2.z = ui->p2zSB->value();

    w.x = ui->wxSB->value();
    w.y = ui->wySB->value();
    w.z = ui->wzSB->value();

    shape1Selected = ui->shape1Combo->currentIndex();
    shape2Selected = ui->shape2Combo->currentIndex();
    shapeSelected  = ui->shapeCombo->currentIndex();
    shapeSSelected = ui->shapeComboS->currentIndex();

    lastMode = ui->TabView->currentIndex();
}

tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::GetPoint()
{
    return p;
}

tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::GetPoint1()
{
    return p1;
}

tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::GetPoint2()
{
    return p2;
}

tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::GetW()
{
    return w;
}

tigl::CTiglPoint TIGLCreatorShapeIntersectionDialog::GetNormal()
{
    return n;
}

QString TIGLCreatorShapeIntersectionDialog::GetShape1UID()
{
    return ui->shape1Combo->currentText();
}

QString TIGLCreatorShapeIntersectionDialog::GetShape2UID()
{
    return ui->shape2Combo->currentText();
}

QString TIGLCreatorShapeIntersectionDialog::GetShapeUID()
{
    return ui->shapeCombo->currentText();
}

QString TIGLCreatorShapeIntersectionDialog::GetShapeSUID()
{
    return ui->shapeComboS->currentText();
}


int TIGLCreatorShapeIntersectionDialog::GetMode()
{
    return lastMode;
}

TIGLCreatorShapeIntersectionDialog::~TIGLCreatorShapeIntersectionDialog()
{
    delete ui;
}
