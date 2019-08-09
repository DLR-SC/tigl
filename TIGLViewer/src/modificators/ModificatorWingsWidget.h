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

#ifndef MODIFICATORWINGSWIDGET_H
#define MODIFICATORWINGSWIDGET_H

#include <QWidget>
#include "CCPACSWings.h"
#include "ModificatorWidget.h"

namespace Ui
{
class ModificatorWingsWidget;
}

class ModificatorWingsWidget : public ModificatorWidget
{
    Q_OBJECT

signals:
    void undoCommandRequired();

public slots:

    void execNewWingDialog();

public:
    explicit ModificatorWingsWidget(QWidget *parent = nullptr);
    ~ModificatorWingsWidget();

    void setWings(tigl::CCPACSWings& wings, QStringList profilesUID);

private:
    Ui::ModificatorWingsWidget *ui;
    tigl::CCPACSWings* wings;
    QStringList profilesUID;
};

#endif // MODIFICATORWINGSWIDGET_H
