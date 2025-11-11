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

#include <QLabel>
#include <QLineEdit>

class QKeyEvent;
class QDoubleValidator;

class TiGLDoubleLineEdit : public QLabel
{
    Q_OBJECT
public:
    explicit TiGLDoubleLineEdit(QWidget *parent = nullptr);

    void setValue(double value);

    void setRange(double min, double max);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;

    void focusInEvent(QFocusEvent* event) override;

    void focusOutEvent(QFocusEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

private:
    void enterEditMode();

    void enterViewMode();

    void cancelEditMode();

private:
    bool isEditMode;
    bool isTransitioningFocus; // Flag to prevent focusOutEvent interference
    QString current_value; // Rounded value for display
    double full_precision_value; // Full precision value for internal use
    double min_value; // Minimum allowed value
    double max_value; // Maximum allowed value
    QLineEdit* editor = nullptr; // Editor widget for editing mode
    QDoubleValidator* validator; // Validator for range validation
};



#endif // TIGLDOUBLELINEEDIT_H
