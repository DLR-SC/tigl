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

#include <TIGLViewerSettings.h>
#include <QSettings>

TIGLViewerSettings::TIGLViewerSettings() {
	_tesselationAccuracy = 0.0001;
	_triangulationAccuracy = 0.0001;
	_bgcolor = QColor(255,235,163);
}

void TIGLViewerSettings::setTesselationAccuracy(double accu){
	_tesselationAccuracy = accu;
}

void TIGLViewerSettings::setTriangulationAccuracy(double accu){
	_triangulationAccuracy = accu;
}

void TIGLViewerSettings::setBGColor(const QColor& col) {
	_bgcolor = col;
}

double TIGLViewerSettings::tesselationAccuracy() const{
	return _tesselationAccuracy;
}

double TIGLViewerSettings::triangulationAccuracy() const{
	return _triangulationAccuracy;
}

const QColor& TIGLViewerSettings::BGColor() const{
	return _bgcolor;
}

void TIGLViewerSettings::loadSettings(){
    QSettings settings("DLR SC-VK","TIGLViewer");

    _tesselationAccuracy   = settings.value("tesselation_accuracy"  , tesselationAccuracy()).toDouble();
    _triangulationAccuracy = settings.value("triangulation_accuracy", triangulationAccuracy()).toDouble();
    _bgcolor = settings.value("background_color", BGColor()).value<QColor>();
}

void TIGLViewerSettings::storeSettings(){
    QSettings settings("DLR SC-VK","TIGLViewer");

    settings.setValue("tesselation_accuracy"  , tesselationAccuracy());
    settings.setValue("triangulation_accuracy", triangulationAccuracy());
    settings.setValue("background_color", BGColor());
}

TIGLViewerSettings::~TIGLViewerSettings() {
}
