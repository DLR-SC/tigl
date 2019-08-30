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
#include <QDir>

class TIGLViewerSettings
{
public:
    static TIGLViewerSettings& Instance();
    
    void loadSettings();
    void storeSettings();

    // Display settings Tab
    void setTesselationAccuracy(double);
    void setTriangulationAccuracy(double);
    void setBGColor(const QColor&);

    double tesselationAccuracy() const;
    double triangulationAccuracy() const;

    const QColor& BGColor() const;

    QDir templateDir() const;
    void setTemplateDir(QString path);

    QString profilesDBPath() const;
    void setProfilesDBPath(QString path);


    // Debugging Tab
    void setDebugBooleanOperationsEnabled(bool);
    void setEnumerateFacesEnabled(bool);
    void setNumberOfUIsolinesPerFace(int);
    void setNumberOfVIsolinesPerFace(int);
    
    bool debugBooleanOperations() const;
    bool enumerateFaces() const;
    int  numFaceUIsosForDisplay() const;
    int  numFaceVIsosForDisplay() const;

    void restoreDefaults();

    virtual ~TIGLViewerSettings();
private:
    TIGLViewerSettings();
    
    double _tesselationAccuracy;
    double _triangulationAccuracy;
    QColor _bgcolor;
    
    bool _debugBOPs;
    bool _enumFaces;
    int  _nUIsosPerFace;
    int  _nVIsosPerFace;

    QDir _templateDir;

    QString _profilesDBPath;

};

#endif /* TIGLVIEWERSETTINGS_H_ */
