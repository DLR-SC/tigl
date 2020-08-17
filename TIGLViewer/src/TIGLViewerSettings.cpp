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

#include <algorithm>
#include "TIGLViewerMaterials.h"

const double DEFAULT_TESSELATION_ACCURACY = 0.000316;
const double DEFAULT_TRIANGULATION_ACCURACY = 0.00070;
const QColor DEFAULT_BGCOLOR(169,237,255);
const QColor DEFAULT_SHAPE_COLOR(0, 170 ,255, 255);
const QColor DEFAULT_SHAPE_SYMMETRY_COLOR(251, 255, 169, 255);
const bool DEFAULT_DEBUG_BOPS = false;
const bool DEFAULT_ENUM_FACES = false;
const bool DEFAULT_DRAW_FACE_BOUNDARIES = true;
const int DEFAULT_NISO_FACES = 0;


TIGLViewerSettings& TIGLViewerSettings::Instance()
{
    static TIGLViewerSettings settings;
    return settings;
}

TIGLViewerSettings::TIGLViewerSettings()
{
    restoreDefaults();
}

void TIGLViewerSettings::setTesselationAccuracy(double accu)
{
    _tesselationAccuracy = accu;
}

void TIGLViewerSettings::setTriangulationAccuracy(double accu)
{
    _triangulationAccuracy = accu;
}

void TIGLViewerSettings::setBGColor(const QColor& col)
{
    _bgcolor = col;
}

void TIGLViewerSettings::setShapeColor(const QColor& col)
{
    _shapecolor = col;
}

void TIGLViewerSettings::setShapeSymmetryColor(const QColor& col)
{
    _shapesymmetrycolor = col;
}

void TIGLViewerSettings::setDefaultMaterial(Graphic3d_NameOfMaterial material)
{
    _defaultMaterial = material;
}

Graphic3d_NameOfMaterial TIGLViewerSettings::defaultMaterial() const
{
    return _defaultMaterial;
}

void TIGLViewerSettings::setDefaultShapeColor(int r, int g, int b, int a)
{
    _shapecolor = QColor(r,g,b,a);
    for (auto & listener : _settingsListener)
    {
        listener->defaultShapeColorHasChanged();
    }
}

void TIGLViewerSettings::setDefaultShapeSymmetryColor(int r, int g, int b, int a)
{
    _shapesymmetrycolor = QColor(r,g,b,a);
    for (auto & listener : _settingsListener)
    {
        listener->defaultShapeSymmetryColorHasChanged();
    }
}

void TIGLViewerSettings::setBackgroundColor(int r, int g, int b, int a)
{
    _bgcolor = QColor(r,g,b,a);
    for (auto & listener : _settingsListener)
    {
        listener->backgroundColorHasChanged();
    }
}

void TIGLViewerSettings::setDefaultMaterial(const QString& material)
{
    _defaultMaterial = tiglMaterials::materialMap[material];
    for (auto & listener : _settingsListener)
    {
        listener->defaultMaterialHasChanged();
    }
}

double TIGLViewerSettings::tesselationAccuracy() const
{
    return _tesselationAccuracy;
}

double TIGLViewerSettings::triangulationAccuracy() const
{
    return _triangulationAccuracy;
}

const QColor& TIGLViewerSettings::BGColor() const
{
    return _bgcolor;
}

const QColor& TIGLViewerSettings::shapeColor() const
{
    return _shapecolor;
}

const QColor& TIGLViewerSettings::shapeSymmetryColor() const
{
    return _shapesymmetrycolor;
}

void TIGLViewerSettings::setDebugBooleanOperationsEnabled(bool enabled)
{
    _debugBOPs = enabled;
}

void TIGLViewerSettings::setEnumerateFacesEnabled(bool enabled)
{
    _enumFaces = enabled;
}

void TIGLViewerSettings::setNumberOfUIsolinesPerFace(int nlines)
{
    _nUIsosPerFace = nlines;
}

void TIGLViewerSettings::setNumberOfVIsolinesPerFace(int nlines)
{
    _nVIsosPerFace = nlines;
}

