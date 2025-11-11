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

#ifndef TIGLCREATORADDSPOTLIGHTDIALOG_H
#define TIGLCREATORADDSPOTLIGHTDIALOG_H

#include <QDialog>
#include <CTiglPoint.h>
#include <QDoubleSpinBox>
#include <QLabel>
#include <TIGLDoubleLineEdit.h>


class TIGLCreatorAddSpotlightDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TIGLCreatorAddSpotlightDialog(QWidget *parent = 0);

    tigl::CTiglPoint getPosition() const;
    tigl::CTiglPoint getDirection() const;
    double getConcentration() const;

private:
    QDoubleSpinBox* position_x;
    QDoubleSpinBox* position_y;
    QDoubleSpinBox* position_z;
    QDoubleSpinBox* dx;
    QDoubleSpinBox* dy;
    QDoubleSpinBox* dz;
    TiGLDoubleLineEdit* concentration;
};

#endif // TIGLCREATORADDSPOTLIGHTDIALOG_H
