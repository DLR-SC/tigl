/*
* Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
*
* Created: 2025-10-22 Sven Goldberg <sven.goldberg@dlr.de>
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

#include "TIGLCreatorAddSpotlightDialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

TIGLCreatorAddSpotlightDialog::TIGLCreatorAddSpotlightDialog(QWidget *parent)
    : QDialog(parent)
    , position_x(new QDoubleSpinBox())
    , position_y(new QDoubleSpinBox())
    , position_z(new QDoubleSpinBox())
    , dx(new QDoubleSpinBox())
    , dy(new QDoubleSpinBox())
    , dz(new QDoubleSpinBox())
    , concentration(new TIGLDoubleLineEdit())
{
    setWindowTitle("Add a spotlight");

    // set decimals for all spinboxes
    int decimals = 7;
    position_x->setDecimals(decimals);
    position_y->setDecimals(decimals);
    position_z->setDecimals(decimals);
    dx->setDecimals(decimals);
    dy->setDecimals(decimals);
    dz->setDecimals(decimals);

    // set ranges for all spinboxes
    double position_min = -1e4;
    double position_max = 1e4;
    position_x->setRange(position_min, position_max);
    position_y->setRange(position_min, position_max);
    position_z->setRange(position_min, position_max);

    double direction_min = -1e4;
    double direction_max = 1e4;
    dx->setRange(direction_min, direction_max);
    dy->setRange(direction_min, direction_max);
    dz->setRange(direction_min, direction_max);

    concentration->setRange(0., 1.);

    // set default values for all spinboxes
    position_x->setValue(0.);
    position_y->setValue(0.);
    position_z->setValue(0.);
    dx->setValue(1.);
    dy->setValue(1.);
    dz->setValue(1.);

    concentration->setValue(0.5, 2);

    // The main layout is vertical with a horizontal layout on top
    // and the dialog buttons below
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    setLayout(verticalLayout);

    // create a horizontal layout for the position and direction
    QHBoxLayout* horizontalLayout = new QHBoxLayout();

    // A group box for placing the lightsource
    QGroupBox* groupPosition = new QGroupBox("Spotlight position");
    QGridLayout* groupPositionLayout = new QGridLayout();
    groupPositionLayout->addWidget(new QLabel("X = "), 0, 0);
    groupPositionLayout->addWidget(position_x, 0, 1);
    groupPositionLayout->addWidget(new QLabel("Y = "), 1, 0);
    groupPositionLayout->addWidget(position_y, 1, 1);
    groupPositionLayout->addWidget(new QLabel("Z = "), 2, 0);
    groupPositionLayout->addWidget(position_z, 2, 1);
    groupPosition->setLayout(groupPositionLayout);
    horizontalLayout->addWidget(groupPosition);

    // A group box for defining the direction
    QGroupBox* groupDirection = new QGroupBox("Spotlight direction");
    QGridLayout* groupDirectionLayout = new QGridLayout();
    groupDirectionLayout->addWidget(new QLabel("dX = "), 0, 0);
    groupDirectionLayout->addWidget(dx, 0, 1);
    groupDirectionLayout->addWidget(new QLabel("dY = "), 1, 0);
    groupDirectionLayout->addWidget(dy, 1, 1);
    groupDirectionLayout->addWidget(new QLabel("dZ = "), 2, 0);
    groupDirectionLayout->addWidget(dz, 2, 1);
    groupDirection->setLayout(groupDirectionLayout);
    horizontalLayout->addWidget(groupDirection);

    QGroupBox* groupConcentration = new QGroupBox("Spotlight concentration");
    QGridLayout* groupConcentrationLayout = new QGridLayout();
    groupConcentrationLayout->addWidget(new QLabel("= 0.0: Light spreads into all directions"), 0, 0);
    groupConcentrationLayout->addWidget(new QLabel("= 1.0: Light is bundled"), 1, 0);
    QLabel *concLabel = new QLabel("concentration = ");
    concLabel->setAlignment(Qt::AlignRight);
    groupConcentrationLayout->addWidget(concLabel, 2, 0);
    groupConcentrationLayout->addWidget(concentration, 2, 1);
    groupConcentration->setLayout(groupConcentrationLayout);

    // create Ok and Cancel buttons and connect them
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Cancel|QDialogButtonBox::Ok,
        Qt::Horizontal
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TIGLCreatorAddSpotlightDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TIGLCreatorAddSpotlightDialog::reject);

    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->addWidget(groupConcentration, 0, Qt::AlignCenter);
    verticalLayout->addWidget(buttonBox);
    verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
}

tigl::CTiglPoint TIGLCreatorAddSpotlightDialog::getPosition() const
{
    tigl::CTiglPoint point(0,0,0);
    point.x = position_x->value();
    point.y = position_y->value();
    point.z = position_z->value();

    return point;
}

tigl::CTiglPoint TIGLCreatorAddSpotlightDialog::getDirection() const
{
    tigl::CTiglPoint direction(0,0,0);
    direction.x = dx->value();
    direction.y = dy->value();
    direction.z = dz->value();

    return direction;
}

double TIGLCreatorAddSpotlightDialog::getConcentration() const
{
    double value = concentration->text().toDouble();

    // Probably not needed anymore
    // Intention of this part is to round the user input to some defined decimal places
    /*if(auto *validator = qobject_cast<const QDoubleValidator*>(concentration->validator())) {
        int maxDecimals = validator->decimals();

        double factorRounding = std::pow(10., maxDecimals);
        value = std::round(value * factorRounding) / factorRounding;
    }*/
    return value;
}
