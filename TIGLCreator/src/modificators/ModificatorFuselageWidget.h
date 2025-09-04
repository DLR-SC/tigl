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

#include "modificators/ModificatorWidget.h"
#include "CCPACSFuselage.h"
#include "ProfilesDBManager.h"

namespace Ui
{
class ModificatorFuselageWidget;
}

class ModificatorFuselageWidget : public ModificatorWidget
{
    Q_OBJECT

public:
    explicit ModificatorFuselageWidget(QWidget* parent = nullptr);
    ~ModificatorFuselageWidget();

    void setFuselage(tigl::CCPACSFuselage& fuselage, ProfilesDBManager* profilesDB);

    bool apply() override;

    void reset() override;

private:
    Ui::ModificatorFuselageWidget* ui;

    tigl::CCPACSFuselage* fuselage;
    ProfilesDBManager* profilesDB;

    double internalLength;
    double internalWidth;
    double internalHeight;
    QString internalProfile;

};

#endif // MODIFICATORFUSELAGEWIDGET_H