void TIGLViewerSettings::setDrawFaceBoundariesEnabled(bool enabled)
{
    _drawFaceBoundaries = enabled;
}

bool TIGLViewerSettings::debugBooleanOperations() const
{
    return _debugBOPs;
}

bool TIGLViewerSettings::enumerateFaces() const
{
    return _enumFaces;
}

int  TIGLViewerSettings::numFaceUIsosForDisplay() const
{
    return _nUIsosPerFace;
}

int TIGLViewerSettings::numFaceVIsosForDisplay() const
{
    return _nVIsosPerFace;
}

bool TIGLViewerSettings::drawFaceBoundaries() const
{
    return _drawFaceBoundaries;
}

void TIGLViewerSettings::loadSettings()
{
    QSettings settings("DLR SC-HPC", "TiGLViewer3");

    _tesselationAccuracy   = settings.value("tesselation_accuracy"  , tesselationAccuracy()).toDouble();
    _triangulationAccuracy = settings.value("triangulation_accuracy", triangulationAccuracy()).toDouble();
    _bgcolor = settings.value("background_color", BGColor()).value<QColor>();
    _shapecolor = settings.value("shape_color", shapeColor()).value<QColor>();
    _shapesymmetrycolor = settings.value("shape_symmetry_color", shapeSymmetryColor()).value<QColor>();
    _defaultMaterial = static_cast<Graphic3d_NameOfMaterial>(settings.value("shape_material", defaultMaterial()).toInt());

    _debugBOPs = settings.value("debug_bops", false).toBool();
    _enumFaces = settings.value("enumerate_faces", false).toBool();
    _nUIsosPerFace = settings.value("number_uisolines_per_face", 0).toInt();
    _nVIsosPerFace = settings.value("number_visolines_per_face", 0).toInt();
    _drawFaceBoundaries = settings.value("draw_face_boundaries", true).toBool();
}

void TIGLViewerSettings::storeSettings()
{
    QSettings settings("DLR SC-HPC", "TiGLViewer3");

    settings.setValue("tesselation_accuracy"  , tesselationAccuracy());
    settings.setValue("triangulation_accuracy", triangulationAccuracy());
    settings.setValue("background_color", BGColor());
    settings.setValue("shape_color", shapeColor());
    settings.setValue("shape_symmetry_color", shapeSymmetryColor());
    settings.setValue("shape_material", static_cast<int>(defaultMaterial()));

    settings.setValue("debug_bops", _debugBOPs);
    settings.setValue("enumerate_faces", _enumFaces);
    settings.setValue("number_uisolines_per_face", _nUIsosPerFace);
    settings.setValue("number_visolines_per_face", _nVIsosPerFace);
    settings.setValue("draw_face_boundaries", _drawFaceBoundaries);
}

void TIGLViewerSettings::restoreDefaults()
{
    _tesselationAccuracy = DEFAULT_TESSELATION_ACCURACY;
    _triangulationAccuracy = DEFAULT_TRIANGULATION_ACCURACY;
    _bgcolor = DEFAULT_BGCOLOR;
    _shapecolor = DEFAULT_SHAPE_COLOR;
    _shapesymmetrycolor = DEFAULT_SHAPE_SYMMETRY_COLOR;
    _debugBOPs = DEFAULT_DEBUG_BOPS;
    _enumFaces = DEFAULT_ENUM_FACES;
    _nUIsosPerFace = DEFAULT_NISO_FACES;
    _nVIsosPerFace = DEFAULT_NISO_FACES;
    _drawFaceBoundaries = DEFAULT_DRAW_FACE_BOUNDARIES;
    _defaultMaterial = Graphic3d_NOM_METALIZED;
}

void TIGLViewerSettings::addSettingsListener(ITIGLViewerSettingsChangedListener* listener)
{
    if (listener != nullptr) {
        _settingsListener.push_back(listener);
    }
}

void TIGLViewerSettings::removeSettingsListener(ITIGLViewerSettingsChangedListener* listener)
{
    if (listener != nullptr) {
        _settingsListener.erase(std::remove(_settingsListener.begin(), _settingsListener.end(), listener),_settingsListener.end());
    }
}
