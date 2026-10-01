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

#ifndef TIGLCREATORLIGHTSOURCEMANAGER_H
#define TIGLCREATORLIGHTSOURCEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <V3d_Light.hxx>
#include <Quantity_Color.hxx>
#include <gp_Pnt.hxx>

class TIGLCreatorWidget;
class TIGLCreatorContext;
class AIS_InteractiveContext;
class AIS_LightSource;

// Struct to store the spotlight's name together with the OCCT object since there is no internal field 'name'.
// Additionally, the direction is stored externally. That is due to OCCT storing only the normalized direction vector.
// When a user enters a new entry (e.g. with the default vector (1,1,1)) and edits it, the read-out and shown values would change due to normalization.
struct SpotlightData
{
    QString name;
    Handle(V3d_Light) light;
    gp_Pnt direction;
};

class TIGLCreatorLightSourceManager : public QObject
{
    Q_OBJECT

public:
    explicit TIGLCreatorLightSourceManager(TIGLCreatorWidget* widget, QObject* parent = nullptr);

    void addSpotlight(double x, double y, double z, double dx, double dy, double dz, double concentration);
    void removeSpotlight(int index);
    void updateSpotlight(int index, double x, double y, double z, double dx, double dy, double dz, double concentration);
    void copySpotlight(int index);
    bool setSpotlightEnabled(int index, bool enabled);
    bool isSpotlightEnabled(int index) const;

    // The fixed viewer-level default lights can only be toggled on/off as a group
    bool setDefaultLightEnabled(bool enabled);
    bool isDefaultLightEnabled() const;

    const QList<SpotlightData>& getSpotlights() const;

    // Show or hide the 3D symbol (position marker + direction cone) of the given spotlight in the viewer
    bool setSpotlightSymbolVisible(int index, bool visible);
    bool isSpotlightSymbolVisible(int index) const;

signals:
    void spotlightsChanged();

private:
    // This overload is needed when a spotlight is copied to also copy the state (on/off) correctly
    void addSpotlight(double x, double y, double z, double dx, double dy, double dz, double concentration, bool enabled);

    // Returns the length of a spotlight's cone
    // For better visibility, the cone's real-world-size is not always the same. It is adjusted that way that the size on the 
    // screen appears independent of zooming when activated
    double symbolLength() const;
    static double coneAngleFromConcentration(double concentration);
    void displaySpotlightSymbol(int index);
    void eraseSpotlightSymbol(int index);
    Handle(AIS_InteractiveContext) getContext() const;

    TIGLCreatorWidget* myWidget;
    TIGLCreatorContext* myContext;
    QList<SpotlightData> mySpotlights;
    QList<Handle(AIS_LightSource)> mySpotlightSymbols;
    int myNextId;

    // The original colors of the viewer's default lights, saved once when they are first dimmed
    QList<Quantity_Color> myDefaultLightsOriginalColor;
    bool myDefaultLightsEnabled;
};

#endif // TIGLCREATORLIGHTSOURCEMANAGER_H
