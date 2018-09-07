/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#pragma once

#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>

#include "generated/CPACSLongFloorBeamPosition.h"
#include "tigl.h"
#include "Cache.h"

namespace tigl
{

class CCPACSLongFloorBeamPosition : public generated::CPACSLongFloorBeamPosition
{
public:
    TIGL_EXPORT CCPACSLongFloorBeamPosition(CCPACSLongFloorBeam* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void SetPositionY(const double& value) OVERRIDE;

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT gp_Pnt GetCrossBeamIntersection(TiglCoordinateSystem cs = GLOBAL_COORDINATE_SYSTEM) const;
    TIGL_EXPORT TopoDS_Face GetCutGeometryForCrossBeam(TiglCoordinateSystem cs = GLOBAL_COORDINATE_SYSTEM) const;

private:
    void BuildCutGeometryForCrossBeam(TopoDS_Face& cache) const;

private:
    Cache<TopoDS_Face, CCPACSLongFloorBeamPosition> m_cutGeometryForCrossBeam;
};

} // namespace tigl
