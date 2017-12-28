/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-06-23 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef TIGLVIEWERVTKEXPORTDIALOG_H
#define TIGLVIEWERVTKEXPORTDIALOG_H

#include "tigl_internal.h"
#include <QDialog>

namespace Ui {
class TIGLViewerVTKExportDialog;
}

class TIGLViewerVTKExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TIGLViewerVTKExportDialog(QWidget *parent = 0);

    void setDeflection(double d);
    double getDeflection() const;

    void setNormalsEnabled(bool enabled);
    bool normalsEnabled() const;

    ~TIGLViewerVTKExportDialog() OVERRIDE;

private slots:
    void onOkayPressed() const;

private:
    Ui::TIGLViewerVTKExportDialog *ui;
};

#endif // TIGLVIEWERVTKEXPORTDIALOG_H
