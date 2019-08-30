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
#include <QCoreApplication>
#include "TIGLViewerSettings.h"

const double DEFAULT_TESSELATION_ACCURACY = 0.000316;
const double DEFAULT_TRIANGULATION_ACCURACY = 0.00070;
const QColor DEFAULT_BGCOLOR(169,237,255);
const bool DEFAULT_DEBUG_BOPS = false;
const bool DEFAULT_ENUM_FACES = false;
const int DEFAULT_NISO_FACES = 0;

static QString DEFAULT_TEMPLATE_DIR_PATH = "";
static QString DEFAULT_PROFILES_FILE_PATH = "./profilesDB.xml";

TIGLViewerSettings& TIGLViewerSettings::Instance()
{
    static TIGLViewerSettings settings;
    return settings;
}

TIGLViewerSettings::TIGLViewerSettings()
{
    DEFAULT_TEMPLATE_DIR_PATH = QCoreApplication::applicationDirPath();
#ifdef __APPLE__
    DEFAULT_TEMPLATE_DIR_PATH += "/../Resources/templates";
#else
    DEFAULT_TEMPLATE_DIR_PATH += "/../share/tigl3/templates";
#endif
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

void TIGLViewerSettings::loadSettings()
{
    QSettings settings("DLR SC-HPC", "TiGLViewer3");

    _tesselationAccuracy   = settings.value("tesselation_accuracy"  , tesselationAccuracy()).toDouble();
    _triangulationAccuracy = settings.value("triangulation_accuracy", triangulationAccuracy()).toDouble();
    _bgcolor = settings.value("background_color", BGColor()).value<QColor>();
    
    _debugBOPs = settings.value("debug_bops", false).toBool();
    _enumFaces = settings.value("enumerate_faces", false).toBool();
    _nUIsosPerFace = settings.value("number_uisolines_per_face", 0).toInt();
    _nVIsosPerFace = settings.value("number_visolines_per_face", 0).toInt();

    setTemplateDir(settings.value("template_dir_path", DEFAULT_TEMPLATE_DIR_PATH ).toString());
    _profilesDBPath = settings.value("profiles_file_path", DEFAULT_PROFILES_FILE_PATH).toString();
}

void TIGLViewerSettings::storeSettings()
{
    QSettings settings("DLR SC-HPC", "TiGLViewer3");

    settings.setValue("tesselation_accuracy"  , tesselationAccuracy());
    settings.setValue("triangulation_accuracy", triangulationAccuracy());
    settings.setValue("background_color", BGColor());

    settings.setValue("debug_bops", _debugBOPs);
    settings.setValue("enumerate_faces", _enumFaces);
    settings.setValue("number_uisolines_per_face", _nUIsosPerFace);
    settings.setValue("number_visolines_per_face", _nVIsosPerFace);

    settings.setValue("template_dir_path", _templateDir.absolutePath());
    settings.setValue("profiles_file_path", _profilesDBPath);
}

void TIGLViewerSettings::restoreDefaults()
{
    _tesselationAccuracy = DEFAULT_TESSELATION_ACCURACY;
    _triangulationAccuracy = DEFAULT_TRIANGULATION_ACCURACY;
    _bgcolor = DEFAULT_BGCOLOR;
    _debugBOPs = DEFAULT_DEBUG_BOPS;
    _enumFaces = DEFAULT_ENUM_FACES;
    _nUIsosPerFace = DEFAULT_NISO_FACES;
    _nVIsosPerFace = DEFAULT_NISO_FACES;
    // Possible issue:
    // restoreDefaults() is called in the constructor
    // -> the dir will be always create at start up of the application
    // even if the user has set another dir
    setTemplateDir(DEFAULT_TEMPLATE_DIR_PATH) ;
    _profilesDBPath = DEFAULT_PROFILES_FILE_PATH;
}

TIGLViewerSettings::~TIGLViewerSettings() {}

QDir TIGLViewerSettings::templateDir() const
{
    return _templateDir;
}

void TIGLViewerSettings::setTemplateDir(QString path)
{
    _templateDir = QDir(path);
    // create the directory if not exist
    if (!_templateDir.exists()) {
        _templateDir.mkpath(_templateDir.absolutePath());
    }
}

QString TIGLViewerSettings::profilesDBPath() const
{
    return _profilesDBPath;
}

void TIGLViewerSettings::setProfilesDBPath(QString path)
{
    _profilesDBPath = path;  // not check on the file is performed
}
