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

#include <TIGLViewerSettingsDialog.h>
#include <TIGLViewerSettings.h>
#include <cmath>
#include <iostream>

#define WORST_TESSELATION 0.01
#define BEST_TESSELATION 0.00001

#define WORST_TRIANGULATION 0.01
#define BEST_TRIANGULATION 0.00005


TIGLViewerSettingsDialog::TIGLViewerSettingsDialog(TIGLViewerSettings& settings, QWidget *parent)
: _settings(settings), QDialog(parent) {
	setupUi(this);

	tessAccuEdit->setText (QString("%1").arg(sliderTesselationAccuracy->value()));
	trianAccuEdit->setText(QString("%1").arg(sliderTriangulationAccuracy->value()));

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSettingsAccepted()));
	connect(sliderTesselationAccuracy,   SIGNAL(valueChanged(int)), this, SLOT(onSliderTesselationChanged(int)));
	connect(sliderTriangulationAccuracy, SIGNAL(valueChanged(int)), this, SLOT(onSliderTriangulationChanged(int)));
}

double TIGLViewerSettingsDialog::calcTesselationAccu(int value){
	// calculate tesselation accuracy, slider range is from 0 ... 10
	// we map it onto 0.1 to 0.00001 - logarithmic slider
	double dmax = WORST_TESSELATION, dmin = BEST_TESSELATION;
	int imin = sliderTesselationAccuracy->minimum(), imax = sliderTesselationAccuracy->maximum();

	double mu = log(dmax/dmin)/double(imax-imin);
	double c  = dmax / exp(-mu * (double)imin);

	return c * exp(-mu * (double)value);
}

double TIGLViewerSettingsDialog::calcTriangulationAccu(int value){
	// calculate triangulation accuracy, slider range is from 0 ... 10
	// we map it onto 0.1 to 0.00001 - logarithmic slider
	double dmax = WORST_TRIANGULATION, dmin = BEST_TRIANGULATION;
	int imin = sliderTriangulationAccuracy->minimum(), imax = sliderTriangulationAccuracy->maximum();

	double mu = log(dmax/dmin)/double(imax-imin);
	double c  = dmax / exp(-mu * (double)imin);

	return c * exp(-mu * (double)value);
}


void TIGLViewerSettingsDialog::onSettingsAccepted(){
	_settings.setTesselationAccuracy(calcTesselationAccu(sliderTesselationAccuracy->value()));
	_settings.setTriangulationAccuracy(calcTriangulationAccu(sliderTriangulationAccuracy->value()));
}

void TIGLViewerSettingsDialog::updateEntries(){
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
}

void TIGLViewerSettingsDialog::onSliderTesselationChanged(int val){
	tessAccuEdit->setText(QString("%1").arg(val));
}

void TIGLViewerSettingsDialog::onSliderTriangulationChanged(int val){
	trianAccuEdit->setText(QString("%1").arg(val));
}

TIGLViewerSettingsDialog::~TIGLViewerSettingsDialog() {
}
