/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 * Author: Malo Drougard
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

#ifndef SYMMETRYCOMBOBOXWIDGET_H
#define SYMMETRYCOMBOBOXWIDGET_H

#include <QWidget>
#include "tigl.h"

namespace Ui
{
class SymmetryComboBoxWidget;
}

class SymmetryComboBoxWidget : public QWidget
{
    Q_OBJECT

public slots:

    void setGUIFromInternal();
    void setInternalFromGUI();

public:
    explicit SymmetryComboBoxWidget(QWidget* parent = nullptr);
    ~SymmetryComboBoxWidget();

    void setInternal(TiglSymmetryAxis symmetryAxis);

    TiglSymmetryAxis getInternalSymmetry();

    bool hasChanged();

private:
    QString TiglSymmetryAxisToQString(TiglSymmetryAxis symmetryAxis);
    TiglSymmetryAxis QStringToTiglAxis(QString symmetry);

    Ui::SymmetryComboBoxWidget* ui;

    TiglSymmetryAxis internalSymmetry;
};

#endif // SYMMETRYCOMBOBOXWIDGET_H
