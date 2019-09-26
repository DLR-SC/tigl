/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef MODIFICATORPOSITIONINGWIDGET_H
#define MODIFICATORPOSITIONINGWIDGET_H

#include <QWidget>
#include "ModificatorWidget.h"
#include "CCPACSPositioning.h"
#include "CCPACSWing.h"
#include "CCPACSFuselage.h"

namespace Ui
{
class ModificatorPositioningWidget;
}

class ModificatorPositioningWidget : public ModificatorWidget
{
    Q_OBJECT

public:
    explicit ModificatorPositioningWidget(QWidget* parent = nullptr);
    ~ModificatorPositioningWidget();

    void setPositioning(tigl::CCPACSFuselage& fuselage, tigl::CCPACSPositioning& pos);
    void setPositioning(tigl::CCPACSWing& wing, tigl::CCPACSPositioning& pos);

    bool apply() override;
    void reset() override;

private:
    void setPositioning(tigl::CCPACSPositioning& pos);
    QStringList getSectionUIDsList();

    Ui::ModificatorPositioningWidget* ui;

    tigl::CCPACSPositioning* positioning;
    // dirty trick to get the wing or fuselage invalidate, because for the moment in tigl is not possible to tell
    // the wing or fuselage that need to be rebuilded after we set the positioning
    tigl::CCPACSFuselage* associatedFuselage;
    tigl::CCPACSWing* associatedWing;
};

#endif // MODIFICATORPOSITIONINGWIDGET_H
