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

#ifndef TIGLDOUBLEVALIDATOR_H
#define TIGLDOUBLEVALIDATOR_H

#include <QDoubleValidator>
#include <QString>


class TIGLDoubleValidator : public QDoubleValidator
{
    Q_OBJECT

public:
    TIGLDoubleValidator(double bottom, double top, int decimals, QObject *parent = nullptr);

    // Defines valid status of String inputs
    State validate(QString &input, int &pos) const override;
};

#endif // TIGLDOUBLEVALIDATOR_H
