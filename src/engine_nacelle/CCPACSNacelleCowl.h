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
#include "CTiglAbstractGeometricComponent.h"
#include "tigl_internal.h"
#include "Geom_Curve.hxx"

namespace tigl
{

class CCPACSNacelleCowl : public generated::CPACSNacelleCowl
{
public:
    // Constructor
    TIGL_EXPORT CCPACSNacelleCowl(CCPACSEngineNacelle* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT PNamedShape BuildLoft() const;

private:
    struct WireCache {
        std::vector<TopoDS_Wire> profiles;
        std::vector<std::pair<double,TopoDS_Wire>> guideCurves;
    };
    Cache<WireCache, CCPACSNacelleCowl> wireCache;

    void BuildOuterShapeWires(WireCache& cache) const;
    Handle(Geom_Curve) GetGuideCurve(double zeta) const;
    TopoDS_Face GetStartZetaBlendingSurface(TopoDS_Face& innerShape, TopoDS_Shape& outerShape) const;
    TopoDS_Face GetEndZetaBlendingSurface(TopoDS_Face& innerShape, TopoDS_Shape& outerShape) const;
    TopoDS_Shape BuildOuterShape() const;
};

} //namespae tigl
