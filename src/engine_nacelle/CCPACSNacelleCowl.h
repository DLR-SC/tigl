/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#pragma once

#include "generated/CPACSNacelleCowl.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "tigl_internal.h"
#include "Geom_Curve.hxx"

namespace tigl
{

class CCPACSNacelleCowl : public generated::CPACSNacelleCowl, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT CCPACSNacelleCowl(CCPACSEngineNacelle* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE { return TIGL_COMPONENT_ENGINE_NACELLE; }
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE { return TIGL_INTENT_PHYSICAL; }

protected:
    virtual PNamedShape BuildLoft() const OVERRIDE;

private:
    struct WireCache {
        std::vector<TopoDS_Wire> profiles;
        std::vector<std::pair<double,TopoDS_Wire>> guideCurves;
    };
    Cache<WireCache, CCPACSNacelleCowl> wireCache;

    void BuildOuterShapeWires(WireCache& cache) const;
    Handle(Geom_Curve) GetGuideCurve(double zeta) const;
    TopoDS_Face GetStartZetaBlendingSurface(TopoDS_Face& innerShape) const;
    TopoDS_Face GetEndZetaBlendingSurface(TopoDS_Face& innerShape) const;
    TopoDS_Shape BuildOuterShape() const;
};

} //namespae tigl
