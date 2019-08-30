/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-05 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef TIGLVIEWERSETTINGSDIALOG_H_
#define TIGLVIEWERSETTINGSDIALOG_H_

#include "ui_TIGLViewerSettingsDialog.h"
#include "tigl_internal.h"

#include <Graphic3d_NameOfMaterial.hxx>
#include <QColor>

class TIGLViewerSettingsDialog : public QDialog, private Ui::TIGLViewerSettingsDialog
{
    Q_OBJECT

public:
    explicit TIGLViewerSettingsDialog(class TIGLViewerSettings&, class QWidget *parent=nullptr);

    ~TIGLViewerSettingsDialog() override = default;

public slots:
    void updateEntries();

private slots:
    void onSettingsAccepted();
    void onSliderTesselationChanged(int);
    void onSliderTriangulationChanged(int);
    void onComboBoxIndexChanged(const QString&);

    void onColorChoserPushed();
    void onShapeColorChoserPushed();
    void onShapeSymmetryColorChoserPushed();

    void onSettingsListChanged(int);
    void restoreDefaults();
    void onBrowseTemplateDir();
    void onBrowseProfilesDB();

private:
    double calcTesselationAccu(int value);
    double calcTriangulationAccu(int value);
    void updateBGColorButton();
    void updateShapeColorButton();
    void updateShapeSymmetryColorButton();

    class TIGLViewerSettings& _settings;
    QColor _bgcolor;
    QColor _shapecolor;
    QColor _shapesymmetrycolor;
    Graphic3d_NameOfMaterial _material;
};

#endif /* TIGLVIEWERSETTINGSDIALOG_H_ */
