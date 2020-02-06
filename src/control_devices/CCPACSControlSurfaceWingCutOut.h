/*
 * Copyright (C) 2007-2020 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#ifndef CCPACSCONTROLSURFACEWINGCUTOUT_H
#define CCPACSCONTROLSURFACEWINGCUTOUT_H

#include "tigl_internal.h"
#include "PNamedShape.h"
#include "generated/CPACSControlSurfaceWingCutOut.h"
#include "CTiglControlSurfaceBorderCoordinateSystem.h"

#include <TopoDS_Wire.hxx>

class gp_Vec;

namespace tigl
{

class CCPACSControlSurfaceOuterShapeTrailingEdge;
class CCPACSControlSurfaceBorderTrailingEdge;
class CCPACSWingComponentSegment;

class CCPACSControlSurfaceWingCutOut : public generated::CPACSControlSurfaceWingCutOut
{
public:
    TIGL_EXPORT CCPACSControlSurfaceWingCutOut(CCPACSTrailingEdgeDevice* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT PNamedShape GetLoft(PNamedShape wingCleanShape,
                                    const CCPACSControlSurfaceOuterShapeTrailingEdge&,
                                    const gp_Vec& upDir) const;

private:

    enum class CutoutPosition
    {
        InnerBorder,
        OuterBorder
    };

    TopoDS_Wire GetCutoutWire(CutoutPosition pos, PNamedShape wingCleanShape,
                              const CCPACSControlSurfaceBorderTrailingEdge *outerBorder, gp_Vec upDir) const;

    CTiglControlSurfaceBorderCoordinateSystem GetCutoutCS(bool isInnerBorder,
                                                          const CCPACSControlSurfaceBorderTrailingEdge*,
                                                          const gp_Vec& upDir) const;
};

const CCPACSWingComponentSegment& ComponentSegment(const CCPACSControlSurfaceWingCutOut&);

} // namespace tigl

#endif // CCPACSCONTROLSURFACEWINGCUTOUT_H
