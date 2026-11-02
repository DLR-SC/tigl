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

#include "TIGLDoubleValidator.h"

TIGLDoubleValidator::TIGLDoubleValidator(double bottom, double top, int decimals, QObject *parent)
    : QDoubleValidator(bottom, top, decimals, parent)
{}

TIGLDoubleValidator::State TIGLDoubleValidator::validate(QString &input, int &pos) const {
    Q_UNUSED(pos);

    // Allow empty input and number with leading "."
    if (input.isEmpty() || input == ".") {
        return Intermediate;
    }
    // A leading "-" is only allowed if the range includes negative values
    if (bottom() < 0 && input == "-") {
        return Intermediate;
    }

    bool valueValid = false;
    double value = input.toDouble(&valueValid);

    // Input that is not convertible to a C++ double or lies outside the defined range is invalid
    if (!valueValid || value < bottom() || value > top()) {
        return Invalid;
    }

    return Intermediate;
}
