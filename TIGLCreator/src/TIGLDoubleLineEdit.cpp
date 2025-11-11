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

#include <TIGLDoubleLineEdit.h>
#include <QDoubleValidator>
#include <QString>
#include "tiglcommonfunctions.h"
#include <QKeyEvent>

TiGLDoubleLineEdit::TiGLDoubleLineEdit(QWidget* parent)
    : QLabel(parent)
    , isEditMode(false)
    , isTransitioningFocus(false) // Initialize the flag
    , current_value("0.000")
    , full_precision_value(0.0)
    , min_value(-std::numeric_limits<double>::max())
    , max_value(std::numeric_limits<double>::max())
{
    setText(current_value); // Initialize with default value
    setAlignment(Qt::AlignRight); // Align text to the right for better readability
    setFocusPolicy(Qt::StrongFocus); // Enable focus for tab navigation
    setStyleSheet(
        "QLabel { "
        "  border: 1px solid #76797C; " // Subtle border to indicate interactivity
        "  background-color: #404244; "  // dark background color
        "  border-radius: 3px; "         // Rounded corners for a modern look
        "  padding: 2px; "               // Add some padding for better readability
        "}"
    );

    // Default validator with no range restrictions
    validator = new QDoubleValidator(min_value, max_value, 15, this);
}

void TiGLDoubleLineEdit::setValue(double value)
{
    full_precision_value = Clamp(value, min_value, max_value);
    current_value = QString::number(value, 'f', 3); // Format to 3 decimal places
    setText(current_value); // Update QLabel text
}

void TiGLDoubleLineEdit::setRange(double min, double max)
{
    min_value = min;
    max_value = max;

    // Update the validator with the new range
    validator->setBottom(min_value);
    validator->setTop(max_value);
}

void TiGLDoubleLineEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
    QLabel::mouseDoubleClickEvent(event);
    enterEditMode();
}

void TiGLDoubleLineEdit::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (isEditMode) {
            enterViewMode();
        } else {
            enterEditMode();
        }
    } else if (event->key() == Qt::Key_Escape) {
        if (isEditMode) {
            cancelEditMode(); // Reset value and exit edit mode
        }
    } else {
        QLabel::keyPressEvent(event);
    }
}

void TiGLDoubleLineEdit::focusInEvent(QFocusEvent* event)
{
    QLabel::focusInEvent(event); // Call the base class implementation
    if (!isEditMode && !isTransitioningFocus) {
        enterEditMode(); // Enter edit mode when the widget gains focus
    }
}

void TiGLDoubleLineEdit::focusOutEvent(QFocusEvent* event)
{
    QLabel::focusOutEvent(event);
    if (isEditMode && !isTransitioningFocus) {
        enterViewMode(); // Exit edit mode when the widget loses focus
    }
}

void TiGLDoubleLineEdit::paintEvent(QPaintEvent* event)
{
    // Suppress QLabel painting while in edit mode
    if (!isEditMode) {
        QLabel::paintEvent(event);
    }
}

void TiGLDoubleLineEdit::enterEditMode()
{
    if (!isEditMode) {
        isEditMode = true;
        isTransitioningFocus = true; // Set the flag to prevent focusOutEvent interference

        // Create a QLineEdit for editing
        editor = new QLineEdit(this);
        editor->setValidator(validator);
        editor->setText(QString::number(full_precision_value, 'g', 15)); // Use full precision value
        editor->selectAll(); // Select all text for editing
        editor->setGeometry(this->rect()); // Match QLabel's geometry

        // Adjust font size and margins for better readability
        QFont font = this->font();
        editor->setFont(font);
        editor->setStyleSheet(
            "QLineEdit { "
            "  border: 1px solid #404244; " // Blue border to indicate active editing
            "  background-color: #222222; " // White background for editing
            "  border-radius: 3px; "     // Rounded corners for consistency
            "  padding: 2px; "           // Add some padding for better readability
            "}"
        );

        editor->show();
        editor->setFocus(); // Set focus to the editor

        // Connect signals to handle editing completion
        connect(editor, &QLineEdit::editingFinished, this, &TiGLDoubleLineEdit::enterViewMode);

        isTransitioningFocus = false; // Reset the flag after focus transition
    }
}

void TiGLDoubleLineEdit::enterViewMode()
{
    if (isEditMode) {
        isEditMode = false;

        // Retrieve the value from the editor
        if (editor) {
            bool ok;
            double value = editor->text().toDouble(&ok);
            if (ok) {
                value = Clamp(value, min_value, max_value);
                full_precision_value = value; // Store the full precision value
                current_value = QString::number(value, 'f', 3); // Format to 3 decimal places for display
            } 
            editor->deleteLater(); // Clean up the editor
            editor = nullptr;
        }

        setText(current_value); // Update QLabel text
    }
}

void TiGLDoubleLineEdit::cancelEditMode()
{
    if (isEditMode) {
        isEditMode = false;

        // Clean up the editor without saving changes
        if (editor) {
            editor->deleteLater();
            editor = nullptr;
        }

        // Restore the QLabel text
        setText(current_value);
    }
}