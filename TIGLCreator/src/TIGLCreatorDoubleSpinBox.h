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

#ifndef TIGLCREATORDOUBLESPINBOX_H
#define TIGLCREATORDOUBLESPINBOX_H

#include <QDoubleSpinBox>

/**
 * @brief A QDoubleSpinBox with a more user-friendly behavior:
 *
 *  - The mouse wheel never changes the value. The wheel event is passed
 *    to the parent instead (e.g. to scroll a surrounding scroll area).
 *  - The box has two modes:
 *      - display mode: the value is shown rounded to
 *        displayDecimals() decimals (default: 4),
 *      - edit mode: the value is shown with the full stored precision
 *        of decimals() decimals (default: 12).
 *    Edit mode is entered, when the user navigates into the box (mouse click,
 *    Tab, shortcut) or starts typing, and left, when the box loses the focus.
 *    In both modes, trailing zeros are omitted, but at least one decimal
 *    is shown (e.g. 0.0).
 *  - Only plain numbers can be typed: an optional minus sign, digits and one
 *    decimal separator ('.' or the one of the locale, e.g. ',').
 *  - No more digits can be typed than are stored, i.e. at most decimals()
 *    decimals and 15 significant digits (the precision of a double).
 *  - Input that moves the value beyond the range is rejected
 *    (e.g. a minus sign or 2 for the range [0, 1]).
 *
 * Note: Unlike for QDoubleSpinBox, decimals() and setDecimals() only refer to
 * the stored precision, not to the shown one (see setDisplayDecimals()).
 * Prefix and suffix are not supported.
 */
class TIGLCreatorDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY(int displayDecimals READ displayDecimals WRITE setDisplayDecimals)

public:
    explicit TIGLCreatorDoubleSpinBox(QWidget* parent = nullptr);

    /// Returns the number of decimals the value is rounded to in display mode
    int displayDecimals() const;

    /// Sets the number of decimals the value is rounded to in display mode.
    /// The stored value is not changed.
    void setDisplayDecimals(int decimals);

    QValidator::State validate(QString& input, int& pos) const override;
    double valueFromText(const QString& text) const override;
    QString textFromValue(double value) const override;
    void fixup(QString& input) const override;

    QSize sizeHint() const override;

protected:
    void wheelEvent(QWheelEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QString normalized(const QString& text) const;
    QString formatValue(double value, int decimals) const;
    void enterEditMode();

    int m_displayDecimals{4}; // default number of decimals shown in display mode
    bool m_editMode{false};
};

#endif // TIGLCREATORDOUBLESPINBOX_H
