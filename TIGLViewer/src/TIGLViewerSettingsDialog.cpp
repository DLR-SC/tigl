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

#include <QDialog>

#include <QColorDialog>

#include <TIGLViewerSettingsDialog.h>
#include <TIGLViewerSettings.h>
#include <cmath>
#include <iostream>

#include <QFileDialog>
#include <QMessageBox>
#include "TIGLViewerSettingsDialog.h"


#define WORST_TESSELATION 0.01
#define BEST_TESSELATION 0.00001

#define WORST_TRIANGULATION 0.01
#define BEST_TRIANGULATION 0.00005

#define BTN_STYLE "#buttonColorChoser {background-color: %1; color: black; border: 1px solid black; border-radius: 5px;} #buttonColorChoser:hover {border: 1px solid white;}"

TIGLViewerSettingsDialog::TIGLViewerSettingsDialog(TIGLViewerSettings& settings, QWidget *parent)
    : QDialog(parent), _settings(settings)
{
    setupUi(this);

    tessAccuEdit->setText (QString("%1").arg(sliderTesselationAccuracy->value()));
    trianAccuEdit->setText(QString("%1").arg(sliderTriangulationAccuracy->value()));
    settingsList->item(0)->setSelected(true);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSettingsAccepted()));
    connect(sliderTesselationAccuracy,   SIGNAL(valueChanged(int)), this, SLOT(onSliderTesselationChanged(int)));
    connect(sliderTriangulationAccuracy, SIGNAL(valueChanged(int)), this, SLOT(onSliderTriangulationChanged(int)));
    connect(buttonColorChoser, SIGNAL(clicked()), this, SLOT(onColorChoserPushed()));
    connect(settingsList, SIGNAL(currentRowChanged(int)), this, SLOT(onSettingsListChanged(int)));
    connect(btnRestoreDefaults, SIGNAL(clicked(bool)), this, SLOT(restoreDefaults()));
    connect(browseTemplateDirButton, SIGNAL(clicked(bool)), this, SLOT(onBrowseTemplateDir()));
    connect(browseProfilesDBButton, SIGNAL(clicked(bool)), this, SLOT(onBrowseProfilesDB()));
}

double TIGLViewerSettingsDialog::calcTesselationAccu(int value)
{
    // calculate tesselation accuracy, slider range is from 0 ... 10
    // we map it onto 0.1 to 0.00001 - logarithmic slider
    double dmax = WORST_TESSELATION, dmin = BEST_TESSELATION;
    int imin = sliderTesselationAccuracy->minimum(), imax = sliderTesselationAccuracy->maximum();

    double mu = log(dmax/dmin)/double(imax-imin);
    double c  = dmax / exp(-mu * (double)imin);

    return c * exp(-mu * (double)value);
}

double TIGLViewerSettingsDialog::calcTriangulationAccu(int value)
{
    // calculate triangulation accuracy, slider range is from 0 ... 10
    // we map it onto 0.1 to 0.00001 - logarithmic slider
    double dmax = WORST_TRIANGULATION, dmin = BEST_TRIANGULATION;
    int imin = sliderTriangulationAccuracy->minimum(), imax = sliderTriangulationAccuracy->maximum();

    double mu = log(dmax/dmin)/double(imax-imin);
    double c  = dmax / exp(-mu * (double)imin);

    return c * exp(-mu * (double)value);
}


void TIGLViewerSettingsDialog::onSettingsAccepted()
{
    _settings.setTesselationAccuracy(calcTesselationAccu(sliderTesselationAccuracy->value()));
    _settings.setTriangulationAccuracy(calcTriangulationAccu(sliderTriangulationAccuracy->value()));
    _settings.setBGColor(_bgcolor);
    
    _settings.setDebugBooleanOperationsEnabled(debugBopCB->isChecked());
    _settings.setEnumerateFacesEnabled(enumerateFaceCB->isChecked());
    _settings.setNumberOfUIsolinesPerFace(numUIsoLinesSB->value());
    _settings.setNumberOfVIsolinesPerFace(numVIsoLinesSB->value());

    _settings.setTemplateDir(templateLineEdit->text());
    _settings.setProfilesDBPath(profilesDBLineEdit->text());
}

void TIGLViewerSettingsDialog::updateEntries()
{
    // calculate tesselation accuracy, slider range is from 0 ... 10
    // we map it onto 0.1 to 0.00001 - logarithmic slider
    double dmax = WORST_TESSELATION, dmin = BEST_TESSELATION;
    int imin = sliderTesselationAccuracy->minimum(), imax = sliderTesselationAccuracy->maximum();

    double mu = log(dmax/dmin)/double(imax-imin);
    double c  = dmax / exp(-mu * (double)imin);

    int tessVal = int (log(c/_settings.tesselationAccuracy())/mu);
    sliderTesselationAccuracy->setValue(tessVal);

    dmax = WORST_TRIANGULATION, dmin = BEST_TRIANGULATION;
    imin = sliderTriangulationAccuracy->minimum(), imax = sliderTriangulationAccuracy->maximum();

    mu = log(dmax/dmin)/double(imax-imin);
    c  = dmax / exp(-mu * (double)imin);

    int triaVal = int (log(c/_settings.triangulationAccuracy())/mu);
    sliderTriangulationAccuracy->setValue(triaVal);

    _bgcolor = _settings.BGColor();
    updateBGColorButton();

    debugBopCB->setChecked(_settings.debugBooleanOperations());
    enumerateFaceCB->setChecked(_settings.enumerateFaces());
    numUIsoLinesSB->setValue(_settings.numFaceUIsosForDisplay());
    numVIsoLinesSB->setValue(_settings.numFaceVIsosForDisplay());

    templateLineEdit->setText(_settings.templateDir().absolutePath());
    profilesDBLineEdit->setText(_settings.profilesDBPath());
}

void TIGLViewerSettingsDialog::onSliderTesselationChanged(int val)
{
    tessAccuEdit->setText(QString("%1").arg(val));
}

void TIGLViewerSettingsDialog::onSliderTriangulationChanged(int val)
{
    trianAccuEdit->setText(QString("%1").arg(val));
}

void TIGLViewerSettingsDialog::onColorChoserPushed()
{
    QColor col = QColorDialog::getColor(_bgcolor, this);
    if (col.isValid()) {
        _bgcolor = col;
        updateBGColorButton();
    }
}

void TIGLViewerSettingsDialog::updateBGColorButton()
{
    QString qss = QString(BTN_STYLE).arg(_bgcolor.name());
    buttonColorChoser->setStyleSheet(qss);
}

void TIGLViewerSettingsDialog::onSettingsListChanged(int index)
{
    stackedWidget->setCurrentIndex(index);
}

void TIGLViewerSettingsDialog::restoreDefaults()
{
    _settings.restoreDefaults();
    updateEntries();
}

TIGLViewerSettingsDialog::~TIGLViewerSettingsDialog() {}

void TIGLViewerSettingsDialog::onBrowseTemplateDir()
{
    QDir newDir = QFileDialog::getExistingDirectory(this, "Choose template directory", _settings.templateDir().path());
    templateLineEdit->setText(newDir.absolutePath());
}

void TIGLViewerSettingsDialog::onBrowseProfilesDB()
{
    QString newFile =
        QFileDialog::getOpenFileName(this, "Choose a profile DB file. Remark, the profile DB file need to have the same "
                                           "structure as a CPACS \"profiles\" section and have .xml suffix.");
    profilesDBLineEdit->setText(newFile);
}
