#pragma once
/*
 * Copyright (C) 2018 German Aerospace Center (DLR/SC)
 *
 * Created: 2025 Ole Albers <ole.albers@dlr.de>
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

#include "CCPACSWingComponentSegment.h"
#include "CTiglControlSurfaceBorderCoordinateSystem.h"
#include "PNamedShape.h"
#include "generated/CPACSControlSurfaceBorderLeadingEdge.h"
#include <TopoDS_Wire.hxx>

namespace tigl
{

class CCPACSControlSurfaceBorderLeadingEdge : public generated::CPACSControlSurfaceBorderLeadingEdge
{
public:
    TIGL_EXPORT CCPACSControlSurfaceBorderLeadingEdge(CCPACSControlSurfaceOuterShapeLeadingEdge* parent,
                                                      CTiglUIDManager* uidMgr);

    TIGL_EXPORT TopoDS_Wire GetWire(PNamedShape wingShape, gp_Vec upDir) const;

    TIGL_EXPORT CTiglControlSurfaceBorderCoordinateSystem GetCoordinateSystem(gp_Vec upDir) const;

    enum class ShapeType
    {
        SIMPLE   = 0,
        LE_SHAPE = 1,
        AIRFOIL  = 2
    };

    TIGL_EXPORT ShapeType GetShapeType() const;

    // TODO: Rename
    TIGL_EXPORT double getEtaTE() const;
    TIGL_EXPORT double getEtaLE() const;
    TIGL_EXPORT double getXsiLE() const;
    TIGL_EXPORT double getXsiTE() const;

private:
    const CTiglUIDManager& uidMgr() const;
    TopoDS_Wire GetAirfoilWire(CTiglControlSurfaceBorderCoordinateSystem& coords) const;
};

TIGL_EXPORT const CCPACSWingComponentSegment& ComponentSegment(const CCPACSControlSurfaceBorderLeadingEdge&);

} // namespace tigl
