/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef TIGLVIEWERERRORDIALOG_H
#define TIGLVIEWERERRORDIALOG_H

#include "tigl_internal.h"
#include <QDialog>
#include <QPushButton>

namespace Ui
{
    class TIGLViewerErrorDialog;
}

class TIGLViewerErrorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TIGLViewerErrorDialog(QWidget *parent = 0);

    void setMessage(const QString& msg);
    void setDetailsText(const QString& msg);
    //void setHistoryLog(ITiglLogger*);
    ~TIGLViewerErrorDialog() OVERRIDE;

private slots:
    void toggleDetails();
    void setDetailsVisible(bool);
    void readjustSize();

private:
    Ui::TIGLViewerErrorDialog *ui;
    QPushButton* detailsButton;
    bool detailsVisible;
};

#endif // TIGLVIEWERERRORDIALOG_H
