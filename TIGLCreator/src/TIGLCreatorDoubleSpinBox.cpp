/*
 * Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
 *
 * Created: 2026-10-26 Sven Goldberg <sven.goldberg@dlr.de>
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

#include "TIGLCreatorDoubleSpinBox.h"

#include <QWheelEvent>
#include <QLineEdit>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>
#include <limits>

TIGLCreatorDoubleSpinBox::TIGLCreatorDoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent)
{
    // the box can get focus by clicking or tabbing, but not by the mouse wheel
    setFocusPolicy(Qt::StrongFocus);
    // default precision of the stored value, shown in edit mode
    setDecimals(12);
    lineEdit()->installEventFilter(this);
}

int TIGLCreatorDoubleSpinBox::displayDecimals() const
{
    return m_displayDecimals;
}

void TIGLCreatorDoubleSpinBox::setDisplayDecimals(int decimals)
{
    m_displayDecimals = std::max(decimals, 0);
    // refresh the shown text and the size hint
    setValue(value());
    updateGeometry();
}

void TIGLCreatorDoubleSpinBox::focusInEvent(QFocusEvent* event)
{
    // Only switch to edit mode, if the user navigated into the box. Automatic
    // focus changes, e.g. the initial focus of a dialog, keep the display mode.
    // Edit mode is entered before the base class possibly selects the text.
    const Qt::FocusReason reason = event->reason();
    if (reason == Qt::MouseFocusReason || reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason
        || reason == Qt::ShortcutFocusReason) {
        enterEditMode();
    }
    QDoubleSpinBox::focusInEvent(event);
}

void TIGLCreatorDoubleSpinBox::focusOutEvent(QFocusEvent* event)
{
    // Stay in edit mode, if the focus is only lost temporarily, e.g. when
    // switching to another window. Otherwise leave the edit mode.
    // The base class first interprets the edited text, which is independent
    // of the mode, and then updates the shown text according to the mode.
    if (event->reason() != Qt::ActiveWindowFocusReason && event->reason() != Qt::PopupFocusReason) {
        m_editMode = false;
    }
    QDoubleSpinBox::focusOutEvent(event);
}

void TIGLCreatorDoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
    // typing into a focused box in display mode must not edit the rounded text
    enterEditMode();
    QDoubleSpinBox::keyPressEvent(event);
}

bool TIGLCreatorDoubleSpinBox::eventFilter(QObject* watched, QEvent* event)
{
    // clicks into the text of a focused box in display mode, which are delivered to the line edit
    if (watched == lineEdit()
        && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)) {
        enterEditMode();
    }
    return QDoubleSpinBox::eventFilter(watched, event);
}

void TIGLCreatorDoubleSpinBox::enterEditMode()
{
    if (m_editMode) {
        return;
    }

    QLineEdit* edit = lineEdit();
    const bool allSelected = edit->hasSelectedText() && edit->selectedText() == edit->text();
    const bool cursorAtEnd = edit->cursorPosition() == edit->text().size();

    // show the full precision
    m_editMode = true;
    setValue(value());

    // keep the selection or cursor position in the longer text
    if (allSelected) {
        selectAll();
    }
    else if (cursorAtEnd) {
        edit->end(false);
    }
}

void TIGLCreatorDoubleSpinBox::wheelEvent(QWheelEvent* event)
{
    // never change the value by scrolling, pass the event to the parent widget instead,
    // e.g. to scroll a surrounding scroll area
    event->ignore();
}

QString TIGLCreatorDoubleSpinBox::normalized(const QString& text) const
{
    // uses '.' as decimal separator, e.g. "1,5" -> "1.5"
    return text.trimmed().replace(locale().decimalPoint(), QLatin1Char('.'));
}

QValidator::State TIGLCreatorDoubleSpinBox::validate(QString& input, int& /*pos*/) const
{
    // only an optional minus sign, digits and one decimal separator, e.g. "-12.5",
    // captures the integer digits without leading zeros and the decimals
    static const QRegularExpression plainNumber(QStringLiteral("^-?0*([0-9]*)(?:\\.([0-9]*))?$"));
    const QString text = normalized(input);
    const QRegularExpressionMatch match = plainNumber.match(text);
    if (!match.hasMatch()) {
        return QValidator::Invalid;
    }

    // no more digits than can be stored, i.e. decimals() and 15 significant digits of a double
    const int numDecimals = match.capturedLength(2);
    if (numDecimals > decimals() || match.capturedLength(1) + numDecimals > std::numeric_limits<double>::digits10) {
        return QValidator::Invalid;
    }

    // a minus sign only makes sense for ranges with negative values
    if (text.startsWith(QLatin1Char('-')) && minimum() >= 0.) {
        return QValidator::Invalid;
    }

    // incomplete input, e.g. "", "-" or "."
    bool ok = false;
    const double value = text.toDouble(&ok);
    if (!ok) {
        return QValidator::Intermediate;
    }

    // Typing further digits moves the value away from zero. Hence, a value beyond the
    // bound on its side of zero can't get into the range anymore (e.g. 2 for [0, 1]),
    // but a value between zero and the range can (e.g. 1 for [5, 10] when typing 10).
    if ((value > 0. && value > maximum()) || (value < 0. && value < minimum())) {
        return QValidator::Invalid;
    }
    if (value < minimum() || value > maximum()) {
        return QValidator::Intermediate;
    }
    return QValidator::Acceptable;
}

double TIGLCreatorDoubleSpinBox::valueFromText(const QString& text) const
{
    return normalized(text).toDouble();
}

QString TIGLCreatorDoubleSpinBox::textFromValue(double value) const
{
    return formatValue(value, m_editMode ? decimals() : m_displayDecimals);
}

QString TIGLCreatorDoubleSpinBox::formatValue(double value, int decimals) const
{
    // large values get less decimals, as a double holds only 15 significant digits
    if (std::fabs(value) >= 1.) {
        const int integerDigits = static_cast<int>(std::log10(std::fabs(value))) + 1;
        decimals = std::max(0, std::min(decimals, std::numeric_limits<double>::digits10 - integerDigits));
    }

    // omit trailing zeros, but keep at least one decimal, e.g. 2.0 instead of 2.0000
    QString text = QString::number(value, 'f', decimals);
    for (; decimals > 1 && text.endsWith(QLatin1Char('0')); --decimals) {
        text.chop(1);
    }

    // avoid "-0.0" for small negative values that are rounded to zero
    if (text.toDouble() == 0.) {
        text.remove(QLatin1Char('-'));
    }
    return text.replace(QLatin1Char('.'), locale().decimalPoint());
}

void TIGLCreatorDoubleSpinBox::fixup(QString& input) const
{
    // clamp e.g. 1 for [5, 10] when leaving the box
    bool ok = false;
    const double value = normalized(input).toDouble(&ok);
    if (ok) {
        // use the full precision, independent of the mode
        input = formatValue(std::min(std::max(value, minimum()), maximum()), decimals());
    }
}

QSize TIGLCreatorDoubleSpinBox::sizeHint() const
{
    // The base class measures the texts of minimum and maximum, which have only
    // one decimal (e.g. 10000.0). Add space for the other displayed decimals.
    QSize hint = QDoubleSpinBox::sizeHint();
    hint.rwidth() += fontMetrics().boundingRect(QString(std::max(m_displayDecimals - 1, 0), QLatin1Char('0'))).width();
    return hint;
}
