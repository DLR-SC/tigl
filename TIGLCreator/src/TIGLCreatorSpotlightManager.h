/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-09-02 Sven Goldberg <sven.goldberg@dlr.de>
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

#ifndef TIGLCREATORSPOTLIGHTMANAGER_H
#define TIGLCREATORSPOTLIGHTMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <V3d_Light.hxx>
#include <gp_Pnt.hxx>

class TIGLCreatorWidget;

// Struct to store the spotlight's name together with the OCCT object since there is no internal field 'name'.
// Additionally, the direction is stored externally. That is due to OCCT storing only the normalized direction vector.
// When a user enters a new entry (e.g. with the default vector (1,1,1)) and edits it, the read-out and shown values would change due to normalization.
struct SpotlightData
{
    QString name;
    Handle(V3d_Light) light;
    gp_Pnt direction;
    bool enabled = true;
};

class TIGLCreatorSpotlightManager : public QObject
{
    Q_OBJECT

public:
    explicit TIGLCreatorSpotlightManager(TIGLCreatorWidget* widget, QObject* parent = nullptr);

    void addSpotlight(double x, double y, double z, double dx, double dy, double dz, double concentration);
    void removeSpotlight(int index);
    bool updateSpotlight(int index, double x, double y, double z, double dx, double dy, double dz, double concentration);
    void copySpotlight(int index);
    bool setSpotlightEnabled(int index, bool enabled);
    bool isSpotlightEnabled(int index) const;

    QList<SpotlightData> getSpotlights() const;

signals:
    void spotlightsChanged();

private:
    // This overload is needed when a spotlight is copied to also copy the state (on/off) correctly
    void addSpotlight(double x, double y, double z, double dx, double dy, double dz, double concentration, bool enabled);

    TIGLCreatorWidget* myWidget;
    QList<SpotlightData> mySpotlights;
    int myNextId;
};

#endif // TIGLCREATORSPOTLIGHTMANAGER_H
