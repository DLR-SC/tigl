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

#ifndef TIGLCREATORSETTINGSDIALOG_H_
#define TIGLCREATORSETTINGSDIALOG_H_

#include "ui_TIGLCreatorSettingsDialog.h"
#include "tigl_internal.h"

#include <Graphic3d_NameOfMaterial.hxx>
#include <QColor>

class TIGLCreatorSettingsDialog : public QDialog, private Ui::TIGLCreatorSettingsDialog
{
    Q_OBJECT

public:
    explicit TIGLCreatorSettingsDialog(class TIGLCreatorSettings&, class QWidget *parent=nullptr);

    ~TIGLCreatorSettingsDialog() override = default;

signals:
    void settingsUpdated();

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

    class TIGLCreatorSettings& _settings;
    QColor _bgcolor;
    QColor _shapecolor;
    QColor _shapesymmetrycolor;
    Graphic3d_NameOfMaterial _material;
};

#endif /* TIGLCREATORSETTINGSDIALOG_H_ */
