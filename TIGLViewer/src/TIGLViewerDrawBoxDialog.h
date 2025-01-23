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

#ifndef TIGLVIEWERDRAWBOXDIALOG_H
#define TIGLVIEWERDRAWBOXDIALOG_H

#include <QDialog>
#include <CTiglPoint.h>


class QDoubleSpinBox;

class TIGLViewerDrawBoxDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TIGLViewerDrawBoxDialog(const QString& title = "Draw Vector", QWidget *parent = 0);

    tigl::CTiglPoint getPoint() const;
    tigl::CTiglPoint getDirection() const;
    
private:
    QDoubleSpinBox* origin_x;
    QDoubleSpinBox* origin_y;
    QDoubleSpinBox* origin_z;
    QDoubleSpinBox* dx;
    QDoubleSpinBox* dy;
    QDoubleSpinBox* dz;
};

#endif // TIGLVIEWERDRAWBOXDIALOG_H
