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

#ifndef TIGLVIEWERSETTINGS_H_
#define TIGLVIEWERSETTINGS_H_

#include <QColor>
#include <QMetaType>
#include <QtCore/QObject>
#include <Graphic3d_NameOfMaterial.hxx>
#include <QDir>

class TIGLViewerSettings : public QObject
{
    Q_OBJECT

public:
    ~TIGLViewerSettings() override = default;
    static TIGLViewerSettings& Instance();

    void loadSettings();
    void storeSettings();

    // Display settings Tab
    void setTesselationAccuracy(double);
    void setTriangulationAccuracy(double);

    double tesselationAccuracy() const;
    double triangulationAccuracy() const;

    void setBGColor(const QColor&);
    const QColor& BGColor() const;

    void setShapeColor(const QColor&);
    const QColor& shapeColor() const;

    void setShapeSymmetryColor(const QColor&);
    const QColor& shapeSymmetryColor() const;

    void setDefaultMaterial(Graphic3d_NameOfMaterial material);
    Graphic3d_NameOfMaterial defaultMaterial() const;

    QDir templateDir() const;
    void setTemplateDir(QString path);

    QString profilesDBPath() const;
    void setProfilesDBPath(QString path);


    // Debugging Tab
    void setDebugBooleanOperationsEnabled(bool);
    void setEnumerateFacesEnabled(bool);
    void setNumberOfUIsolinesPerFace(int);
    void setNumberOfVIsolinesPerFace(int);
    void setDrawFaceBoundariesEnabled(bool);

    bool debugBooleanOperations() const;
    bool enumerateFaces() const;
    int  numFaceUIsosForDisplay() const;
    int  numFaceVIsosForDisplay() const;
    bool drawFaceBoundaries() const;

    void restoreDefaults();


public slots:
    void setDefaultShapeColor(int r, int g, int b, int a = 0);
    void setDefaultShapeSymmetryColor(int r, int g, int b, int a = 0);
    void setDefaultMaterial(const QString& material);


private:
    TIGLViewerSettings();

    double _tesselationAccuracy;
    double _triangulationAccuracy;
    QColor _bgcolor;
    QColor _shapecolor;
    QColor _shapesymmetrycolor;
    Graphic3d_NameOfMaterial _defaultMaterial;

    bool _debugBOPs;
    bool _enumFaces;
    int  _nUIsosPerFace;
    int  _nVIsosPerFace;
    bool _drawFaceBoundaries;

    QDir _templateDir;

    QString _profilesDBPath;

};

Q_DECLARE_METATYPE(TIGLViewerSettings*)


#endif /* TIGLVIEWERSETTINGS_H_ */
