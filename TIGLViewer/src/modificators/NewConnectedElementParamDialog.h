/*
 * Copyright (C) 2025 German Aerospace Center (DLR/SC)
 *
 * Created: 2025 Sven Goldberg <sven.goldberg@dlr.com>
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

#ifndef NEWCONNECTEDELEMENTPARAMDIALOG_H
#define NEWCONNECTEDELEMENTPARAMDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui
{
class NewConnectedElementParamDialog;
}

class NewConnectedElementParamDialog : public QDialog
{
    Q_OBJECT

public:

    explicit NewConnectedElementParamDialog(QWidget* parent = nullptr);
    ~NewConnectedElementParamDialog();

    double getParam();

private:
    Ui::NewConnectedElementParamDialog* ui;
};

#endif // NEWCONNECTEDELEMENTPARAMDIALOG_H
