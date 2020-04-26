/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-06-29 Martin Siggel <Martin.Siggel@dlr.de>
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


#ifndef TIGLSLIDERDIALOG_H
#define TIGLSLIDERDIALOG_H

#include "tigl_internal.h"
#include <QDialog>

namespace Ui {
class TIGLSliderDialog;
}

class TIGLSliderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TIGLSliderDialog(QWidget *parent = 0);
    ~TIGLSliderDialog() override;

    void setIntValue(int value);
    void setIntRange(int min, int max);

signals:
    void intValueChanged(int);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::TIGLSliderDialog *ui;
};

#endif // TIGLSLIDERDIALOG_H
