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

#ifndef TIGLCREATORSPINBOX_H
#define TIGLCREATORSPINBOX_H

#include <QSpinBox>

/**
 * @brief A QSpinBox, whose value is never changed by the mouse wheel.
 * The wheel event is passed to the parent instead (e.g. to scroll a surrounding
 * scroll area). This is the integer counterpart of TIGLCreatorDoubleSpinBox.
 */
class TIGLCreatorSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit TIGLCreatorSpinBox(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;
};

#endif // TIGLCREATORSPINBOX_H
