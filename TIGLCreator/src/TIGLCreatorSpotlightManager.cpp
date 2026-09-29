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
#include "TIGLCreatorContext.h"
#include "CTiglLogging.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_LightSource.hxx>
#include <TCollection_AsciiString.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <V3d_Light.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <algorithm>
#include <cmath>

TIGLCreatorSpotlightManager::TIGLCreatorSpotlightManager(TIGLCreatorWidget* widget, QObject* parent)
    : QObject(parent)
    , myWidget(widget)
    , myNextId(1)
    , myDefaultLightsEnabled(true)
{
    Q_ASSERT(myWidget != nullptr);
}

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

    const QString name = QString("Spotlight %1").arg(myNextId++);

    Handle(V3d_Light) light = new V3d_Light(Graphic3d_TypeOfLightSource::V3d_SPOT);
    light->SetName(TCollection_AsciiString(name.toUtf8().constData()));
    light->SetPosition(gp_Pnt(x, y, z));
    light->SetDirection(gp_Dir(dx, dy, dz));
    light->SetConcentration(concentration);
    light->SetAngle(static_cast<Standard_ShortReal>(coneAngleFromConcentration(concentration)));

    SpotlightData data;
    data.name = name;
    data.light = light;
    data.direction = gp_Pnt(dx, dy, dz);

    // A new light is not active in the view until it is explicitly turned on,
    // so no deactivation is needed here for the disabled case
    if (enabled) {
        myWidget->activateLight(light);
    }
    mySpotlights.append(data);
    mySpotlightSymbols.append(Handle(AIS_LightSource)());
    emit spotlightsChanged();
}

void TIGLCreatorSpotlightManager::removeSpotlight(int index)
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::removeSpotlight: Invalid spotlight index " << index << ".";
        return;
    }
    eraseSpotlightSymbol(index);
    myWidget->removeLight(mySpotlights[index].light);
    mySpotlights.removeAt(index);
    mySpotlightSymbols.removeAt(index);
    emit spotlightsChanged();
}

void TIGLCreatorSpotlightManager::copySpotlight(int index)
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::copySpotlight: Invalid spotlight index " << index << ".";
        return;
    }

    gp_Pnt position = mySpotlights[index].light->Position();
    gp_Pnt direction = mySpotlights[index].direction;
    double concentration = mySpotlights[index].light->Concentration();
    bool sourceEnabled = isSpotlightEnabled(index);

    addSpotlight(position.X(), position.Y(), position.Z(),
                 direction.X(), direction.Y(), direction.Z(),
                 concentration,
                 sourceEnabled);
}

void TIGLCreatorSpotlightManager::updateSpotlight(int index, double x, double y, double z,
                                                  double dx, double dy, double dz,
                                                  double concentration)
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::updateSpotlight: Invalid spotlight index " << index << ".";
        return;
    }
    if (concentration < 0.0 || concentration > 1.0) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::updateSpotlight: Invalid concentration " << concentration << ". Concentration must be inside [0.0,1.0].";
        return;
    }
    if (dx*dx + dy*dy + dz*dz < 1e-8) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::updateSpotlight: Direction must not be the zero vector or very close to it.";
        return;
    }

    SpotlightData& data = mySpotlights[index];
    data.light->SetPosition(gp_Pnt(x, y, z));
    data.light->SetDirection(gp_Dir(dx, dy, dz));
    data.light->SetConcentration(concentration);
    data.light->SetAngle(static_cast<Standard_ShortReal>(coneAngleFromConcentration(concentration)));
    data.direction = gp_Pnt(dx, dy, dz);

    if (myWidget->isLightEnabled(data.light)) {
        myWidget->activateLight(data.light);
    } else {
        myWidget->refreshLights();
    }

    // Refresh the symbol so that it follows the changed position, direction and cone angle.
    // SetToUpdate is required, otherwise AIS_InteractiveContext::Update leaves the
    // old presentation in place (same pattern as AIS_LightSource::SetLight)
    if (!mySpotlightSymbols[index].IsNull()) {
        Handle(AIS_InteractiveContext) context = getContext();
        if (context) {
            mySpotlightSymbols[index]->SetToUpdate();
            context->Update(mySpotlightSymbols[index], Standard_False);
        }
    }

    emit spotlightsChanged();
}

bool TIGLCreatorSpotlightManager::setSpotlightEnabled(int index, bool enabled)
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::setSpotlightEnabled: Invalid spotlight index " << index << ".";
        return false;
    }

    SpotlightData& data = mySpotlights[index];
    if (myWidget->isLightEnabled(data.light) == enabled) {
        return true;
    }

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

    return myWidget->isLightEnabled(mySpotlights[index].light);
}

bool TIGLCreatorSpotlightManager::setDefaultLightEnabled(bool enabled)
{
    if (!myWidget->viewerContext) {
        return false;
    }

    const QList<Handle(V3d_Light)>& lights = myWidget->viewerContext->defaultLights();
    if (lights.isEmpty()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::setDefaultLightEnabled: No default lights found in the viewer.";
        return false;
    }

    // Instead of switching the default lights off, dim their colors while the
    // "TiGL Default" option is unchecked (a fully unlit scene would render the
    // geometry and the trihedron pitch black) and restore them when re-enabled.
    // The colors are used because the shading shader reads the light colors live
    // from the viewer's light list, whereas SetIntensity() only affects the
    // ray-tracing path (which is not used here).
    const Standard_Real dimFactor = 0.01;
    if (!enabled && myDefaultLightsOriginalColor.isEmpty()) {
        for (const Handle(V3d_Light)& light : lights) {
            myDefaultLightsOriginalColor.append(light.IsNull() ? Quantity_Color(1.0, 1.0, 1.0, Quantity_TOC_RGB)
                                                               : light->Color());
        }
    }
    for (int i = 0; i < lights.size(); ++i) {
        if (lights[i].IsNull()) {
            continue;
        }
        if (!enabled) {
            const Quantity_Color original = myDefaultLightsOriginalColor[i];
            lights[i]->SetColor(Quantity_Color(original.Red() * dimFactor,
                                               original.Green() * dimFactor,
                                               original.Blue() * dimFactor,
                                               Quantity_TOC_RGB));
        }
        else if (i < myDefaultLightsOriginalColor.size()) {
            lights[i]->SetColor(myDefaultLightsOriginalColor[i]);
        }
    }
    myDefaultLightsEnabled = enabled;
    myWidget->setSceneDarkened(!enabled);
    myWidget->refreshLights();
    return true;
}

