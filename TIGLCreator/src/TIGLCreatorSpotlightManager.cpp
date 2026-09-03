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

#include "TIGLCreatorSpotlightManager.h"
#include "TIGLCreatorWidget.h"
#include "CTiglLogging.h"

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <V3d_Light.hxx>

TIGLCreatorSpotlightManager::TIGLCreatorSpotlightManager(TIGLCreatorWidget* widget, QObject* parent)
    : QObject(parent)
    , myWidget(widget)
    , myNextId(1)
{}

void TIGLCreatorSpotlightManager::addSpotlight(double x, double y, double z,
                                               double dx, double dy, double dz,
                                               double concentration)
{
    addSpotlight(x, y, z, dx, dy, dz, concentration, true);
}

void TIGLCreatorSpotlightManager::addSpotlight(double x, double y, double z,
                                               double dx, double dy, double dz,
                                               double concentration,
                                               bool enabled)
{
    if (concentration < 0.0 || concentration > 1.0) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::addSpotlight: Invalid concentration " << concentration << ". Concentration must be inside [0.0,1.0].";
        return;
    }
    if (dx*dx + dy*dy + dz*dz < 1e-8) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::addSpotlight: Direction must not be the zero vector or very close to it.";
        return;
    }

    Handle(V3d_Light) light = new V3d_Light(Graphic3d_TypeOfLightSource::V3d_SPOT);
    light->SetPosition(gp_Pnt(x, y, z));
    light->SetDirection(gp_Dir(dx, dy, dz));
    light->SetConcentration(concentration);

    SpotlightData data;
    data.name = QString("Spotlight %1").arg(myNextId++);
    data.light = light;
    data.direction = gp_Pnt(dx, dy, dz);
    data.enabled = enabled;

    if (enabled) {
        myWidget->activateLight(light);
    }
    mySpotlights.append(data);
    emit spotlightsChanged();
}

void TIGLCreatorSpotlightManager::removeSpotlight(int index)
{
    if (index < 0 || index >= mySpotlights.size()) {
        return;
    }
    myWidget->removeLight(mySpotlights[index].light);
    mySpotlights.removeAt(index);
    emit spotlightsChanged();
}

void TIGLCreatorSpotlightManager::copySpotlight(int index)
{
    if (index < 0 || index >= mySpotlights.size()) {
        return;
    }

    gp_Pnt position = mySpotlights[index].light->Position();
    gp_Pnt direction = mySpotlights[index].direction;
    double concentration = mySpotlights[index].light->Concentration();
    bool sourceEnabled = mySpotlights[index].enabled;

    addSpotlight(position.X(), position.Y(), position.Z(),
                 direction.X(), direction.Y(), direction.Z(),
                 concentration,
                 sourceEnabled);
}

bool TIGLCreatorSpotlightManager::updateSpotlight(int index, double x, double y, double z,
                                                  double dx, double dy, double dz,
                                                  double concentration)
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::updateSpotlight: Invalid spotlight index " << index << ".";
        return false;
    }
    if (concentration < 0.0 || concentration > 1.0) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::updateSpotlight: Invalid concentration " << concentration << ". Concentration must be inside [0.0,1.0].";
        return false;
    }
    if (dx*dx + dy*dy + dz*dz < 1e-8) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::updateSpotlight: Direction must not be the zero vector or very close to it.";
        return false;
    }

    SpotlightData& data = mySpotlights[index];
    data.light->SetPosition(gp_Pnt(x, y, z));
    data.light->SetDirection(gp_Dir(dx, dy, dz));
    data.light->SetConcentration(concentration);
    data.direction = gp_Pnt(dx, dy, dz);

    if (data.enabled) {
        myWidget->activateLight(data.light);
    } else {
        myWidget->deactivateLight(data.light);
    }

    emit spotlightsChanged();
    return true;
}

bool TIGLCreatorSpotlightManager::setSpotlightEnabled(int index, bool enabled)
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::setSpotlightEnabled: Invalid spotlight index " << index << ".";
        return false;
    }

    SpotlightData& data = mySpotlights[index];
    if (data.enabled == enabled) {
        return true;
    }

    data.enabled = enabled;

    if (enabled) {
        myWidget->activateLight(data.light);
    } else {
        myWidget->deactivateLight(data.light);
    }

    emit spotlightsChanged();
    return true;
}

bool TIGLCreatorSpotlightManager::isSpotlightEnabled(int index) const
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::isSpotlightEnabled: Invalid spotlight index " << index << ".";
        return false;
    }

    return mySpotlights[index].enabled;
}

QList<SpotlightData> TIGLCreatorSpotlightManager::getSpotlights() const
{
    return mySpotlights;
}
