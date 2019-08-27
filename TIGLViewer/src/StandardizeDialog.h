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

#ifndef STANDARDIZEDIALOG_H
#define STANDARDIZEDIALOG_H

#include "CCPACSConfiguration.h"
#include <QDialog>
#include <QString>

namespace Ui
{
class StandardizeDialog;
}

class StandardizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StandardizeDialog(tigl::CCPACSConfiguration& config, QWidget* parent = nullptr);
    ~StandardizeDialog();

    QString getSelectedUID();
    bool isSelectedUIDAWing();
    bool isSelectedUIDAFuselage();
    bool useSimpleDecomposition();

private:
    Ui::StandardizeDialog* ui;
    tigl::CCPACSConfiguration* config;
    QString wingPrefix;
    QString fuselagePrefix;
};

#endif // STANDARDIZEDIALOG_H
