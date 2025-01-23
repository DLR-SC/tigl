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

#include "TIGLViewerDrawBoxDialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

TIGLViewerDrawBoxDialog::TIGLViewerDrawBoxDialog(const QString& title, QWidget *parent)
    : QDialog(parent)
    , origin_x(new QDoubleSpinBox())
    , origin_y(new QDoubleSpinBox())
    , origin_z(new QDoubleSpinBox())
    , dx(new QDoubleSpinBox())
    , dy(new QDoubleSpinBox())
    , dz(new QDoubleSpinBox())
{
    setWindowTitle(title);

    // set decimals for all spinboxes
    int decimals = 7;
    origin_x->setDecimals(decimals);
    origin_y->setDecimals(decimals);
    origin_z->setDecimals(decimals);
    dx->setDecimals(decimals);
    dy->setDecimals(decimals);
    dz->setDecimals(decimals);

    // set ranges for all spinboxes
    double origin_min = -1e4;
    double origin_max = 1e4;
    origin_x->setRange(origin_min, origin_max);
    origin_y->setRange(origin_min, origin_max);
    origin_z->setRange(origin_min, origin_max);

    double extents_min = 1e-7;
    double extents_max = 1e4;
    dx->setRange(extents_min, extents_max);
    dy->setRange(extents_min, extents_max);
    dz->setRange(extents_min, extents_max);

    // set default values for all spinboxes
    origin_x->setValue(0.);
    origin_y->setValue(0.);
    origin_z->setValue(0.);
    dx->setValue(1.);
    dy->setValue(1.);
    dz->setValue(1.);

    // The main layout is vertical with a horizontal layout on top
    // and the dialog buttons below
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    setLayout(verticalLayout);

    // crate a horizontal layout for the position and extents
    QHBoxLayout* horizontalLayout = new QHBoxLayout();

    // A group box for setting the origin
    QGroupBox* groupPoint = new QGroupBox("Position");
    QGridLayout* groupPointLayout = new QGridLayout();
    groupPointLayout->addWidget(new QLabel("X = "), 0, 0);
    groupPointLayout->addWidget(origin_x, 0, 1);
    groupPointLayout->addWidget(new QLabel("Y = "), 1, 0);
    groupPointLayout->addWidget(origin_y, 1, 1);
    groupPointLayout->addWidget(new QLabel("z = "), 2, 0);
    groupPointLayout->addWidget(origin_z, 2, 1);
    groupPoint->setLayout(groupPointLayout);
    horizontalLayout->addWidget(groupPoint);

    // A group box for setting the Extents
    QGroupBox* groupExtents = new QGroupBox("Extents");
    QGridLayout* groupExtentsLayout = new QGridLayout();
    groupExtentsLayout->addWidget(new QLabel("dx = "), 0, 0);
    groupExtentsLayout->addWidget(dx, 0, 1);
    groupExtentsLayout->addWidget(new QLabel("dy = "), 1, 0);
    groupExtentsLayout->addWidget(dy, 1, 1);
    groupExtentsLayout->addWidget(new QLabel("dz = "), 2, 0);
    groupExtentsLayout->addWidget(dz, 2, 1);
    groupExtents->setLayout(groupExtentsLayout);
    horizontalLayout->addWidget(groupExtents);

    // create Ok and Cancel buttons and connect them
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Cancel|QDialogButtonBox::Ok,
        Qt::Horizontal
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TIGLViewerDrawBoxDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TIGLViewerDrawBoxDialog::reject);

    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->addWidget(buttonBox);
    verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
}

tigl::CTiglPoint TIGLViewerDrawBoxDialog::getPoint() const
{
    tigl::CTiglPoint point(0,0,0);
    point.x = origin_x->value();
    point.y = origin_y->value();
    point.z = origin_z->value();
    
    return point;
}

tigl::CTiglPoint TIGLViewerDrawBoxDialog::getDirection() const
{
    tigl::CTiglPoint direction(0,0,0);
    direction.x = dx->value();
    direction.y = dy->value();
    direction.z = dz->value();
    
    return direction;
}
