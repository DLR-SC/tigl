/*
 * Copyright (C) 2018 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef MODIFICATORFUSELAGEWIDGET_H
#define MODIFICATORFUSELAGEWIDGET_H

#include "ModificatorWidget.h"
#include "CCPACSFuselage.h"

namespace Ui
{
class ModificatorFuselageWidget;
}

class ModificatorFuselageWidget : public ModificatorWidget
{
    Q_OBJECT

public slots:

    void recomputeProperties();

    // Recompute the height and area based on the new width value
    void setWidth(double newWidth);
    // Recompute the widht and area based on the new height value
    void setHeight(double newHeight);
    // Recompute the height and width based on the new area value
    void setArea(double newArea);

public:
    explicit ModificatorFuselageWidget(QWidget* parent = nullptr);
    ~ModificatorFuselageWidget();

    void setFuselage(tigl::CCPACSFuselage& fuselage);

    bool apply() override;

    void reset() override;

private:
    Ui::ModificatorFuselageWidget* ui;

    tigl::CCPACSFuselage* fuselage;

    double internalLength;

    // to keep track of which parameter was modified at lasg
    QString lastModifiedDimensionalParameter;

    double internalWidth;
    double internalHeight;
    double internalArea;
};

#endif // MODIFICATORFUSELAGEWIDGET_H
