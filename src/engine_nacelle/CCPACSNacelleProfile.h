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

#include "generated/CPACSProfileGeometry2D.h"
#include "tigl_internal.h"
#include "CTiglWingProfilePointList.h"
#include "Cache.h"

#include "TopoDS_Wire.hxx"

namespace tigl
{

class CCPACSNacelleProfile : public generated::CPACSProfileGeometry2D
{
public:
    CCPACSNacelleProfile(CTiglUIDManager* uidMgr);

    // get profile algorithm type
    TIGL_EXPORT ITiglWingProfileAlgo* GetProfileAlgo();
    TIGL_EXPORT const ITiglWingProfileAlgo* GetProfileAlgo() const;

    // Returns the nacelle profile wire
    TIGL_EXPORT TopoDS_Edge GetUpperWire   (TiglShapeModifier mod = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Edge GetLowerWire   (TiglShapeModifier mod = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Edge GetTrailingEdge(TiglShapeModifier mod = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Wire GetWire        (TiglShapeModifier mod = UNMODIFIED_SHAPE) const;

    // Checks, whether the trailing edge is blunt or not
    TIGL_EXPORT bool HasBluntTE() const;

protected:
    void buildPointListAlgo(unique_ptr<CTiglWingProfilePointList>& cache) const;

private:
    Cache<unique_ptr<CTiglWingProfilePointList>, CCPACSNacelleProfile> pointListAlgo;  // is created in case the wing profile alg is a point list, otherwise cst2d constructed in the base class is used
};

} //namespace tigl
