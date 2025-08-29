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

#ifndef MODIFICATORFUSELAGESWIDGET_H
#define MODIFICATORFUSELAGESWIDGET_H

#include <QWidget>
#include "ModificatorWidget.h"
#include "CCPACSFuselages.h"
#include <QPushButton>
#include "ProfilesDBManager.h"

namespace Ui
{
class ModificatorFuselagesWidget;
}

class ModificatorFuselagesWidget : public ModificatorWidget
{
    Q_OBJECT

signals:
    void undoCommandRequired();

public slots:

    void execNewFuselageDialog();
    void execDeleteFuselageDialog();

public:
    explicit ModificatorFuselagesWidget(QWidget* parent = nullptr);
    ~ModificatorFuselagesWidget();

    void setFuselages(tigl::CCPACSFuselages& fuselages, ProfilesDBManager* profilesUID);

private:
    Ui::ModificatorFuselagesWidget* ui;
    tigl::CCPACSFuselages* fuselages;
    ProfilesDBManager* profilesDB;
};

#endif // MODIFICATORFUSELAGESWIDGET_H