bool TIGLCreatorSpotlightManager::isDefaultLightEnabled() const
{
    if (!myWidget->viewerContext) {
        return false;
    }

    // The default lights are never switched off, only dimmed, so the state is tracked here
    return myDefaultLightsEnabled;
}

const QList<SpotlightData>& TIGLCreatorSpotlightManager::getSpotlights() const
{
    return mySpotlights;
}

double TIGLCreatorSpotlightManager::coneAngleFromConcentration(double concentration)
{
    // Maps the concentration to the cone angle of the spotlight:
    // a fully concentrated light gets a narrow cone (30 deg), a fully diffuse light a wide one (85 deg).
    // The angle must stay below 90 deg, because the spot shading evaluates pow(cos(angle), exponent)
    // and a negative cosine (possible for angles > 90 deg) is undefined in the shader and renders as black
    constexpr double minAngle = M_PI / 6.0;
    constexpr double maxAngle = 85.0 * M_PI / 180.0;
    return minAngle + (1.0 - concentration) * (maxAngle - minAngle);
}

Handle(AIS_InteractiveContext) TIGLCreatorSpotlightManager::getContext() const
{
    if (!myWidget->viewerContext) {
        return Handle(AIS_InteractiveContext)();
    }
    return myWidget->viewerContext->getContext();
}

double TIGLCreatorSpotlightManager::symbolLength() const
{
    Handle(V3d_View) view = myWidget->getView();
    if (!view) {
        return 0.0;
    }

    // World-unit length that corresponds to roughly 120 pixels in the current view
    Standard_Integer baseX = 0;
    Standard_Integer baseY = 0;
    view->Convert(0.0, 0.0, 0.0, baseX, baseY);

    double pixelsPerUnit = 0.0;
    for (int axis = 0; axis < 3; ++axis) {
        Standard_Integer x = 0;
        Standard_Integer y = 0;
        view->Convert(axis == 0 ? 1.0 : 0.0, axis == 1 ? 1.0 : 0.0, axis == 2 ? 1.0 : 0.0, x, y);
        pixelsPerUnit = std::max(pixelsPerUnit, std::hypot((double)(x - baseX), (double)(y - baseY)));
    }
    if (pixelsPerUnit < 1e-9) {
        return 0.1;
    }
    return 120.0 / pixelsPerUnit;
}

void TIGLCreatorSpotlightManager::displaySpotlightSymbol(int index)
{
    if (index < 0 || index >= mySpotlights.size() || !mySpotlights[index].light) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::displaySpotlightSymbol: Invalid spotlight index " << index << ".";
        return;
    }
    if (!mySpotlightSymbols[index].IsNull()) {
        return;
    }

    Handle(AIS_InteractiveContext) context = getContext();
    if (!context) {
        return;
    }

    Handle(AIS_LightSource) symbol = new AIS_LightSource(mySpotlights[index].light);
    const double length = symbolLength();
    if (length > 0.0) {
        symbol->SetSize(2.0 * length);
    }
    // The spotlight is turned on/off via the checkbox in the spotlight list,
    // so disable toggling the light by clicking on the symbol
    symbol->SetSwitchOnClick(false);
    context->Display(symbol, Standard_True);
    // The cone is purely a visual aid: it must not be selectable (and therefore
    // neither deletable nor highlightable) in the 3D viewer
    context->Deactivate(symbol);
    mySpotlightSymbols[index] = symbol;
}

void TIGLCreatorSpotlightManager::eraseSpotlightSymbol(int index)
{
    if (index < 0 || index >= mySpotlightSymbols.size()) {
        return;
    }

    if (!mySpotlightSymbols[index].IsNull()) {
        Handle(AIS_InteractiveContext) context = getContext();
        if (context) {
            context->Erase(mySpotlightSymbols[index], Standard_True);
        }
        mySpotlightSymbols[index].Nullify();
    }
}

bool TIGLCreatorSpotlightManager::setSpotlightSymbolVisible(int index, bool visible)
{
    if (index < 0 || index >= mySpotlights.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::setSpotlightSymbolVisible: Invalid spotlight index " << index << ".";
        return false;
    }

    if (visible) {
        if (mySpotlightSymbols[index].IsNull()) {
            displaySpotlightSymbol(index);
        }
    } else {
        eraseSpotlightSymbol(index);
    }
    return true;
}

bool TIGLCreatorSpotlightManager::isSpotlightSymbolVisible(int index) const
{
    if (index < 0 || index >= mySpotlightSymbols.size()) {
        LOG(ERROR) << "TIGLCreatorSpotlightManager::isSpotlightSymbolVisible: Invalid spotlight index " << index << ".";
        return false;
    }

    return !mySpotlightSymbols[index].IsNull();
}
