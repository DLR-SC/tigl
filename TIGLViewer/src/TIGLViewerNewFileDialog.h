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


#ifndef TIGLVIEWERNEWFILEDIALOG_H
#define TIGLVIEWERNEWFILEDIALOG_H

#include <QDialog>
#include <QDir>
#include <QStringListModel>
#include <QFile>

namespace Ui
{
class TIGLViewerNewFileDialog;
}

/**
 * @brief This Dialog is used to create a new file based on a template.
 *
 * When the dialog is executed, we first show every available template.
 * (The available templates are simply the cpacs files contained in the
 * template directory of the TIGLViewerSettings class.)
 * Then, when the user selects a template, the content of the template file
 * is copied into a string and opened with TIXI. The dialog is closed.
 * Once the dialog is closed, the template filename can be returned calling
 * the "getNewFileName" function. If the user rejects the dialog or an
 * error occurs during the creation of the file, the filename will be "".
 *
 */
class TIGLViewerNewFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TIGLViewerNewFileDialog(QWidget* parent = nullptr);
    ~TIGLViewerNewFileDialog();

    QString getNewFileName();

private slots:

    void accept() override;

protected:
    void populate();

private:
    Ui::TIGLViewerNewFileDialog* ui;
    QStringListModel templateListModel;
    QString newCPACSFileName;
};

#endif // TIGLVIEWERNEWFILEDIALOG_H
