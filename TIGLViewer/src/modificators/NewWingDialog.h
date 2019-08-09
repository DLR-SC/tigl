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

#ifndef NEWWINGDIALOG_H
#define NEWWINGDIALOG_H

#include <QDialog>

namespace Ui
{
class NewWingDialog;
}

class NewWingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewWingDialog(QStringList profileUIDs, QWidget *parent = nullptr);
    ~NewWingDialog();


    int getNbSection() const;
    QString getUID() const;
    QString getProfileUID() const;


private:
    Ui::NewWingDialog *ui;
};

#endif // NEWWINGDIALOG_H
