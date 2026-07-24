/*
* Copyright (C) 2025 German Aerospace Center (DLR/SC)
*
* Created: 2025-11-04 Sven Goldberg <Sven.Goldberg@dlr.de>
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

#ifndef TIGLDOUBLELINEEDIT_H
#define TIGLDOUBLELINEEDIT_H

#include <QLineEdit>

class TIGLDoubleLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    TIGLDoubleLineEdit();
    TIGLDoubleLineEdit(double minValue, double maxValue, double value, int nrDecimalsPrint);

    void setRange(double minValue, double maxValue);

    void setValue(double value, int nrDecimalsPrint);

};

#endif // TIGLDOUBLELINEEDIT_H
